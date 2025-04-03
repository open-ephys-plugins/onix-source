/*
	------------------------------------------------------------------

	Copyright (C) Open Ephys

	------------------------------------------------------------------

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "MemoryMonitor.h"
#include "DigitalIO.h"

MemoryMonitorUsage::MemoryMonitorUsage(GenericProcessor* p)
	: LevelMonitor(p)
{
	device = nullptr;
}

void MemoryMonitorUsage::timerCallback()
{
	if (device != nullptr)
	{
		setFillPercentage(std::log(device->getLastPercentUsedValue() + 1) / maxLogarithmicValue);
		repaint();
	}
}

void MemoryMonitorUsage::setMemoryMonitor(std::shared_ptr<MemoryMonitor> memoryMonitor)
{
	device = memoryMonitor;
}

void MemoryMonitorUsage::startAcquisition()
{
	startTimerHz(TimerFrequencyHz);
}

void MemoryMonitorUsage::stopAcquisition()
{
	stopTimer();
	setFillPercentage(0.0f);
	repaint();
}

MemoryMonitor::MemoryMonitor(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, OnixDeviceType::MEMORYMONITOR, deviceIdx_, oni_ctx)
{
	StreamInfo percentUsedStream = StreamInfo(
		name + "-PercentUsed",
		"Percent of available memory that is currently used",
		"onix - memorymonitor.data.percentused",
		1,
		samplesPerSecond,
		"Percent",
		ContinuousChannel::Type::AUX,
		1.0f,
		"%",
		{""});
	streamInfos.add(percentUsedStream);

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;
}

int MemoryMonitor::configureDevice()
{
	setEnabled(true);

	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -1;

	deviceContext->writeRegister(deviceIdx, (uint32_t)MemoryMonitorRegisters::ENABLE, 1);
	if (deviceContext->getLastResult() != ONI_ESUCCESS) return deviceContext->getLastResult();

	totalMemory = deviceContext->readRegister(deviceIdx, (oni_reg_addr_t)MemoryMonitorRegisters::TOTAL_MEM);

	return deviceContext->getLastResult();
}

bool MemoryMonitor::updateSettings()
{
	oni_reg_val_t clkHz = deviceContext->readRegister(deviceIdx, (oni_reg_addr_t)MemoryMonitorRegisters::CLK_HZ);

	deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)MemoryMonitorRegisters::CLK_DIV, clkHz / samplesPerSecond);

	return deviceContext->getLastResult() == ONI_ESUCCESS;
}

void MemoryMonitor::startAcquisition()
{
	currentFrame = 0;
	sampleNumber = 0;

	lastPercentUsedValue = 0.0f;
}

void MemoryMonitor::stopAcquisition()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}
}

void MemoryMonitor::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void MemoryMonitor::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	for (StreamInfo streamInfo : streamInfos)
	{
		sourceBuffers.add(new DataBuffer(streamInfo.getNumChannels(), (int)streamInfo.getSampleRate() * bufferSizeInSeconds));

		if (streamInfo.getChannelPrefix().equalsIgnoreCase("Percent"))
			percentUsedBuffer = sourceBuffers.getLast();
	}
}

float MemoryMonitor::getLastPercentUsedValue()
{
	return lastPercentUsedValue;
}

void MemoryMonitor::processFrames()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		uint32_t* dataPtr = (uint32_t*)frame->data;
		uint64_t timestamp = frame->time;

		timestamps[currentFrame] = frame->time;

		percentUsedSamples[currentFrame] = 100.0f * float(*(dataPtr + 2)) / totalMemory;

		lastPercentUsedValue = percentUsedSamples[currentFrame];

		oni_destroy_frame(frame);

		sampleNumbers[currentFrame] = sampleNumber++;

		prevWord = m_digitalIO != nullptr && m_digitalIO->hasEventWord() ? m_digitalIO->getEventWord() : prevWord;

		eventCodes[currentFrame] = prevWord;

		currentFrame++;

		if (currentFrame >= numFrames)
		{
			shouldAddToBuffer = true;
			currentFrame = 0;
		}

		if (shouldAddToBuffer)
		{
			shouldAddToBuffer = false;
			percentUsedBuffer->addToBuffer(percentUsedSamples, sampleNumbers, timestamps, eventCodes, numFrames);
		}
	}
}

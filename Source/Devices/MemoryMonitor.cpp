/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2020 Allen Institute for Brain Science and Open Ephys

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

MemoryMonitorUsage::MemoryMonitorUsage(GenericProcessor* p)
	: LevelMonitor(p)
{
	device = nullptr;
}

MemoryMonitorUsage::~MemoryMonitorUsage()
{
}

void MemoryMonitorUsage::timerCallback()
{
	if (device != nullptr)
	{
		setFillPercentage(device->getLastPercentUsedValue() / 100.0f);
		repaint();
	}
}

void MemoryMonitorUsage::setMemoryMonitor(std::shared_ptr<MemoryMonitor> memoryMonitor)
{
	device = memoryMonitor;
}

void MemoryMonitorUsage::startAcquisition()
{
	startTimerHz(10);
}

void MemoryMonitorUsage::stopAcquisition()
{
	stopTimer();
	setFillPercentage(0.0f);
	repaint();
}

MemoryMonitor::MemoryMonitor(String name, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_)
	: OnixDevice(name, OnixDeviceType::MEMORYMONITOR, deviceIdx_, ctx_)
{
	StreamInfo percentUsedStream;
	percentUsedStream.name = name + "-PercentUsed";
	percentUsedStream.description = "Percent of available memory that is currently used";
	percentUsedStream.identifier = "onix-memorymonitor.data.percentused";
	percentUsedStream.numChannels = 1;
	percentUsedStream.sampleRate = samplesPerSecond;
	percentUsedStream.channelPrefix = "Percent";
	percentUsedStream.bitVolts = 1.0f;
	percentUsedStream.channelType = ContinuousChannel::Type::AUX;
	streams.add(percentUsedStream);

	StreamInfo bytesUsedStream;
	bytesUsedStream.name = name + "-BytesUsed";
	bytesUsedStream.description = "Number of bytes that are currently used";
	bytesUsedStream.identifier = "onix-memorymonitor.data.bytesused";
	bytesUsedStream.numChannels = 1;
	bytesUsedStream.sampleRate = samplesPerSecond;
	bytesUsedStream.channelPrefix = "Bytes";
	bytesUsedStream.bitVolts = 1.0f;
	bytesUsedStream.channelType = ContinuousChannel::Type::AUX;
	streams.add(bytesUsedStream);

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;
}

MemoryMonitor::~MemoryMonitor()
{
}

int MemoryMonitor::configureDevice()
{
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (uint32_t)MemoryMonitorRegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0)));

	return 0;
}

int MemoryMonitor::updateSettings()
{
	oni_reg_val_t clkHz;

	ONI_OK_RETURN_INT(oni_read_reg(ctx, deviceIdx, (oni_reg_addr_t)MemoryMonitorRegisters::CLK_HZ, &clkHz));

	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)MemoryMonitorRegisters::CLK_DIV, clkHz / samplesPerSecond));

	ONI_OK_RETURN_INT(oni_read_reg(ctx, deviceIdx, (oni_reg_addr_t)MemoryMonitorRegisters::TOTAL_MEM, &totalMemory));

	return 0;
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

	currentFrame = 0;
	sampleNumber = 0;
}

void MemoryMonitor::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void MemoryMonitor::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	for (StreamInfo streamInfo : streams)
	{
		sourceBuffers.add(new DataBuffer(streamInfo.numChannels, (int)streamInfo.sampleRate * bufferSizeInSeconds));

		if (streamInfo.channelPrefix.equalsIgnoreCase("Percent"))
			percentUsedBuffer = sourceBuffers.getLast();
		else if (streamInfo.channelPrefix.equalsIgnoreCase("Bytes"))
			bytesUsedBuffer = sourceBuffers.getLast();
	}
}

void MemoryMonitor::setSamplesPerSecond(uint32_t samplesPerSecond_) 
{
	samplesPerSecond = samplesPerSecond_;

	for (const auto& stream : streams)
	{
		streams.getReference(0).sampleRate = samplesPerSecond;
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

		timestamps[currentFrame] = *(uint64_t*)frame->data;

		percentUsedSamples[currentFrame] = 100.0f * float(*(dataPtr + 2)) / totalMemory;

		lastPercentUsedValue = percentUsedSamples[currentFrame];

		bytesUsedSamples[currentFrame] = float(*(dataPtr + 2)) * 4.0f;

		oni_destroy_frame(frame);

		sampleNumbers[currentFrame] = sampleNumber++;

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
			bytesUsedBuffer->addToBuffer(bytesUsedSamples, sampleNumbers, timestamps, eventCodes, numFrames);
		}
	}
}

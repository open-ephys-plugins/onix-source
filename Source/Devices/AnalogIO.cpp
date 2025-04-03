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

#include "AnalogIO.h"

AnalogIO::AnalogIO(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, OnixDeviceType::ANALOGIO, deviceIdx_, oni_ctx)
{
	StreamInfo analogInputStream = StreamInfo(
		name + "-AnalogInput",
		"Analog Input data",
		"onix-analogio.data.input",
		12,
		std::floor(AnalogIOFrequencyHz / framesToAverage),
		"AnalogInput",
		ContinuousChannel::Type::ADC,
		20.0f / numberOfDivisions, // NB: +/- 10 Volts
		"V",
		{});
	streamInfos.add(analogInputStream);

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;

	for (int i = 0; i < numChannels; i += 1)
	{
		channelDirection[i] = AnalogIODirection::Input;
		channelVoltageRange[i] = AnalogIOVoltageRange::TenVolts;
	}

	dataType = AnalogIODataType::Volts;
}

int AnalogIO::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -1;

	deviceContext->writeRegister(deviceIdx, (uint32_t)AnalogIORegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0));

	return deviceContext->getLastResult();
}

bool AnalogIO::updateSettings()
{
	for (int i = 0; i < numChannels; i += 1)
	{
		deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH00_IN_RANGE + i, (oni_reg_val_t)channelVoltageRange[i]); 
		if (deviceContext->getLastResult() != ONI_ESUCCESS) return false;
	}

	uint32_t ioReg = 0;

	for (int i = 0; i < numChannels; i += 1)
	{
		ioReg = (ioReg & ~((uint32_t)1 << i)) | ((uint32_t)(channelDirection[i]) << i);
	}

	deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CHDIR, ioReg);
	if (deviceContext->getLastResult() != ONI_ESUCCESS) return false;

	for (int i = 0; i < numChannels; i += 1)
	{
		voltsPerDivision[i] = getVoltsPerDivision(channelVoltageRange[i]);
	}

	return true;
}

float AnalogIO::getVoltsPerDivision(AnalogIOVoltageRange voltageRange)
{
	switch (voltageRange)
	{
	case AnalogIOVoltageRange::TenVolts:
		return 20.0f / numberOfDivisions;
	case AnalogIOVoltageRange::TwoPointFiveVolts:
		return 5.0f / numberOfDivisions;
	case AnalogIOVoltageRange::FiveVolts:
		return 10.0f / numberOfDivisions;
	default:
		return 0.0f;
	}
}

void AnalogIO::startAcquisition()
{
	currentFrame = 0;
	currentAverageFrame = 0;
	sampleNumber = 0;

	analogInputSamples.fill(0);
}

void AnalogIO::stopAcquisition()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}
}

void AnalogIO::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void AnalogIO::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	for (StreamInfo streamInfo : streamInfos)
	{
		sourceBuffers.add(new DataBuffer(streamInfo.getNumChannels(), (int)streamInfo.getSampleRate() * bufferSizeInSeconds));

		if (streamInfo.getChannelPrefix().equalsIgnoreCase("AnalogInput"))
			analogInputBuffer = sourceBuffers.getLast();
	}
}

void AnalogIO::processFrames()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		int16_t* dataPtr = (int16_t*)frame->data;

		int dataOffset = 4;

		for (int i = 0; i < numChannels; i += 1)
		{
			if (dataType == AnalogIODataType::S16)
				analogInputSamples[currentFrame + i * numFrames] += *(dataPtr + dataOffset + i);
			else
				analogInputSamples[currentFrame + i * numFrames] += *(dataPtr + dataOffset + i) * voltsPerDivision[i];
		}

		currentAverageFrame += 1;

		if (currentAverageFrame >= framesToAverage)
		{
			for (int i = 0; i < numChannels; i += 1)
			{
				analogInputSamples[currentFrame + i * numFrames] /= framesToAverage;
			}

			currentAverageFrame = 0;

			timestamps[currentFrame] = frame->time;
			sampleNumbers[currentFrame] = sampleNumber++;

			currentFrame++;
		}

		oni_destroy_frame(frame);

		if (currentFrame >= numFrames)
		{
			shouldAddToBuffer = true;
			currentFrame = 0;
		}

		if (shouldAddToBuffer)
		{
			shouldAddToBuffer = false;
			analogInputBuffer->addToBuffer(analogInputSamples.data(), sampleNumbers, timestamps, eventCodes, numFrames);

			analogInputSamples.fill(0);
		}
	}
}

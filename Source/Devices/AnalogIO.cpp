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

using namespace OnixSourcePlugin;

AnalogIO::AnalogIO(std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, hubName, AnalogIO::getDeviceType(), deviceIdx_, oni_ctx)
{
	StreamInfo analogInputStream = StreamInfo(
		OnixDevice::createStreamName({ getHubName(), name, "AnalogInput" }),
		"Analog Input data",
		getStreamIdentifier(),
		getNumChannels(),
		std::floor(AnalogIOFrequencyHz / framesToAverage),
		"AnalogInput",
		ContinuousChannel::Type::ADC,
		getVoltsPerDivision(AnalogIOVoltageRange::TenVolts), // NB: +/- 10 Volts
		"V",
		{},
		{ "input" });
	streamInfos.add(analogInputStream);

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;

	for (int i = 0; i < numChannels; i++)
	{
		channelDirection[i] = AnalogIODirection::Input;
		channelVoltageRange[i] = AnalogIOVoltageRange::TenVolts;
	}

	dataType = AnalogIODataType::Volts;
}

OnixDeviceType AnalogIO::getDeviceType()
{
	return OnixDeviceType::ANALOGIO;
}

int AnalogIO::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) 
		throw error_str("Device context is not initialized properly for	" + getName());
	
	return deviceContext->writeRegister(deviceIdx, (uint32_t)AnalogIORegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0));
}

bool AnalogIO::updateSettings()
{
	int rc = 0;

	for (int i = 0; i < numChannels; i++)
	{
		rc = deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH00_IN_RANGE + i, (oni_reg_val_t)channelVoltageRange[i]);
		if (rc != ONI_ESUCCESS) return false;
	}

	uint32_t ioReg = 0;

	for (int i = 0; i < numChannels; i++)
	{
		ioReg = (ioReg & ~((uint32_t)1 << i)) | ((uint32_t)(channelDirection[i]) << i);
	}

	rc = deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CHDIR, ioReg);
	if (rc != ONI_ESUCCESS) return false;

	for (int i = 0; i < numChannels; i++)
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

		for (int i = 0; i < numChannels; i++)
		{
			if (dataType == AnalogIODataType::S16)
				analogInputSamples[currentFrame + i * numFrames] += *(dataPtr + dataOffset + i);
			else
				analogInputSamples[currentFrame + i * numFrames] += *(dataPtr + dataOffset + i) * voltsPerDivision[i];
		}

		currentAverageFrame++;

		if (currentAverageFrame >= framesToAverage)
		{
			for (int i = 0; i < numChannels; i++)
			{
				analogInputSamples[currentFrame + i * numFrames] /= framesToAverage;
			}

			currentAverageFrame = 0;

			timestamps[currentFrame] = deviceContext->convertTimestampToSeconds(frame->time);
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

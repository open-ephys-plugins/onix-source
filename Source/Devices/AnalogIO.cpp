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
		numChannels,
		getSampleRate(),
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

int AnalogIO::getSampleRate()
{
	return std::floor(AnalogIOFrequencyHz / framesToAverage);
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

AnalogIODirection AnalogIO::getChannelDirection(int channelNumber)
{
	if (channelNumber > numChannels || channelNumber < 0)
	{
		LOGE("Channel number must be between 0 and " + std::to_string(channelNumber));
		return AnalogIODirection::Input;
	}

	return channelDirection[channelNumber];
}

std::string AnalogIO::getChannelDirection(AnalogIODirection direction)
{
	switch (direction)
	{
	case AnalogIODirection::Input:
		return "Input";
	case AnalogIODirection::Output:
		return "Output";
	default:
		return "";
	}
}

void AnalogIO::setChannelDirection(int channelNumber, AnalogIODirection direction)
{
	if (channelNumber > numChannels || channelNumber < 0)
	{
		LOGE("Channel number must be between 0 and " + std::to_string(channelNumber));
		return;
	}

	channelDirection[channelNumber] = direction;
}

AnalogIOVoltageRange AnalogIO::getChannelVoltageRange(int channelNumber)
{
	if (channelNumber > numChannels || channelNumber < 0)
	{
		LOGE("Channel number must be between 0 and " + std::to_string(channelNumber));
		return AnalogIOVoltageRange::FiveVolts;
	}

	return channelVoltageRange[channelNumber];
}

void AnalogIO::setChannelVoltageRange(int channelNumber, AnalogIOVoltageRange direction)
{
	if (channelNumber > numChannels || channelNumber < 0)
	{
		LOGE("Channel number must be between 0 and " + std::to_string(channelNumber));
		return;
	}

	channelVoltageRange[channelNumber] = direction;
}

AnalogIODataType AnalogIO::getDataType() const
{
	return dataType;
}

void AnalogIO::setDataType(AnalogIODataType type)
{
	dataType = type;
}

void AnalogIO::startAcquisition()
{
	currentFrame = 0;
	currentAverageFrame = 0;
	sampleNumber = 0;

	analogInputSamples.fill(0);
}

int AnalogIO::getNumberOfFrames()
{
	return frameQueue.size_approx();
}

void AnalogIO::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	sourceBuffers.add(new DataBuffer(streamInfos.getFirst().getNumChannels(), (int)streamInfos.getFirst().getSampleRate() * bufferSizeInSeconds));
	analogInputBuffer = sourceBuffers.getLast();
}

void AnalogIO::processFrame(uint64_t eventWord)
{
	oni_frame_t* frame;
	if (!frameQueue.try_dequeue(frame)) { //NB: This method should never be called unless a frame is sure to be there
		jassertfalse;
	}

	int16_t* dataPtr = (int16_t*)frame->data;

	int dataOffset = 4;

	for (size_t i = 0; i < numChannels; i++)
	{
		if (dataType == AnalogIODataType::S16)
			analogInputSamples[currentFrame + i * numFrames] += *(dataPtr + dataOffset + i);
		else
			analogInputSamples[currentFrame + i * numFrames] += *(dataPtr + dataOffset + i) * voltsPerDivision[i];
	}

	currentAverageFrame++;

	if (currentAverageFrame >= framesToAverage)
	{
		for (size_t i = 0; i < numChannels; i++)
		{
			analogInputSamples[currentFrame + i * numFrames] /= framesToAverage;
		}

		currentAverageFrame = 0;

		timestamps[currentFrame] = deviceContext->convertTimestampToSeconds(frame->time);
		sampleNumbers[currentFrame] = sampleNumber++;
		eventCodes[currentFrame] = eventWord;

		currentFrame++;
	}

	oni_destroy_frame(frame);

	if (currentFrame >= numFrames)
	{
		analogInputBuffer->addToBuffer(analogInputSamples.data(), sampleNumbers, timestamps, eventCodes, numFrames);

		analogInputSamples.fill(0);
		currentFrame = 0;
	}
}

void AnalogIO::processFrames()
{
	while (frameQueue.peek() != nullptr)
	{
		processFrame();
	}
}

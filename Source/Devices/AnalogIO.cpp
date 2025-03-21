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

#include "AnalogIO.h"

AnalogIO::AnalogIO(String name, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_)
	: OnixDevice(name, OnixDeviceType::ANALOGIO, deviceIdx_, ctx_)
{
	StreamInfo analogInputStream;
	analogInputStream.name = name + "-AnalogInput";
	analogInputStream.description = "Analog Input data";
	analogInputStream.identifier = "onix-analogio.data.input";
	analogInputStream.numChannels = 12;
	analogInputStream.sampleRate = std::floor(100000 / framesToAverage);
	analogInputStream.channelPrefix = "AnalogInput";
	analogInputStream.bitVolts = 1.0f;
	analogInputStream.channelType = ContinuousChannel::Type::ADC;
	streams.add(analogInputStream);

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;

	for (int i = 0; i < numChannels; i += 1)
	{
		channelDirection[i] = AnalogIODirection::Input;
		channelVoltageRange[i] = AnalogIOVoltageRange::FiveVolts;
	}

	dataType = AnalogIODataType::S16;
}

int AnalogIO::configureDevice()
{
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (uint32_t)AnalogIORegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0)));

	return 0;
}

int AnalogIO::updateSettings()
{
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH00_IN_RANGE, (oni_reg_val_t)channelVoltageRange[0]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH01_IN_RANGE, (oni_reg_val_t)channelVoltageRange[1]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH02_IN_RANGE, (oni_reg_val_t)channelVoltageRange[2]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH03_IN_RANGE, (oni_reg_val_t)channelVoltageRange[3]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH04_IN_RANGE, (oni_reg_val_t)channelVoltageRange[4]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH05_IN_RANGE, (oni_reg_val_t)channelVoltageRange[5]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH06_IN_RANGE, (oni_reg_val_t)channelVoltageRange[6]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH07_IN_RANGE, (oni_reg_val_t)channelVoltageRange[7]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH08_IN_RANGE, (oni_reg_val_t)channelVoltageRange[8]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH09_IN_RANGE, (oni_reg_val_t)channelVoltageRange[9]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH10_IN_RANGE, (oni_reg_val_t)channelVoltageRange[10]));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CH11_IN_RANGE, (oni_reg_val_t)channelVoltageRange[11]));

	uint32_t ioReg = 0;

	for (int i = 0; i < numChannels; i += 1)
	{
		ioReg = (ioReg & ~((uint32_t)1 << i)) | ((uint32_t)(channelDirection[i]) << i);
	}

	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)AnalogIORegisters::CHDIR, ioReg));

	for (int i = 0; i < numChannels; i += 1)
	{
		voltsPerDivision[i] = getVoltsPerDivision(channelVoltageRange[i]);
	}

	return 0;
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
	for (StreamInfo streamInfo : streams)
	{
		sourceBuffers.add(new DataBuffer(streamInfo.numChannels, (int)streamInfo.sampleRate * bufferSizeInSeconds));

		if (streamInfo.channelPrefix.equalsIgnoreCase("AnalogInput"))
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

			currentFrame++;

			timestamps[currentFrame] = *(uint64_t*)frame->data;
			sampleNumbers[currentFrame] = sampleNumber++;
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

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

#include "Bno055.h"

#include <oni.h>
#include <onix.h>

using namespace Onix;

Bno055::Bno055(String name, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_)
	: OnixDevice(name, BNO, deviceIdx_, ctx_)
{
	const float bitVolts = 1.0;

	StreamInfo eulerAngleStream;
	eulerAngleStream.name = name + "-Euler";
	eulerAngleStream.description = "Bosch Bno055 9-axis inertial measurement unit (IMU) Euler angle";
	eulerAngleStream.identifier = "onix-bno055.data.euler";
	eulerAngleStream.numChannels = 3;
	eulerAngleStream.sampleRate = 100.0f;
	eulerAngleStream.channelPrefix = "Euler";
	eulerAngleStream.bitVolts = bitVolts;
	eulerAngleStream.channelType = ContinuousChannel::Type::AUX;
	streams.add(eulerAngleStream);

	StreamInfo quaternionStream;
	quaternionStream.name = name + "-Quaternion";
	quaternionStream.description = "Bosch Bno055 9-axis inertial measurement unit (IMU) Quaternion";
	quaternionStream.identifier = "onix-bno055.data.quat";
	quaternionStream.numChannels = 4;
	quaternionStream.sampleRate = 100.0f;
	quaternionStream.channelPrefix = "Quaternion";
	quaternionStream.bitVolts = bitVolts;
	quaternionStream.channelType = ContinuousChannel::Type::AUX;
	streams.add(quaternionStream);

	StreamInfo accelerationStream;
	accelerationStream.name = name + "-Acceleration";
	accelerationStream.description = "Bosch Bno055 9-axis inertial measurement unit (IMU) Acceleration";
	accelerationStream.identifier = "onix-bno055.data.acc";
	accelerationStream.numChannels = 3;
	accelerationStream.sampleRate = 100.0f;
	accelerationStream.channelPrefix = "Acceleration";
	accelerationStream.bitVolts = bitVolts;
	accelerationStream.channelType = ContinuousChannel::Type::AUX;
	streams.add(accelerationStream);

	StreamInfo gravityStream;
	gravityStream.name = name + "-Gravity";
	gravityStream.description = "Bosch Bno055 9-axis inertial measurement unit (IMU) Gravity";
	gravityStream.identifier = "onix-bno055.data.grav";
	gravityStream.numChannels = 3;
	gravityStream.sampleRate = 100.0f;
	gravityStream.channelPrefix = "Gravity";
	gravityStream.bitVolts = bitVolts;
	gravityStream.channelType = ContinuousChannel::Type::AUX;
	streams.add(gravityStream);

	StreamInfo temperatureStream;
	temperatureStream.name = name + "-Temperature";
	temperatureStream.description = "Bosch Bno055 9-axis inertial measurement unit (IMU) Temperature";
	temperatureStream.identifier = "onix-bno055.data.temp";
	temperatureStream.numChannels = 1;
	temperatureStream.sampleRate = 100.0f;
	temperatureStream.channelPrefix = "Temperature";
	temperatureStream.bitVolts = bitVolts;
	temperatureStream.channelType = ContinuousChannel::Type::AUX;
	streams.add(temperatureStream);

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;
}

Bno055::~Bno055() 
{
}

int Bno055::enableDevice()
{
	oni_write_reg(ctx, deviceIdx, (uint32_t)Bno055Registers::ENABLE, (uint32_t)1);

	return 0;
}

void Bno055::startAcquisition()
{
	startThread();
}

void Bno055::stopAcquisition()
{
	if (isThreadRunning())
		signalThreadShouldExit();

	waitForThreadToExit(2000);


	while (!frameArray.isEmpty())
	{
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}

	currentFrame = 0;
	sampleNumber = 0;
}

void Bno055::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void Bno055::run()
{
	while (!threadShouldExit())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());

		if (!frameArray.isEmpty())
		{
			oni_frame_t* frame = frameArray.removeAndReturn(0);

			int16_t* dataPtr = (int16_t*)frame->data;

			bnoTimestamps[currentFrame] = *(uint64_t*)frame->data;

			int dataOffset = 4;

			const float eulerAngleScale = 1.0f / 16; // 1 degree = 16 LSB
			const float quaternionScale = 1.0f / (1 << 14); // 1 = 2^14 LSB
			const float accelerationScale = 1.0f / 100; // 1m / s^2 = 100 LSB

			for (int i = 0; i < 3; i++)
			{
				eulerSamples[currentFrame + i * numFrames] = float(*(dataPtr + dataOffset)) * eulerAngleScale;
				dataOffset++;
			}

			for (int i = 0; i < 4; i++)
			{
				quaternionSamples[currentFrame + i * numFrames] = float(*(dataPtr + dataOffset)) * quaternionScale;
				dataOffset++;
			}

			for (int i = 0; i < 3; i++)
			{
				accelerationSamples[currentFrame + i * numFrames] = float(*(dataPtr + dataOffset)) * accelerationScale;
				dataOffset++;
			}

			for (int i = 0; i < 3; i++)
			{
				gravitySamples[currentFrame + i * numFrames] = float(*(dataPtr + dataOffset)) * accelerationScale;
				dataOffset++;
			}

			temperatureSamples[currentFrame] = float(*((uint8_t*)(dataPtr + dataOffset)));

			oni_destroy_frame(frame);

			sampleNumbers[currentFrame] = sampleNumber;

			currentFrame++;

			if (currentFrame >= numFrames)
			{
				shouldAddToBuffer = true;
				currentFrame = 0;
			}
		}

		if (shouldAddToBuffer)
		{
			shouldAddToBuffer = false;
			eulerBuffer->addToBuffer(eulerSamples, sampleNumbers, bnoTimestamps, eventCodes, numFrames);
			quaternionBuffer->addToBuffer(quaternionSamples, sampleNumbers, bnoTimestamps, eventCodes, numFrames);
			accelerationBuffer->addToBuffer(accelerationSamples, sampleNumbers, bnoTimestamps, eventCodes, numFrames);
			gravityBuffer->addToBuffer(gravitySamples, sampleNumbers, bnoTimestamps, eventCodes, numFrames);
			temperatureBuffer->addToBuffer(temperatureSamples, sampleNumbers, bnoTimestamps, eventCodes, numFrames);
		}
	}
}
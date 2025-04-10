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

#include "Bno055.h"

Bno055::Bno055(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx)
	: OnixDevice(name, OnixDeviceType::BNO, deviceIdx_, ctx)
{
	const float bitVolts = 1.0;

	StreamInfo eulerAngleStream = StreamInfo(
		OnixDevice::createStreamName(PortController::getPortName(PortController::getPortFromIndex(deviceIdx)), getName(), "Euler"),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Euler angle",
		"onix-bno055.data.euler",
		3,
		sampleRate,
		"Euler",
		ContinuousChannel::Type::AUX,
		bitVolts,
		"Degrees",
		{ "Yaw", "Roll", "Pitch" });
	streamInfos.add(eulerAngleStream);

	StreamInfo quaternionStream = StreamInfo(
		OnixDevice::createStreamName(PortController::getPortName(PortController::getPortFromIndex(deviceIdx)), getName(), "Quaternion"),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Quaternion",
		"onix-bno055.data.quat",
		4,
		sampleRate,
		"Quaternion",
		ContinuousChannel::Type::AUX,
		bitVolts,
		"u",
		{ "W", "X", "Y", "Z" });
	streamInfos.add(quaternionStream);

	StreamInfo accelerationStream = StreamInfo(
		OnixDevice::createStreamName(PortController::getPortName(PortController::getPortFromIndex(deviceIdx)), getName(), "Acceleration"),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Acceleration",
		"onix-bno055.data.acc",
		3,
		sampleRate,
		"Acceleration",
		ContinuousChannel::Type::AUX,
		bitVolts,
		"m / s ^ 2",
		{ "X", "Y", "Z" });
	streamInfos.add(accelerationStream);

	StreamInfo gravityStream = StreamInfo(
		OnixDevice::createStreamName(PortController::getPortName(PortController::getPortFromIndex(deviceIdx)), getName(), "Gravity"),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Gravity",
		"onix-bno055.data.grav",
		3,
		sampleRate,
		"Gravity",
		ContinuousChannel::Type::AUX,
		bitVolts,
		"m/s^2",
		{ "X", "Y", "Z" });
	streamInfos.add(gravityStream);

	StreamInfo temperatureStream = StreamInfo(
		OnixDevice::createStreamName(PortController::getPortName(PortController::getPortFromIndex(deviceIdx)), getName(), "Temperature"),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Temperature",
		"onix-bno055.data.temp",
		1,
		sampleRate,
		"Temperature",
		ContinuousChannel::Type::AUX,
		bitVolts,
		"Celsius",
		{ "" });
	streamInfos.add(temperatureStream);

	// TODO: Add calibration stream here?

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;
}

int Bno055::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -1;

	return deviceContext->writeRegister(deviceIdx, (uint32_t)Bno055Registers::ENABLE, isEnabled() ? 1 : 0);
}

bool Bno055::updateSettings()
{
	return true;
}

void Bno055::startAcquisition()
{
	currentFrame = 0;
	sampleNumber = 0;
}

void Bno055::stopAcquisition()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
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

void Bno055::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	sourceBuffers.add(new DataBuffer(numberOfChannels, (int)sampleRate * bufferSizeInSeconds));

	bnoBuffer = sourceBuffers.getLast();
}

void Bno055::processFrames()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		int16_t* dataPtr = (int16_t*)frame->data;

		bnoTimestamps[currentFrame] = frame->time;

		int dataOffset = 4;

		int channelOffset = 0;

		// Euler
		for (int i = 0; i < 3; i++)
		{
			bnoSamples[currentFrame + channelOffset * numFrames] = float(*(dataPtr + dataOffset)) * eulerAngleScale;
			dataOffset++;
			channelOffset += 1;
		}

		// Quaternion
		for (int i = 0; i < 4; i++)
		{
			bnoSamples[currentFrame + channelOffset * numFrames] = float(*(dataPtr + dataOffset)) * quaternionScale;
			dataOffset++;
			channelOffset += 1;
		}

		// Acceleration
		for (int i = 0; i < 3; i++)
		{
			bnoSamples[currentFrame + channelOffset * numFrames] = float(*(dataPtr + dataOffset)) * accelerationScale;
			dataOffset++;
			channelOffset += 1;
		}

		// Gravity
		for (int i = 0; i < 3; i++)
		{
			bnoSamples[currentFrame + channelOffset * numFrames] = float(*(dataPtr + dataOffset)) * accelerationScale;
			dataOffset++;
			channelOffset += 1;
		}

		// Temperature
		bnoSamples[currentFrame + channelOffset * numFrames] = float(*((uint8_t*)(dataPtr + dataOffset)));

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
			bnoBuffer->addToBuffer(bnoSamples, sampleNumbers, bnoTimestamps, eventCodes, numFrames);
		}
	}
}

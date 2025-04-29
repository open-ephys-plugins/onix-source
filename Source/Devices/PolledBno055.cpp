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

#include "PolledBno055.h"

PolledBno055::PolledBno055(String name, String headstageName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx)
	: OnixDevice(name, headstageName, OnixDeviceType::POLLEDBNO, deviceIdx_, ctx),
	I2CRegisterContext(Bno055Address, deviceIdx_, ctx)
{
	auto streamIdentifier = getStreamIdentifier();

	String port = PortController::getPortName(PortController::getPortFromIndex(deviceIdx));
	StreamInfo eulerAngleStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHeadstageName(), getName(), "Euler" }),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Euler angle",
		streamIdentifier,
		3,
		sampleRate,
		"Eul",
		ContinuousChannel::Type::AUX,
		eulerAngleScale,
		"Degrees",
		{ "Y", "R", "P" },
		"euler",
		{ "y", "r", "p" }
	);
	streamInfos.add(eulerAngleStream);
	
	StreamInfo quaternionStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHeadstageName(), getName(), "Quaternion" }),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Quaternion",
		streamIdentifier,
		4,
		sampleRate,
		"Quat",
		ContinuousChannel::Type::AUX,
		quaternionScale,
		"",
		{ "W", "X", "Y", "Z" },
		"quaternion",
		{ "w", "x", "y", "z" }
	);
	streamInfos.add(quaternionStream);

	StreamInfo accelerationStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHeadstageName(), getName(), "Acceleration" }),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Acceleration",
		streamIdentifier,
		3,
		sampleRate,
		"Acc",
		ContinuousChannel::Type::AUX,
		accelerationScale,
		"m / s ^ 2",
		{ "X", "Y", "Z" },
		"acceleration",
		{ "x","y","z" }
	);
	streamInfos.add(accelerationStream);

	StreamInfo gravityStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHeadstageName(), getName(), "Gravity" }),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Gravity",
		streamIdentifier,
		3,
		sampleRate,
		"Grav",
		ContinuousChannel::Type::AUX,
		accelerationScale,
		"m/s^2",
		{ "X", "Y", "Z" },
		"gravity",
		{ "x", "y", "z" }
	);
	streamInfos.add(gravityStream);

	StreamInfo temperatureStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHeadstageName(), getName(), "Temperature" }),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Temperature",
		streamIdentifier,
		1,
		sampleRate,
		"Temp",
		ContinuousChannel::Type::AUX,
		1.0f,
		"Celsius",
		{ "" },
		"temperature"
	);
	streamInfos.add(temperatureStream);

	StreamInfo calibrationStatusStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHeadstageName(), getName(), "Calibration" }),
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Calibration",
		streamIdentifier,
		1,
		sampleRate,
		"Cal",
		ContinuousChannel::Type::AUX,
		1.0f,
		"",
		{ "" },
		"calibration"
	);
	streamInfos.add(calibrationStatusStream);

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;
}

int PolledBno055::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -1;

	deserializer = std::make_unique<I2CRegisterContext>(DS90UB9x::DES_ADDR, deviceIdx, deviceContext);
	uint32_t alias = Bno055Address << 1;
	int rc = deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID4, alias);
	if (rc != ONI_ESUCCESS) return -2;
	rc = deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias4, alias);
	if (rc != ONI_ESUCCESS) return -2;

	return ONI_ESUCCESS;
}

bool PolledBno055::updateSettings()
{
	if (!isEnabled()) return true;

	int rc = WriteByte(0x3E, 0x00); // Power mode normal
	if (rc != ONI_ESUCCESS) return false;

	rc = WriteByte(0x07, 0x00); // Page ID address 0
	if (rc != ONI_ESUCCESS) return false;

	rc = WriteByte(0x3F, 0x00); // Internal oscillator
	if (rc != ONI_ESUCCESS) return false;

	rc = WriteByte(0x41, (uint32_t)axisMap);  // Axis map config
	if (rc != ONI_ESUCCESS) return false;

	rc = WriteByte(0x42, (uint32_t)axisSign); // Axis sign
	if (rc != ONI_ESUCCESS) return false;

	rc = WriteByte(0x3D, 0x0C); // Operation mode is NDOF
	if (rc != ONI_ESUCCESS) return false;

	rc = set933I2cRate(i2cRate);

	return rc == ONI_ESUCCESS;
}

void PolledBno055::startAcquisition()
{
	sampleNumber = 0;
	currentFrame = 0;

	if (isEnabled())
		startTimer(timerIntervalInMilliseconds);
}

void PolledBno055::stopAcquisition()
{
	stopTimer();

	while (isTimerRunning())
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void PolledBno055::addFrame(oni_frame_t* frame)
{
	oni_destroy_frame(frame);
}

void PolledBno055::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	sourceBuffers.add(new DataBuffer(numberOfChannels, (int)sampleRate * bufferSizeInSeconds));

	bnoBuffer = sourceBuffers.getLast();
}

int16_t PolledBno055::readInt16(uint32_t startAddress)
{
	oni_reg_val_t byte1 = 0, byte2 = 0;

	int rc = ReadByte(startAddress, &byte1);
	if (rc != ONI_ESUCCESS) return 0;
	rc = ReadByte(startAddress + 1, &byte2);
	if (rc != ONI_ESUCCESS) return 0;

	return (static_cast<int16_t>(byte2) << 8) | byte1;
}

void PolledBno055::hiResTimerCallback()
{
	int offset = 0;

	// Euler
	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress) * eulerAngleScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 2) * eulerAngleScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 4) * eulerAngleScale;
	offset++;

	// Quaternion
	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 6) * quaternionScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 8) * quaternionScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 10) * quaternionScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 12) * quaternionScale;
	offset++;

	// Acceleration

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 14) * accelerationScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 16) * accelerationScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 18) * accelerationScale;
	offset++;

	// Gravity

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 20) * accelerationScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 22) * accelerationScale;
	offset++;

	bnoSamples[offset * numFrames + currentFrame] = readInt16(EulerHeadingLsbAddress + 24) * accelerationScale;
	offset++;

	// Temperature

	oni_reg_val_t byte;
	ReadByte(EulerHeadingLsbAddress + 26, &byte);
	bnoSamples[offset * numFrames + currentFrame] = static_cast<uint8_t>(byte);
	offset++;

	// Calibration Status

	ReadByte(EulerHeadingLsbAddress + 27, &byte);
	bnoSamples[offset * numFrames + currentFrame] = byte;
	offset++;

	oni_reg_val_t timestampL = 0, timestampH = 0;
	int rc = deviceContext->readRegister(deviceIdx, DS90UB9x::LASTI2CL, &timestampL);
	if (rc != ONI_ESUCCESS) return;
	rc = (uint64_t)deviceContext->readRegister(deviceIdx, DS90UB9x::LASTI2CH, &timestampH);
	if (rc != ONI_ESUCCESS) return;

	bnoTimestamps[currentFrame] = deviceContext->convertTimestampToSeconds((uint64_t(timestampH) << 32) | uint64_t(timestampL));

	sampleNumbers[currentFrame] = sampleNumber++;

	currentFrame++;

	if (currentFrame >= numFrames)
	{
		bnoBuffer->addToBuffer(bnoSamples.data(), sampleNumbers, bnoTimestamps, eventCodes, numFrames);
		currentFrame = 0;
	}
}

void PolledBno055::setBnoAxisMap(Bno055AxisMap map)
{
	axisMap = map;
}

void PolledBno055::setBnoAxisSign(Bno055AxisSign sign)
{
	axisSign = sign;
}

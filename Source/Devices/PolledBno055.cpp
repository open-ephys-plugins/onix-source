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

PolledBno055::PolledBno055(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx)
	: OnixDevice(name, OnixDeviceType::POLLEDBNO, deviceIdx_, ctx),
	I2CRegisterContext(Bno055Address, deviceIdx_, ctx)
{
	const float bitVolts = 1.0;

	StreamInfo eulerAngleStream = StreamInfo(
		name + "-Euler",
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
		name + "-Quaternion",
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Quaternion",
		"onix-bno055.data.quat",
		4,
		sampleRate,
		"Quaternion",
		ContinuousChannel::Type::AUX,
		bitVolts,
		"",
		{ "W", "X", "Y", "Z" });
	streamInfos.add(quaternionStream);

	StreamInfo accelerationStream = StreamInfo(
		name + "-Acceleration",
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
		name + "-Gravity",
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
		name + "-Temperature",
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

	StreamInfo calibrationStatusStream = StreamInfo(
		name + "-Calibration",
		"Bosch Bno055 9-axis inertial measurement unit (IMU) Calibration",
		"onix-bno055.data.cal",
		1,
		sampleRate,
		"Calibration",
		ContinuousChannel::Type::AUX,
		bitVolts,
		"",
		{ "" });
	streamInfos.add(calibrationStatusStream);

	eventCode = 0;
}

int PolledBno055::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -1;

	deserializer = std::make_unique<I2CRegisterContext>(DS90UB9x::DES_ADDR, deviceIdx, deviceContext);
	uint32_t alias = Bno055Address << 1;
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID4, alias);
	if (deserializer->getLastResult() != ONI_ESUCCESS) return -2;
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias4, alias);
	if (deserializer->getLastResult() != ONI_ESUCCESS) return -2;

	return ONI_ESUCCESS;
}

bool PolledBno055::updateSettings()
{
	if (!isEnabled()) return true;

	WriteByte(0x3E, 0x00); // Power mode normal
	if (getLastResult() != ONI_ESUCCESS) return false;

	WriteByte(0x07, 0x00); // Page ID address 0
	if (getLastResult() != ONI_ESUCCESS) return false;

	WriteByte(0x3F, 0x00); // Internal oscillator
	if (getLastResult() != ONI_ESUCCESS) return false;

	WriteByte(0x41, (uint32_t)axisMap);  // Axis map config
	if (getLastResult() != ONI_ESUCCESS) return false;

	WriteByte(0x42, (uint32_t)axisSign); // Axis sign
	if (getLastResult() != ONI_ESUCCESS) return false;

	WriteByte(0x3D, 0x0C); // Operation mode is NDOF
	if (getLastResult() != ONI_ESUCCESS) return false;

	return true;
}

void PolledBno055::startAcquisition()
{
	sampleNumber = 0;

	if (isEnabled())
		startTimer(timerIntervalInMilliseconds);
}

void PolledBno055::stopAcquisition()
{
	stopTimer();
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

void PolledBno055::hiResTimerCallback()
{
	oni_reg_val_t byte1 = 0, byte2 = 0;
	int offset = 0;

	// Euler
	ReadByte(EulerHeadingLsbAddress + 0, &byte1);
	ReadByte(EulerHeadingLsbAddress + 1, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 2, &byte1);
	ReadByte(EulerHeadingLsbAddress + 3, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 4, &byte1);
	ReadByte(EulerHeadingLsbAddress + 5, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	// Quaternion
	ReadByte(EulerHeadingLsbAddress + 6, &byte1);
	ReadByte(EulerHeadingLsbAddress + 7, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 8, &byte1);
	ReadByte(EulerHeadingLsbAddress + 9, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 10, &byte1);
	ReadByte(EulerHeadingLsbAddress + 11, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 12, &byte1);
	ReadByte(EulerHeadingLsbAddress + 13, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	// Acceleration

	ReadByte(EulerHeadingLsbAddress + 14, &byte1);
	ReadByte(EulerHeadingLsbAddress + 15, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 16, &byte1);
	ReadByte(EulerHeadingLsbAddress + 17, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 18, &byte1);
	ReadByte(EulerHeadingLsbAddress + 19, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	// Gravity

	ReadByte(EulerHeadingLsbAddress + 20, &byte1);
	ReadByte(EulerHeadingLsbAddress + 21, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 22, &byte1);
	ReadByte(EulerHeadingLsbAddress + 23, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	ReadByte(EulerHeadingLsbAddress + 24, &byte1);
	ReadByte(EulerHeadingLsbAddress + 25, &byte2);
	bnoSamples[offset] = byte1 << 8 | byte2;
	offset++;

	// Temperature

	ReadByte(EulerHeadingLsbAddress + 26, &byte1);
	bnoSamples[offset] = byte1;
	offset++;

	// Calibration Status

	ReadByte(EulerHeadingLsbAddress + 27, &byte1);
	bnoSamples[offset] = byte1;
	offset++;

	bnoTimestamp = deviceContext->readRegister(deviceIdx, DS90UB9x::LASTI2CL);
	bnoTimestamp += (uint64_t)deviceContext->readRegister(deviceIdx, DS90UB9x::LASTI2CH) << 32;

	bnoBuffer->addToBuffer(bnoSamples.data(), &sampleNumber, &bnoTimestamp, &eventCode, 1);
}

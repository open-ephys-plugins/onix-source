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

#pragma once

#include "../OnixDevice.h"
#include "../I2CRegisterContext.h"
#include "DS90UB9x.h"

class PolledBno055 : public OnixDevice,
	public I2CRegisterContext,
	public HighResolutionTimer
{
public:

	/** Constructor */
	PolledBno055(String name, const oni_dev_idx_t, std::shared_ptr<Onix1> ctx);

	~PolledBno055()
	{
		stopTimer();
	}

	int configureDevice() override;

	/** Update the settings of the device */
	bool updateSettings() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	void addFrame(oni_frame_t*) override;

	void processFrames() override {};

	void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;

	void hiResTimerCallback() override;

private:

	DataBuffer* bnoBuffer;

	static const int Bno055Address = 0x28;
	static const int EulerHeadingLsbAddress = 0x1A;

	static const int numberOfBytes = 28;

	std::unique_ptr<I2CRegisterContext> deserializer;

	enum class Bno055AxisMap : uint32_t
	{
		XYZ = 0b00100100, // Specifies that X->X', Y->Y', Z->Z' (chip default). 
		XZY = 0b00011000, // Specifies that X->X', Z->Y', Y->Z'
		YXZ = 0b00100001, // Specifies that Y->X', X->Y', Z->Z'
		YZX = 0b00001001, // Specifies that Y->X', Z->Y', X->Z'
		ZXY = 0b00010010, // Specifies that Z->X', X->Y', Y->Z'
		ZYX = 0b00000110, // Specifies that Z->X', Y->Y', X->Z'
	};

	enum class Bno055AxisSign : uint32_t
	{
		Default = 0b00000000, // Specifies that all axes are positive (chip default).
		MirrorZ = 0b00000001, // Specifies that Z' axis should be mirrored.
		MirrorY = 0b00000010, // Specifies that Y' axis should be mirrored.
		MirrorX = 0b00000100, // Specifies that X' axis should be mirrored.
	};

	enum class PolledBno055Registers : int32_t
	{
		EulerAngle = 0x1, // Specifies that the Euler angles will be polled.
		Quaternion = 0x2, // Specifies that the quaternion will be polled.
		Acceleration = 0x4, // Specifies that the linear acceleration will be polled.
		Gravity = 0x8, // Specifies that the gravity vector will be polled.
		Temperature = 0x10, // Specifies that the temperature measurement will be polled.
		Calibration = 0x20, // Specifies that the sensor calibration status will be polled.
		All = EulerAngle | Quaternion | Acceleration | Gravity | Temperature | Calibration, // Specifies that all sensor measurements and calibration status will be polled.
	};

	const Bno055AxisMap axisMap = Bno055AxisMap::XYZ;
	const Bno055AxisSign axisSign = Bno055AxisSign::Default;

	static const int numberOfChannels = 3 + 3 + 4 + 3 + 1 + 1;
	static constexpr float sampleRate = 100.0f;

	static const int timerIntervalInMilliseconds = (int)(1e3 * (1 / sampleRate));

	std::array<float, numberOfChannels> bnoSamples;

	double bnoTimestamp;
	uint64 eventCode;

	int64_t sampleNumber = 0;

	JUCE_LEAK_DETECTOR(PolledBno055);
};


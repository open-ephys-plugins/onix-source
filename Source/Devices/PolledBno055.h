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
#include "PortController.h"

#include <chrono>

namespace OnixSourcePlugin
{
	class PolledBno055 : public OnixDevice,
		public I2CRegisterContext,
		public HighResolutionTimer
	{
	public:

		/** Constructor */
		PolledBno055(std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1> ctx);

		~PolledBno055();

		int configureDevice() override;
		bool updateSettings() override;
		void startAcquisition() override;
		void stopAcquisition() override;
		void addFrame(oni_frame_t*) override;
		void processFrames() override;
		void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;

		void hiResTimerCallback() override;

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
			MirrorX = 0b00000100  // Specifies that X' axis should be mirrored.
		};

		void setBnoAxisMap(Bno055AxisMap map);
		void setBnoAxisSign(uint32_t sign);

		static OnixDeviceType getDeviceType();

	private:

		DataBuffer* bnoBuffer;

		static constexpr int Bno055Address = 0x28;
		static constexpr int EulerHeadingLsbAddress = 0x1A;

		static constexpr double I2cRate = 400e3;

		static constexpr int NumberOfBytes = 28;

		static constexpr float EulerAngleScale = 1.0f / 16; // 1 degree = 16 LSB
		static constexpr float QuaternionScale = 1.0f / (1 << 14); // 1 = 2^14 LSB
		static constexpr float AccelerationScale = 1.0f / 100; // 1m / s^2 = 100 LSB

		std::unique_ptr<I2CRegisterContext> deserializer;

		Bno055AxisMap axisMap = Bno055AxisMap::XYZ;
		uint32_t axisSign = (uint32_t)Bno055AxisSign::Default; // NB: Holds the uint value of the flag. Allows for combinations of X/Y/Z to combined together

		static constexpr int NumberOfChannels = 3 + 3 + 4 + 3 + 1 + 4;
		static constexpr double SampleRate = 30.0;

		static constexpr int TimerIntervalInMilliseconds = (int)(1e3 * (1 / SampleRate));

		static constexpr int NumFrames = 2;

		std::array<float, NumberOfChannels * NumFrames> bnoSamples;

		double bnoTimestamps[NumFrames];
		int64 sampleNumbers[NumFrames];
		uint64 eventCodes[NumFrames];

		unsigned short currentFrame = 0;
		int sampleNumber = 0;

		int16_t readInt16(uint32_t);

		JUCE_LEAK_DETECTOR(PolledBno055);
	};
}

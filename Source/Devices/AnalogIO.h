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

namespace OnixSourcePlugin
{
	enum class AnalogIORegisters : uint32_t
	{
		ENABLE = 0,
		CHDIR = 1,
		CH00_IN_RANGE = 2,
		CH01_IN_RANGE = 3,
		CH02_IN_RANGE = 4,
		CH03_IN_RANGE = 5,
		CH04_IN_RANGE = 6,
		CH05_IN_RANGE = 7,
		CH06_IN_RANGE = 8,
		CH07_IN_RANGE = 9,
		CH08_IN_RANGE = 10,
		CH09_IN_RANGE = 11,
		CH10_IN_RANGE = 12,
		CH11_IN_RANGE = 13,
	};

	enum class AnalogIOVoltageRange : uint32_t
	{
		TenVolts = 0,
		TwoPointFiveVolts = 1,
		FiveVolts = 2
	};

	enum class AnalogIODirection : uint32_t
	{
		Input = 0,
		Output = 1
	};

	enum class AnalogIODataType : uint32_t
	{
		S16 = 0,
		Volts = 1
	};

	/*
		Configures and streams data from an AnalogIO device on a Breakout Board
	*/
	class AnalogIO : public OnixDevice
	{
	public:
		AnalogIO(std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

		int configureDevice() override;
		bool updateSettings() override;
		void startAcquisition() override;
		void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;
		void processFrames() override;

		void processFrame(uint64_t eventWord = 0);

		AnalogIODirection getChannelDirection(int channelNumber);
		static std::string getChannelDirection(AnalogIODirection direction);
		void setChannelDirection(int channelNumber, AnalogIODirection direction);

		AnalogIOVoltageRange getChannelVoltageRange(int channelNumber);
		void setChannelVoltageRange(int channelNumber, AnalogIOVoltageRange direction);

		AnalogIODataType getDataType() const;
		void setDataType(AnalogIODataType type);

		static OnixDeviceType getDeviceType();

		static int getSampleRate();

		int getNumberOfFrames();

	private:

		DataBuffer* analogInputBuffer = nullptr;

		static constexpr int AnalogIOFrequencyHz = 100000;
		static constexpr int framesToAverage = 4; // NB: Downsampling from 100 kHz to 25 kHz

		static constexpr int numFrames = 25;
		static constexpr int numChannels = 12;

		static constexpr int numberOfDivisions = 1 << 16;
		static constexpr int dacMidScale = 1 << 15;

		std::array<AnalogIODirection, numChannels> channelDirection;
		std::array<AnalogIOVoltageRange, numChannels> channelVoltageRange;

		AnalogIODataType dataType = AnalogIODataType::Volts;

		unsigned short currentFrame = 0;
		unsigned short currentAverageFrame = 0;
		int sampleNumber = 0;

		std::array<float, numFrames* numChannels> analogInputSamples;

		double timestamps[numFrames];
		int64 sampleNumbers[numFrames];
		uint64 eventCodes[numFrames];

		std::array<float, numChannels> voltsPerDivision;

		static float getVoltsPerDivision(AnalogIOVoltageRange voltageRange);

		JUCE_LEAK_DETECTOR(AnalogIO);
	};
}

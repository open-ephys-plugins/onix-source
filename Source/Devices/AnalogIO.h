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

#ifndef __ANALOGIO_H__
#define __ANALOGIO_H__

#include "../OnixDevice.h"

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
	AnalogIO(String name, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

	/** Configures the device so that it is ready to stream with default settings */
	int configureDevice() override;

	/** Update the settings of the device */
	bool updateSettings() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	/** Given the sourceBuffers from OnixSource, add all streams for the current device to the array */
	void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;

	void addFrame(oni_frame_t*) override;

	void processFrames() override;

	AnalogIODirection getChannelDirection(int channelNumber)
	{
		if (channelNumber > numChannels || channelNumber < 0)
		{
			LOGE("Channel number must be between 0 and " + String(channelNumber));
			return AnalogIODirection::Input;
		}

		return channelDirection[channelNumber];
	}

	void setChannelDirection(int channelNumber, AnalogIODirection direction)
	{
		if (channelNumber > numChannels || channelNumber < 0)
		{
			LOGE("Channel number must be between 0 and " + String(channelNumber));
			return;
		}

		channelDirection[channelNumber] = direction;
	}

	AnalogIOVoltageRange getChannelVoltageRange(int channelNumber)
	{
		if (channelNumber > numChannels || channelNumber < 0)
		{
			LOGE("Channel number must be between 0 and " + String(channelNumber));
			return AnalogIOVoltageRange::FiveVolts;
		}

		return channelVoltageRange[channelNumber];
	}

	void setChannelVoltageRange(int channelNumber, AnalogIOVoltageRange direction)
	{
		if (channelNumber > numChannels || channelNumber < 0)
		{
			LOGE("Channel number must be between 0 and " + String(channelNumber));
			return;
		}

		channelVoltageRange[channelNumber] = direction;
	}

	AnalogIODataType getDataType() const { return dataType; }

	void setDataType(AnalogIODataType type) { dataType = type; }

	int getNumChannels() { return numChannels; }

private:

	DataBuffer* analogInputBuffer = nullptr;

	static const int numFrames = 25;
	static const int framesToAverage = 4; // NB: Downsampling from 100 kHz to 25 kHz
	static const int numChannels = 12;

	static const int numberOfDivisions = 1 << 16;
	const int dacMidScale = 1 << 15;

	std::array<AnalogIODirection, numChannels> channelDirection;
	std::array<AnalogIOVoltageRange, numChannels> channelVoltageRange;

	AnalogIODataType dataType = AnalogIODataType::Volts;

	Array<oni_frame_t*, CriticalSection, numFrames> frameArray;

	unsigned short currentFrame = 0;
	unsigned short currentAverageFrame = 0;
	int sampleNumber = 0;

	bool shouldAddToBuffer = false;

	std::array<float, numFrames* numChannels> analogInputSamples;

	double timestamps[numFrames];
	int64 sampleNumbers[numFrames];
	uint64 eventCodes[numFrames];

	std::array<float, numChannels> voltsPerDivision;

	static float getVoltsPerDivision(AnalogIOVoltageRange voltageRange);

	JUCE_LEAK_DETECTOR(AnalogIO);
};

#endif

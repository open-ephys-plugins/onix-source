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

enum class DigitalIORegisters : uint32_t
{
	ENABLE = 0
};

enum class DigitalPortState : uint16_t
{
	Pin0 = 0x1,
	Pin1 = 0x2,
	Pin2 = 0x4,
	Pin3 = 0x8,
	Pin4 = 0x10,
	Pin5 = 0x20,
	Pin6 = 0x40,
	Pin7 = 0x80
};

enum class BreakoutButtonState : uint16_t
{
	Moon = 0x1,
	Triangle = 0x2,
	X = 0x4,
	Check = 0x8,
	Circle = 0x10,
	Square = 0x20,
	Reserved0 = 0x40,
	Reserved1 = 0x80,
	PortDOn = 0x100,
	PortCOn = 0x200,
	PortBOn = 0x400,
	PortAOn = 0x800
};

/*
	Configures and streams data from an AnalogIO device on a Breakout Board
*/
class DigitalIO : public OnixDevice
{
public:
	DigitalIO(String name, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

	/** Configures the device so that it is ready to stream with default settings */
	int configureDevice() override;

	/** Update the settings of the device */
	bool updateSettings() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	/** Given the sourceBuffers from OnixSource, add all streams for the current device to the array */
	void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override {};

	EventChannel::Settings getEventChannelSettings();

	void addFrame(oni_frame_t*) override;

	void processFrames() override;

	uint64_t getEventWord();

	bool hasEventWord();

private:

	static const int numDigitalInputs = 8;
	static const int numButtons = 6;

	Array<oni_frame_t*, CriticalSection, 10> frameArray;
	Array<uint64_t, CriticalSection, 64> eventWords;

	JUCE_LEAK_DETECTOR(DigitalIO);
};

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
	enum class HarpSyncInputRegisters : uint32_t
	{
		ENABLE = 0,
		SOURCE = 1
	};

	enum class HarpSyncSource : uint32_t
	{
		Breakout = 0,
		ClockAdapter = 1
	};

	/*
		Configures and streams data from a HarpSyncInput device on a Breakout Board
	*/
	class HarpSyncInput : public OnixDevice
	{
	public:
		HarpSyncInput(std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

		int configureDevice() override;
		bool updateSettings() override;
		void startAcquisition() override;
		void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;
		void processFrames() override;

		static OnixDeviceType getDeviceType();

	private:

		DataBuffer* harpTimeBuffer;

		static const int numFrames = 2;

		unsigned short currentFrame = 0;
		int sampleNumber = 0;

		bool shouldAddToBuffer = false;

		float harpTimeSamples[numFrames];

		double timestamps[numFrames];
		int64 sampleNumbers[numFrames];
		uint64 eventCodes[numFrames];

		JUCE_LEAK_DETECTOR(HarpSyncInput);
	};
}

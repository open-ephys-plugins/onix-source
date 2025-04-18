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
		HarpSyncInput(String name, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

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

		void addFrame(oni_frame_t* frame) override;

		void processFrames() override;

	private:

		DataBuffer* harpTimeBuffer;

		static const int numFrames = 2;

		Array<oni_frame_t*, CriticalSection, numFrames> frameArray;

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

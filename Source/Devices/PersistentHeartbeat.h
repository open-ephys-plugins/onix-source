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
	enum class PersistentHeartbeatRegisters : uint32_t
	{
		ENABLE = 0,		// Heartbeat enable state (read only; always enabled for this device).
		CLK_DIV = 1,	// Heartbeat clock divider ratio. Minimum value is CLK_HZ / 10e6.
						// Maximum value is CLK_HZ / MIN_HB_HZ.Attempting to set to a value outside
						// this range will result in error.
		CLK_HZ = 2,		// The frequency parameter, CLK_HZ, used in the calculation of CLK_DIV
		MIN_HB_HZ = 3	// The minimum allowed beat frequency, in Hz, for this device
	};

	/*
		Configures a Heartbeat device on a Breakout Board
	*/
	class PersistentHeartbeat : public OnixDevice
	{
	public:
		PersistentHeartbeat(std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

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

		static OnixDeviceType getDeviceType();

		void setBeatsPerSecond(uint32_t value);

		uint32_t getBeatsPerSecond() const;

	private:

		uint32_t minimumHeartbeatHz = 100;
		uint32_t beatsPerSecond = 100;

		static constexpr uint32_t MaxBeatsPerSecond = 10e6;

		JUCE_LEAK_DETECTOR(PersistentHeartbeat);
	};
}

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
	enum class OutputClockRegisters : uint32_t
	{
		NULL_REGISTER = 0,
		CLOCK_GATE = 1,
		HIGH_CYCLES = 2,
		LOW_CYCLES = 3,
		DELAY_CYCLES = 4,
		GATE_RUN = 5,
		BASE_FREQ_HZ = 6
	};

	/*
		Configures an OutputClock device on a Breakout Board
	*/
	class OutputClock : public OnixDevice
	{
	public:
		OutputClock(std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

		/** Device is always enabled */
		int configureDevice() override;

		/** Update the settings of the device */
		bool updateSettings() override;

		/** Starts probe data streaming */
		void startAcquisition() override {};

		/** Stops probe data streaming*/
		void stopAcquisition() override {};

		/** Given the sourceBuffers from OnixSource, add all streams for the current device to the array */
		void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override {};

		void addFrame(oni_frame_t* frame) override { oni_destroy_frame(frame); }

		void processFrames() override {};

		double getFrequencyHz() const { return frequencyHz; }

		void setFrequencyHz(double frequency) { frequencyHz = frequency; }

		int32_t getDutyCycle() const { return dutyCycle; }

		void setDutyCycle(int32_t dutyCycle_) { dutyCycle = dutyCycle_; }

		int32_t getDelay() const { return delay; }

		void setDelay(int32_t delay_) { delay = delay_; }

		bool getGateRun() const { return gateRun; }

		void setGateRun(bool gate, bool writeToRegister = false)
		{
			gateRun = gate;
			if (writeToRegister) writeGateRunRegister();
		}

		static OnixDeviceType getDeviceType();

	private:

		double frequencyHz = 1e6;
		int32_t dutyCycle = 50;
		int32_t delay = 0;

		bool gateRun = true;

		void writeGateRunRegister() { deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)OutputClockRegisters::GATE_RUN, gateRun ? 1 : 0); }

		JUCE_LEAK_DETECTOR(OutputClock);
	};
}

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

#ifndef __OUTPUTCLOCK_H__
#define __OUTPUTCLOCK_H__

#include "../OnixDevice.h"

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
	OutputClock(String name, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

	~OutputClock();

	/** Device is always enabled */
	int configureDevice() override { setEnabled(true); return 0; };

	/** Update the settings of the device */
	bool updateSettings() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	/** Given the sourceBuffers from OnixSource, add all streams for the current device to the array */
	void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override {};

	void addFrame(oni_frame_t* frame) override { oni_destroy_frame(frame); }

	void processFrames() override {};

	float getFrequencyHz() const { return frequencyHz; }

	void setFrequencyHz(float frequency) { frequencyHz = frequency; }

	uint32_t getDutyCycle() const { return dutyCycle; }

	void setDutyCycle(uint32_t dutyCycle_) { dutyCycle = dutyCycle_; }

	uint32_t getDelay() const { return delay; }

	void setDelay(uint32_t delay_) { delay = delay_; }

	bool getGateRun() const { return gateRun; }

	void setGateRun(bool gate, bool writeToRegister = false)
	{ 
		gateRun = gate;
		if (writeToRegister) writeGateRunRegister();
	}

private:

	float frequencyHz = 1e6;
	uint32_t dutyCycle = 50;
	uint32_t delay = 0;

	bool gateRun = true;

	void writeGateRunRegister() { deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)OutputClockRegisters::GATE_RUN, gateRun ? 1 : 0); }

	JUCE_LEAK_DETECTOR(OutputClock);
};

#endif // !__OUTPUTCLOCK_H__

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

#include "OutputClock.h"

OutputClock::OutputClock(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, OnixDeviceType::OUTPUTCLOCK, deviceIdx_, oni_ctx)
{
}

OutputClock::~OutputClock()
{
	if (deviceContext != nullptr && deviceContext->isInitialized()) setClockGate(false, true);
}

bool OutputClock::updateSettings()
{
	oni_reg_val_t baseFreqHz = deviceContext->readRegister(deviceIdx, (oni_reg_addr_t)OutputClockRegisters::BASE_FREQ_HZ);

	auto periodCycles = (uint32_t)(baseFreqHz / frequencyHz);
	auto h = (uint32_t)(periodCycles * (dutyCycle / 100));
	auto l = periodCycles - h;
	auto delayCycles = (uint32_t)(delay * baseFreqHz);

	deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)OutputClockRegisters::HIGH_CYCLES, h); if (deviceContext->getLastResult() != ONI_ESUCCESS) return false;
	deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)OutputClockRegisters::LOW_CYCLES, l); if (deviceContext->getLastResult() != ONI_ESUCCESS) return false;
	deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)OutputClockRegisters::DELAY_CYCLES, delayCycles); if (deviceContext->getLastResult() != ONI_ESUCCESS) return false;

	deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)OutputClockRegisters::GATE_RUN, 1); if (deviceContext->getLastResult() != ONI_ESUCCESS) return false;
}

void OutputClock::startAcquisition()
{
	writeClockGateRegister();
}

void OutputClock::stopAcquisition()
{
	setClockGate(false, true);
}
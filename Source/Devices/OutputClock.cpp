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

OutputClock::OutputClock(String name, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_)
	: OnixDevice(name, OnixDeviceType::OUTPUTCLOCK, deviceIdx_, ctx_)
{
}

OutputClock::~OutputClock()
{
	setClockGate(false, true);
}

int OutputClock::updateSettings()
{
	oni_reg_val_t baseFreqHz;
	ONI_OK_RETURN_INT(oni_read_reg(ctx, deviceIdx, (oni_reg_addr_t)OutputClockRegisters::BASE_FREQ_HZ, &baseFreqHz));

	auto periodCycles = (uint32_t)(baseFreqHz / frequencyHz);
	auto h = (uint32_t)(periodCycles * (dutyCycle / 100));
	auto l = periodCycles - h;
	auto delayCycles = (uint32_t)(delay * baseFreqHz);

	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)OutputClockRegisters::HIGH_CYCLES, h));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)OutputClockRegisters::LOW_CYCLES, l));
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)OutputClockRegisters::DELAY_CYCLES, delayCycles));

	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)OutputClockRegisters::GATE_RUN, 1));
}

void OutputClock::startAcquisition()
{
	writeClockGateRegister();
}

void OutputClock::stopAcquisition()
{
	setClockGate(false, true);
}
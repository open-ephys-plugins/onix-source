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

#include "Heartbeat.h"

Heartbeat::Heartbeat(String name, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_)
	: OnixDevice(name, OnixDeviceType::HEARTBEAT, deviceIdx_, ctx_)
{
}

int Heartbeat::configureDevice()
{
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (uint32_t)HeartbeatRegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0)));

	return 0;
}

int Heartbeat::updateSettings()
{
	writeBeatsPerSecondRegister();
	return 0;
}

void Heartbeat::setBeatsPerSecond(uint32_t beats, bool writeToRegister)
{
	beatsPerSecond = beats;
	if (writeToRegister) writeBeatsPerSecondRegister();
}

void Heartbeat::writeBeatsPerSecondRegister()
{
	oni_reg_val_t clkHz;

	ONI_OK(oni_read_reg(ctx, deviceIdx, (oni_reg_addr_t)HeartbeatRegisters::CLK_HZ, &clkHz));
	ONI_OK(oni_write_reg(ctx, deviceIdx, (oni_reg_addr_t)HeartbeatRegisters::CLK_DIV, clkHz / beatsPerSecond));
}

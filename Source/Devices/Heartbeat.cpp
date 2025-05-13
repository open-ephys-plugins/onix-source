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

#include "Heartbeat.h"

using namespace OnixSourcePlugin;

Heartbeat::Heartbeat(std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, hubName, Heartbeat::getDeviceType(), deviceIdx_, oni_ctx)
{
}

OnixDeviceType Heartbeat::getDeviceType()
{
	return OnixDeviceType::HEARTBEAT;
}

int Heartbeat::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized())
		throw error_str("Device context is not initialized properly for	" + getName());

	setEnabled(true);

	return deviceContext->writeRegister(deviceIdx, (uint32_t)HeartbeatRegisters::ENABLE, 1);
}

bool Heartbeat::updateSettings()
{
	oni_reg_val_t clkHz;
	int rc = deviceContext->readRegister(deviceIdx, (oni_reg_addr_t)HeartbeatRegisters::CLK_HZ, &clkHz);
	rc = deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)HeartbeatRegisters::CLK_DIV, clkHz / beatsPerSecond);

	if (rc == ONI_EREADONLY) return true; // NB: Ignore read-only errors

	return rc == ONI_ESUCCESS;
}

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

#include "PersistentHeartbeat.h"

using namespace OnixSourcePlugin;

PersistentHeartbeat::PersistentHeartbeat(std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, hubName, PersistentHeartbeat::getDeviceType(), deviceIdx_, oni_ctx)
{
}

OnixDeviceType PersistentHeartbeat::getDeviceType()
{
	return OnixDeviceType::PERSISTENTHEARTBEAT;
}

int PersistentHeartbeat::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized())
		throw error_str("Device context is not initialized properly for	" + getName());

	setEnabled(true); // NB: PersistentHeart is always enabled.

	return ONI_ESUCCESS;
}

bool PersistentHeartbeat::updateSettings()
{
	oni_reg_val_t clkHz;

	int rc = deviceContext->readRegister(deviceIdx, (oni_reg_addr_t)PersistentHeartbeatRegisters::CLK_HZ, &clkHz);
	if (rc != ONI_ESUCCESS) return false;

	rc = deviceContext->readRegister(deviceIdx, (oni_reg_addr_t)PersistentHeartbeatRegisters::MIN_HB_HZ, &minimumHeartbeatHz);
	if (rc != ONI_ESUCCESS) return false;

	if (beatsPerSecond < minimumHeartbeatHz || beatsPerSecond > MaxBeatsPerSecond)
	{
		LOGE("Invalid beats per second for persistent heartbeat device. Value must be between " + std::to_string(minimumHeartbeatHz) +
			" and " + std::to_string(MaxBeatsPerSecond));
		return false;
	}

	rc = deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)PersistentHeartbeatRegisters::CLK_DIV, clkHz / beatsPerSecond);

	return rc == ONI_ESUCCESS;
}

void PersistentHeartbeat::startAcquisition()
{
}

void PersistentHeartbeat::stopAcquisition()
{
}

void PersistentHeartbeat::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
}

void PersistentHeartbeat::addFrame(oni_frame_t* frame)
{
	oni_destroy_frame(frame);
}

void PersistentHeartbeat::processFrames()
{
}

void PersistentHeartbeat::setBeatsPerSecond(uint32_t value)
{
	beatsPerSecond = value;
}

uint32_t PersistentHeartbeat::getBeatsPerSecond() const
{
	return beatsPerSecond;
}

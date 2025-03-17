/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2023 Allen Institute for Brain Science and Open Ephys

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

#include "PortController.h"

PortController::PortController(PortName port_, oni_ctx ctx_) :
	OnixDevice(getPortNameString(port_), OnixDeviceType::PORT_CONTROL, (oni_dev_idx_t)port_, ctx_),
	port(port_)
{
}

PortController::~PortController()
{
}

int PortController::configureDevice()
{
	ONI_OK_RETURN_INT(oni_write_reg(ctx, deviceIdx, (uint32_t)PortControllerRegister::ENABLE, (uint32_t)1))

	return 0;
}

void PortController::startAcquisition()
{
	errorFlag = false;
}

void PortController::stopAcquisition()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}
}

void PortController::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void PortController::processFrames()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		int8_t* dataPtr = (int8_t*)frame->data;

		int dataOffset = 8;

		uint32_t code = (uint32_t) *(dataPtr + dataOffset);
		uint32_t data = (uint32_t) *(dataPtr + dataOffset + 1);

		errorFlag = errorFlag || ((uint32_t)data & LINKSTATE_SL) == 0;

		oni_destroy_frame(frame);

		LOGE("Port status changed for " + getName() + ".");
	}
}

void PortController::updateDiscoveryParameters(DiscoveryParameters parameters)
{
	discoveryParameters = parameters;
}

DiscoveryParameters PortController::getHeadstageDiscoveryParameters(String headstage)
{
	if (headstage == "Neuropixels 1.0f")
	{
		return DiscoveryParameters(5.0f, 7.0f, 1.0f, 0.2f);
	}

	return DiscoveryParameters();
}

bool PortController::configureVoltage(float voltage)
{
	if (ctx == NULL) return false;

	if (voltage == defaultVoltage)
	{
		if (discoveryParameters == DiscoveryParameters()) return false;

		for (voltage = discoveryParameters.minVoltage; voltage <= discoveryParameters.maxVoltage; voltage += discoveryParameters.voltageIncrement)
		{
			setVoltage(voltage);

			if (checkLinkState())
			{
				setVoltage(voltage + discoveryParameters.voltageOffset);
				return checkLinkState();
			}
		}
	}
	else
	{
		setVoltage(voltage);

		return checkLinkState();
	}

	return false;
}

void PortController::setVoltageOverride(float voltage, bool waitToSettle)
{
	if (ctx == NULL) return;

	ONI_OK(oni_write_reg(ctx, (oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, (oni_reg_val_t)(voltage * 10)));
	if (waitToSettle) sleep_for(std::chrono::milliseconds(500));
}

void PortController::setVoltage(float voltage)
{
	if (ctx == NULL) return;

	ONI_OK(oni_write_reg(ctx, (oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, 0));
	sleep_for(std::chrono::milliseconds(300));
	
	ONI_OK(oni_write_reg(ctx, (oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, (oni_reg_val_t)(voltage * 10)));
	sleep_for(std::chrono::milliseconds(500));
}

bool PortController::checkLinkState() const
{
	if (ctx == NULL) return false;

	oni_reg_val_t linkState;
	int result = oni_read_reg(ctx, (oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::LINKSTATE, &linkState);

	if (result != 0) { LOGE(oni_error_str(result)); return false; }
	else if ((linkState & LINKSTATE_SL) == 0) { LOGE("Unable to acquire communication lock."); return false; }
	else return true;
}

String PortController::getPortNameString(PortName portName)
{
	switch (portName)
	{
	case PortName::PortA:
		return "Port A";
	case PortName::PortB:
		return "Port B";
	default:
		break;
	}
}

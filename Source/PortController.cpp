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

PortController::PortController(PortName port_) :
	port(port_)
{
}

PortController::~PortController()
{
}

void PortController::updateDiscoveryParameters(DiscoveryParameters parameters)
{
	discoveryParameters = parameters;
}

DiscoveryParameters PortController::getHeadstageDiscoveryParameters(String headstage)
{
	if (headstage == NEUROPIXELSV1F_HEADSTAGE_NAME)
	{
		return DiscoveryParameters(5.0f, 7.0f, 1.0f, 0.2f);
	}
	else if (headstage == "TEST HEADSTAGE") // NOTE: Remove this before merging
	{
		return DiscoveryParameters(1.0f, 2.0f, 0.0f, 0.5f);
	}

	return DiscoveryParameters();
}

bool PortController::configureVoltage(oni_ctx ctx, float voltage) const
{
	if (ctx == NULL) return false;

	if (voltage == defaultVoltage)
	{
		if (discoveryParameters == DiscoveryParameters()) return false;

		for (voltage = discoveryParameters.minVoltage; voltage <= discoveryParameters.maxVoltage; voltage += discoveryParameters.voltageIncrement)
		{
			if (!setVoltage(ctx, voltage)) return false;

			if (checkLinkState(ctx))
			{
				if (!setVoltage(ctx, voltage + discoveryParameters.voltageOffset)) return false;

				auto val = 1;
				ONI_OK_RETURN_BOOL(oni_set_opt(ctx, ONI_OPT_RESET, &val, sizeof(val)));

				sleep_for(std::chrono::milliseconds(200));

				return checkLinkState(ctx);
			}
		}
	}
	else
	{
		if (!setVoltage(ctx, voltage + discoveryParameters.voltageOffset)) return false;

		auto val = 1;
		ONI_OK_RETURN_BOOL(oni_set_opt(ctx, ONI_OPT_RESET, &val, sizeof(val)));

		sleep_for(std::chrono::milliseconds(200));

		return checkLinkState(ctx);
	}
}

bool PortController::setVoltage(oni_ctx ctx, float voltage) const
{
	if (ctx == NULL) return false;

	ONI_OK_RETURN_BOOL(oni_write_reg(ctx, (oni_dev_idx_t)port, voltageRegister, 0));

	if (voltage == 0.0f) return true;

	sleep_for(std::chrono::milliseconds(300));

	ONI_OK_RETURN_BOOL(oni_write_reg(ctx, (oni_dev_idx_t)port, voltageRegister, (oni_reg_val_t)(voltage * 10)));

	sleep_for(std::chrono::milliseconds(500));

	return true;
}

bool PortController::checkLinkState(oni_ctx ctx) const
{
	if (ctx == NULL) return false;

	oni_reg_val_t linkState;
	int result = oni_read_reg(ctx, (oni_dev_idx_t)port, linkStateRegister, &linkState);

	if (result != 0) { LOGE(oni_error_str(result)); return false; }
	else if ((linkState & (uint32_t)0x1) == 0) { LOGE("Unable to acquire communication lock."); return false; }
	else return true;
}

PortName PortController::getPortFromIndex(oni_dev_idx_t index)
{
	return index & (1 << 8) ? PortName::PortA : PortName::PortB;
}

Array<PortName> PortController::getUniquePortsFromIndices(std::vector<int> indices)
{
	Array<PortName> ports;

	for (auto index : indices) 
	{ 
		ports.addIfNotAlreadyThere(PortController::getPortFromIndex(index)); 
	}

	return ports;
}

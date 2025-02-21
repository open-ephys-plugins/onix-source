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

#ifndef __PORTCONTROLLER_H__
#define __PORTCONTROLLER_H__

#include <thread>
#include <chrono>

#include "oni.h"
#include "OnixDevice.h"

using namespace std::this_thread;

struct DiscoveryParameters
{
	float minVoltage = 0.0f;
	float maxVoltage = 0.0f;
	float voltageOffset = 0.0f;
	float voltageIncrement = 0.0f;

	DiscoveryParameters() {};

	DiscoveryParameters(float minVoltage_, float maxVoltage_, float voltageOffset_, float voltageIncrement_)
	{
		minVoltage = minVoltage_;
		maxVoltage = maxVoltage_;
		voltageOffset = voltageOffset_;
		voltageIncrement = voltageIncrement_;
	}

	bool operator==(const DiscoveryParameters& rhs) const
	{
		return rhs.minVoltage == minVoltage && rhs.maxVoltage == maxVoltage && rhs.voltageOffset == voltageOffset && rhs.voltageIncrement == voltageIncrement;
	}
};

class PortController
{
public:
	PortController(PortName port_);

	~PortController();

	void updateDiscoveryParameters(DiscoveryParameters parameters);

	bool configureVoltage(oni_ctx ctx, float voltage = defaultVoltage) const;

	bool setVoltage(oni_ctx ctx, float voltage) const;

	bool checkLinkState(oni_ctx ctx) const;

	static DiscoveryParameters getHeadstageDiscoveryParameters(String headstage);

	static int getPortOffset(PortName port) { return (uint32_t)port << 8; }

	static String getPortName(PortName port) { return port == PortName::PortA ? "Port A" : "Port B"; }

	static PortName getPortFromIndex(oni_dev_idx_t index);

	static Array<PortName> getUniquePortsFromIndices(std::vector<int>);

private:
	const PortName port;

	static constexpr float defaultVoltage = -1.0f;

	const oni_reg_addr_t voltageRegister = 3;
	const oni_reg_addr_t linkStateRegister = 5;

	DiscoveryParameters discoveryParameters;
};

#endif // !__PORTCONTROLLER_H__

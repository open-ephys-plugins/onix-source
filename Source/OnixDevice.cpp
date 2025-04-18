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

#include "OnixDevice.h"

using namespace OnixSourcePlugin;

OnixDevice::OnixDevice(String name_, String headstageName, OnixDeviceType type_, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx)
	: type(type_), deviceIdx(deviceIdx_)
{
	deviceContext = ctx;
	name = name_;
	m_headstageName = headstageName;

	if (type == OnixDeviceType::NEUROPIXELSV2E || type == OnixDeviceType::POLLEDBNO)
		isPassthrough = true;
}

oni_dev_idx_t OnixDevice::getDeviceIdx(bool getPassthroughIndex)
{
	if (isPassthrough && !getPassthroughIndex)
		return getDeviceIndexFromPassthroughIndex(deviceIdx);
	else
		return deviceIdx;
}

oni_dev_idx_t OnixDevice::getDeviceIndexFromPassthroughIndex(oni_dev_idx_t passthroughIndex) const
{
	oni_dev_idx_t idx = (passthroughIndex - 7) << 8;

	if (type == OnixDeviceType::POLLEDBNO)
		idx++;

	return idx;
}

OnixDeviceType OnixDevice::getDeviceType() const
{
	return type;
}

String OnixDevice::getStreamIdentifier()
{
	String streamIdentifier = "onix";

	// Insert the headstage or breakout board
	if (getHeadstageName() == NEUROPIXELSV1F_HEADSTAGE_NAME)
	{
		streamIdentifier += ".npx1f";
	}
	else if (getHeadstageName() == NEUROPIXELSV2E_HEADSTAGE_NAME)
	{
		streamIdentifier += ".npx2e";
	}
	else if (getHeadstageName() == BREAKOUT_BOARD_NAME)
	{
		streamIdentifier += ".breakout";
	}
	else
	{
		streamIdentifier += ".headstage";
	}

	// Insert the device
	if (getDeviceType() == OnixDeviceType::ANALOGIO)
	{
		streamIdentifier += ".analogio";
	}
	else if (getDeviceType() == OnixDeviceType::BNO || getDeviceType() == OnixDeviceType::POLLEDBNO)
	{
		streamIdentifier += ".9dof";
	}
	else if (getDeviceType() == OnixDeviceType::DIGITALIO)
	{
		streamIdentifier += ".digitalio";
	}
	else if (getDeviceType() == OnixDeviceType::HARPSYNCINPUT)
	{
		streamIdentifier += ".harp";
	}
	else if (getDeviceType() == OnixDeviceType::MEMORYMONITOR)
	{
		streamIdentifier += ".memory";
	}
	else if (getDeviceType() == OnixDeviceType::NEUROPIXELSV1F)
	{
		streamIdentifier += ".npx1f";
	}
	else if (getDeviceType() == OnixDeviceType::NEUROPIXELSV2E)
	{
		streamIdentifier += ".npx2e";
	}
	else
	{
		streamIdentifier += ".device";
	}

	return streamIdentifier;
}

int OnixDevice::getPortOffset(PortName port)
{
	return (uint32_t)port << 8;
}

String OnixDevice::getPortName(int offset)
{
	switch (offset)
	{
	case 0:
		return "";
	case HubAddressPortA:
		return "Port A";
	case HubAddressPortB:
		return "Port B";
	default:
		return "";
	}
}

String OnixDevice::getPortName(PortName port)
{
	return getPortName(getPortOffset(port));
}

String OnixDevice::getPortNameFromIndex(oni_dev_idx_t index)
{
	return getPortName(getOffsetFromIndex(index));
}

PortName OnixDevice::getPortFromIndex(oni_dev_idx_t index)
{
	return index & (1 << 8) ? PortName::PortA : PortName::PortB;
}

int OnixDevice::getOffsetFromIndex(oni_dev_idx_t index)
{
	return index & 0b1100000000;
}

Array<int> OnixDevice::getUniqueOffsetsFromIndices(std::vector<int> indices)
{
	Array<int> offsets;

	for (auto index : indices)
	{
		offsets.addIfNotAlreadyThere(getOffsetFromIndex(index));
	}

	return offsets;
}

Array<PortName> OnixDevice::getUniquePortsFromIndices(std::vector<int> indices)
{
	Array<PortName> ports;

	for (auto index : indices)
	{
		ports.addIfNotAlreadyThere(getPortFromIndex(index));
	}

	return ports;
}

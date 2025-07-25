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

OnixDevice::OnixDevice(std::string name_, std::string hubName, OnixDeviceType type_, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx, bool passthrough)
	: type(type_), deviceIdx(deviceIdx_), frameQueue(32)
{
	deviceContext = ctx;
	name = name_;
	m_hubName = hubName;
	isPassthrough = passthrough;
}

oni_dev_idx_t OnixDevice::getDeviceIdx(bool getPassthroughIndex)
{
	if (isPassthrough && !getPassthroughIndex)
		return getDeviceIndexFromPassthroughIndex(deviceIdx);
	else
		return deviceIdx;
}

std::string OnixDevice::createStreamName(std::vector<std::string> names)
{
	std::string streamName;

	for (int i = 0; i < names.size(); i++)
	{
		streamName += names[i];

		if (i != names.size() - 1) streamName += "-";
	}

	streamName.erase(std::remove(streamName.begin(), streamName.end(), ' '), streamName.end());

	return streamName;
}

oni_dev_idx_t OnixDevice::getHubIndexFromPassthroughIndex(oni_dev_idx_t passthroughIndex)
{
	if (passthroughIndex != (uint32_t)PassthroughIndex::A && passthroughIndex != (uint32_t)PassthroughIndex::B)
	{
		LOGE("Invalid passthrough index given. Value was ", passthroughIndex);
		return 0;
	}

	return (passthroughIndex - 7) << 8;
}

oni_dev_idx_t OnixDevice::getDeviceIndexFromPassthroughIndex(oni_dev_idx_t passthroughIndex) const
{
	if (passthroughIndex != (uint32_t)PassthroughIndex::A && passthroughIndex != (uint32_t)PassthroughIndex::B)
	{
		LOGE("Invalid passthrough index given. Value was ", passthroughIndex);
		return 0;
	}

	auto idx = getHubIndexFromPassthroughIndex(passthroughIndex);

	if (type == OnixDeviceType::POLLEDBNO && (m_hubName == NEUROPIXELSV2E_HEADSTAGE_NAME || m_hubName == NEUROPIXELSV1E_HEADSTAGE_NAME))
		idx++;

	return idx;
}

OnixDeviceType OnixDevice::getDeviceType() const
{
	return type;
}

std::string OnixDevice::getStreamIdentifier()
{
	std::string streamIdentifier = "onix";

	// Insert the headstage or breakout board
	if (getHubName() == NEUROPIXELSV1F_HEADSTAGE_NAME)
	{
		streamIdentifier += ".npx1f";
	}
	else if (getHubName() == NEUROPIXELSV2E_HEADSTAGE_NAME)
	{
		streamIdentifier += ".npx2e";
	}
	else if (getHubName() == BREAKOUT_BOARD_NAME)
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

std::string OnixDevice::getPortName(oni_dev_idx_t index)
{
	switch (getOffset(index))
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

std::string OnixDevice::getPortName(PortName port)
{
	return getPortName(getPortOffset(port));
}

PortName OnixDevice::getPortFromIndex(oni_dev_idx_t index)
{
	return index & (1 << 8) ? PortName::PortA : PortName::PortB;
}

int OnixDevice::getOffset(oni_dev_idx_t index)
{
	return index & 0x0000FF00;
}

std::vector<int> OnixDevice::getUniqueOffsets(std::vector<int> indices, bool ignoreBreakoutBoard)
{
	std::set<int> offsets;

	for (auto index : indices)
	{
		auto offset = getOffset(index);

		if (offset == HubAddressBreakoutBoard && ignoreBreakoutBoard) continue;

		offsets.emplace(offset);
	}

	return std::vector<int>(offsets.begin(), offsets.end());
}

std::vector<int> OnixDevice::getUniqueOffsets(OnixDeviceMap devices, bool ignoreBreakoutBoard)
{
	std::vector<int> indices;

	for (const auto& [key, _] : devices) 
	{ 
		indices.emplace_back(key);
	}

	return getUniqueOffsets(indices, ignoreBreakoutBoard);
}

Array<PortName> OnixDevice::getUniquePorts(std::vector<int> indices)
{
	Array<PortName> ports;

	for (auto index : indices)
	{
		ports.addIfNotAlreadyThere(getPortFromIndex(index));
	}

	return ports;
}

bool OnixDevice::compareIndex(uint32_t index)
{
	return index == deviceIdx;
}

void OnixDevice::addFrame(oni_frame_t* frame)
{
	frameQueue.enqueue(frame);
}

void OnixDevice::stopAcquisition()
{
	oni_frame_t* frame;
	while (frameQueue.try_dequeue(frame))
	{
		oni_destroy_frame(frame);
	}
}

CompositeDevice::CompositeDevice(std::string name_, std::string hubName, CompositeDeviceType type_, OnixDeviceVector devices_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name_, hubName, OnixDeviceType::COMPOSITE, devices_.at(0)->getDeviceIdx(), oni_ctx)
{
	compositeType = type_;
	devices = devices_;
}

CompositeDeviceType CompositeDevice::getCompositeDeviceType() const
{
	return compositeType;
}

bool CompositeDevice::compareIndex(uint32_t index)
{
	for (const auto& device : devices)
	{
		if (device->getDeviceIdx() == index)
			return true;
	}

	return false;
}

bool CompositeDevice::isEnabled() const
{
	bool enabled = true;

	for (const auto& device : devices)
	{
		enabled &= device->isEnabled();
	}

	return enabled;
}

bool CompositeDevice::isEnabled(uint32_t index)
{
	for (const auto& device : devices)
	{
		if (device->compareIndex(index))
			return device->isEnabled();
	}

	Onix1::showWarningMessageBoxAsync(
		"Unknown Index",
		"Could not get the enabled status of a device at index " + std::to_string(index) + ", it was not found in " + getName()
	);

	return false;
}

void CompositeDevice::setEnabled(bool newState)
{
	for (const auto& device : devices)
	{
		device->setEnabled(newState);
	}
}

void CompositeDevice::setEnabled(uint32_t index, bool newState)
{
	for (const auto& device : devices)
	{
		if (device->compareIndex(index))
		{
			device->setEnabled(newState);
			return;
		}
	}

	Onix1::showWarningMessageBoxAsync(
		"Unknown Index",
		"Could not set the enabled status of a device at index " + std::to_string(index) + ", it was not found in " + getName()
	);

	return;
}

int CompositeDevice::configureDevice()
{
	int result = ONI_ESUCCESS;

	for (const auto& device : devices)
	{
		result |= device->configureDevice();
	}

	return result;
}

bool CompositeDevice::updateSettings()
{
	bool result = true;

	for (const auto& device : devices)
	{
		result &= device->updateSettings();
	}

	return result;
}

void CompositeDevice::startAcquisition()
{
	for (const auto& device : devices)
	{
		device->startAcquisition();
	}
}

void CompositeDevice::stopAcquisition()
{
	for (const auto& device : devices)
	{
		device->stopAcquisition();
	}
}

void CompositeDevice::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	for (const auto& device : devices)
	{
		device->addSourceBuffers(sourceBuffers);
	}
}

void CompositeDevice::addFrame(oni_frame_t* frame)
{
	for (const auto& device : devices)
	{
		if (device->compareIndex(frame->dev_idx))
		{
			device->addFrame(frame);
			return;
		}
	}
}

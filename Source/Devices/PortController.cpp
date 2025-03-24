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

PortController::PortController(PortName port_, std::shared_ptr<Onix1> ctx_) :
	OnixDevice(getPortName(port_), OnixDeviceType::PORT_CONTROL, (oni_dev_idx_t)port_, ctx_),
	port(port_)
{
}

int PortController::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -5;

	deviceContext->writeRegister(deviceIdx, (uint32_t)PortControllerRegister::ENABLE, 1);

	return deviceContext->getLastResult();
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
	if (headstage == NEUROPIXELSV1F_HEADSTAGE_NAME)
	{
		return DiscoveryParameters(5.0f, 7.0f, 1.0f, 0.2f);
	}

	return DiscoveryParameters();
}

String PortController::getPortName(int offset)
{
	switch (offset)
	{
	case 0:
		return "";
	case 256: // NB: Port A
		return "Port A";
	case 512: // NB: Port B
		return "Port B";
	default:
		return "";
	}
}

bool PortController::configureVoltage(float voltage)
{
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
	else if (voltage >= 0.0f && voltage <= 7.0f)
	{
		setVoltageOverride(0.0f, true);
		setVoltageOverride(voltage, true);
		sleep_for(std::chrono::milliseconds(600));

		return checkLinkState();
	}

	return false;
}

void PortController::setVoltageOverride(float voltage, bool waitToSettle)
{
	if (voltage < 0.0f && voltage > 7.0f) { LOGE("Invalid voltage value. Tried to set the port to " + String(voltage) + " V."); return; }

	deviceContext->writeRegister((oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, voltage * 10);
	if (deviceContext->getLastResult() != ONI_ESUCCESS) return;
	if (waitToSettle) sleep_for(std::chrono::milliseconds(500));
}

void PortController::setVoltage(float voltage)
{
	if (voltage < 0.0f && voltage > 7.0f) { LOGE("Invalid voltage value. Tried to set the port to " + String(voltage) + " V."); return; }

	deviceContext->writeRegister((oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, 0);
	if (deviceContext->getLastResult() != ONI_ESUCCESS) return;
	sleep_for(std::chrono::milliseconds(300));

	deviceContext->writeRegister((oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, voltage * 10);
	if (deviceContext->getLastResult() != ONI_ESUCCESS) return;
	sleep_for(std::chrono::milliseconds(500));
}

bool PortController::checkLinkState() const
{
	oni_reg_val_t linkState = deviceContext->readRegister((oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::LINKSTATE);

	if (deviceContext->getLastResult() != ONI_ESUCCESS) { return false; }
	else if ((linkState & LINKSTATE_SL) == 0) { LOGE("Unable to acquire communication lock."); return false; }
	else return true;
}

PortName PortController::getPortFromIndex(oni_dev_idx_t index)
{
	return index & (1 << 8) ? PortName::PortA : PortName::PortB;
}

int PortController::getOffsetFromIndex(oni_dev_idx_t index)
{
	return index & 0b1100000000;
}

Array<int> PortController::getUniqueOffsetsFromIndices(std::vector<int> indices)
{
	Array<int> offsets;

	for (auto index : indices)
	{
		offsets.addIfNotAlreadyThere(getOffsetFromIndex(index));
	}

	return offsets;
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

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

#include "PortController.h"

using namespace OnixSourcePlugin;

PortController::PortController(PortName port_, std::shared_ptr<Onix1> ctx_) :
	OnixDevice(OnixDevice::getPortName(port_), BREAKOUT_BOARD_NAME, OnixDeviceType::PORT_CONTROL, (oni_dev_idx_t)port_, ctx_),
	port(port_)
{
}

int PortController::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return 1;

	return deviceContext->writeRegister(deviceIdx, (uint32_t)PortControllerRegister::ENABLE, 1);
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

		uint32_t code = (uint32_t) * (dataPtr + dataOffset);
		uint32_t data = (uint32_t) * (dataPtr + dataOffset + 1);

		errorFlag = errorFlag || ((uint32_t)data & LINKSTATE_SL) == 0;

		oni_destroy_frame(frame);

		LOGE("Port status changed for " + getName() + ".");
	}
}

void PortController::updateDiscoveryParameters(DiscoveryParameters parameters)
{
	discoveryParameters = parameters;
}

DiscoveryParameters PortController::getHeadstageDiscoveryParameters(std::string headstage)
{
	if (headstage == NEUROPIXELSV1E_HEADSTAGE_NAME)
	{
		return DiscoveryParameters(3.3f, 5.5f, 1.0f, 0.2f);
	}
	else if (headstage == NEUROPIXELSV1F_HEADSTAGE_NAME)
	{
		return DiscoveryParameters(5.0f, 7.0f, 1.0f, 0.2f);
	}
	else if (headstage == NEUROPIXELSV2E_HEADSTAGE_NAME)
	{
		return DiscoveryParameters(3.3f, 5.5f, 1.0f, 0.2f);
	}

	return DiscoveryParameters();
}

bool PortController::configureVoltage(double voltage)
{
	if (voltage == defaultVoltage)
	{
		if (discoveryParameters == DiscoveryParameters() || discoveryParameters.voltageIncrement <= 0)
			return false;

		ConfigureVoltageWithProgressBar progressBar = ConfigureVoltageWithProgressBar(discoveryParameters, this);
		progressBar.runThread();

		bool result = progressBar.getResult();

		if (!result)
			setVoltageOverride(0, false);

		return result;
	}
	else if (voltage >= 0.0 && voltage <= 7.0)
	{
		setVoltage(voltage);

		bool result = checkLinkState();

		if (!result)
			setVoltageOverride(0, false);

		return result;
	}

	return false;
}

void PortController::setVoltageOverride(double voltage, bool waitToSettle)
{
	if (voltage < 0.0 && voltage > 7.0) { LOGE("Invalid voltage value. Tried to set the port to " + std::to_string(voltage) + " V."); return; }

	int rc = deviceContext->writeRegister((oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, voltage * 10);
	if (rc != ONI_ESUCCESS) return;

	lastVoltageSet = voltage;

	if (waitToSettle) sleep_for(std::chrono::milliseconds(500));
}

void PortController::setVoltage(double voltage)
{
	if (voltage < 0.0 && voltage > 7.0) { LOGE("Invalid voltage value. Tried to set the port to " + std::to_string(voltage) + " V."); return; }

	int rc = deviceContext->writeRegister((oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, 0);
	sleep_for(std::chrono::milliseconds(300));
	if (rc != ONI_ESUCCESS) return;

	rc = deviceContext->writeRegister((oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::PORTVOLTAGE, voltage * 10);
	if (rc != ONI_ESUCCESS) return;

	lastVoltageSet = voltage;

	sleep_for(std::chrono::milliseconds(500));
}

bool PortController::checkLinkState() const
{
	oni_reg_val_t linkState;
	int rc = deviceContext->readRegister((oni_dev_idx_t)port, (oni_reg_addr_t)PortControllerRegister::LINKSTATE, &linkState);

	if (rc != ONI_ESUCCESS) { return false; }
	else if ((linkState & LINKSTATE_SL) == 0) { LOGD("Unable to acquire communication lock."); return false; }
	else return true;
}

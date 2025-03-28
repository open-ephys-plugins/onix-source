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
#include "../OnixDevice.h"

using namespace std::this_thread;

enum class PortControllerRegister : uint32_t
{
	ENABLE = 0,
	GPOSTATE = 1,
	DESPWR = 2,
	PORTVOLTAGE = 3,
	SAVEVOLTAGE = 4,
	LINKSTATE = 5
};

enum class PortStatusCode : uint32_t
{
	SerdesLock = 0x0001,
	SerdesParityPass = 0x0002,
	CrcError = 0x0100,
	TooManyDevices = 0x0200,
	InitializationError = 0x0400,
	BadPacketFormat = 0x0800,
	InitializationCrcError = 0x1000,
};

class DiscoveryParameters
{
public:
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

	~DiscoveryParameters() {};

	bool operator==(const DiscoveryParameters& rhs) const
	{
		return rhs.minVoltage == minVoltage && rhs.maxVoltage == maxVoltage && rhs.voltageOffset == voltageOffset && rhs.voltageIncrement == voltageIncrement;
	}
};

class PortController : public OnixDevice
{
public:
	PortController(PortName port_, std::shared_ptr<Onix1> ctx_);

	int configureDevice() override;

	bool updateSettings() override { return true; }

	void startAcquisition() override;

	void stopAcquisition() override;

	void addFrame(oni_frame_t*) override;

	void processFrames() override;

	void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override {};

	void updateDiscoveryParameters(DiscoveryParameters parameters);

	bool configureVoltage(float voltage = defaultVoltage);

	/** Sets the voltage to the given value, after setting the voltage to zero */
	void setVoltage(float voltage);

	/** Overrides the voltage setting and directly sets it to the given voltage */
	void setVoltageOverride(float voltage, bool waitToSettle = true);

	bool checkLinkState() const;

	static DiscoveryParameters getHeadstageDiscoveryParameters(String headstage);

	static int getPortOffset(PortName port) { return (uint32_t)port << 8; }

	static String getPortName(int offset);

	static String getPortName(PortName port) { return port == PortName::PortA ? "Port A" : "Port B"; }

	static PortName getPortFromIndex(oni_dev_idx_t index);

	static int getOffsetFromIndex(oni_dev_idx_t index);

	static Array<int> getUniqueOffsetsFromIndices(std::vector<int> indices);

	static Array<PortName> getUniquePortsFromIndices(std::vector<int>);

	/** Check if the port status changed and there is an error reported */
	bool getErrorFlag() { return errorFlag; }

private:
	Array<oni_frame_t*, CriticalSection, 10> frameArray;

	const PortName port;

	static constexpr float defaultVoltage = -1.0f;

	const uint32_t LINKSTATE_PP = 0x2; // parity check pass bit
	const uint32_t LINKSTATE_SL = 0x1; // SERDES lock bit

	DiscoveryParameters discoveryParameters;

	std::atomic<bool> errorFlag = false;

	JUCE_LEAK_DETECTOR(PortController);
};

#endif // !__PORTCONTROLLER_H__

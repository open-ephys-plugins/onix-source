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

#pragma once

#include <thread>
#include <chrono>

#include "oni.h"
#include "../OnixDevice.h"

using namespace std::this_thread;

namespace OnixSourcePlugin
{
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
		double minVoltage = 0.0;
		double maxVoltage = 0.0;
		double voltageOffset = 0.0;
		double voltageIncrement = 0.0;

		DiscoveryParameters() {};

		DiscoveryParameters(double minVoltage_, double maxVoltage_, double voltageOffset_, double voltageIncrement_)
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

		bool configureVoltage(double voltage = defaultVoltage);

		/** Sets the voltage to the given value, after setting the voltage to zero */
		void setVoltage(double voltage);

		/** Overrides the voltage setting and directly sets it to the given voltage */
		void setVoltageOverride(double voltage, bool waitToSettle = true);

		bool checkLinkState() const;

		static DiscoveryParameters getHeadstageDiscoveryParameters(std::string headstage);

		std::string getPortName() const { return OnixDevice::getPortName(port); }

		/** Check if the port status changed and there is an error reported */
		bool getErrorFlag() { return errorFlag; }

		double getLastVoltageSet() const { return lastVoltageSet; }

	private:
		Array<oni_frame_t*, CriticalSection, 10> frameArray;

		const PortName port;

		static constexpr double defaultVoltage = -1.0;

		double lastVoltageSet = 0.0;

		static constexpr uint32_t LINKSTATE_PP = 0x2; // parity check pass bit
		static constexpr uint32_t LINKSTATE_SL = 0x1; // SERDES lock bit

		DiscoveryParameters discoveryParameters;

		std::atomic<bool> errorFlag = false;

		JUCE_LEAK_DETECTOR(PortController);
	};

	class ConfigureVoltageWithProgressBar : public ThreadWithProgressWindow
	{
	public:
		ConfigureVoltageWithProgressBar(DiscoveryParameters params, PortController* port)
			: ThreadWithProgressWindow("Configuring voltage on " + port->getPortName(), true, false)
		{
			m_params = params;
			m_port = port;
		}

		void run() override
		{
			double voltage, progress = 0.0;

			double increment = m_params.voltageIncrement / (m_params.maxVoltage - m_params.minVoltage);

			for (voltage = m_params.minVoltage; voltage <= m_params.maxVoltage; voltage += m_params.voltageIncrement)
			{
				progress += increment;
				setProgress(progress);

				m_port->setVoltage(voltage);

				if (m_port->checkLinkState())
				{
					setProgress(0.95);
					m_port->setVoltage(voltage + m_params.voltageOffset);
					result = m_port->checkLinkState();;
					setProgress(1.0);
					return;
				}
			}

			result = false;
			return;
		}

		bool getResult() const { return result; }

	private:

		DiscoveryParameters m_params;

		PortController* m_port;

		bool result = false;

		JUCE_LEAK_DETECTOR(ConfigureVoltageWithProgressBar);
	};
}

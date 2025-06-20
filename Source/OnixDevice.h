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

#include <DataThreadHeaders.h>

#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>

#include "Onix1.h"

using namespace std::chrono;

namespace OnixSourcePlugin
{
	enum class PortName
	{
		PortA = 1,
		PortB = 2
	};

	enum class OnixDeviceType {
		BNO,
		POLLEDBNO,
		NEUROPIXELSV1E,
		NEUROPIXELSV1F,
		NEUROPIXELSV2E,
		PORT_CONTROL,
		MEMORYMONITOR,
		OUTPUTCLOCK,
		HEARTBEAT,
		PERSISTENTHEARTBEAT,
		HARPSYNCINPUT,
		ANALOGIO,
		DIGITALIO,
	};

	struct StreamInfo {
	public:
		StreamInfo() {}

		StreamInfo(std::string name, std::string description, std::string streamIdentifier, int numChannels, float sampleRate, std::string channelPrefix, ContinuousChannel::Type channelType,
			float bitVolts, std::string units, std::vector<std::string> channelNameSuffixes, std::string channelIdentifierDataType, std::vector<std::string> channelIdentifierSubTypes = {})
		{
			m_name = name;
			m_description = description;
			m_streamIdentifier = streamIdentifier;
			m_numChannels = numChannels;
			m_sampleRate = sampleRate;
			m_channelPrefix = channelPrefix;
			m_channelType = channelType;
			m_bitVolts = bitVolts;
			m_units = units;
			m_channelNameSuffixes = channelNameSuffixes;
			m_channelIdentifierDataType = channelIdentifierDataType;
			m_channelIdentifierSubTypes = channelIdentifierSubTypes;

			if (m_numChannels != m_channelNameSuffixes.size())
			{
				if (m_channelNameSuffixes.size() != 0)
					LOGE("Difference between number of channels and channel name suffixes. Generating default suffixes instead.");

				m_channelNameSuffixes.clear();
				m_channelNameSuffixes.reserve(numChannels);

				for (int i = 0; i < m_numChannels; i++)
				{
					m_channelNameSuffixes.push_back(std::to_string(i + 1));
				}
			}

			if (m_channelIdentifierSubTypes.size() > 0 && m_numChannels != m_channelIdentifierSubTypes.size())
			{
				if (m_channelIdentifierSubTypes.size() == 1)
				{
					m_channelIdentifierSubTypes.reserve(m_numChannels);

					for (int i = 1; i < m_numChannels; i++)
					{
						m_channelIdentifierSubTypes.push_back(m_channelIdentifierSubTypes[0]);
					}
				}
				else
				{
					LOGE("Difference between number of channels and channel identifier subtypes. Generating default subtypes instead.");

					m_channelIdentifierSubTypes.clear();
					m_channelIdentifierSubTypes.reserve(numChannels);

					for (int i = 0; i < m_numChannels; i++)
					{
						m_channelIdentifierSubTypes.push_back(std::to_string(i + 1));
					}
				}
			}
		};

		std::string getName() const { return m_name; }
		std::string getDescription() const { return m_description; }
		std::string getStreamIdentifier() const { return m_streamIdentifier; }
		int getNumChannels() const { return m_numChannels; }
		float getSampleRate() const { return m_sampleRate; }
		std::string getChannelPrefix() const { return m_channelPrefix; }
		ContinuousChannel::Type getChannelType() const { return m_channelType; }
		float getBitVolts() const { return m_bitVolts; }
		std::string getUnits() const { return m_units; }
		std::vector<std::string> getChannelNameSuffixes() const { return m_channelNameSuffixes; }
		std::string getChannelIdentifierDataType() const { return m_channelIdentifierDataType; }
		std::vector<std::string> getChannelIdentifierSubTypes() const { return m_channelIdentifierSubTypes; }

	private:
		std::string m_name = "name";
		std::string m_description = "description";
		std::string m_streamIdentifier = "identifier";
		int m_numChannels = 0;
		float m_sampleRate = 0;
		std::string m_channelPrefix = "channelPrefix";
		ContinuousChannel::Type m_channelType = ContinuousChannel::Type::INVALID;
		float m_bitVolts = 1.0f;
		std::string m_units = "units";
		std::vector<std::string> m_channelNameSuffixes = { "suffixes" };
		std::string m_channelIdentifierDataType = "datatype";
		std::vector<std::string> m_channelIdentifierSubTypes = { "subtypes" };
	};

	/**

		Streams data from an ONIX device

	*/
	class OnixDevice
	{
	public:

		/** Constructor */
		OnixDevice(std::string name_, std::string hubName, OnixDeviceType type_, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx, bool passthrough = false);

		/** Destructor */
		~OnixDevice() { }

		virtual void addFrame(oni_frame_t*) {};

		virtual void processFrames() {};

		const std::string getName() { return name; }

		bool isEnabled() const { return enabled; }

		void setEnabled(bool newState) { enabled = newState; }

		virtual int configureDevice() { return -1; };

		virtual bool updateSettings() { return false; };

		virtual void startAcquisition() {};

		virtual void stopAcquisition() {};

		/** Given the sourceBuffers from OnixSource, add all streams for the current device to the array */
		virtual void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) {};

		oni_dev_idx_t getDeviceIdx(bool getPassthroughIndex = false);

		/** Creates a stream name using the provided inputs, returning a string following the pattern: name[0]-name[1]-name[2]-etc., with all spaces removed */
		static std::string createStreamName(std::vector<std::string> names);

		Array<StreamInfo> streamInfos;

		const int bufferSizeInSeconds = 10;

		static constexpr int HubAddressBreakoutBoard = 0;
		static constexpr int HubAddressPortA = 256;
		static constexpr int HubAddressPortB = 512;

		std::string getHubName() { return m_hubName; }
		void setHubName(std::string hubName) { m_hubName = hubName; }

		static int getPortOffset(PortName port);
		static std::string getPortName(int offset);
		static std::string getPortName(PortName port);
		static std::string getPortNameFromIndex(oni_dev_idx_t index);
		static PortName getPortFromIndex(oni_dev_idx_t index);
		static int getOffsetFromIndex(oni_dev_idx_t index);
		static std::vector<int> getUniqueOffsetsFromIndices(std::vector<int> indices, bool ignoreBreakoutBoard = true);
		static Array<PortName> getUniquePortsFromIndices(std::vector<int> indices);

		OnixDeviceType getDeviceType() const;

		/** Returns a string for this device that follows the pattern: onix.[hub].[device] */
		std::string getStreamIdentifier();

		static oni_dev_idx_t getHubIndexFromPassthroughIndex(oni_dev_idx_t passthroughIndex);

	protected:

		oni_dev_idx_t getDeviceIndexFromPassthroughIndex(oni_dev_idx_t passthroughIndex) const;

		const oni_dev_idx_t deviceIdx;
		std::shared_ptr<Onix1> deviceContext;

	private:

		std::string name;

		bool enabled = true;
		bool isPassthrough = false;

		std::string m_hubName;

		const OnixDeviceType type;

		enum class PassthroughIndex : uint32_t
		{
			A = 8,
			B = 9
		};

		JUCE_LEAK_DETECTOR(OnixDevice);
	};

	using OnixDeviceVector = std::vector<std::shared_ptr<OnixDevice>>;
}

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

#include "I2CRegisterContext.h"
#include "NeuropixComponents.h"
#include "Onix1.h"

using namespace std::chrono;

enum class PortName
{
	PortA = 1,
	PortB = 2
};

enum class OnixDeviceType {
	BNO,
	POLLEDBNO,
	NEUROPIXELS_1,
	NEUROPIXELSV2E,
	PORT_CONTROL,
	MEMORYMONITOR,
	OUTPUTCLOCK,
	HEARTBEAT,
	HARPSYNCINPUT,
	ANALOGIO,
	DIGITALIO,
};

struct StreamInfo {
public:
	StreamInfo() {}

	StreamInfo(String name, String description, String streamIdentifier, int numChannels, float sampleRate, String channelPrefix, ContinuousChannel::Type channelType,
		float bitVolts, String units, StringArray channelNameSuffixes, String channelIdentifierDataType, StringArray channelIdentifierSubTypes = {})
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
			m_channelNameSuffixes.ensureStorageAllocated(numChannels);

			for (int i = 0; i < m_numChannels; i++)
			{
				m_channelNameSuffixes.add(String(i + 1));
			}
		}

		if (m_channelIdentifierSubTypes.size() > 0 && m_numChannels != m_channelIdentifierSubTypes.size())
		{
			if (m_channelIdentifierSubTypes.size() == 1)
			{
				for (int i = 1; i < m_numChannels; i++)
				{
					m_channelIdentifierSubTypes.add(m_channelIdentifierSubTypes[0]);
				}
			}
			else
			{
				LOGE("Difference between number of channels and channel identifier subtypes. Generating default subtypes instead.");

				m_channelIdentifierSubTypes.clear();
				m_channelIdentifierSubTypes.ensureStorageAllocated(numChannels);

				for (int i = 0; i < m_numChannels; i++)
				{
					m_channelIdentifierSubTypes.add(String(i + 1));
				}
			}
		}
	};

	String getName() const { return m_name; }
	String getDescription() const { return m_description; }
	String getStreamIdentifier() const { return m_streamIdentifier; }
	int getNumChannels() const { return m_numChannels; }
	float getSampleRate() const { return m_sampleRate; }
	String getChannelPrefix() const { return m_channelPrefix; }
	ContinuousChannel::Type getChannelType() const { return m_channelType; }
	float getBitVolts() const { return m_bitVolts; }
	String getUnits() const { return m_units; }
	StringArray getChannelNameSuffixes() const { return m_channelNameSuffixes; }
	String getChannelIdentifierDataType() const { return m_channelIdentifierDataType; }
	StringArray getChannelIdentifierSubTypes() const { return m_channelIdentifierSubTypes; }

private:
	String m_name = "name";
	String m_description = "description";
	String m_streamIdentifier = "identifier";
	int m_numChannels = 0;
	float m_sampleRate = 0;
	String m_channelPrefix = "channelPrefix";
	ContinuousChannel::Type m_channelType = ContinuousChannel::Type::INVALID;
	float m_bitVolts = 1.0f;
	String m_units = "units";
	StringArray m_channelNameSuffixes = { "suffixes" };
	String m_channelIdentifierDataType = "datatype";
	StringArray m_channelIdentifierSubTypes = { "subtypes" };
};

/**

	Streams data from an ONIX device

*/
class OnixDevice
{
public:

	/** Constructor */
	OnixDevice(String name_, String headstageName, OnixDeviceType type_, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

	/** Destructor */
	~OnixDevice() { }

	virtual void addFrame(oni_frame_t*) {};

	virtual void processFrames() {};

	const String getName() { return name; }

	bool isEnabled() const { return enabled; }

	void setEnabled(bool newState) { enabled = newState; }

	virtual int configureDevice() { return -1; };

	virtual bool updateSettings() { return false; };

	virtual void startAcquisition() {};

	virtual void stopAcquisition() {};

	/** Given the sourceBuffers from OnixSource, add all streams for the current device to the array */
	virtual void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) {};

	oni_dev_idx_t getDeviceIdx(bool getPassthroughIndex = false);

	/** Creates a stream name using the provided inputs, returning a String following the pattern: name[0]-name[1]-name[2]-etc., with all spaces removed */
	static String createStreamName(std::vector<String> names)
	{
		String streamName;

		for (int i = 0; i < names.size(); i++)
		{
			streamName += names[i].removeCharacters(" ");

			if (i != names.size() - 1) streamName += "-";
		}

		return streamName;
	}

	Array<StreamInfo> streamInfos;

	const int bufferSizeInSeconds = 10;

	String getHeadstageName() { return m_headstageName; }
	void setHeadstageName(String headstage) { m_headstageName = headstage; }

	OnixDeviceType getDeviceType() const;

	/** Returns a string for this device that follows the pattern: onix.[headstage|breakout].[device] */
	String getStreamIdentifier();

protected:

	oni_dev_idx_t getDeviceIndexFromPassthroughIndex(oni_dev_idx_t hubIndex);

	const oni_dev_idx_t deviceIdx;
	std::shared_ptr<Onix1> deviceContext;

private:

	String name;

	bool enabled = true;
	bool isPassthrough = false;

	String m_headstageName;

	const OnixDeviceType type;

	JUCE_LEAK_DETECTOR(OnixDevice);
};

using OnixDeviceVector = std::vector<std::shared_ptr<OnixDevice>>;

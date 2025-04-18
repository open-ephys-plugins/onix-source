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
	HS64,
	BNO,
	POLLEDBNO,
	NEUROPIXELS_1,
	NEUROPIXELSV2E,
	ADC,
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
	StreamInfo()
	{
		name_ = "name";
		description_ = "description";
		identifier_ = "identifier";
		numChannels_ = 0;
		sampleRate_ = 0;
		channelPrefix_ = "channelPrefix";
		channelType_ = ContinuousChannel::Type::INVALID;
		bitVolts_ = 1.0f;
		units_ = "units";
		suffixes_ = { "suffixes" };
	}

	StreamInfo(String name, String description, String identifier, int numChannels, float sampleRate, String channelPrefix, ContinuousChannel::Type channelType,
		float bitVolts, String units, StringArray suffixes)
	{
		name_ = name;
		description_ = description;
		identifier_ = identifier;
		numChannels_ = numChannels;
		sampleRate_ = sampleRate;
		channelPrefix_ = channelPrefix;
		channelType_ = channelType;
		bitVolts_ = bitVolts;
		units_ = units;
		suffixes_ = suffixes;

		if (numChannels_ != suffixes_.size())
		{
			if (suffixes_.size() != 0)
				LOGE("Difference between number of channels and suffixes. Generating default suffixes instead.");

			suffixes_.clear();
			suffixes_.ensureStorageAllocated(numChannels);
			
			for (int i = 0; i < numChannels_; i += 1)
			{
				suffixes_.add(String(i + 1));
			}
		}
	};

	String getName() const { return name_; }
	String getDescription() const { return description_; }
	String getIdentifier() const { return identifier_; }
	int getNumChannels() const { return numChannels_; }
	float getSampleRate() const { return sampleRate_; }
	String getChannelPrefix() const { return channelPrefix_; }
	ContinuousChannel::Type getChannelType() const { return channelType_; }
	float getBitVolts() const { return bitVolts_; }
	String getUnits() const { return units_; }
	StringArray getSuffixes() const { return suffixes_; }

private:
	String name_;
	String description_;
	String identifier_;
	int numChannels_;
	float sampleRate_;
	String channelPrefix_;
	ContinuousChannel::Type channelType_;
	float bitVolts_;
	String units_;
	StringArray suffixes_;
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

	const OnixDeviceType type;

	Array<StreamInfo> streamInfos;

	const int bufferSizeInSeconds = 10;

	String getHeadstageName() { return m_headstageName; }
	void setHeadstageName(String headstage) { m_headstageName = headstage; }

protected:

	oni_dev_idx_t getDeviceIndexFromPassthroughIndex(oni_dev_idx_t hubIndex);

	const oni_dev_idx_t deviceIdx;
	std::shared_ptr<Onix1> deviceContext;

private:

	String name;

	bool enabled = true;
	bool isPassthrough = false;

	String m_headstageName;

	JUCE_LEAK_DETECTOR(OnixDevice);
};

using OnixDeviceVector = std::vector<std::shared_ptr<OnixDevice>>;

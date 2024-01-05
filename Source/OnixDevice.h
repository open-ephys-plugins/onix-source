/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2020 Allen Institute for Brain Science and Open Ephys

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

#ifndef __OnixDevice_H__
#define __OnixDevice_H__

#include <DataThreadHeaders.h>

#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>

#define PI 3.14159f
#define MAX_SAMPLES_PER_BUFFER 300

using namespace std::chrono;


enum OnixDeviceType {
	HS64,
	BNO,
	NEUROPIXELS_1,
	NEUROPIXELS_2,
	ADC
};

struct StreamInfo {
	String name;
	String description;
	String identifier;
	int numChannels;
	float sampleRate;
	String channelPrefix;
	ContinuousChannel::Type channelType;
	float bitVolts;
};

/** 
	
	Streams data from an ONIX device

*/
class OnixDevice : public Thread
{
public:

	/** Constructor */
	OnixDevice(String name, OnixDeviceType type);

	/** Destructor */
	~OnixDevice() { }

	virtual void addFrame() = 0;

	const String getName() { return name; }

	OnixDeviceType type;

	/** Holds incoming data */
	DataBuffer* deviceBuffer;

	Array<StreamInfo> streams;

private:

	/** Updates buffer during acquisition */
	void run() override;

	std::vector<float>* data;
	int availableSamples;
    int samplesPerBuffer;

	int64 numSamples;
	uint64 eventCode;

	String name;

	// float samples[384 * MAX_SAMPLES_PER_BUFFER];
	// int64 sampleNumbers[MAX_SAMPLES_PER_BUFFER];
    // double timestamps[MAX_SAMPLES_PER_BUFFER];
	// uint64 event_codes[MAX_SAMPLES_PER_BUFFER];

};


#endif
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

#ifndef SIMULATEDSOURCE_H_DEFINED
#define SIMULATEDSOURCE_H_DEFINED

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

/** 
	
	Streams data from an ONIX device

*/
class OnixDevice : public Thread
{
public:

	/** Constructor */
	OnixDevice(String name, int channels, float sampleRate, OnixDeviceType type);

	/** Destructor */
	~OnixDevice() { }

	/** Holds incoming data */
	DataBuffer* buffer;

private:

	/** Updates buffer during acquisition */
	void run() override;

	OnixDeviceType type;

	std::vector<float>* data;
	int availableSamples;
    int samplesPerBuffer;

	int numChannels;
	int packetSize;
	float sampleRate;
	int64 numSamples;
	uint64 eventCode;

	float samples[384 * MAX_SAMPLES_PER_BUFFER];
	int64 sampleNumbers[MAX_SAMPLES_PER_BUFFER];
    double timestamps[MAX_SAMPLES_PER_BUFFER];
	uint64 event_codes[MAX_SAMPLES_PER_BUFFER];

};


#endif
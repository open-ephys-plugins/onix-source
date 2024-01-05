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

#ifndef NEUROPIXELS1_H_DEFINED
#define NEUROPIXELS1_H_DEFINED

#include "../OnixDevice.h"

#include <ctime>
#include <chrono>

enum NeuropixelsRegisters
{
	OP_MODE = 0x00,
	REC_MOD = 0x01,
	CAL_MOD = 0x02
};

/** 
	
	Streams data from an ONIX device

*/
class Neuropixels_1 : public OnixDevice
{
public:

	/** Constructor */
	Neuropixels_1(String name);

	/** Destructor */
	~Neuropixels_1();

	DataBuffer* apBuffer = deviceBuffer;
	DataBuffer* lfpBuffer;

	void addFrame() override;

private:

	float samples[384 * MAX_SAMPLES_PER_BUFFER];
	int64 sampleNumbers[MAX_SAMPLES_PER_BUFFER];
    double timestamps[MAX_SAMPLES_PER_BUFFER];
	uint64 event_codes[MAX_SAMPLES_PER_BUFFER];

};


#endif
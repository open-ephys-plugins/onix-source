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

#ifndef BNO055_H_DEFINED
#define BNO055_H_DEFINED

#include "../OnixDevice.h"

enum Bno055Registers
{
	ENABLE = 0x00
};

class Bno055 : public OnixDevice
{
public:

	/** Constructor */
	Bno055(String name, const oni_dev_idx_t, const oni_ctx);

	/** Destructor */
	~Bno055();

	int enableDevice() override;

	/** Update the settings of the device */
	int updateSettings() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	void addFrame(oni_frame_t*) override;

	DataBuffer* eulerBuffer = deviceBuffer;
	DataBuffer* quaternionBuffer;
	DataBuffer* accelerationBuffer;
	DataBuffer* gravityBuffer;
	DataBuffer* temperatureBuffer;

private:

	static const int numFrames = 1;

	/** Updates buffer during acquisition */
	void run() override;

	Array<oni_frame_t*, CriticalSection, numFrames> frameArray;

	bool shouldAddToBuffer = false;

	float eulerSamples[3 * numFrames];
	float quaternionSamples[4 * numFrames];
	float accelerationSamples[3 * numFrames];
	float gravitySamples[3 * numFrames];
	float temperatureSamples[numFrames];

	double bnoTimestamps[numFrames];
	int64 sampleNumbers[numFrames];
	uint64 eventCodes[numFrames];

	unsigned short currentFrame = 0;
	int sampleNumber = 0;
};

#endif
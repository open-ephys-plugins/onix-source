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

enum class Bno055Registers
{
	ENABLE = 0x00
};

/*
	Configures and streams data from a BNO055 device
*/
class Bno055 : public OnixDevice
{
public:

	/** Constructor */
	Bno055(String name, const oni_dev_idx_t, std::shared_ptr<Onix1> ctx);

	int configureDevice() override;

	/** Update the settings of the device */
	bool updateSettings() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	void addFrame(oni_frame_t*) override;

	void processFrames() override;

	void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;

private:

	DataBuffer* bnoBuffer;

	static const int numFrames = 2;

	Array<oni_frame_t*, CriticalSection, numFrames> frameArray;

	bool shouldAddToBuffer = false;

	static const int numberOfChannels = 3 + 3 + 4 + 3 + 1;
	static constexpr float sampleRate = 100.0f;

	float bnoSamples[numberOfChannels * numFrames];

	double bnoTimestamps[numFrames];
	int64 sampleNumbers[numFrames];
	uint64 eventCodes[numFrames];

	unsigned short currentFrame = 0;
	int sampleNumber = 0;

	JUCE_LEAK_DETECTOR(Bno055);
};

#endif

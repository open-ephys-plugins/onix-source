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

#ifndef MEMORYMONITOR_H_DEFINED
#define MEMORYMONITOR_H_DEFINED

#include "../OnixDevice.h"

enum class MemoryMonitorRegisters : uint32_t
{
	ENABLE = 0,
	CLK_DIV = 1,
	CLK_HZ = 2,
	TOTAL_MEM = 3
};

/*
	Streams data from a MemoryMonitor device on a Breakout Board
*/
class MemoryMonitor : public OnixDevice
{
public:
	MemoryMonitor(String name, const oni_dev_idx_t, const oni_ctx);

	~MemoryMonitor();

	/** Enables the device so that it is ready to stream with default settings */
	int configureDevice() override;

	/** Update the settings of the device */
	int updateSettings() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	/** Given the sourceBuffers from OnixSource, add all streams for the current device to the array */
	void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;

	void addFrame(oni_frame_t*) override;

	void processFrames() override;

	uint32_t getSamplesPerSecond() const { return samplesPerSecond; }

	void setSamplesPerSecond(uint32_t samplesPerSecond_) { samplesPerSecond = samplesPerSecond_; }

	DataBuffer* percentUsedBuffer = deviceBuffer;
	DataBuffer* bytesUsedBuffer;

private:

	static const int numFrames = 2;

	Array<oni_frame_t*, CriticalSection, numFrames> frameArray;

	unsigned short currentFrame = 0;
	int sampleNumber = 0;
	
	/** The frequency at which memory use is recorded in Hz. */
	uint32_t samplesPerSecond = 10;

	/** The total amount of memory, in 32-bit words, on the hardware that is available for data buffering*/
	uint32_t totalMemory;

	bool shouldAddToBuffer = false;

	float percentUsedSamples[numFrames];
	float bytesUsedSamples[numFrames];

	double timestamps[numFrames];
	int64 sampleNumbers[numFrames];
	uint64 eventCodes[numFrames];
};

#endif
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

#include <VisualizerEditorHeaders.h>
#include "../OnixDevice.h"

namespace OnixSourcePlugin
{
	enum class MemoryMonitorRegisters : uint32_t
	{
		ENABLE = 0,
		CLK_DIV = 1,
		CLK_HZ = 2,
		TOTAL_MEM = 3
	};

	/*
		Configures and streams data from a MemoryMonitor device on a Breakout Board
	*/
	class MemoryMonitor : public OnixDevice
	{
	public:
		MemoryMonitor(std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

		int configureDevice() override;
		bool updateSettings() override;
		void startAcquisition() override;
		void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;
		void processFrames() override;

		float getLastPercentUsedValue();

		static OnixDeviceType getDeviceType();

	private:

		DataBuffer* percentUsedBuffer;

		static const int numFrames = 10;

		unsigned short currentFrame = 0;
		int sampleNumber = 0;

		/** The frequency at which memory use is recorded in Hz. */
		const uint32_t samplesPerSecond = 100;

		bool shouldAddToBuffer = false;

		float percentUsedSamples[numFrames];
		float bytesUsedSamples[numFrames];

		double timestamps[numFrames];
		int64_t sampleNumbers[numFrames];
		uint64_t eventCodes[numFrames];

		/** The total amount of memory, in 32-bit words, on the hardware that is available for data buffering*/
		uint32_t totalMemory;

		std::atomic<float> lastPercentUsedValue = 0.0f;

		JUCE_LEAK_DETECTOR(MemoryMonitor);
	};

	/*
		Tracks the MemoryMonitor usage while data acquisition is running
	*/
	class MemoryMonitorUsage : public LevelMonitor
	{
	public:
		MemoryMonitorUsage(GenericProcessor*);

		void timerCallback() override;

		void setMemoryMonitor(std::shared_ptr<MemoryMonitor> memoryMonitor);
		void startAcquisition();
		void stopAcquisition();
		void setPassiveTooltip();
		static std::string getNewTooltip(float memoryUsage);

	private:

		std::shared_ptr<MemoryMonitor> device;

		// NB: Calculate the maximum logarithmic value to convert from linear scale (x: 0-100) to logarithmic scale (y: 0-1)
		//	   using the following equation: y = log_e(x + 1) / log_e(x_max + 1);
		const float maxLogarithmicValue = std::log(101);

		const int TimerFrequencyHz = 10;

		JUCE_LEAK_DETECTOR(MemoryMonitorUsage);
	};
}

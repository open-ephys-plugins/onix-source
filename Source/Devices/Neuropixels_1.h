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

namespace Onix
{
	enum NeuropixelsRegisters
	{
		OP_MODE = 0x00,
		REC_MOD = 0x01,
		CAL_MOD = 0x02,
		STATUS = 0X08
	};

	enum OpMode
	{
		TEST = 1 << 3, // Enable Test mode
		DIG_TEST = 1 << 4, // Enable Digital Test mode
		CALIBRATE = 1 << 5, // Enable calibration mode
		RECORD = 1 << 6, // Enable recording mode
		POWER_DOWN = 1 << 7, // Enable power down mode
	};

	enum RecMod
	{
		DIG_AND_CH_RESET = 0,
		RESET_ALL = 1 << 5, // 1 = Set analog SR chains to default values
		DIG_NRESET = 1 << 6, // 0 = Reset the MUX, ADC, and PSB counter, 1 = Disable reset
		CH_NRESET = 1 << 7, // 0 = Reset channel pseudo-registers, 1 = Disable reset

		// Useful combinations
		SR_RESET = RESET_ALL | CH_NRESET | DIG_NRESET,
		DIG_RESET = CH_NRESET, // Yes, this is actually correct
		CH_RESET = DIG_NRESET, // Yes, this is actually correct
		ACTIVE = DIG_NRESET | CH_NRESET
	};

	// ADC number to frame index mapping
	static const int adcToFrameIndex[] = {
		0, 7 , 14, 21, 28,
		1, 8 , 15, 22, 29,
		2, 9 , 16, 23, 30,
		3, 10, 17, 24, 31,
		4, 11, 18, 25, 32,
		5, 12, 19, 26, 33,
		6, 13
	};

	// ADC to muxed channel mapping
	static const int adcToChannel[] = {
		0, 1, 24, 25, 48, 49, 72, 73, 96, 97,
		120, 121, 144, 145, 168, 169, 192, 193,
		216, 217, 240, 241, 264, 265, 288, 289,
		312, 313, 336, 337, 360, 361
	};

	const int superFramesPerUltraFrame = 12;
	const int framesPerSuperFrame = 13;
	const int framesPerUltraFrame = superFramesPerUltraFrame * framesPerSuperFrame;
	const int numUltraFrames = 12;
	const int dataOffset = 1;

	/**

		Streams data from an ONIX device

	*/
	class Neuropixels_1 : public OnixDevice
	{
	public:

		/** Constructor */
		Neuropixels_1(String name, float portVoltage, const oni_dev_idx_t, const oni_ctx);

		/** Destructor */
		~Neuropixels_1();

		int enableDevice() override;

		/** Starts probe data streaming */
		void startAcquisition() override;

		/** Stops probe data streaming*/
		void stopAcquisition() override;

		void addFrame(oni_frame_t*) override;

		DataBuffer* apBuffer = deviceBuffer;
		DataBuffer* lfpBuffer;

	private:

		/** Updates buffer during acquisition */
		void run() override;

		float lfpSamples[384 * numUltraFrames];
		float apSamples[384 * numUltraFrames * superFramesPerUltraFrame];

		int64 apSampleNumbers[numUltraFrames * superFramesPerUltraFrame];
		double apTimestamps[numUltraFrames * superFramesPerUltraFrame];
		uint64 apEventCodes[numUltraFrames * superFramesPerUltraFrame];

		int64 lfpSampleNumbers[numUltraFrames];
		double lfpTimestamps[numUltraFrames];
		uint64 lfpEventCodes[numUltraFrames];

		bool shouldAddToBuffer = false;
		int superFrameCount = 0;
		int ultraFrameCount = 0;

		int apSampleNumber = 0;
		int lfpSampleNumber = 0;

		float portVoltage_ = 5.0;

		const float minVoltage = 4.5;
		const float maxVoltage = 6.5;
	};
}

#endif

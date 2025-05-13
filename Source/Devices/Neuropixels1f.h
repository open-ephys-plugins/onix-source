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

#include "../OnixDevice.h"
#include "../NeuropixelsComponents.h"
#include "../I2CRegisterContext.h"

namespace OnixSourcePlugin
{
	/**

		Configures and streams data from a Neuropixels 1.0f device

	*/
	class Neuropixels1f : public INeuropixel<NeuropixelsV1Values::numberOfChannels, NeuropixelsV1Values::numberOfElectrodes>,
		public OnixDevice,
		public I2CRegisterContext
	{
	public:
		/** Constructor */
		Neuropixels1f(std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1>);

		/** Configures the device so that it is ready to stream with default settings */
		int configureDevice() override;

		/** Update the settings of the device by writing to hardware */
		bool updateSettings() override;

		/** Starts probe data streaming */
		void startAcquisition() override;

		/** Stops probe data streaming*/
		void stopAcquisition() override;

		/** Given the sourceBuffers from OnixSource, add all streams for the current device to the array */
		void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;

		void addFrame(oni_frame_t*) override;

		void processFrames() override;

		NeuropixelsV1Gain getGainEnum(int index);

		int getGainValue(NeuropixelsV1Gain);

		NeuropixelsV1Reference getReference(int index);

		String getAdcCalibrationFilePath();
		void setAdcCalibrationFilePath(String filepath);
		String getGainCalibrationFilePath();
		void setGainCalibrationFilePath(String filepath);

		void writeShiftRegisters(ShankBitset shankBits, ConfigBitsArray configBits, Array<NeuropixelsV1Adc> adcs, double lfpGainCorrection, double apGainCorrection);

		// INeuropixels methods
		void setSettings(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings_, int index = 0) override;

		void defineMetadata(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings) override;

		uint64_t getProbeSerialNumber(int index = 0) override;

		/** Select a preset electrode configuration */
		std::vector<int> selectElectrodeConfiguration(String config) override;

		static OnixDeviceType getDeviceType();

	private:

		String adcCalibrationFilePath;
		String gainCalibrationFilePath;

		// ADC number to frame index mapping
		static constexpr int adcToFrameIndex[] = {
			0, 7 , 14, 21, 28,
			1, 8 , 15, 22, 29,
			2, 9 , 16, 23, 30,
			3, 10, 17, 24, 31,
			4, 11, 18, 25, 32,
			5, 12, 19, 26, 33,
			6, 13
		};

		// ADC to muxed channel mapping
		static constexpr int adcToChannel[] = {
			0, 1, 24, 25, 48, 49, 72, 73, 96, 97,
			120, 121, 144, 145, 168, 169, 192, 193,
			216, 217, 240, 241, 264, 265, 288, 289,
			312, 313, 336, 337, 360, 361
		};

		DataBuffer* apBuffer;
		DataBuffer* lfpBuffer;

		const uint32_t ENABLE = 0x8000;

		static constexpr int superFramesPerUltraFrame = 12;
		static constexpr int framesPerSuperFrame = 13;
		static constexpr int framesPerUltraFrame = superFramesPerUltraFrame * framesPerSuperFrame;
		static constexpr int numUltraFrames = 12;
		static constexpr int dataOffset = 1;

		static constexpr int secondsToSettle = 5;
		static constexpr int samplesToAverage = 100;

		static constexpr uint32_t numLfpSamples = 384 * numUltraFrames;
		static constexpr uint32_t numApSamples = 384 * numUltraFrames * superFramesPerUltraFrame;

		static constexpr float lfpSampleRate = 2500.0f;
		static constexpr float apSampleRate = 30000.0f;

		bool lfpOffsetCalculated = false;
		bool apOffsetCalculated = false;

		std::array<float, numberOfChannels> apOffsets;
		std::array<float, numberOfChannels> lfpOffsets;

		std::vector<std::vector<float>> apOffsetValues;
		std::vector<std::vector<float>> lfpOffsetValues;

		void updateLfpOffsets(std::array<float, numLfpSamples>&, int64);
		void updateApOffsets(std::array<float, numApSamples>&, int64);

		static constexpr int ProbeI2CAddress = 0x70;

		Array<oni_frame_t*, CriticalSection, numUltraFrames> frameArray;

		uint64_t probeNumber = 0;

		std::array<float, numLfpSamples> lfpSamples;
		std::array<float, numApSamples> apSamples;

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

		int apGain = 1000;
		int lfpGain = 50;

		JUCE_LEAK_DETECTOR(Neuropixels1f);
	};

	/*

		A thread that updates probe settings in the background and shows a progress bar

	*/
	class NeuropixelsV1fBackgroundUpdater : public ThreadWithProgressWindow
	{
	public:
		NeuropixelsV1fBackgroundUpdater(Neuropixels1f* d);

		void run() override;

		bool updateSettings();

	private:

		Neuropixels1f* device;

		std::atomic<bool> result = false;

		JUCE_LEAK_DETECTOR(NeuropixelsV1fBackgroundUpdater);
	};
}

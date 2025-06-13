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
	class Neuropixels1 : public INeuropixel<NeuropixelsV1Values::numberOfChannels, NeuropixelsV1Values::numberOfElectrodes>,
		public OnixDevice,
		public I2CRegisterContext
	{
	public:

		Neuropixels1(std::string, std::string, OnixDeviceType, const oni_dev_idx_t, std::shared_ptr<Onix1>);

		NeuropixelsV1Gain getGainEnum(int index);

		int getGainValue(NeuropixelsV1Gain);

		NeuropixelsV1Reference getReference(int index);

		std::string getAdcCalibrationFilePath();
		void setAdcCalibrationFilePath(std::string filepath);
		std::string getGainCalibrationFilePath();
		void setGainCalibrationFilePath(std::string filepath);

		// INeuropixels methods
		void defineMetadata(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings) override;

		/** Select a preset electrode configuration, based on the index of the given enum */
		std::vector<int> selectElectrodeConfiguration(int electrodeConfigurationIndex) override;

		uint64_t getProbeSerialNumber(int index = 0) override;

		void setSettings(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings_, int index = 0) override;

		bool parseGainCalibrationFile();
		bool parseAdcCalibrationFile();

	protected:

		DataBuffer* apBuffer;
		DataBuffer* lfpBuffer;

		std::string adcCalibrationFilePath;
		std::string gainCalibrationFilePath;

		double apGainCorrection = 0;
		double lfpGainCorrection = 0;

		uint64_t probeNumber = 0;

		const uint32_t ENABLE = 0x8000;

		static constexpr int ProbeI2CAddress = 0x70;

		static constexpr int superFramesPerUltraFrame = 12;
		static constexpr int framesPerSuperFrame = 13;
		static constexpr int framesPerUltraFrame = superFramesPerUltraFrame * framesPerSuperFrame;
		static constexpr int numUltraFrames = 12;
		static constexpr int dataOffset = 4 + 1; // NB: 4 bytes [hubClock] + 1 byte [probeIndex]

		static constexpr uint16_t NumberOfAdcBins = 1024;
		static constexpr float DataMidpoint = NumberOfAdcBins / 2;

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

		Array<oni_frame_t*, CriticalSection, numUltraFrames> frameArray;

		std::array<float, numLfpSamples> lfpSamples;
		std::array<float, numApSamples> apSamples;

		int64 apSampleNumbers[numUltraFrames * superFramesPerUltraFrame];
		double apTimestamps[numUltraFrames * superFramesPerUltraFrame];
		uint64 apEventCodes[numUltraFrames * superFramesPerUltraFrame];

		int64 lfpSampleNumbers[numUltraFrames];
		double lfpTimestamps[numUltraFrames];
		uint64 lfpEventCodes[numUltraFrames];

		int superFrameCount = 0;
		int ultraFrameCount = 0;

		int apSampleNumber = 0;
		int lfpSampleNumber = 0;

		int apGain = 1000;
		int lfpGain = 50;

		std::vector<NeuropixelsV1Adc> adcValues;

		void updateLfpOffsets(std::array<float, numLfpSamples>&, int64);
		void updateApOffsets(std::array<float, numApSamples>&, int64);

		enum class ElectrodeConfiguration : int32_t
		{
			BankA = 0,
			BankB = 1,
			BankC = 2,
			SingleColumn = 3,
			Tetrodes = 4
		};

		std::map<ElectrodeConfiguration, std::string> electrodeConfiguration = {
			{ElectrodeConfiguration::BankA, "Bank A"},
			{ElectrodeConfiguration::BankB, "Bank B"},
			{ElectrodeConfiguration::BankC, "Bank C"},
			{ElectrodeConfiguration::SingleColumn, "Single Column"},
			{ElectrodeConfiguration::Tetrodes, "Tetrodes"}
		};

		JUCE_LEAK_DETECTOR(Neuropixels1);
	};

	/*
		A thread that updates Neuropixels 1.0 probe settings in the background and shows a progress bar
	*/
	class NeuropixelsV1BackgroundUpdater : public ThreadWithProgressWindow
	{
	public:
		NeuropixelsV1BackgroundUpdater(Neuropixels1* d);

		bool updateSettings();

	protected:

		Neuropixels1* device;

		std::atomic<bool> result = false;

	private:

		JUCE_LEAK_DETECTOR(NeuropixelsV1BackgroundUpdater);
	};
}

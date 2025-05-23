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
	enum class NeuropixelsRegisters : uint32_t
	{
		OP_MODE = 0x00,
		REC_MOD = 0x01,
		CAL_MOD = 0x02,
		STATUS = 0X08,
		SYNC = 0X09
	};

	enum class CalMode : uint32_t
	{
		CAL_OFF = 0,
		OSC_ACTIVE = 1 << 4, // 0 = external osc inactive, 1 = activate the external calibration oscillator
		ADC_CAL = 1 << 5, // Enable ADC calibration
		CH_CAL = 1 << 6, // Enable channel gain calibration
		PIX_CAL = 1 << 7, // Enable pixel + channel gain calibration

		// Useful combinations
		OSC_ACTIVE_AND_ADC_CAL = OSC_ACTIVE | ADC_CAL,
		OSC_ACTIVE_AND_CH_CAL = OSC_ACTIVE | CH_CAL,
		OSC_ACTIVE_AND_PIX_CAL = OSC_ACTIVE | PIX_CAL,
	};

	enum class OpMode : uint32_t
	{
		TEST = 1 << 3, // Enable Test mode
		DIG_TEST = 1 << 4, // Enable Digital Test mode
		CALIBRATE = 1 << 5, // Enable calibration mode
		RECORD = 1 << 6, // Enable recording mode
		POWER_DOWN = 1 << 7, // Enable power down mode
	};

	enum class RecMod : uint32_t
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

	enum class NeuropixelsV1fReference : unsigned char
	{
		External = 0b001,
		Tip = 0b010
	};

	enum class NeuropixelsGain : unsigned char
	{
		Gain50 = 0b000,
		Gain125 = 0b001,
		Gain250 = 0b010,
		Gain500 = 0b011,
		Gain1000 = 0b100,
		Gain1500 = 0b101,
		Gain2000 = 0b110,
		Gain3000 = 0b111
	};

	enum class ShiftRegisters : uint32_t
	{
		SR_CHAIN1 = 0X0E,
		SR_CHAIN3 = 0X0C,
		SR_CHAIN2 = 0X0D,
		SR_LENGTH2 = 0X0F,
		SR_LENGTH1 = 0X10,
		SOFT_RESET = 0X11
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

	struct NeuropixelsV1fAdc
	{
	public:
		const int compP;
		const int compN;
		const int slope;
		const int coarse;
		const int fine;
		const int cfix;
		const int offset;
		const int threshold;

		NeuropixelsV1fAdc(int compP_ = 16, int compN_ = 16, int slope_ = 0, int coarse_ = 0, int fine_ = 0, int cfix_ = 0, int offset_ = 0, int threshold_ = 512)
			: compP(compP_), compN(compN_), slope(slope_), coarse(coarse_), fine(fine_), cfix(cfix_), offset(offset_), threshold(threshold_)
		{
		}
	};

	/**

		Configures and streams data from a Neuropixels 1.0f device

	*/
	class Neuropixels1f : public INeuropixel<NeuropixelsV1fValues::numberOfChannels, NeuropixelsV1fValues::numberOfElectrodes>,
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

		NeuropixelsGain getGainEnum(int index);

		int getGainValue(NeuropixelsGain);

		NeuropixelsV1fReference getReference(int index);

		static const int shankConfigurationBitCount = 968;
		static const int BaseConfigurationBitCount = 2448;

		using ShankBitset = std::bitset<shankConfigurationBitCount>;
		using CongigBitsArray = std::array<std::bitset<BaseConfigurationBitCount>, 2>;

		String adcCalibrationFilePath;
		String gainCalibrationFilePath;

		bool getCorrectOffset() const { return correctOffset; }

		void setCorrectOffset(bool value) { correctOffset = value; }

		ShankBitset static makeShankBits(NeuropixelsV1fReference reference, std::array<int, numberOfChannels> channelMap);

		CongigBitsArray static makeConfigBits(NeuropixelsV1fReference reference, NeuropixelsGain spikeAmplifierGain, NeuropixelsGain lfpAmplifierGain, bool spikeFilterEnabled, Array<NeuropixelsV1fAdc> adcs);

		void writeShiftRegisters(ShankBitset shankBits, CongigBitsArray configBits, Array<NeuropixelsV1fAdc> adcs, double lfpGainCorrection, double apGainCorrection);

		// INeuropixels methods
		void setSettings(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings_, int index = 0) override;

		void defineMetadata(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings) override;

		uint64_t getProbeSerialNumber(int index = 0) override { return probeNumber; }

		/** Select a preset electrode configuration */
		std::vector<int> selectElectrodeConfiguration(String config) override;

		static OnixDeviceType getDeviceType();

	private:

		DataBuffer* apBuffer;
		DataBuffer* lfpBuffer;

		const uint32_t ENABLE = 0x8000;

		static const int superFramesPerUltraFrame = 12;
		static const int framesPerSuperFrame = 13;
		static const int framesPerUltraFrame = superFramesPerUltraFrame * framesPerSuperFrame;
		static const int numUltraFrames = 12;
		static const int dataOffset = 1;

		static const int secondsToSettle = 5;
		static const int samplesToAverage = 100;

		static const uint32_t numLfpSamples = 384 * numUltraFrames;
		static const uint32_t numApSamples = 384 * numUltraFrames * superFramesPerUltraFrame;

		static constexpr float lfpSampleRate = 2500.0f;
		static constexpr float apSampleRate = 30000.0f;

		bool lfpOffsetCalculated = false;
		bool apOffsetCalculated = false;

		bool correctOffset = true;

		std::array<float, numberOfChannels> apOffsets;
		std::array<float, numberOfChannels> lfpOffsets;

		std::vector<std::vector<float>> apOffsetValues;
		std::vector<std::vector<float>> lfpOffsetValues;

		void updateLfpOffsets(std::array<float, numLfpSamples>&, int64);
		void updateApOffsets(std::array<float, numApSamples>&, int64);

		static const int ProbeI2CAddress = 0x70;

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
	class BackgroundUpdaterWithProgressWindow : public ThreadWithProgressWindow
	{
	public:
		BackgroundUpdaterWithProgressWindow(Neuropixels1f* d);

		void run() override;

		bool updateSettings();

	private:

		Neuropixels1f* device;

		std::atomic<bool> result = false;

		JUCE_LEAK_DETECTOR(BackgroundUpdaterWithProgressWindow);
	};
}

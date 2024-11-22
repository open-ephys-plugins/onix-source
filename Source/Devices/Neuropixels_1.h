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
#include "../NeuropixComponents.h"

#include <ctime>
#include <chrono>
#include <bitset>
#include <vector>
#include <string>
#include <map>

enum NeuropixelsRegisters
{
	OP_MODE = 0x00,
	REC_MOD = 0x01,
	CAL_MOD = 0x02,
	STATUS = 0X08,
	SYNC = 0X09
};

enum CalMode
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

enum NeuropixelsReference
{
	External = 0b001,
	Tip = 0b010
};

enum NeuropixelsGain
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

enum ShiftRegisters
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

struct NeuropixelsV1Adc
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

	NeuropixelsV1Adc(int compP_ = 16, int compN_ = 16, int slope_ = 0, int coarse_ = 0, int fine_ = 0, int cfix_ = 0, int offset_ = 0, int threshold_ = 512)
		: compP(compP_), compN(compN_), slope(slope_), coarse(coarse_), fine(fine_), cfix(cfix_), offset(offset_), threshold(threshold_)
	{
	}
};

const int superFramesPerUltraFrame = 12;
const int framesPerSuperFrame = 13;
const int framesPerUltraFrame = superFramesPerUltraFrame * framesPerSuperFrame;
const int numUltraFrames = 12;
const int dataOffset = 1;

const int shankConfigurationBitCount = 968;
const int BaseConfigurationBitCount = 2448;

const int numberOfChannels = 384;

const float lfpSampleRate = 2500.0f;
const float apSampleRate = 30000.0f;

const int ProbeI2CAddress = 0x70;

/**

	Streams data from an ONIX device

*/
class Neuropixels_1 : public OnixDevice,
	public I2CRegisterContext
{
public:

	/** Constructor */
	Neuropixels_1(String name, float portVoltage, String adcFile, String gainFile, const oni_dev_idx_t, const oni_ctx);

	/** Destructor */
	~Neuropixels_1();

	int enableDevice() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	void addFrame(oni_frame_t*) override;

	int64 getProbeNumber() const { return probeNumber; }

	void updateSettings(ProbeSettings p) { settings = p; }

	/** Select a preset electrode configuration */
	Array<int> selectElectrodeConfiguration(String config);

	DataBuffer* apBuffer = deviceBuffer;
	DataBuffer* lfpBuffer;

private:

	/** Updates buffer during acquisition */
	void run() override;

	std::bitset<shankConfigurationBitCount> static makeShankBits(NeuropixelsReference reference, Array<int, DummyCriticalSection, numberOfChannels> channelMap);

	std::vector<std::bitset<BaseConfigurationBitCount>> static makeConfigBits(NeuropixelsReference reference, NeuropixelsGain spikeAmplifierGain, NeuropixelsGain lfpAmplifierGain, bool spikeFilterEnabled, Array<NeuropixelsV1Adc> adcs);

	template<int N> std::vector<unsigned char> static toBitReversedBytes(std::bitset<N> shankBits);

	void writeShiftRegisters(std::bitset<shankConfigurationBitCount> shankBits, std::vector<std::bitset<BaseConfigurationBitCount>> configBits, Array<NeuropixelsV1Adc> adcs, double lfpGainCorrection, double apGainCorrection);

	void defineMetadata(ProbeSettings& settings);

	Array<oni_frame_t*, CriticalSection, numUltraFrames> frameArray;

	const std::map<NeuropixelsGain, int> gainToIndex{
		{NeuropixelsGain::Gain50, 0},
		{NeuropixelsGain::Gain125, 1},
		{NeuropixelsGain::Gain250, 2},
		{NeuropixelsGain::Gain500, 3},
		{NeuropixelsGain::Gain1000, 4},
		{NeuropixelsGain::Gain1500, 5},
		{NeuropixelsGain::Gain2000, 6},
		{NeuropixelsGain::Gain3000, 7},
	};

	String adcCalibrationFile;
	String gainCalibrationFile;

	int64 probeNumber = 0;

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

#endif

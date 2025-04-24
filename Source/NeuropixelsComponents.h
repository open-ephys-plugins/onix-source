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

#include <DataThreadHeaders.h>
#include <stdio.h>
#include <string.h>
#include <bitset>

#include "UI/ActivityView.h"

namespace OnixSourcePlugin
{
	enum class VisualizationMode
	{
		ENABLE_VIEW,
		AP_GAIN_VIEW,
		LFP_GAIN_VIEW,
		REFERENCE_VIEW,
		ACTIVITY_VIEW
	};

	enum class ProbeType
	{
		NONE = 1,
		NPX_V1E,
		NPX_V2E,
		NPX_V2E_BETA,
	};

	enum class Bank
	{
		NONE = -1,
		A = 0,
		B = 1,
		C = 2,
		D = 3,
		E = 4,
		F = 5,
		G = 6,
		H = 7,
		I = 8,
		J = 9,
		K = 10,
		L = 11,
		M = 12,
	};

	enum class ElectrodeStatus
	{
		CONNECTED,
		DISCONNECTED
	};

	enum class ElectrodeType
	{
		ELECTRODE,
		REFERENCE
	};

	struct ElectrodeMetadata
	{
		int global_index;
		int shank_local_index;
		int shank;
		int column_index;
		int channel;
		int row_index;
		float xpos; // position on shank, in microns
		float ypos; // position on shank, in microns
		float site_width; // in microns
		Bank bank;
		ElectrodeStatus status;
		ElectrodeType type;
		bool isSelected;
		Colour colour;
	};

	struct ProbeMetadata
	{
		int shank_count;
		int electrodes_per_shank;
		int num_adcs;
		int adc_bits;
		std::vector<std::array<float, 2>> shankOutline;
		std::vector<std::array<float, 2>> probeContour;
		int columns_per_shank;
		int rows_per_shank;
		String name;
		bool switchable;
	};

	enum class NeuropixelsV1Registers : uint32_t
	{
		OP_MODE = 0x00,
		REC_MOD = 0x01,
		CAL_MOD = 0x02,
		TEST_CONFIG1 = 0x03,
		TEST_CONFIG2 = 0x04,
		TEST_CONFIG3 = 0x05,
		TEST_CONFIG4 = 0x06,
		TEST_CONFIG5 = 0x07,
		STATUS = 0X08,
		SYNC = 0X09
	};

	enum class NeuropixelsV1ShiftRegisters : uint32_t
	{
		SR_CHAIN1 = 0X0E,
		SR_CHAIN3 = 0X0C,
		SR_CHAIN2 = 0X0D,
		SR_LENGTH2 = 0X0F,
		SR_LENGTH1 = 0X10,
		SOFT_RESET = 0X11
	};

	enum class NeuropixelsV1CalibrationRegisterValues : uint32_t
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

	enum class NeuropixelsV1OperationRegisterValues : uint32_t
	{
		TEST = 1 << 3, // Enable Test mode
		DIG_TEST = 1 << 4, // Enable Digital Test mode
		CALIBRATE = 1 << 5, // Enable calibration mode
		RECORD = 1 << 6, // Enable recording mode
		POWER_DOWN = 1 << 7, // Enable power down mode

		// Useful combinations
		RECORD_AND_DIG_TEST = RECORD | DIG_TEST,
		RECORD_AND_CALIBRATE = RECORD | CALIBRATE,
	};

	enum class NeuropixelsV1RecordRegisterValues : uint32_t
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

	enum class NeuropixelsV1Reference : unsigned char
	{
		External = 0b001,
		Tip = 0b010
	};

	enum class NeuropixelsV1Gain : unsigned char
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

	struct NeuropixelsV1Values
	{
		static const int numberOfChannels = 384;
		static const int numberOfElectrodes = 960;
		static const int numberOfShanks = 1;
		static const int numberOfSettings = 1;
	};

	struct NeuropixelsV2eValues
	{
		static const int numberOfChannels = 384;
		static const int electrodesPerShank = 1280;
		static const int numberOfShanks = 4;
		static const int numberOfElectrodes = numberOfShanks * electrodesPerShank;
		static const int numberOfSettings = 2;
	};

	template<int numChannels, int numElectrodes>
	struct ProbeSettings
	{
		void updateProbeSettings(ProbeSettings* newSettings)
		{
			availableElectrodeConfigurations = newSettings->availableElectrodeConfigurations;
			availableApGains = newSettings->availableApGains;
			availableLfpGains = newSettings->availableLfpGains;
			availableReferences = newSettings->availableReferences;
			availableBanks = newSettings->availableBanks;

			electrodeConfigurationIndex = newSettings->electrodeConfigurationIndex;
			apGainIndex = newSettings->apGainIndex;
			lfpGainIndex = newSettings->lfpGainIndex;
			referenceIndex = newSettings->referenceIndex;
			apFilterState = newSettings->apFilterState;

			selectedBank = newSettings->selectedBank;
			selectedShank = newSettings->selectedShank;
			selectedElectrode = newSettings->selectedElectrode;
			electrodeMetadata = newSettings->electrodeMetadata;

			probeType = newSettings->probeType;

			probeMetadata = newSettings->probeMetadata;
		};

		void clearElectrodeSelection()
		{
			selectedBank.fill(Bank::A);
			selectedShank.fill(0);
			selectedElectrode.fill(-1);
		}

		void selectElectrodes(std::vector<int> electrodes)
		{
			for (int i = 0; i < electrodes.size(); i++)
			{
				selectElectrode(electrodes[i]);
			}
		}

		void selectElectrode(int electrode)
		{
			Bank bank = electrodeMetadata[electrode].bank;
			int channel = electrodeMetadata[electrode].channel;
			int shank = electrodeMetadata[electrode].shank;
			int global_index = electrodeMetadata[electrode].global_index;

			for (int j = 0; j < electrodeMetadata.size(); j++)
			{
				if (electrodeMetadata[j].channel == channel)
				{
					if (electrodeMetadata[j].bank == bank && electrodeMetadata[j].shank == shank)
					{
						electrodeMetadata[j].status = ElectrodeStatus::CONNECTED;
					}

					else
					{
						electrodeMetadata[j].status = ElectrodeStatus::DISCONNECTED;
					}
				}
			}

			selectedBank[channel] = bank;
			selectedShank[channel] = shank;
			selectedElectrode[channel] = global_index;
		}

		Array<String> availableElectrodeConfigurations;
		Array<float> availableApGains; // Available AP gain values for each channel (if any)
		Array<float> availableLfpGains; // Available LFP gain values for each channel (if any)
		Array<String> availableReferences; // reference types
		Array<Bank> availableBanks; // bank inds

		int electrodeConfigurationIndex;
		int apGainIndex;
		int lfpGainIndex;
		int referenceIndex;
		bool apFilterState;

		std::array<Bank, numChannels> selectedBank;
		std::array<int, numChannels> selectedShank;
		std::array<int, numChannels> selectedElectrode;
		std::array<ElectrodeMetadata, numElectrodes> electrodeMetadata;

		ProbeType probeType;

		ProbeMetadata probeMetadata;

		bool isValid = false;
	};

	template<int N>
	std::vector<unsigned char> toBitReversedBytes(std::bitset<N> bits)
	{
		std::vector<unsigned char> bytes((bits.size() - 1) / 8 + 1);

		for (int i = 0; i < bytes.size(); i++)
		{
			for (int j = 0; j < 8; j++)
			{
				bytes[i] |= bits[i * 8 + j] << (8 - j - 1);
			}

			// NB: Reverse bytes (http://graphics.stanford.edu/~seander/bithacks.html)
			bytes[i] = (unsigned char)((bytes[i] * 0x0202020202ul & 0x010884422010ul) % 1023);
		}

		return bytes;
	}

	template<int ch, int e>
	class INeuropixel
	{
	public:
		INeuropixel(int numSettings, int numShanks) : numberOfShanks(numShanks)
		{
			if (numSettings > 2) return;

			for (int i = 0; i < numSettings; i++)
			{
				settings.emplace_back(std::make_unique<ProbeSettings<ch, e>>());
			}
		}

		static const int numberOfChannels = ch;
		static const int numberOfElectrodes = e;
		const int numberOfShanks;

		std::vector<std::unique_ptr<ProbeSettings<numberOfChannels, numberOfElectrodes>>> settings;

		virtual void setSettings(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings_, int index) { return; }

		virtual void defineMetadata(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings) { return; }

		virtual uint64_t getProbeSerialNumber(int index) { return 0; }

		virtual std::vector<int> selectElectrodeConfiguration(String config) { return {}; }
	};

	static constexpr int shankConfigurationBitCount = 968;
	static constexpr int BaseConfigurationBitCount = 2448;

	using ShankBitset = std::bitset<shankConfigurationBitCount>;
	using ConfigBitsArray = std::array<std::bitset<BaseConfigurationBitCount>, 2>;

	static class NeuropixelsV1
	{
	public:

		ShankBitset static makeShankBits(NeuropixelsV1Reference reference, std::array<int, NeuropixelsV1Values::numberOfChannels> channelMap)
		{
			constexpr int shankBitExt1 = 965;
			constexpr int shankBitExt2 = 2;
			constexpr int shankBitTip1 = 484;
			constexpr int shankBitTip2 = 483;
			constexpr int internalReferenceChannel = 191;

			std::bitset<shankConfigurationBitCount> shankBits;

			for (auto e : channelMap)
			{
				if (e == internalReferenceChannel) continue;

				int bitIndex = e % 2 == 0
					? 485 + (e / 2)
					: 482 - (e / 2);

				shankBits[bitIndex] = true;
			}

			switch (reference)
			{
			case NeuropixelsV1Reference::External:
				shankBits[shankBitExt1] = true;
				shankBits[shankBitExt2] = true;
				break;
			case NeuropixelsV1Reference::Tip:
				shankBits[shankBitTip1] = true;
				shankBits[shankBitTip2] = true;
				break;
			default:
				break;
			}

			if (channelMap.size() != NeuropixelsV1Values::numberOfChannels)
			{
				LOGE("Invalid number of channels connected for Neuropixels 1.0, configuration might be invalid.");
			}

			return shankBits;
		}

		ConfigBitsArray static makeConfigBits(NeuropixelsV1Reference reference, NeuropixelsV1Gain spikeAmplifierGain, NeuropixelsV1Gain lfpAmplifierGain, bool spikeFilterEnabled, Array<NeuropixelsV1Adc> adcs)
		{
			const int BaseConfigurationConfigOffset = 576;

			ConfigBitsArray baseConfigs;

			for (int i = 0; i < NeuropixelsV1Values::numberOfChannels; i++)
			{
				size_t configIdx = i % 2;

				size_t refIdx = configIdx == 0 ?
					(382 - i) / 2 * 3 :
					(383 - i) / 2 * 3;

				baseConfigs[configIdx][refIdx + 0] = ((unsigned char)reference >> 0 & 0x1) == 1;
				baseConfigs[configIdx][refIdx + 1] = ((unsigned char)reference >> 1 & 0x1) == 1;
				baseConfigs[configIdx][refIdx + 2] = ((unsigned char)reference >> 2 & 0x1) == 1;

				size_t chanOptsIdx = BaseConfigurationConfigOffset + ((i - configIdx) * 4);

				baseConfigs[configIdx][chanOptsIdx + 0] = ((unsigned char)spikeAmplifierGain >> 0 & 0x1) == 1;
				baseConfigs[configIdx][chanOptsIdx + 1] = ((unsigned char)spikeAmplifierGain >> 1 & 0x1) == 1;
				baseConfigs[configIdx][chanOptsIdx + 2] = ((unsigned char)spikeAmplifierGain >> 2 & 0x1) == 1;

				baseConfigs[configIdx][chanOptsIdx + 3] = ((unsigned char)lfpAmplifierGain >> 0 & 0x1) == 1;
				baseConfigs[configIdx][chanOptsIdx + 4] = ((unsigned char)lfpAmplifierGain >> 1 & 0x1) == 1;
				baseConfigs[configIdx][chanOptsIdx + 5] = ((unsigned char)lfpAmplifierGain >> 2 & 0x1) == 1;

				baseConfigs[configIdx][chanOptsIdx + 6] = false;
				baseConfigs[configIdx][chanOptsIdx + 7] = !spikeFilterEnabled;
			}

			int k = 0;

			for (const auto& adc : adcs)
			{
				auto configIdx = k % 2;
				int d = k++ / 2;

				size_t compOffset = 2406 - 42 * (d / 2) + (d % 2) * 10;
				size_t slopeOffset = compOffset + 20 + (d % 2);

				auto compP = std::bitset<8>{ (unsigned char)(adc.compP) };
				auto compN = std::bitset<8>{ (unsigned char)(adc.compN) };
				auto cfix = std::bitset<8>{ (unsigned char)(adc.cfix) };
				auto slope = std::bitset<8>{ (unsigned char)(adc.slope) };
				auto coarse = std::bitset<8>{ (unsigned char)(adc.coarse) };
				auto fine = std::bitset<8>{ (unsigned char)(adc.fine) };

				baseConfigs[configIdx][compOffset + 0] = compP[0];
				baseConfigs[configIdx][compOffset + 1] = compP[1];
				baseConfigs[configIdx][compOffset + 2] = compP[2];
				baseConfigs[configIdx][compOffset + 3] = compP[3];
				baseConfigs[configIdx][compOffset + 4] = compP[4];

				baseConfigs[configIdx][compOffset + 5] = compN[0];
				baseConfigs[configIdx][compOffset + 6] = compN[1];
				baseConfigs[configIdx][compOffset + 7] = compN[2];
				baseConfigs[configIdx][compOffset + 8] = compN[3];
				baseConfigs[configIdx][compOffset + 9] = compN[4];

				baseConfigs[configIdx][slopeOffset + 0] = slope[0];
				baseConfigs[configIdx][slopeOffset + 1] = slope[1];
				baseConfigs[configIdx][slopeOffset + 2] = slope[2];

				baseConfigs[configIdx][slopeOffset + 3] = fine[0];
				baseConfigs[configIdx][slopeOffset + 4] = fine[1];

				baseConfigs[configIdx][slopeOffset + 5] = coarse[0];
				baseConfigs[configIdx][slopeOffset + 6] = coarse[1];

				baseConfigs[configIdx][slopeOffset + 7] = cfix[0];
				baseConfigs[configIdx][slopeOffset + 8] = cfix[1];
				baseConfigs[configIdx][slopeOffset + 9] = cfix[2];
				baseConfigs[configIdx][slopeOffset + 10] = cfix[3];
			}

			return baseConfigs;
		}
	};
}

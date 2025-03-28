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
	Path shankOutline;
	int columns_per_shank;
	int rows_per_shank;
	String name;
	bool switchable;
};

struct NeuropixelsV1fValues
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
		selectedBank.clear();
		selectedShank.clear();
		selectedElectrode.clear();
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

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

#ifndef __NEUROPIXELCOMPONENTS_H__
#define __NEUROPIXELCOMPONENTS_H__

#include <DataThreadHeaders.h>
#include <stdio.h>
#include <string.h>

#include "UI/ActivityView.h"
#include "UI/ProbeNameConfig.h"

# define SAMPLECOUNT 64
# define MAX_HEADSTAGE_CLK_SAMPLE 3221225475
# define MAX_ALLOWABLE_TIMESTAMP_JUMP 4

# define MAXPACKETS 64

class Headstage;
class NeuropixV1Interface;

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

struct ProbeSettings
{
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

	Array<Bank> selectedBank;
	Array<int> selectedShank;
	Array<int> selectedChannel;
	Array<int> selectedElectrode;
	Array<ElectrodeMetadata> electrodeMetadata;

	void clearElectrodeSelection()
	{
		selectedBank.clear();
		selectedShank.clear();
		selectedChannel.clear();
		selectedElectrode.clear();
	}

	ProbeType probeType;

	ProbeMetadata probeMetadata;
};

/** Base class for all Neuropixels components, which must implement the "getInfo" method */
class NeuropixComponent
{
public:

	/** Constructor */
	NeuropixComponent() {}

	/** Pure virtual method for getting component info */
	virtual void getInfo() = 0;
};

#endif  // __NEUROPIXELCOMPONENTS_H__

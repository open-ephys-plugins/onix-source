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

#include "Neuropixels1.h"

using namespace OnixSourcePlugin;

NeuropixelsV1BackgroundUpdater::NeuropixelsV1BackgroundUpdater(Neuropixels1* d)
	: ThreadWithProgressWindow("Writing calibration files to Neuropixels Probe: " + d->getName(), true, false)
{
	device = d;
}

bool NeuropixelsV1BackgroundUpdater::updateSettings()
{
	if (device->isEnabled())
		runThread();
	else
		return false;

	return result;
}

Neuropixels1::Neuropixels1(std::string name, std::string hubName, OnixDeviceType deviceType, const oni_dev_idx_t deviceIndex, std::shared_ptr<Onix1> context) :
	OnixDevice(name, hubName, deviceType, deviceIndex, context, deviceType == OnixDeviceType::NEUROPIXELSV1E),
	I2CRegisterContext(ProbeI2CAddress, deviceIndex, context),
	INeuropixel(NeuropixelsV1Values::numberOfSettings, NeuropixelsV1Values::numberOfShanks)
{
}

void Neuropixels1::setSettings(ProbeSettings<NeuropixelsV1Values::numberOfChannels, NeuropixelsV1Values::numberOfElectrodes>* settings_, int index)
{
	if (index >= settings.size())
	{
		LOGE("Invalid index given when trying to update settings.");
		return;
	}

	settings[index]->updateProbeSettings(settings_);
}

std::vector<int> Neuropixels1::selectElectrodeConfiguration(std::string config)
{
	std::vector<int> selection;

	if (config == "Bank A")
	{
		for (int i = 0; i < 384; i++)
			selection.emplace_back(i);
	}
	else if (config == "Bank B")
	{
		for (int i = 384; i < 768; i++)
			selection.emplace_back(i);
	}
	else if (config == "Bank C")
	{
		for (int i = 576; i < 960; i++)
			selection.emplace_back(i);
	}
	else if (config == "Single Column")
	{
		for (int i = 0; i < 384; i += 2)
			selection.emplace_back(i);

		for (int i = 385; i < 768; i += 2)
			selection.emplace_back(i);
	}
	else if (config == "Tetrodes")
	{
		for (int i = 0; i < 384; i += 8)
		{
			for (int j = 0; j < 4; j++)
				selection.emplace_back(i + j);
		}

		for (int i = 388; i < 768; i += 8)
		{
			for (int j = 0; j < 4; j++)
				selection.emplace_back(i + j);
		}
	}

	assert(selection.size() == numberOfChannels && "Invalid number of selected channels.");

	return selection;
}

void Neuropixels1::updateApOffsets(std::array<float, numApSamples>& samples, int64 sampleNumber)
{
	if (sampleNumber > apSampleRate * secondsToSettle)
	{
		uint32_t counter = 0;

		while (apOffsetValues[0].size() < samplesToAverage)
		{
			if (counter >= superFramesPerUltraFrame * numUltraFrames) break;

			for (size_t i = 0; i < numberOfChannels; i++)
			{
				apOffsetValues[i].emplace_back(samples[i * superFramesPerUltraFrame * numUltraFrames + counter]);
			}

			counter++;
		}

		if (apOffsetValues[0].size() >= samplesToAverage)
		{
			for (int i = 0; i < numberOfChannels; i++)
			{
				apOffsets[i] = std::reduce(apOffsetValues.at(i).begin(), apOffsetValues.at(i).end()) / apOffsetValues.at(i).size();
			}

			apOffsetCalculated = true;
			apOffsetValues.clear();
		}
	}
}

void Neuropixels1::updateLfpOffsets(std::array<float, numLfpSamples>& samples, int64 sampleNumber)
{
	if (sampleNumber > lfpSampleRate * secondsToSettle)
	{
		uint32_t counter = 0;

		while (lfpOffsetValues[0].size() < samplesToAverage)
		{
			if (counter >= numUltraFrames) break;

			for (size_t i = 0; i < numberOfChannels; i++)
			{
				lfpOffsetValues[i].emplace_back(samples[i * numUltraFrames + counter]);
			}

			counter++;
		}

		if (lfpOffsetValues[0].size() >= samplesToAverage)
		{
			for (int i = 0; i < numberOfChannels; i++)
			{
				lfpOffsets[i] = std::reduce(lfpOffsetValues.at(i).begin(), lfpOffsetValues.at(i).end()) / lfpOffsetValues.at(i).size();
			}

			lfpOffsetCalculated = true;
			lfpOffsetValues.clear();
		}
	}
}

void Neuropixels1::defineMetadata(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings)
{
	settings->probeType = ProbeType::NPX_V1;
	settings->probeMetadata.name = "Neuropixels 1.0";

	std::vector<std::array<float, 2>> shankOutline{
		{27, 31},
		{27, 514},
		{27 + 5, 522},
		{27 + 10, 514},
		{27 + 10, 31}
	};

	std::vector<std::array<float, 2>> probeContour{
		{0, 155},
		{35, 0},
		{70, 155},
		{70, 9770},
		{0, 9770},
		{0, 155}
	};

	settings->probeMetadata.shank_count = 1;
	settings->probeMetadata.electrodes_per_shank = numberOfElectrodes;
	settings->probeMetadata.rows_per_shank = numberOfElectrodes / 2;
	settings->probeMetadata.columns_per_shank = 2;
	settings->probeMetadata.shankOutline = shankOutline;
	settings->probeMetadata.probeContour = probeContour;
	settings->probeMetadata.num_adcs = 32; // NB: Is this right for 1.0e?
	settings->probeMetadata.adc_bits = 10; // NB: Is this right for 1.0e?

	settings->availableBanks = {
		Bank::A,
		Bank::B,
		Bank::C,
		Bank::NONE //disconnected
	};

	Array<float> xpositions = { 27.0f, 59.0f, 11.0f, 43.0f };

	for (int i = 0; i < numberOfElectrodes; i++)
	{
		ElectrodeMetadata metadata;

		metadata.shank = 0;
		metadata.shank_local_index = i % settings->probeMetadata.electrodes_per_shank;
		metadata.global_index = i;
		metadata.xpos = xpositions[i % 4];
		metadata.ypos = (i - (i % 2)) * 10.0f;
		metadata.site_width = 12;
		metadata.column_index = i % 2;
		metadata.row_index = i / 2;
		metadata.isSelected = false;
		metadata.colour = Colours::lightgrey;

		if (i < 384)
		{
			metadata.bank = Bank::A;
			metadata.channel = i;
			metadata.status = ElectrodeStatus::CONNECTED;
		}
		else if (i >= 384 && i < 768)
		{
			metadata.bank = Bank::B;
			metadata.channel = i - 384;
			metadata.status = ElectrodeStatus::DISCONNECTED;
		}
		else
		{
			metadata.bank = Bank::C;
			metadata.channel = i - 768;
			metadata.status = ElectrodeStatus::DISCONNECTED;
		}

		if (i == 191 || i == 575 || i == 959)
		{
			metadata.type = ElectrodeType::REFERENCE;
		}
		else
		{
			metadata.type = ElectrodeType::ELECTRODE;
		}

		settings->electrodeMetadata[i] = metadata;
	}

	settings->apGainIndex = 4;	// NB:  AP Gain Index of 4 = Gain1000
	settings->lfpGainIndex = 0;	// NB: LFP Gain Index of 0 = Gain50
	settings->referenceIndex = 0;
	settings->apFilterState = true;

	for (int i = 0; i < numberOfChannels; i++)
	{
		settings->selectedBank[i] = Bank::A;
		settings->selectedShank[i] = 0;
		settings->selectedElectrode[i] = i;
	}

	settings->availableApGains.add(50.0f);
	settings->availableApGains.add(125.0f);
	settings->availableApGains.add(250.0f);
	settings->availableApGains.add(500.0f);
	settings->availableApGains.add(1000.0f);
	settings->availableApGains.add(1500.0f);
	settings->availableApGains.add(2000.0f);
	settings->availableApGains.add(3000.0f);

	settings->availableLfpGains.add(50.0f);
	settings->availableLfpGains.add(125.0f);
	settings->availableLfpGains.add(250.0f);
	settings->availableLfpGains.add(500.0f);
	settings->availableLfpGains.add(1000.0f);
	settings->availableLfpGains.add(1500.0f);
	settings->availableLfpGains.add(2000.0f);
	settings->availableLfpGains.add(3000.0f);

	settings->availableReferences.add("Ext");
	settings->availableReferences.add("Tip");

	settings->availableElectrodeConfigurations.add("Bank A");
	settings->availableElectrodeConfigurations.add("Bank B");
	settings->availableElectrodeConfigurations.add("Bank C");
	settings->availableElectrodeConfigurations.add("Single column");
	settings->availableElectrodeConfigurations.add("Tetrodes");

	settings->electrodeConfigurationIndex = 0;

	settings->isValid = true;
}

uint64_t Neuropixels1::getProbeSerialNumber(int index)
{
	return probeNumber;
}

bool Neuropixels1::parseGainCalibrationFile()
{
	if (gainCalibrationFilePath == "None" || gainCalibrationFilePath == "")
	{
		Onix1::showWarningMessageBoxAsync("Missing File", "Missing gain calibration file for probe " + std::to_string(probeNumber));
		return false;
	}

	File gainFile = File(gainCalibrationFilePath);

	if (!gainFile.existsAsFile())
	{
		Onix1::showWarningMessageBoxAsync("Invalid File", "Invalid gain calibration file for probe " + std::to_string(probeNumber));
		return false;
	}

	StringArray gainFileLines;
	gainFile.readLines(gainFileLines);

	auto gainSN = std::stoull(gainFileLines[0].toStdString());

	LOGD("Gain calibration file SN = ", gainSN);

	if (gainSN != probeNumber)
	{
		Onix1::showWarningMessageBoxAsync("Serial Number Mismatch", "Gain calibration file serial number (" + std::to_string(gainSN) + ") does not match probe serial number (" + std::to_string(probeNumber) + ").");
		return false;
	}

	if (gainFileLines.size() != numberOfElectrodes + 2)
	{
		Onix1::showWarningMessageBoxAsync("Invalid Gain Calibration File", "Expected to find " + std::to_string(numberOfElectrodes + 1) + " lines, but found " + std::to_string(gainFileLines.size()) + " instead.");
		return false;
	}

	StringRef gainCalLine = gainFileLines[1];
	StringRef breakCharacters = ",";
	StringRef noQuote = "";

	StringArray calibrationValues = StringArray::fromTokens(gainCalLine, breakCharacters, noQuote);

	static constexpr int NumberOfGainFactors = 8;

	if (calibrationValues.size() != NumberOfGainFactors * 2 + 1)
	{
		Onix1::showWarningMessageBoxAsync("Gain Calibration File Error", "Expected to find " + std::to_string(NumberOfGainFactors * 2 + 1) + " elements per line, but found " + std::to_string(calibrationValues.size()) + " instead.");
		return false;
	}

	apGainCorrection = std::stod(calibrationValues[settings[0]->apGainIndex + 1].toStdString());
	lfpGainCorrection = std::stod(calibrationValues[settings[0]->lfpGainIndex + NumberOfGainFactors + 1].toStdString());

	LOGD("AP gain correction = ", apGainCorrection, ", LFP gain correction = ", lfpGainCorrection);

	return true;
}

bool Neuropixels1::parseAdcCalibrationFile()
{
	if (adcCalibrationFilePath == "None" || adcCalibrationFilePath == "")
	{
		Onix1::showWarningMessageBoxAsync("Missing File", "Missing ADC calibration file for probe " + std::to_string(probeNumber));
		return false;
	}

	File adcFile = File(adcCalibrationFilePath);

	if (!adcFile.existsAsFile())
	{
		Onix1::showWarningMessageBoxAsync("Invalid File", "Invalid ADC calibration file for probe " + std::to_string(probeNumber));
		return false;
	}

	StringArray adcFileLines;
	adcFile.readLines(adcFileLines);

	auto adcSN = std::stoull(adcFileLines[0].toStdString());

	LOGD("ADC calibration file SN = ", adcSN);

	if (adcSN != probeNumber)
	{
		Onix1::showWarningMessageBoxAsync("Serial Number Mismatch", "ADC calibration serial number (" + std::to_string(adcSN) + ") does not match probe serial number (" + std::to_string(probeNumber)+").");
		return false;
	}

	if (adcFileLines.size() != NeuropixelsV1Values::AdcCount + 2)
	{
		Onix1::showWarningMessageBoxAsync("ADC Calibration File Error", "ADC calibration file does not have the correct number of lines. Expected " + std::to_string(NeuropixelsV1Values::AdcCount + 1) + " lines, found " + std::to_string(adcFileLines.size()) + " instead.");
		return false;
	}

	static constexpr int NumAdcValues = 9; // NB: ADC number + 8 values

	StringRef gainCalLine = adcFileLines[1];
	StringRef breakCharacters = ",";
	StringRef noQuote = "";

	adcValues.clear();

	for (int i = 1; i < adcFileLines.size() - 1; i++)
	{
		auto adcLine = StringArray::fromTokens(adcFileLines[i], breakCharacters, noQuote);

		if (adcLine.size() != NumAdcValues)
		{
			Onix1::showWarningMessageBoxAsync("ADC Calibration File Error", "ADC Calibration file line " + std::to_string(i) + " is invalid. Expected " + std::to_string(NumAdcValues) + " values, found " + std::to_string(adcLine.size()) + " instead.");
			return false;
		}

		adcValues.emplace_back(
			NeuropixelsV1Adc(
				std::stoi(adcLine[1].toStdString()),
				std::stoi(adcLine[2].toStdString()),
				std::stoi(adcLine[3].toStdString()),
				std::stoi(adcLine[4].toStdString()),
				std::stoi(adcLine[5].toStdString()),
				std::stoi(adcLine[6].toStdString()),
				std::stoi(adcLine[7].toStdString()),
				std::stoi(adcLine[8].toStdString())
			));
	}

	return true;
}

NeuropixelsV1Gain Neuropixels1::getGainEnum(int index)
{
	switch (index)
	{
	case 0:
		return NeuropixelsV1Gain::Gain50;
	case 1:
		return NeuropixelsV1Gain::Gain125;
	case 2:
		return NeuropixelsV1Gain::Gain250;
	case 3:
		return NeuropixelsV1Gain::Gain500;
	case 4:
		return NeuropixelsV1Gain::Gain1000;
	case 5:
		return NeuropixelsV1Gain::Gain1500;
	case 6:
		return NeuropixelsV1Gain::Gain2000;
	case 7:
		return NeuropixelsV1Gain::Gain3000;
	default:
		return NeuropixelsV1Gain::Gain50;
	}
}

int Neuropixels1::getGainValue(NeuropixelsV1Gain gain)
{
	switch (gain)
	{
	case NeuropixelsV1Gain::Gain50:
		return 50;
	case NeuropixelsV1Gain::Gain125:
		return 125;
	case NeuropixelsV1Gain::Gain250:
		return 250;
	case NeuropixelsV1Gain::Gain500:
		return 500;
	case NeuropixelsV1Gain::Gain1000:
		return 1000;
	case NeuropixelsV1Gain::Gain1500:
		return 1500;
	case NeuropixelsV1Gain::Gain2000:
		return 2000;
	case NeuropixelsV1Gain::Gain3000:
		return 3000;
	default:
		return 50;
	}
}

NeuropixelsV1Reference Neuropixels1::getReference(int index)
{
	switch (index)
	{
	case 0:
		return NeuropixelsV1Reference::External;
	case 1:
		return NeuropixelsV1Reference::Tip;
	default:
		break;
	}

	return NeuropixelsV1Reference::External;
}

std::string Neuropixels1::getAdcCalibrationFilePath()
{
	return adcCalibrationFilePath;
}

void Neuropixels1::setAdcCalibrationFilePath(std::string filepath)
{
	adcCalibrationFilePath = filepath;
}

std::string Neuropixels1::getGainCalibrationFilePath()
{
	return gainCalibrationFilePath;
}

void Neuropixels1::setGainCalibrationFilePath(std::string filepath)
{
	gainCalibrationFilePath = filepath;
}

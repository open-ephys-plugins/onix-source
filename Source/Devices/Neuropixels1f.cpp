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

#include "Neuropixels1f.h"

using namespace OnixSourcePlugin;

BackgroundUpdaterWithProgressWindow::BackgroundUpdaterWithProgressWindow(Neuropixels1f* d)
	: ThreadWithProgressWindow("Writing calibration files to Neuropixels Probe: " + d->getName(), true, false)
{
	device = d;
}

bool BackgroundUpdaterWithProgressWindow::updateSettings()
{
	if (device->isEnabled())
		runThread();
	else
		return 0;

	return result;
}

void BackgroundUpdaterWithProgressWindow::run()
{
	setProgress(0);

	// Parse ADC and Gain calibration files
	String adcPath = device->adcCalibrationFilePath;
	String gainPath = device->gainCalibrationFilePath;

	if (adcPath == "None" || gainPath == "None")
	{
		result = false;

		if (adcPath == "None")
		{
			Onix1::showWarningMessageBoxAsync("Missing File", "Missing ADC calibration file for probe " + std::to_string(device->getProbeSerialNumber()));
		}
		else if (gainPath == "None")
		{
			Onix1::showWarningMessageBoxAsync("Missing File", "Missing Gain calibration file for probe " + std::to_string(device->getProbeSerialNumber()));
		}

		return;
	}

	File adcFile = File(adcPath);
	File gainFile = File(gainPath);

	if (!adcFile.existsAsFile() || !gainFile.existsAsFile())
	{
		result = false;

		if (!adcFile.existsAsFile())
		{
			Onix1::showWarningMessageBoxAsync("Invalid File", "Invalid ADC calibration file for probe " + std::to_string(device->getProbeSerialNumber()));
		}
		else if (!gainFile.existsAsFile())
		{
			Onix1::showWarningMessageBoxAsync("Invalid File", "Invalid gain calibration file for probe " + std::to_string(device->getProbeSerialNumber()));
		}

		return;
	}

	setProgress(0.2);

	StringArray gainFileLines;
	gainFile.readLines(gainFileLines);

	auto gainSN = std::stoull(gainFileLines[0].toStdString());

	LOGD("Gain calibration file SN = ", gainSN);

	if (gainSN != device->getProbeSerialNumber())
	{
		result = false;

		Onix1::showWarningMessageBoxAsync("Invalid Serial Number", "Gain calibration file serial number (" + std::to_string(gainSN) + ") does not match probe serial number (" + std::to_string(device->getProbeSerialNumber()) + ").");

		return;
	}

	StringRef gainCalLine = gainFileLines[1];
	StringRef breakCharacters = ",";
	StringRef noQuote = "";

	StringArray calibrationValues = StringArray::fromTokens(gainCalLine, breakCharacters, noQuote);

	double apGainCorrection = std::stod(calibrationValues[device->settings[0]->apGainIndex + 1].toStdString());
	double lfpGainCorrection = std::stod(calibrationValues[device->settings[0]->lfpGainIndex + 8].toStdString());

	LOGD("AP gain correction = ", apGainCorrection, ", LFP gain correction = ", lfpGainCorrection);

	setProgress(0.5);

	StringArray adcFileLines;
	adcFile.readLines(adcFileLines);

	auto adcSN = std::stoull(adcFileLines[0].toStdString());

	LOGD("ADC calibration file SN = ", adcSN);

	if (adcSN != device->getProbeSerialNumber())
	{
		result = false;

		Onix1::showWarningMessageBoxAsync("Invalid Serial Number", "ADC calibration file serial number (" + std::to_string(adcSN) + ") does not match probe serial number (" + std::to_string(device->getProbeSerialNumber()) + ").");

		return;
	}

	Array<NeuropixelsV1fAdc> adcs;

	for (int i = 1; i < adcFileLines.size() - 1; i++)
	{
		auto adcLine = StringArray::fromTokens(adcFileLines[i], breakCharacters, noQuote);

		adcs.add(
			NeuropixelsV1fAdc(
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

	setProgress(0.8);

	// Write shift registers
	auto shankBits = device->makeShankBits(device->getReference(device->settings[0]->referenceIndex), device->settings[0]->selectedElectrode);
	auto configBits = device->makeConfigBits(device->getReference(device->settings[0]->referenceIndex), device->getGainEnum(device->settings[0]->apGainIndex), device->getGainEnum(device->settings[0]->lfpGainIndex), true, adcs);

	device->writeShiftRegisters(shankBits, configBits, adcs, lfpGainCorrection, apGainCorrection);

	setProgress(1);

	result = true;
}

Neuropixels1f::Neuropixels1f(std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx_) :
	OnixDevice(name, hubName, Neuropixels1f::getDeviceType(), deviceIdx_, ctx_),
	I2CRegisterContext(ProbeI2CAddress, deviceIdx_, ctx_),
	INeuropixel(NeuropixelsV1fValues::numberOfSettings, NeuropixelsV1fValues::numberOfShanks)
{
	std::string port = getPortNameFromIndex(deviceIdx);
	auto streamIdentifier = getStreamIdentifier();

	StreamInfo apStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHubName(), getName(), "AP" }),
		"Neuropixels 1.0 AP band data stream",
		streamIdentifier,
		numberOfChannels,
		apSampleRate,
		"AP",
		ContinuousChannel::Type::ELECTRODE,
		0.195f,
		CharPointer_UTF8("\xc2\xb5V"),
		{},
		"ap"
	);
	streamInfos.add(apStream);

	StreamInfo lfpStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHubName(), getName(), "LFP" }),
		"Neuropixels 1.0 LFP band data stream",
		streamIdentifier,
		numberOfChannels,
		lfpSampleRate,
		"LFP",
		ContinuousChannel::Type::ELECTRODE,
		0.195f,
		CharPointer_UTF8("\xc2\xb5V"),
		{},
		"lfp"
	);
	streamInfos.add(lfpStream);

	defineMetadata(settings[0].get());

	adcCalibrationFilePath = "None";
	gainCalibrationFilePath = "None";

	for (int i = 0; i < numUltraFrames; i++)
	{
		apEventCodes[i] = 0;
		lfpEventCodes[i] = 0;
	}
}

NeuropixelsGain Neuropixels1f::getGainEnum(int index)
{
	switch (index)
	{
	case 0:
		return NeuropixelsGain::Gain50;
	case 1:
		return NeuropixelsGain::Gain125;
	case 2:
		return NeuropixelsGain::Gain250;
	case 3:
		return NeuropixelsGain::Gain500;
	case 4:
		return NeuropixelsGain::Gain1000;
	case 5:
		return NeuropixelsGain::Gain1500;
	case 6:
		return NeuropixelsGain::Gain2000;
	case 7:
		return NeuropixelsGain::Gain3000;

	default:
		break;
	}

	return NeuropixelsGain::Gain50;
}

int Neuropixels1f::getGainValue(NeuropixelsGain gain)
{
	switch (gain)
	{
	case NeuropixelsGain::Gain50:
		return 50;
	case NeuropixelsGain::Gain125:
		return 125;
	case NeuropixelsGain::Gain250:
		return 250;
	case NeuropixelsGain::Gain500:
		return 500;
	case NeuropixelsGain::Gain1000:
		return 1000;
	case NeuropixelsGain::Gain1500:
		return 1500;
	case NeuropixelsGain::Gain2000:
		return 2000;
	case NeuropixelsGain::Gain3000:
		return 3000;

	default:
		break;
	}
}

NeuropixelsV1fReference Neuropixels1f::getReference(int index)
{
	switch (index)
	{
	case 0:
		return NeuropixelsV1fReference::External;
	case 1:
		return NeuropixelsV1fReference::Tip;
	default:
		break;
	}

	return NeuropixelsV1fReference::External;
}

OnixDeviceType Neuropixels1f::getDeviceType()
{
	return OnixDeviceType::NEUROPIXELSV1F;
}

int Neuropixels1f::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized())
		throw error_str("Device context is not initialized properly for " + getName());

	int rc = deviceContext->writeRegister(deviceIdx, ENABLE, isEnabled() ? 1 : 0);
	if (rc != ONI_ESUCCESS)
		throw error_str("Unable to enable " + getName());

	if (!isEnabled())
	{
		return ONI_ESUCCESS;
	}

	// Get Probe SN
	uint32_t eepromOffset = 0;
	uint32_t i2cAddr = 0x50;
	int errorCode = 0;

	for (int i = 0; i < 8; i++)
	{
		oni_reg_addr_t reg_addr = ((eepromOffset + i) << 7) | i2cAddr;

		oni_reg_val_t reg_val;
		rc = deviceContext->readRegister(deviceIdx, reg_addr, &reg_val);

		if (rc != ONI_ESUCCESS)
		{
			LOGE(oni_error_str(rc));
			throw error_str("Could not communicate with " + getName() + " on " + getHubName() + ". Ensure that the flex connection is properly seated, or disable the device if it is not connected.");
		}

		if (reg_val <= 0xFF)
		{
			probeNumber |= (((uint64_t)reg_val) << (i * 8));
		}
	}

	LOGD("Probe SN: ", probeNumber);

	// Enable device streaming
	rc = deviceContext->writeRegister(deviceIdx, 0x8000, 1);
	if (rc != ONI_ESUCCESS)
		throw error_str("Unable to activate streaming for device at address " + std::to_string(deviceIdx));

	rc = WriteByte((uint32_t)NeuropixelsRegisters::CAL_MOD, (uint32_t)CalMode::CAL_OFF);
	if (rc != ONI_ESUCCESS)
		throw error_str("Error configuring device at address " + std::to_string(deviceIdx));

	rc = WriteByte((uint32_t)NeuropixelsRegisters::SYNC, (uint32_t)0);
	if (rc != ONI_ESUCCESS)
		throw error_str("Error configuring device at address " + std::to_string(deviceIdx));

	rc = WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::DIG_AND_CH_RESET);
	if (rc != ONI_ESUCCESS)
		throw error_str("Error configuring device at address " + std::to_string(deviceIdx));

	rc = WriteByte((uint32_t)NeuropixelsRegisters::OP_MODE, (uint32_t)OpMode::RECORD);
	if (rc != ONI_ESUCCESS)
		throw error_str("Error configuring device at address " + std::to_string(deviceIdx));

	return rc;
}

bool Neuropixels1f::updateSettings()
{
	BackgroundUpdaterWithProgressWindow updater = BackgroundUpdaterWithProgressWindow(this);

	return updater.updateSettings();
}

void Neuropixels1f::setSettings(ProbeSettings<NeuropixelsV1fValues::numberOfChannels, NeuropixelsV1fValues::numberOfElectrodes>* settings_, int index)
{
	if (index >= settings.size())
	{
		LOGE("Invalid index given when trying to update settings.");
		return;
	}

	settings[index]->updateProbeSettings(settings_);
}

std::vector<int> Neuropixels1f::selectElectrodeConfiguration(String config)
{
	std::vector<int> selection;

	if (config.equalsIgnoreCase("Bank A"))
	{
		for (int i = 0; i < 384; i++)
			selection.emplace_back(i);
	}
	else if (config.equalsIgnoreCase("Bank B"))
	{
		for (int i = 384; i < 768; i++)
			selection.emplace_back(i);
	}
	else if (config.equalsIgnoreCase("Bank C"))
	{
		for (int i = 576; i < 960; i++)
			selection.emplace_back(i);
	}
	else if (config.equalsIgnoreCase("Single Column"))
	{
		for (int i = 0; i < 384; i += 2)
			selection.emplace_back(i);

		for (int i = 385; i < 768; i += 2)
			selection.emplace_back(i);
	}
	else if (config.equalsIgnoreCase("Tetrodes"))
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

void Neuropixels1f::startAcquisition()
{
	apGain = getGainValue(getGainEnum(settings[0]->apGainIndex));
	lfpGain = getGainValue(getGainEnum(settings[0]->lfpGainIndex));

	apOffsetValues.clear();
	apOffsetValues.reserve(numberOfChannels);
	lfpOffsetValues.clear();
	lfpOffsetValues.reserve(numberOfChannels);

	for (int i = 0; i < numberOfChannels; i++)
	{
		apOffsets[i] = 0;
		lfpOffsets[i] = 0;

		apOffsetValues.emplace_back(std::vector<float>{});
		lfpOffsetValues.emplace_back(std::vector<float>{});
	}

	lfpOffsetCalculated = false;
	apOffsetCalculated = false;

	WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::ACTIVE);

	superFrameCount = 0;
	ultraFrameCount = 0;
	shouldAddToBuffer = false;
	apSampleNumber = 0;
	lfpSampleNumber = 0;
}

void Neuropixels1f::stopAcquisition()
{
	WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::RESET_ALL);

	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}
}

void Neuropixels1f::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	for (StreamInfo streamInfo : streamInfos)
	{
		sourceBuffers.add(new DataBuffer(streamInfo.getNumChannels(), (int)streamInfo.getSampleRate() * bufferSizeInSeconds));

		if (streamInfo.getChannelPrefix().equalsIgnoreCase("AP"))
			apBuffer = sourceBuffers.getLast();
		else if (streamInfo.getChannelPrefix().equalsIgnoreCase("LFP"))
			lfpBuffer = sourceBuffers.getLast();
	}
}

void Neuropixels1f::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void Neuropixels1f::processFrames()
{
	const float apConversion = (1171.875 / apGain) * -1.0f;
	const float lfpConversion = (1171.875 / lfpGain) * -1.0f;

	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		uint16_t* dataPtr = (uint16_t*)frame->data;

		apTimestamps[superFrameCount] = deviceContext->convertTimestampToSeconds(frame->time);
		apSampleNumbers[superFrameCount] = apSampleNumber++;

		for (int i = 0; i < framesPerSuperFrame; i++)
		{
			if (i == 0) // LFP data
			{
				int superCountOffset = superFrameCount % superFramesPerUltraFrame;
				if (superCountOffset == 0)
				{
					lfpTimestamps[ultraFrameCount] = apTimestamps[superFrameCount];
					lfpSampleNumbers[ultraFrameCount] = lfpSampleNumber++;
				}

				for (int adc = 0; adc < 32; adc++)
				{
					int chanIndex = adcToChannel[adc] + superCountOffset * 2; // map the ADC to muxed channel
					lfpSamples[(chanIndex * numUltraFrames) + ultraFrameCount] =
						lfpConversion * float((*(dataPtr + adcToFrameIndex[adc] + dataOffset) >> 5) - 512) - lfpOffsets.at(chanIndex);
				}
			}
			else // AP data
			{
				int chanOffset = 2 * (i - 1);
				for (int adc = 0; adc < 32; adc++)
				{
					int chanIndex = adcToChannel[adc] + chanOffset; //  map the ADC to muxed channel.
					apSamples[(chanIndex * superFramesPerUltraFrame * numUltraFrames) + superFrameCount] =
						apConversion * float((*(dataPtr + adcToFrameIndex[adc] + i * 36 + dataOffset) >> 5) - 512) - apOffsets.at(chanIndex);
				}
			}
		}

		oni_destroy_frame(frame);

		superFrameCount++;

		if (superFrameCount % superFramesPerUltraFrame == 0)
		{
			ultraFrameCount++;
		}

		if (ultraFrameCount >= numUltraFrames)
		{
			ultraFrameCount = 0;
			superFrameCount = 0;
			shouldAddToBuffer = true;
		}

		if (shouldAddToBuffer)
		{
			shouldAddToBuffer = false;
			lfpBuffer->addToBuffer(lfpSamples.data(), lfpSampleNumbers, lfpTimestamps, lfpEventCodes, numUltraFrames);
			apBuffer->addToBuffer(apSamples.data(), apSampleNumbers, apTimestamps, apEventCodes, numUltraFrames * superFramesPerUltraFrame);

			if (!lfpOffsetCalculated) updateLfpOffsets(lfpSamples, lfpSampleNumbers[0]);
			if (!apOffsetCalculated) updateApOffsets(apSamples, apSampleNumbers[0]);
		}
	}
}

void Neuropixels1f::updateApOffsets(std::array<float, numApSamples>& samples, int64 sampleNumber)
{
	if (sampleNumber > apSampleRate * secondsToSettle)
	{
		uint32_t counter = 0;

		while (apOffsetValues[0].size() < samplesToAverage)
		{
			if (counter >= superFramesPerUltraFrame * numUltraFrames) break;

			for (int i = 0; i < numberOfChannels; i++)
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

void Neuropixels1f::updateLfpOffsets(std::array<float, numLfpSamples>& samples, int64 sampleNumber)
{
	if (sampleNumber > lfpSampleRate * secondsToSettle)
	{
		uint32_t counter = 0;

		while (lfpOffsetValues[0].size() < samplesToAverage)
		{
			if (counter >= numUltraFrames) break;

			for (int i = 0; i < numberOfChannels; i++)
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

Neuropixels1f::ShankBitset Neuropixels1f::makeShankBits(NeuropixelsV1fReference reference, std::array<int, numberOfChannels> channelMap)
{
	const int shankBitExt1 = 965;
	const int shankBitExt2 = 2;
	const int shankBitTip1 = 484;
	const int shankBitTip2 = 483;
	const int internalReferenceChannel = 191;

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
	case NeuropixelsV1fReference::External:
		shankBits[shankBitExt1] = true;
		shankBits[shankBitExt2] = true;
		break;
	case NeuropixelsV1fReference::Tip:
		shankBits[shankBitTip1] = true;
		shankBits[shankBitTip2] = true;
		break;
	default:
		break;
	}

	if (channelMap.size() != numberOfChannels)
	{
		LOGE("Invalid number of channels connected for Neuropixels 1.0f, configuration might be invalid.");
	}

	return shankBits;
}

Neuropixels1f::CongigBitsArray Neuropixels1f::makeConfigBits(NeuropixelsV1fReference reference, NeuropixelsGain spikeAmplifierGain, NeuropixelsGain lfpAmplifierGain, bool spikeFilterEnabled, Array<NeuropixelsV1fAdc> adcs)
{
	const int BaseConfigurationConfigOffset = 576;

	CongigBitsArray baseConfigs;

	for (int i = 0; i < numberOfChannels; i++)
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

void Neuropixels1f::writeShiftRegisters(ShankBitset shankBits, CongigBitsArray configBits, Array<NeuropixelsV1fAdc> adcs, double lfpGainCorrection, double apGainCorrection)
{
	auto shankBytes = toBitReversedBytes<shankConfigurationBitCount>(shankBits);

	int rc = WriteByte((uint32_t)ShiftRegisters::SR_LENGTH1, (uint32_t)shankBytes.size() % 0x100);
	if (rc != ONI_ESUCCESS) return;

	rc = WriteByte((uint32_t)ShiftRegisters::SR_LENGTH2, (uint32_t)shankBytes.size() / 0x100);
	if (rc != ONI_ESUCCESS) return;

	for (auto b : shankBytes)
	{
		rc = WriteByte((uint32_t)ShiftRegisters::SR_CHAIN1, b);
		if (rc != ONI_ESUCCESS) return;
	}

	const uint32_t shiftRegisterSuccess = 1 << 7;

	for (int i = 0; i < configBits.size(); i++)
	{
		auto srAddress = i == 0 ? (uint32_t)ShiftRegisters::SR_CHAIN2 : (uint32_t)ShiftRegisters::SR_CHAIN3;

		for (int j = 0; j < 2; j++)
		{
			auto baseBytes = toBitReversedBytes<BaseConfigurationBitCount>(configBits[i]);

			rc = WriteByte((uint32_t)ShiftRegisters::SR_LENGTH1, (uint32_t)baseBytes.size() % 0x100);
			if (rc != ONI_ESUCCESS) return;

			rc = WriteByte((uint32_t)ShiftRegisters::SR_LENGTH2, (uint32_t)baseBytes.size() / 0x100);
			if (rc != ONI_ESUCCESS) return;

			for (auto b : baseBytes)
			{
				rc = WriteByte(srAddress, b);
				if (rc != ONI_ESUCCESS) return;
			}
		}

		oni_reg_val_t value;
		rc = ReadByte((uint32_t)NeuropixelsRegisters::STATUS, &value);

		if (rc != ONI_ESUCCESS || value != shiftRegisterSuccess)
		{
			LOGE("Shift register ", srAddress, " status check failed.");
			return;
		}
	}

	const uint32_t ADC01_00_OFF_THRESH = 0x8001;

	for (uint32_t i = 0; i < adcs.size(); i += 2)
	{
		auto value = (uint32_t)(adcs[i + 1].offset << 26 | adcs[i + 1].threshold << 16 | adcs[i].offset << 10 | adcs[i].threshold);
		rc = deviceContext->writeRegister(deviceIdx, ADC01_00_OFF_THRESH + i, value);

		if (rc != ONI_ESUCCESS)
		{
			LOGE("Error writing to register ", ADC01_00_OFF_THRESH + i, ".");
			return;
		}
	}

	auto fixedPointLfPGain = (uint32_t)(lfpGainCorrection * (1 << 14)) & 0xFFFF;
	auto fixedPointApGain = (uint32_t)(apGainCorrection * (1 << 14)) & 0xFFFF;

	const uint32_t CHAN001_000_LFPGAIN = 0x8011;
	const uint32_t CHAN001_000_APGAIN = 0x80D1;

	for (uint32_t i = 0; i < numberOfChannels / 2; i++)
	{
		rc = deviceContext->writeRegister(deviceIdx, CHAN001_000_LFPGAIN + i, fixedPointLfPGain << 16 | fixedPointLfPGain);
		if (rc != ONI_ESUCCESS)
		{
			LOGE("Error writing to register ", CHAN001_000_LFPGAIN + i, ".");
			return;
		}

		rc = deviceContext->writeRegister(deviceIdx, CHAN001_000_APGAIN + i, fixedPointApGain << 16 | fixedPointApGain);
		if (rc != ONI_ESUCCESS)
		{
			LOGE("Error writing to register ", CHAN001_000_APGAIN + i, ".");
			return;
		}
	}
}

void Neuropixels1f::defineMetadata(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings)
{
	settings->probeType = ProbeType::NPX_V1E;
	settings->probeMetadata.name = "Neuropixels 1.0f";

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

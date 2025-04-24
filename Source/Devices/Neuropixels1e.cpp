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

#include "Neuropixels1e.h"

using namespace OnixSourcePlugin;

NeuropixelsV1eBackgroundUpdater::NeuropixelsV1eBackgroundUpdater(Neuropixels1e* d)
	: ThreadWithProgressWindow("Writing calibration files to Neuropixels Probe: " + d->getName(), true, false)
{
	device = d;
}

bool NeuropixelsV1eBackgroundUpdater::updateSettings()
{
	if (device->isEnabled())
		runThread();
	else
		return 0;

	return result;
}

void NeuropixelsV1eBackgroundUpdater::run()
{
	setProgress(0);

	device->resetProbe();

	// Parse ADC and Gain calibration files
	String adcPath = device->getAdcCalibrationFilePath();
	String gainPath = device->getGainCalibrationFilePath();

	if (adcPath == "None" || gainPath == "None")
	{
		result = false;

		LOGE("Missing ADC or Gain calibration files" + device->getName());

		if (adcPath == "None")
		{
			CoreServices::sendStatusMessage("Missing ADC calibration file for " + device->getName());
		}
		else if (gainPath == "None")
		{
			CoreServices::sendStatusMessage("Missing Gain calibration file for " + device->getName());
		}

		return;
	}

	File adcFile = File(adcPath);
	File gainFile = File(gainPath);

	if (!adcFile.existsAsFile() || !gainFile.existsAsFile())
	{
		result = false;

		LOGE("Invalid ADC or Gain calibration files for " + device->getName());

		if (!adcFile.existsAsFile())
		{
			CoreServices::sendStatusMessage("Invalid ADC calibration file for " + device->getName());
		}
		else if (!gainFile.existsAsFile())
		{
			CoreServices::sendStatusMessage("Invalid Gain calibration file for " + device->getName());
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

		LOGE("Gain calibration serial number (", gainSN, ") does not match probe serial number (", device->getProbeSerialNumber(), ").");

		CoreServices::sendStatusMessage("Serial Number Mismatch: Gain calibration (" + String(gainSN) + ") does not match " + String(device->getProbeSerialNumber()));

		return;
	}

	StringRef gainCalLine = gainFileLines[1];
	StringRef breakCharacters = ",";
	StringRef noQuote = "";

	StringArray calibrationValues = StringArray::fromTokens(gainCalLine, breakCharacters, noQuote);

	double apGainCorrection = std::stod(calibrationValues[device->settings[0]->apGainIndex + 1].toStdString());
	double lfpGainCorrection = std::stod(calibrationValues[device->settings[0]->lfpGainIndex + 8].toStdString());

	LOGD("AP gain correction = ", apGainCorrection, ", LFP gain correction = ", lfpGainCorrection);

	device->setApGainCorrection(apGainCorrection);
	device->setLfpGainCorrection(lfpGainCorrection);

	setProgress(0.5);

	StringArray adcFileLines;
	adcFile.readLines(adcFileLines);

	auto adcSN = std::stoull(adcFileLines[0].toStdString());

	LOGD("ADC calibration file SN = ", adcSN);

	if (adcSN != device->getProbeSerialNumber())
	{
		result = false;

		LOGE("ADC calibration serial number (", adcSN, ") does not match probe serial number (", device->getProbeSerialNumber(), ").");

		CoreServices::sendStatusMessage("Serial Number Mismatch: ADC calibration (" + String(adcSN) + ") does not match " + String(device->getProbeSerialNumber()));

		return;
	}

	Array<NeuropixelsV1Adc> adcs;

	for (int i = 1; i < adcFileLines.size() - 1; i++)
	{
		auto adcLine = StringArray::fromTokens(adcFileLines[i], breakCharacters, noQuote);

		adcs.add(
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

	setProgress(0.8);

	// Write shift registers
	auto shankBits = NeuropixelsV1::makeShankBits(device->getReference(device->settings[0]->referenceIndex), device->settings[0]->selectedElectrode);
	auto configBits = NeuropixelsV1::makeConfigBits(device->getReference(device->settings[0]->referenceIndex), device->getGainEnum(device->settings[0]->apGainIndex), device->getGainEnum(device->settings[0]->lfpGainIndex), true, adcs);

	device->WriteByte((uint32_t)NeuropixelsV1Registers::CAL_MOD, (uint32_t)NeuropixelsV1CalibrationRegisterValues::CAL_OFF);
	device->WriteByte((uint32_t)NeuropixelsV1Registers::TEST_CONFIG1, 0);
	device->WriteByte((uint32_t)NeuropixelsV1Registers::TEST_CONFIG2, 0);
	device->WriteByte((uint32_t)NeuropixelsV1Registers::TEST_CONFIG3, 0);
	device->WriteByte((uint32_t)NeuropixelsV1Registers::TEST_CONFIG4, 0);
	device->WriteByte((uint32_t)NeuropixelsV1Registers::TEST_CONFIG5, 0);
	device->WriteByte((uint32_t)NeuropixelsV1Registers::SYNC, 0);
	device->WriteByte((uint32_t)NeuropixelsV1Registers::REC_MOD, (uint32_t)NeuropixelsV1RecordRegisterValues::ACTIVE);
	device->WriteByte((uint32_t)NeuropixelsV1Registers::OP_MODE, (uint32_t)NeuropixelsV1OperationRegisterValues::RECORD);

	device->writeShiftRegisters(shankBits, configBits, adcs);

	setProgress(1);

	result = true;
}

Neuropixels1e::Neuropixels1e(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx_) :
	OnixDevice(name, NEUROPIXELSV1E_HEADSTAGE_NAME, OnixDeviceType::NEUROPIXELSV1E, deviceIdx_, ctx_),
	I2CRegisterContext(ProbeI2CAddress, deviceIdx_, ctx_),
	INeuropixel(NeuropixelsV1Values::numberOfSettings, NeuropixelsV1Values::numberOfShanks)
{
	String port = getPortNameFromIndex(deviceIdx);
	StreamInfo apStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHeadstageName(), getName(), "AP" }),
		"Neuropixels 1.0 AP band data stream",
		"onix-neuropixels1.data.ap",
		numberOfChannels,
		apSampleRate,
		"AP",
		ContinuousChannel::Type::ELECTRODE,
		0.195f,
		CharPointer_UTF8("\xc2\xb5V"),
		{});
	streamInfos.add(apStream);

	StreamInfo lfpStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHeadstageName(), getName(), "LFP" }),
		"Neuropixels 1.0 LFP band data stream",
		"onix-neuropixels1.data.lfp",
		numberOfChannels,
		lfpSampleRate,
		"LFP",
		ContinuousChannel::Type::ELECTRODE,
		0.195f,
		CharPointer_UTF8("\xc2\xb5V"),
		{});
	streamInfos.add(lfpStream);

	defineMetadata(settings[0].get());

	adcCalibrationFilePath = "None";
	gainCalibrationFilePath = "None";

	for (int i = 0; i < numUltraFrames; i++)
	{
		apEventCodes[i] = 0;
		lfpEventCodes[i] = 0;
	}

	probeNumber = 0;
}

NeuropixelsV1Gain Neuropixels1e::getGainEnum(int index)
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
		break;
	}

	return NeuropixelsV1Gain::Gain50;
}

int Neuropixels1e::getGainValue(NeuropixelsV1Gain gain)
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
		break;
	}
}

NeuropixelsV1Reference Neuropixels1e::getReference(int index)
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

String Neuropixels1e::getAdcCalibrationFilePath()
{
	return adcCalibrationFilePath;
}

void Neuropixels1e::setAdcCalibrationFilePath(String filepath)
{
	adcCalibrationFilePath = filepath;
}

String Neuropixels1e::getGainCalibrationFilePath()
{
	return gainCalibrationFilePath;
}

void Neuropixels1e::setGainCalibrationFilePath(String filepath)
{
	gainCalibrationFilePath = filepath;
}

bool Neuropixels1e::getCorrectOffset() const
{
	return correctOffset;
}

void Neuropixels1e::setCorrectOffset(bool value)
{
	correctOffset = value;
}

double Neuropixels1e::getApGainCorrection() const
{
	return apGainCorrection;
}

void Neuropixels1e::setApGainCorrection(double value)
{
	apGainCorrection = value;
}

double Neuropixels1e::getLfpGainCorrection() const
{
	return lfpGainCorrection;
}

void Neuropixels1e::setLfpGainCorrection(double value)
{
	lfpGainCorrection = value;
}

int Neuropixels1e::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -5;

	configureSerDes();

	int rc = serializer->set933I2cRate(400e3);
	if (rc != ONI_ESUCCESS) return rc;

	// Get Probe SN

	uint32_t i2cAddr = 0x50;
	int errorCode = 0;

	for (int i = 0; i < 8; i++)
	{
		oni_reg_val_t reg_val;
		rc = flex->ReadByte(OFFSET_ID + i, &reg_val);

		if (rc != ONI_ESUCCESS) return rc;

		if (reg_val <= 0xFF)
		{
			probeNumber |= (((uint64_t)reg_val) << (i * 8));
		}
	}

	LOGD("Probe SN: ", probeNumber);

	return ONI_ESUCCESS;
}

void Neuropixels1e::configureSerDes()
{
	deviceContext->writeRegister(deviceIdx, DS90UB9x::ENABLE, 1);

	deviceContext->writeRegister(deviceIdx, DS90UB9x::TRIGGEROFF, 0);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::TRIGGER, (uint32_t)DS90UB9x::DS90UB9xTriggerMode::Continuous);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::SYNCBITS, 0);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::DATAGATE, 0b00000001000100110000000000000001);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::MARK, (uint32_t)DS90UB9x::DS90UB9xMarkMode::Disabled);

	// configure one magic word-triggered stream for the PSB bus
	deviceContext->writeRegister(deviceIdx, DS90UB9x::READSZ, 851973); // 13 frames/superframe,  7x 140-bit words on each serial line per frame
	deviceContext->writeRegister(deviceIdx, DS90UB9x::MAGIC_MASK, 0b11000000000000000000001111111111); // Enable inverse, wait for non-inverse, 10-bit magic word
	deviceContext->writeRegister(deviceIdx, DS90UB9x::MAGIC, 816); // Super-frame sync word
	deviceContext->writeRegister(deviceIdx, DS90UB9x::MAGIC_WAIT, 0);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::DATAMODE, 913);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::DATALINES0, 0x3245106B); // Sync, psb[0], psb[1], psb[2], psb[3], psb[4], psb[5], psb[6],
	deviceContext->writeRegister(deviceIdx, DS90UB9x::DATALINES1, 0xFFFFFFFF);
	
	std::this_thread::sleep_for(100ms); // Empirical. The gateware seems to need some milliseconds to get i2c initialized.

	deserializer = std::make_unique<I2CRegisterContext>(DS90UB9x::DES_ADDR, deviceIdx, deviceContext);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::PortSel, 0x01); // Enable port 0
	int coaxMode = 0x4 + (uint32_t)(DS90UB9x::DS90UB9xMode::Raw12BitHighFrequency); // 0x4 maintains coax mode
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::PortMode, coaxMode); // 0x4 maintains coax mode
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::I2CConfig, 0b01011000); // 7: i2c pass all (0), 6: i2c pass (1), 5: auto_ack (0), 4: BC enable (1), 3: BC crc en (1), 2: reserved (0) 1:0: bc freq (00) 2.5Mbps
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SerAlias, DS90UB9x::SER_ADDR << 1);
	// Enable backchannel GPIO on deserializer. It is then the serializer task to decide if using them or use manual output
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::GpioCtrl0, 0x10);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::GpioCtrl0, 0x32);

	auto alias = ProbeI2CAddress << 1;
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID1, alias);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias1, alias);

	alias = FlexEepromI2CAddress << 1;
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID2, alias);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias2, alias);

	serializer = std::make_unique<I2CRegisterContext>(DS90UB9x::SER_ADDR, deviceIdx, deviceContext);
	flex = std::make_unique<I2CRegisterContext>(FlexEepromI2CAddress, deviceIdx, deviceContext);
}

void Neuropixels1e::resetProbe()
{
	auto gpo10Config = DefaultGPO10Config & ~Gpo10ResetMask;
	serializer->WriteByte((uint32_t)DS90UB9x::DS90UB933SerializerI2CRegister::Gpio10, gpo10Config);
	std::this_thread::sleep_for(1ms);
	gpo10Config |= Gpo10ResetMask;
	serializer->WriteByte((uint32_t)DS90UB9x::DS90UB933SerializerI2CRegister::Gpio10, gpo10Config);
}

bool Neuropixels1e::updateSettings()
{
	NeuropixelsV1eBackgroundUpdater updater = NeuropixelsV1eBackgroundUpdater(this);

	return updater.updateSettings();
}

void Neuropixels1e::setSettings(ProbeSettings<NeuropixelsV1Values::numberOfChannels, NeuropixelsV1Values::numberOfElectrodes>* settings_, int index)
{
	if (index >= settings.size())
	{
		LOGE("Invalid index given when trying to update settings.");
		return;
	}

	settings[index]->updateProbeSettings(settings_);
}

std::vector<int> Neuropixels1e::selectElectrodeConfiguration(String config)
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

void Neuropixels1e::startAcquisition()
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

	// WONTFIX: Soft reset inside settings.WriteShiftRegisters() above puts probe in reset set that
	// needs to be undone here
	WriteByte((uint32_t)NeuropixelsV1Registers::OP_MODE, (uint32_t)NeuropixelsV1OperationRegisterValues::RECORD);
	WriteByte((uint32_t)NeuropixelsV1Registers::REC_MOD, (uint32_t)NeuropixelsV1RecordRegisterValues::ACTIVE);

	if (ledEnabled)
	{
		auto gpo23Config = DefaultGPO32Config & ~Gpo32LedMask;
		serializer->WriteByte((uint32_t)DS90UB9x::DS90UB933SerializerI2CRegister::Gpio32, gpo23Config);
	}

	superFrameCount = 0;
	ultraFrameCount = 0;
	shouldAddToBuffer = false;
	apSampleNumber = 0;
	lfpSampleNumber = 0;
}

void Neuropixels1e::stopAcquisition()
{
	serializer->WriteByte((uint32_t)DS90UB9x::DS90UB933SerializerI2CRegister::Gpio10, DefaultGPO10Config);
	serializer->WriteByte((uint32_t)DS90UB9x::DS90UB933SerializerI2CRegister::Gpio32, DefaultGPO32Config);

	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}
}

void Neuropixels1e::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
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

void Neuropixels1e::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void Neuropixels1e::processFrames()
{
	const float apConversion = (1171.875 / apGain) * -1.0f; // TODO: This is from 1f, is it needed here?
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
				int superCountOffset = superFrameCount % framesPerRoundRobin;
				if (superCountOffset == 0)
				{
					lfpTimestamps[ultraFrameCount] = apTimestamps[superFrameCount];
					lfpSampleNumbers[ultraFrameCount] = lfpSampleNumber++;
				}

				for (int adc = 0; adc < AdcsPerProbe; adc++)
				{
					// TODO: Use AP/LFP gain correction values here
					int chanIndex = adcToFrameIndex[adc] + superCountOffset * 2; // map the ADC to muxed channel
					lfpSamples[(chanIndex * numUltraFrames) + ultraFrameCount] =
						lfpConversion * float((*(dataPtr + adcToFrameIndex[adc] + dataOffset) >> 5) - 512) - lfpOffsets.at(chanIndex);
				}
			}
			else // AP data
			{
				int chanOffset = 2 * (i - 1);
				for (int adc = 0; adc < AdcsPerProbe; adc++)
				{
					int chanIndex = adcToFrameIndex[adc] + chanOffset; //  map the ADC to muxed channel.
					apSamples[(chanIndex * framesPerRoundRobin * numUltraFrames) + superFrameCount] =
						apConversion * float((*(dataPtr + rawToChannel[adc][i] + i * 36 + dataOffset) >> 5) - 512) - apOffsets.at(chanIndex);
				}
			}
		}

		oni_destroy_frame(frame);

		superFrameCount++;

		if (superFrameCount % framesPerRoundRobin == 0)
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
			apBuffer->addToBuffer(apSamples.data(), apSampleNumbers, apTimestamps, apEventCodes, numUltraFrames * framesPerRoundRobin);

			if (!lfpOffsetCalculated) updateLfpOffsets(lfpSamples, lfpSampleNumbers[0]);
			if (!apOffsetCalculated) updateApOffsets(apSamples, apSampleNumbers[0]);
		}
	}
}

void Neuropixels1e::updateApOffsets(std::array<float, numApSamples>& samples, int64 sampleNumber)
{
	if (sampleNumber > apSampleRate * secondsToSettle)
	{
		uint32_t counter = 0;

		while (apOffsetValues[0].size() < samplesToAverage)
		{
			if (counter >= framesPerRoundRobin * numUltraFrames) break;

			for (int i = 0; i < numberOfChannels; i++)
			{
				apOffsetValues[i].emplace_back(samples[i * framesPerRoundRobin * numUltraFrames + counter]);
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

void Neuropixels1e::updateLfpOffsets(std::array<float, numLfpSamples>& samples, int64 sampleNumber)
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

void Neuropixels1e::writeShiftRegisters(ShankBitset shankBits, ConfigBitsArray configBits, Array<NeuropixelsV1Adc> adcs)
{
	auto shankBytes = toBitReversedBytes<shankConfigurationBitCount>(shankBits);

	int rc = WriteByte((uint32_t)NeuropixelsV1ShiftRegisters::SR_LENGTH1, (uint32_t)shankBytes.size() % 0x100);
	if (rc != ONI_ESUCCESS) return;

	rc = WriteByte((uint32_t)NeuropixelsV1ShiftRegisters::SR_LENGTH2, (uint32_t)shankBytes.size() / 0x100);
	if (rc != ONI_ESUCCESS) return;

	for (auto b : shankBytes)
	{
		rc = WriteByte((uint32_t)NeuropixelsV1ShiftRegisters::SR_CHAIN1, b);
		if (rc != ONI_ESUCCESS) return;
	}

	const uint32_t shiftRegisterSuccess = 1 << 7;

	for (int i = 0; i < configBits.size(); i++)
	{
		auto srAddress = i == 0 ? (uint32_t)NeuropixelsV1ShiftRegisters::SR_CHAIN2 : (uint32_t)NeuropixelsV1ShiftRegisters::SR_CHAIN3;

		for (int j = 0; j < 2; j++)
		{
			// WONTFIX: Without this reset, the ShiftRegisterSuccess check below will always fail
			// on whatever the second shift register write sequence regardless of order or
			// contents. Could be increased current draw during internal process causes MCLK
			// to droop and mess up internal state. Or that MCLK is just not good enough to
			// prevent metastability in some logic in the ASIC that is only entered in between
			// SR accesses.
			WriteByte((uint32_t)NeuropixelsV1ShiftRegisters::SOFT_RESET, 0xFF);
			WriteByte((uint32_t)NeuropixelsV1ShiftRegisters::SOFT_RESET, 0x00);

			auto baseBytes = toBitReversedBytes<BaseConfigurationBitCount>(configBits[i]);

			rc = WriteByte((uint32_t)NeuropixelsV1ShiftRegisters::SR_LENGTH1, (uint32_t)baseBytes.size() % 0x100);
			if (rc != ONI_ESUCCESS) return;

			rc = WriteByte((uint32_t)NeuropixelsV1ShiftRegisters::SR_LENGTH2, (uint32_t)baseBytes.size() / 0x100);
			if (rc != ONI_ESUCCESS) return;

			for (auto b : baseBytes)
			{
				rc = WriteByte(srAddress, b);
				if (rc != ONI_ESUCCESS) return;
			}
		}

		oni_reg_val_t value;
		rc = ReadByte((uint32_t)NeuropixelsV1Registers::STATUS, &value);

		if (rc != ONI_ESUCCESS || value != shiftRegisterSuccess)
		{
			LOGE("Shift register ", srAddress, " status check failed.");
			return;
		}
	}
}

void Neuropixels1e::defineMetadata(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings)
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

uint64_t Neuropixels1e::getProbeSerialNumber(int index)
{
	return probeNumber;
}

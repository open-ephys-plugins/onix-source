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

#include "Neuropixels_1.h"
#include "../OnixSource.h"

BackgroundUpdaterWithProgressWindow::BackgroundUpdaterWithProgressWindow(Neuropixels_1* d)
	: ThreadWithProgressWindow("Writing calibration files to Neuropixels Probe: " + d->getName(), true, false)
{
	device = d;
}

BackgroundUpdaterWithProgressWindow::~BackgroundUpdaterWithProgressWindow()
{
}

int BackgroundUpdaterWithProgressWindow::updateSettings()
{
	runThread();

	return result;
}

void BackgroundUpdaterWithProgressWindow::run()
{
	setProgress(0);

	// Parse ADC and Gain calibration files
	String adcPath = device->getAdcPathParameter();
	String gainPath = device->getGainPathParameter();

	if (adcPath == "None" || gainPath == "None")
	{
		result = -3;
		return;
	}

	File adcFile = File(adcPath);
	File gainFile = File(gainPath);

	if (!adcFile.existsAsFile() || !gainFile.existsAsFile())
	{
		result = -3;
		return;
	}

	setProgress(0.2);

	StringArray gainFileLines;
	gainFile.readLines(gainFileLines);

	auto gainSN = std::stoull(gainFileLines[0].toStdString());

	LOGD("Gain calibration file SN = ", gainSN);

	if (gainSN != device->getProbeNumber()) 
	{ 
		LOGE("Gain calibration serial number (", gainSN, ") does not match probe serial number (", device->getProbeNumber(), ").");
		result = -4;
		return;
	}

	StringRef gainCalLine = gainFileLines[1];
	StringRef breakCharacters = ",";
	StringRef noQuote = "";

	StringArray calibrationValues = StringArray::fromTokens(gainCalLine, breakCharacters, noQuote);

	double apGainCorrection = std::stod(calibrationValues[device->settings.apGainIndex + 1].toStdString());
	double lfpGainCorrection = std::stod(calibrationValues[device->settings.lfpGainIndex + 8].toStdString());

	LOGD("AP gain correction = ", apGainCorrection, ", LFP gain correction = ", lfpGainCorrection);

	setProgress(0.5);

	StringArray adcFileLines;
	adcFile.readLines(adcFileLines);

	auto adcSN = std::stoull(adcFileLines[0].toStdString());

	LOGD("ADC calibration file SN = ", adcSN);

	if (adcSN != device->getProbeNumber()) 
	{ 
		LOGE("ADC calibration serial number (", adcSN, ") does not match probe serial number (", device->getProbeNumber(), ").");
		result = -4;
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
	auto shankBits = device->makeShankBits(device->getReference(device->settings.referenceIndex), device->settings.selectedChannel);
	auto configBits = device->makeConfigBits(device->getReference(device->settings.referenceIndex), device->getGainEnum(device->settings.apGainIndex), device->getGainEnum(device->settings.lfpGainIndex), true, adcs);

	device->writeShiftRegisters(shankBits, configBits, adcs, lfpGainCorrection, apGainCorrection);

	setProgress(1);

	result = 0;
}

Neuropixels_1::Neuropixels_1(String name, OnixSource* s, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_) :
	OnixDevice(name, OnixDeviceType::NEUROPIXELS_1, deviceIdx_, ctx_),
	I2CRegisterContext(ProbeI2CAddress, deviceIdx_, ctx_),
	source(s)
{
	StreamInfo apStream;
	apStream.name = name + "-AP";
	apStream.description = "Neuropixels 1.0 AP band data stream";
	apStream.identifier = "onix-neuropixels1.data.ap";
	apStream.numChannels = 384;
	apStream.sampleRate = 30000.0f;
	apStream.channelPrefix = "AP";
	apStream.bitVolts = 0.195f;
	apStream.channelType = ContinuousChannel::Type::ELECTRODE;
	streams.add(apStream);

	StreamInfo lfpStream;
	lfpStream.name = name + "-LFP";
	lfpStream.description = "Neuropixels 1.0 LFP band data stream";
	lfpStream.identifier = "onix-neuropixels1.data.lfp";
	lfpStream.numChannels = 384;
	lfpStream.sampleRate = 2500.0f;
	lfpStream.channelPrefix = "LFP";
	lfpStream.bitVolts = 0.195f;
	lfpStream.channelType = ContinuousChannel::Type::ELECTRODE;
	streams.add(lfpStream);

	defineMetadata(settings);

	// Set parameters
	StringArray validExtensions = StringArray();
	validExtensions.add("csv");
	//validExtensions.add("*_ADCCalibration.csv");

	source->addPathParameter(Parameter::PROCESSOR_SCOPE, getAdcPathParameterName(), "ADC Calibration File", "Path to the ADC calibration file for this Neuropixels probe",
		"", validExtensions, false, false, true);
	source->getParameter(getAdcPathParameterName())->setNextValue(File::getSpecialLocation(File::userHomeDirectory).getFullPathName());

	//validExtensions.set(0, "*_gainCalValues.csv");

	source->addPathParameter(Parameter::PROCESSOR_SCOPE, getGainPathParameterName(), "Gain Calibration File", "Path to the gain calibration file for this Neuropixels probe",
		"", validExtensions, false, false, true);
	source->getParameter(getGainPathParameterName())->setNextValue(File::getSpecialLocation(File::userHomeDirectory).getFullPathName());
}

Neuropixels_1::~Neuropixels_1()
{
}

String Neuropixels_1::getAdcPathParameter()
{
	return source->getParameter(getAdcPathParameterName())->getValue();
}

String Neuropixels_1::getGainPathParameter()
{
	return source->getParameter(getGainPathParameterName())->getValue();
}

String Neuropixels_1::getAdcPathParameterName()
{
	return "adcCalibrationFile_" + getName();
}

String Neuropixels_1::getGainPathParameterName()
{
	return "gainCalibrationFile_" + getName();
}

NeuropixelsGain Neuropixels_1::getGainEnum(int index)
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

int Neuropixels_1::getGainValue(NeuropixelsGain gain)
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

NeuropixelsReference Neuropixels_1::getReference(int index)
{
	switch (index)
	{
	case 0:
		return NeuropixelsReference::External;
	case 1:
		return NeuropixelsReference::Tip;
	default:
		break;
	}

	return NeuropixelsReference::External;
}

int Neuropixels_1::enableDevice()
{
	// Get Probe SN
	uint32_t eepromOffset = 0;
	uint32_t i2cAddr = 0x50;
	int errorCode = 0;

	for (int i = 0; i < 8; i++)
	{
		oni_reg_addr_t reg_addr = ((eepromOffset + i) << 7) | i2cAddr;
		oni_reg_val_t reg_val;

		errorCode = oni_read_reg(ctx, deviceIdx, reg_addr, &reg_val);

		if (errorCode) { LOGE(oni_error_str(errorCode)); return -1; }

		if (reg_val <= 0xFF)
		{
			probeNumber |= (((uint64_t)reg_val) << (i * 8));
		}
	}

	LOGD("Probe SN: ", probeNumber);

	// Enable device streaming
	const oni_reg_addr_t enable_device_stream = 0x8000;
	errorCode = oni_write_reg(ctx, deviceIdx, enable_device_stream, 1);

	if (errorCode) { LOGE(oni_error_str(errorCode)); return -2; }

	if (WriteByte((uint32_t)NeuropixelsRegisters::CAL_MOD, (uint32_t)CalMode::CAL_OFF) != 0) return -4;
	if (WriteByte((uint32_t)NeuropixelsRegisters::SYNC, (uint32_t)0) != 0) return -4;

	if (WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::DIG_AND_CH_RESET) != 0) return -4;

	if (WriteByte((uint32_t)NeuropixelsRegisters::OP_MODE, (uint32_t)OpMode::RECORD) != 0) return -4;

	// Update all probe settings
	return updateSettings();
}

int Neuropixels_1::updateSettings()
{
	BackgroundUpdaterWithProgressWindow updater = BackgroundUpdaterWithProgressWindow(this);

	return updater.updateSettings();
}

Array<int> Neuropixels_1::selectElectrodeConfiguration(String config)
{
	Array<int> selection;

	if (config.equalsIgnoreCase("Bank A"))
	{
		for (int i = 0; i < 384; i++)
			selection.add(i);
	}
	else if (config.equalsIgnoreCase("Bank B"))
	{
		for (int i = 384; i < 768; i++)
			selection.add(i);
	}
	else if (config.equalsIgnoreCase("Bank C"))
	{
		for (int i = 576; i < 960; i++)
			selection.add(i);
	}
	else if (config.equalsIgnoreCase("Single Column"))
	{
		for (int i = 0; i < 384; i += 2)
			selection.add(i);

		for (int i = 385; i < 768; i += 2)
			selection.add(i);
	}
	else if (config.equalsIgnoreCase("Tetrodes"))
	{
		for (int i = 0; i < 384; i += 8)
		{
			for (int j = 0; j < 4; j++)
				selection.add(i + j);
		}

		for (int i = 388; i < 768; i += 8)
		{
			for (int j = 0; j < 4; j++)
				selection.add(i + j);
		}
	}

	return selection;
}

void Neuropixels_1::startAcquisition()
{
	apGain = getGainValue(getGainEnum(settings.apGainIndex));
	lfpGain = getGainValue(getGainEnum(settings.lfpGainIndex));

	WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::ACTIVE);
}

void Neuropixels_1::stopAcquisition()
{
	WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::RESET_ALL);

	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}

	superFrameCount = 0;
	ultraFrameCount = 0;
	shouldAddToBuffer = false;
	apSampleNumber = 0;
	lfpSampleNumber = 0;
}

void Neuropixels_1::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	for (StreamInfo streamInfo : streams)
	{
		sourceBuffers.add(new DataBuffer(streamInfo.numChannels, (int)streamInfo.sampleRate * bufferSizeInSeconds));

		if (streamInfo.channelPrefix.equalsIgnoreCase("AP"))
			apBuffer = sourceBuffers.getLast();
		else if (streamInfo.channelPrefix.equalsIgnoreCase("LFP"))
			lfpBuffer = sourceBuffers.getLast();
	}
}

void Neuropixels_1::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void Neuropixels_1::processFrames()
{
	float apConversion = 1171.875 / apGain;
	float lfpConversion = 1171.875 / lfpGain;

	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		uint16_t* dataPtr;
		dataPtr = (uint16_t*)frame->data;

		auto dataclock = (unsigned char*)frame->data + 936;
		uint64 hubClock = ((uint64_t)(*(uint16_t*)dataclock) << 48) |
			((uint64_t)(*(uint16_t*)(dataclock + 2)) << 32) |
			((uint64_t)(*(uint16_t*)(dataclock + 4)) << 16) |
			((uint64_t)(*(uint16_t*)(dataclock + 6)) << 0);
		int64_t clockCounter = hubClock;
		apTimestamps[superFrameCount] = clockCounter;
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
						lfpConversion * float((*(dataPtr + adcToFrameIndex[adc] + dataOffset) >> 5) - 512);
				}
			}
			else // AP data
			{
				int chanOffset = 2 * (i - 1);
				for (int adc = 0; adc < 32; adc++)
				{
					int chanIndex = adcToChannel[adc] + chanOffset; //  map the ADC to muxed channel.
					apSamples[(chanIndex * superFramesPerUltraFrame * numUltraFrames) + superFrameCount] =
						apConversion * float((*(dataPtr + adcToFrameIndex[adc] + i * 36 + dataOffset) >> 5) - 512);
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
			lfpBuffer->addToBuffer(lfpSamples, lfpSampleNumbers, lfpTimestamps, lfpEventCodes, numUltraFrames);
			apBuffer->addToBuffer(apSamples, apSampleNumbers, apTimestamps, apEventCodes, numUltraFrames * superFramesPerUltraFrame);
		}
	}
}

std::bitset<Neuropixels_1::shankConfigurationBitCount> Neuropixels_1::makeShankBits(NeuropixelsReference reference, Array<int> channelMap)
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
	case NeuropixelsReference::External:
		shankBits[shankBitExt1] = true;
		shankBits[shankBitExt2] = true;
		break;
	case NeuropixelsReference::Tip:
		shankBits[shankBitTip1] = true;
		shankBits[shankBitTip2] = true;
		break;
	default:
		break;
	}

	return shankBits;
}

std::vector<std::bitset<Neuropixels_1::BaseConfigurationBitCount>> Neuropixels_1::makeConfigBits(NeuropixelsReference reference, NeuropixelsGain spikeAmplifierGain, NeuropixelsGain lfpAmplifierGain, bool spikeFilterEnabled, Array<NeuropixelsV1Adc> adcs)
{
	const int BaseConfigurationConfigOffset = 576;

	std::vector<std::bitset<BaseConfigurationBitCount>> baseConfigs(2);

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

template<int N> std::vector<unsigned char> Neuropixels_1::toBitReversedBytes(std::bitset<N> bits)
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

void Neuropixels_1::writeShiftRegisters(std::bitset<shankConfigurationBitCount> shankBits, std::vector<std::bitset<BaseConfigurationBitCount>> configBits, Array<NeuropixelsV1Adc> adcs, double lfpGainCorrection, double apGainCorrection)
{
	auto shankBytes = toBitReversedBytes<shankConfigurationBitCount>(shankBits);

	if (WriteByte((uint32_t)ShiftRegisters::SR_LENGTH1, (uint32_t)shankBytes.size() % 0x100) != 0) return;
	if (WriteByte((uint32_t)ShiftRegisters::SR_LENGTH2, (uint32_t)shankBytes.size() / 0x100) != 0) return;

	for (auto b : shankBytes)
	{
		if (WriteByte((uint32_t)ShiftRegisters::SR_CHAIN1, b) != 0) return;
	}

	const uint32_t shiftRegisterSuccess = 1 << 7;

	for (int i = 0; i < configBits.size(); i++)
	{
		auto srAddress = i == 0 ? (uint32_t)ShiftRegisters::SR_CHAIN2 : (uint32_t)ShiftRegisters::SR_CHAIN3;

		for (int j = 0; j < 2; j++)
		{
			auto baseBytes = toBitReversedBytes<BaseConfigurationBitCount>(configBits[i]);

			if (WriteByte((uint32_t)ShiftRegisters::SR_LENGTH1, (uint32_t)baseBytes.size() % 0x100) != 0) return;
			if (WriteByte((uint32_t)ShiftRegisters::SR_LENGTH2, (uint32_t)baseBytes.size() / 0x100) != 0) return;

			for (auto b : baseBytes)
			{
				if (WriteByte(srAddress, b) != 0) return;
			}
		}

		oni_reg_val_t value;

		if (ReadByte((uint32_t)NeuropixelsRegisters::STATUS, &value) != 0 || value != shiftRegisterSuccess)
		{
			LOGE("Shift register ", srAddress, " status check failed.");
			return;
		}
	}

	const uint32_t ADC01_00_OFF_THRESH = 0x8001;

	for (uint32_t i = 0; i < adcs.size(); i += 2)
	{
		if (oni_write_reg(ctx, deviceIdx, ADC01_00_OFF_THRESH + i, (uint32_t)(adcs[i + 1].offset << 26 | adcs[i + 1].threshold << 16 | adcs[i].offset << 10 | adcs[i].threshold)) != 0)
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
		if (oni_write_reg(ctx, deviceIdx, CHAN001_000_LFPGAIN + i, fixedPointLfPGain << 16 | fixedPointLfPGain) != 0)
		{
			LOGE("Error writing to register ", CHAN001_000_LFPGAIN + i, ".");
			return;
		}

		if (oni_write_reg(ctx, deviceIdx, CHAN001_000_APGAIN + i, fixedPointApGain << 16 | fixedPointApGain) != 0)
		{
			LOGE("Error writing to register ", CHAN001_000_APGAIN + i, ".");
			return;
		}
	}
}

void Neuropixels_1::defineMetadata(ProbeSettings& settings)
{
	settings.probeType = ProbeType::NPX_V1E;
	settings.probeMetadata.name = "Neuropixels 1.0e";

	Path path;
	path.startNewSubPath(27, 31);
	path.lineTo(27, 514);
	path.lineTo(27 + 5, 522);
	path.lineTo(27 + 10, 514);
	path.lineTo(27 + 10, 31);
	path.closeSubPath();

	settings.probeMetadata.shank_count = 1;
	settings.probeMetadata.electrodes_per_shank = 960;
	settings.probeMetadata.rows_per_shank = 960 / 2;
	settings.probeMetadata.columns_per_shank = 2;
	settings.probeMetadata.shankOutline = path;
	settings.probeMetadata.num_adcs = 32; // NB: Is this right for 1.0e?
	settings.probeMetadata.adc_bits = 10; // NB: Is this right for 1.0e?

	settings.availableBanks = {
		Bank::A,
		Bank::B,
		Bank::C,
		Bank::NONE //disconnected
	};

	Array<float> xpositions = { 27.0f, 59.0f, 11.0f, 43.0f };

	for (int i = 0; i < settings.probeMetadata.electrodes_per_shank * settings.probeMetadata.shank_count; i++)
	{
		ElectrodeMetadata metadata;

		metadata.shank = 0;
		metadata.shank_local_index = i % settings.probeMetadata.electrodes_per_shank;
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

		settings.electrodeMetadata.add(metadata);
	}

	settings.apGainIndex = 4;	// NB:  AP Gain Index of 4 = Gain1000
	settings.lfpGainIndex = 0;	// NB: LFP Gain Index of 0 = Gain50
	settings.referenceIndex = 0;
	settings.apFilterState = true;

	for (int i = 0; i < numberOfChannels; i++)
	{
		settings.selectedBank.add(Bank::A);
		settings.selectedChannel.add(i);
		settings.selectedShank.add(0);
		settings.selectedElectrode.add(i);
	}

	settings.availableApGains.add(50.0f);
	settings.availableApGains.add(125.0f);
	settings.availableApGains.add(250.0f);
	settings.availableApGains.add(500.0f);
	settings.availableApGains.add(1000.0f);
	settings.availableApGains.add(1500.0f);
	settings.availableApGains.add(2000.0f);
	settings.availableApGains.add(3000.0f);

	settings.availableLfpGains.add(50.0f);
	settings.availableLfpGains.add(125.0f);
	settings.availableLfpGains.add(250.0f);
	settings.availableLfpGains.add(500.0f);
	settings.availableLfpGains.add(1000.0f);
	settings.availableLfpGains.add(1500.0f);
	settings.availableLfpGains.add(2000.0f);
	settings.availableLfpGains.add(3000.0f);

	settings.availableReferences.add("Ext");
	settings.availableReferences.add("Tip");

	settings.availableElectrodeConfigurations.add("Bank A");
	settings.availableElectrodeConfigurations.add("Bank B");
	settings.availableElectrodeConfigurations.add("Bank C");
	settings.availableElectrodeConfigurations.add("Single column");
	settings.availableElectrodeConfigurations.add("Tetrodes");

	settings.electrodeConfigurationIndex = 0;
}

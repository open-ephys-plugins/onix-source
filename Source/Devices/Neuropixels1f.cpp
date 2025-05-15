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

NeuropixelsV1fBackgroundUpdater::NeuropixelsV1fBackgroundUpdater(Neuropixels1f* d)
	: NeuropixelsV1BackgroundUpdater(d)
{
}

void NeuropixelsV1fBackgroundUpdater::run()
{
	setProgress(0);

	if (!device->parseGainCalibrationFile())
	{
		result = false;
		return;
	}

	if (!device->parseAdcCalibrationFile())
	{
		result = false;
		return;
	}

	setProgress(0.5);

	try
	{
		device->writeShiftRegisters();
	}
	catch (const error_str& e)
	{
		Onix1::showWarningMessageBoxAsync("Error Writing Shift Registers", e.what());
		result = false;
		return;
	}

	setProgress(1);

	result = true;
}

Neuropixels1f::Neuropixels1f(std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx_) :
	Neuropixels1(name, hubName, OnixDeviceType::NEUROPIXELSV1F, deviceIdx_, ctx_)
{
	std::string port = getPortNameFromIndex(deviceIdx);
	StreamInfo apStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHubName(), getName(), "AP" }),
		"Neuropixels 1.0 AP band data stream",
		getStreamIdentifier(),
		numberOfChannels,
		apSampleRate,
		"AP",
		ContinuousChannel::Type::ELECTRODE,
		0.195f,
		"µV",
		{},
		"ap"
  );
	streamInfos.add(apStream);

	StreamInfo lfpStream = StreamInfo(
		OnixDevice::createStreamName({ port, getHubName(), getName(), "LFP" }),
		"Neuropixels 1.0 LFP band data stream",
		getStreamIdentifier(),
		numberOfChannels,
		lfpSampleRate,
		"LFP",
		ContinuousChannel::Type::ELECTRODE,
		0.195f,
		"µV",
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

	probeNumber = 0;
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

	rc = WriteByte((uint32_t)NeuropixelsV1Registers::CAL_MOD, (uint32_t)NeuropixelsV1CalibrationRegisterValues::CAL_OFF);
	if (rc != ONI_ESUCCESS)
		throw error_str("Error configuring device at address " + std::to_string(deviceIdx));

	rc = WriteByte((uint32_t)NeuropixelsV1Registers::SYNC, (uint32_t)0);
	if (rc != ONI_ESUCCESS)
		throw error_str("Error configuring device at address " + std::to_string(deviceIdx));

	rc = WriteByte((uint32_t)NeuropixelsV1Registers::REC_MOD, (uint32_t)NeuropixelsV1RecordRegisterValues::DIG_AND_CH_RESET);
	if (rc != ONI_ESUCCESS)
		throw error_str("Error configuring device at address " + std::to_string(deviceIdx));

	rc = WriteByte((uint32_t)NeuropixelsV1Registers::OP_MODE, (uint32_t)NeuropixelsV1OperationRegisterValues::RECORD);
	if (rc != ONI_ESUCCESS)
		throw error_str("Error configuring device at address " + std::to_string(deviceIdx));

	return rc;
}

bool Neuropixels1f::updateSettings()
{
	auto updater = NeuropixelsV1fBackgroundUpdater(this);

	return updater.updateSettings() && adcValues.size() == NeuropixelsV1Values::AdcCount;
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

	WriteByte((uint32_t)NeuropixelsV1Registers::REC_MOD, (uint32_t)NeuropixelsV1RecordRegisterValues::ACTIVE);

	superFrameCount = 0;
	ultraFrameCount = 0;
	apSampleNumber = 0;
	lfpSampleNumber = 0;
}

void Neuropixels1f::stopAcquisition()
{
	WriteByte((uint32_t)NeuropixelsV1Registers::REC_MOD, (uint32_t)NeuropixelsV1RecordRegisterValues::RESET_ALL);

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

		if (streamInfo.getChannelPrefix() == "AP")
			apBuffer = sourceBuffers.getLast();
		else if (streamInfo.getChannelPrefix() == "LFP")
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
				size_t superCountOffset = superFrameCount % superFramesPerUltraFrame;
				if (superCountOffset == 0)
				{
					lfpTimestamps[ultraFrameCount] = apTimestamps[superFrameCount];
					lfpSampleNumbers[ultraFrameCount] = lfpSampleNumber++;
				}

				for (int adc = 0; adc < 32; adc++)
				{
					size_t chanIndex = adcToChannel[adc] + superCountOffset * 2; // map the ADC to muxed channel
					lfpSamples[(chanIndex * numUltraFrames) + ultraFrameCount] =
						lfpConversion * float((*(dataPtr + adcToFrameIndex[adc] + dataOffset) >> 5) - 512) - lfpOffsets.at(chanIndex); // TODO: Is this overflowing the unsigned integer?
				}
			}
			else // AP data
			{
				int chanOffset = 2 * (i - 1);
				for (int adc = 0; adc < 32; adc++)
				{
					size_t chanIndex = adcToChannel[adc] + chanOffset; //  map the ADC to muxed channel.
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

			lfpBuffer->addToBuffer(lfpSamples.data(), lfpSampleNumbers, lfpTimestamps, lfpEventCodes, numUltraFrames);
			apBuffer->addToBuffer(apSamples.data(), apSampleNumbers, apTimestamps, apEventCodes, numUltraFrames * superFramesPerUltraFrame);

			if (!lfpOffsetCalculated) updateLfpOffsets(lfpSamples, lfpSampleNumbers[0]);
			if (!apOffsetCalculated) updateApOffsets(apSamples, apSampleNumbers[0]);
		}
	}
}

void Neuropixels1f::writeShiftRegisters()
{
	if (adcValues.size() != NeuropixelsV1Values::AdcCount)
		throw error_str("Invalid number of ADC values found.");

	auto shankBits = NeuropixelsV1::makeShankBits(getReference(settings[0]->referenceIndex), settings[0]->selectedElectrode);
	auto configBits = NeuropixelsV1::makeConfigBits(getReference(settings[0]->referenceIndex), getGainEnum(settings[0]->apGainIndex), getGainEnum(settings[0]->lfpGainIndex), true, adcValues);

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

	const uint32_t ADC01_00_OFF_THRESH = 0x8001;

	for (size_t i = 0; i < adcValues.size(); i += 2)
	{
		auto value = (uint32_t)(adcValues.at(i + 1).offset << 26 | adcValues.at(i + 1).threshold << 16 | adcValues.at(i).offset << 10 | adcValues.at(i).threshold);
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

OnixDeviceType Neuropixels1f::getDeviceType()
{
	return OnixDeviceType::NEUROPIXELSV1F;
}

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
#include <oni.h>
#include <onix.h>

using namespace Onix;

Neuropixels_1::Neuropixels_1(String name, float portVoltage, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_)
	: OnixDevice(name, NEUROPIXELS_1, deviceIdx_, ctx_), I2CRegisterContext(ProbeI2CAddress, deviceIdx_, ctx_)
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

	if (portVoltage >= minVoltage && portVoltage <= maxVoltage)
		portVoltage_ = portVoltage;
}

Neuropixels_1::~Neuropixels_1()
{

}

int Neuropixels_1::enableDevice()
{
	int result = checkLinkState((oni_dev_idx_t)PortName::PortA);

	if (result != 0) return result;

	// Get Probe SN
	uint32_t eepromOffset = 0;
	uint32_t i2cAddr = 0x50;
	uint64_t probeSN = 0;
	int errorCode = 0;

	for (int i = 0; i < 8; i++)
	{
		oni_reg_addr_t reg_addr = ((eepromOffset + i) << 7) | i2cAddr;
		oni_reg_val_t reg_val;

		errorCode = oni_read_reg(ctx, deviceIdx, reg_addr, &reg_val);

		if (errorCode) { LOGE(oni_error_str(errorCode)); return -1; }

		if (reg_val <= 0xFF)
		{
			probeSN |= (((uint64_t)reg_val) << (i * 8));
		}
	}
	
	LOGD ("Probe SN: ", probeSN);

	// Enable device streaming
	const oni_reg_addr_t enable_device_stream = 0x8000;
	errorCode = oni_write_reg(ctx, deviceIdx, enable_device_stream, 1);

	if (errorCode) { LOGE(oni_error_str(errorCode)); return -2; }

	WriteByte((uint32_t)NeuropixelsRegisters::CAL_MOD, (uint32_t)CalMode::CAL_OFF);
	WriteByte((uint32_t)NeuropixelsRegisters::SYNC, (uint32_t)0);

	WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::DIG_AND_CH_RESET);

	WriteByte((uint32_t)NeuropixelsRegisters::OP_MODE, (uint32_t)OpMode::RECORD);

	return 0;
}

void Neuropixels_1::startAcquisition()
{
	startThread();

	WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::ACTIVE);
}

void Neuropixels_1::stopAcquisition()
{
	if (isThreadRunning())
        signalThreadShouldExit();
    
    waitForThreadToExit(2000);

	WriteByte((uint32_t)NeuropixelsRegisters::REC_MOD, (uint32_t)RecMod::RESET_ALL);

	superFrameCount = 0;
	ultraFrameCount = 0;
	shouldAddToBuffer = false;
	apSampleNumber = 0;
	lfpSampleNumber = 0;
}

void Neuropixels_1::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void Neuropixels_1::run()
{
	while (!threadShouldExit())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());

		if (!frameArray.isEmpty())
		{
			oni_frame_t* frame = frameArray.removeAndReturn(0);

			uint16_t* dataPtr;
			dataPtr = (uint16_t*)frame->data;

			auto dataclock = (unsigned char*)frame->data + 936;
			uint64 hubClock = ((uint64_t)(*(uint16_t*)dataclock) << 48) |
				((uint64_t)(*(uint16_t*)(dataclock + 2)) << 32) |
				((uint64_t)(*(uint16_t*)(dataclock + 4)) << 16) |
				((uint64_t)(*(uint16_t*)(dataclock + 6)) << 0);
			int64_t clockCounter = hubClock * sizeof(hubClock);
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
							float(*(dataPtr + adcToFrameIndex[adc] + dataOffset) >> 5) * 0.195f;
					}
				}
				else // AP data
				{
					int chanOffset = 2 * (i - 1);
					for (int adc = 0; adc < 32; adc++)
					{
						int chanIndex = adcToChannel[adc] + chanOffset; //  map the ADC to muxed channel.
						apSamples[(chanIndex * superFramesPerUltraFrame * numUltraFrames) + superFrameCount] =
							float(*(dataPtr + adcToFrameIndex[adc] + dataOffset) >> 5) * 0.195f;
					}
				}
			}

			oni_destroy_frame(frame);

			superFrameCount++;

			if (superFrameCount % superFramesPerUltraFrame == 0)
			{
				ultraFrameCount++;
			}

			if (ultraFrameCount > numUltraFrames)
			{
				ultraFrameCount = 0;
				superFrameCount = 0;
				shouldAddToBuffer = true;
			}
		}

		if (shouldAddToBuffer)
		{
			shouldAddToBuffer = false;
			lfpBuffer->addToBuffer(lfpSamples, lfpSampleNumbers, lfpTimestamps, lfpEventCodes, numUltraFrames);
			apBuffer->addToBuffer(apSamples, apSampleNumbers, apTimestamps, apEventCodes, numUltraFrames * superFramesPerUltraFrame);
		}
	}
}
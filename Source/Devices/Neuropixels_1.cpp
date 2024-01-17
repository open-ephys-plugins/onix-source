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

Neuropixels_1::Neuropixels_1(String name, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_)
	: OnixDevice(name, NEUROPIXELS_1, deviceIdx_, ctx_)
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
}


Neuropixels_1::~Neuropixels_1()
{

}


int Neuropixels_1::enableDevice()
{
	// Enable device streaming
	const oni_reg_addr_t enable_device_stream = 0x8000;
	int errorCode = oni_write_reg(ctx, deviceIdx, enable_device_stream, 1);

	if (errorCode) { LOGE(oni_error_str(errorCode)); return errorCode; }

	// check probe SN


	const oni_reg_addr_t probe_sn_msb = 0x8191;
	const oni_reg_addr_t probe_sn_lsb = 0x8192;

	oni_reg_val_t probe_sn_msb_val;
	oni_reg_val_t probe_sn_lsb_val;

	errorCode =  oni_read_reg(ctx, deviceIdx, probe_sn_msb, &probe_sn_msb_val);
	if (errorCode) { LOGE(oni_error_str(errorCode)); return errorCode; }

	errorCode = oni_read_reg(ctx, deviceIdx, probe_sn_lsb, &probe_sn_lsb_val);
	if (errorCode) { LOGE(oni_error_str(errorCode)); return errorCode; }

	uint64_t probe_sn = ((uint64_t)probe_sn_msb_val << 32) | (uint64_t)probe_sn_lsb_val;

	LOGC ("******* Probe SN: ", probe_sn);

	return 0;
}


void Neuropixels_1::startAcquisition()
{
	startThread();

	int errorCode = oni_write_reg(ctx, deviceIdx, (uint32_t)NeuropixelsRegisters::REC_MOD , (uint32_t)RecMod::DIG_RESET);

	if (errorCode) { LOGE("[", deviceIdx ,"][Register][OP_MODE] ", oni_error_str(errorCode)); }

	errorCode = oni_write_reg(ctx, deviceIdx, (uint32_t)NeuropixelsRegisters::REC_MOD , (uint32_t)RecMod::ACTIVE);

	if (errorCode) { LOGE("[Neuropixels 1][", deviceIdx ,"] Error starting acquisition: ", oni_error_str(errorCode)); }
}

void Neuropixels_1::stopAcquisition()
{
	if (isThreadRunning())
        signalThreadShouldExit();
    
    waitForThreadToExit(2000);

	int errorCode = oni_write_reg(ctx, deviceIdx, (uint32_t)NeuropixelsRegisters::REC_MOD , (uint32_t)RecMod::RESET_ALL);
	if (errorCode) { LOGE("[Neuropixels 1][", deviceIdx ,"] Error stopping acquisition: ", oni_error_str(errorCode)); }

	superFrameCount = 0;
	ultraFrameCount = 0;
	shouldAddToBuffer = false;
	sampleNumber = 0;
}


void Neuropixels_1::addFrame(oni_frame_t* frame)
{
	
	uint16_t* dataPtr;
	dataPtr = (uint16_t*)frame->data;

	auto dataclock = (unsigned char*)frame->data + 936; 
	uint64 hubClock = ((uint64_t)(*(uint16_t*)dataclock) << 48) |
						((uint64_t)(*(uint16_t*)(dataclock + 2)) << 32) |
						((uint64_t)(*(uint16_t*)(dataclock + 4)) << 16) |
						((uint64_t)(*(uint16_t*)(dataclock + 6)) << 0);
    int64_t clockCounter = hubClock * sizeof(hubClock);
	apTimestamps[superFrameCount] = clockCounter;
	apSampleNumbers[superFrameCount] = sampleNumber++;

	for (int i = 0; i < framesPerSuperFrame; i++)
	{
		if (i == 0) // LFP data
		{
			int superCountOffset = superFrameCount % superFramesPerUltraFrame; 
			if (superCountOffset == 0)
			{
				lfpTimestamps[ultraFrameCount] = apTimestamps[superFrameCount];
				lfpSampleNumbers[ultraFrameCount] = apSampleNumbers[superFrameCount];
			}

			for (int adc = 0; adc < 32; adc++)
			{
				
				int chanIndex = adcToChannel[adc] + superCountOffset * 2;
				lfpSamples[(chanIndex * numUltraFrames) + ultraFrameCount ]   = (*(dataPtr + adcToFrameIndex[adc] + dataOffset) >> 5) * 0.195f;
				
			}
		}
		else // AP data
		{
			int chanOffset = 2 * (i - 1);
			for (int adc = 0; adc < 32; adc++)
			{
				int chanIndex = adcToChannel[adc] + chanOffset;
				apSamples[(chanIndex * superFramesPerUltraFrame * numUltraFrames) + superFrameCount] = (*(dataPtr + adcToFrameIndex[adc] + dataOffset) >> 5) * 0.195f;
			}
		}
	}

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

void Neuropixels_1::run()
{
	while (!threadShouldExit())
	{
		if (shouldAddToBuffer)
		{
			shouldAddToBuffer = false;
			lfpBuffer->addToBuffer(lfpSamples, lfpSampleNumbers, lfpTimestamps, lfpEventCodes, numUltraFrames);
			apBuffer->addToBuffer(apSamples, apSampleNumbers, apTimestamps, apEventCodes, numUltraFrames * superFramesPerUltraFrame);
		}
		
	}
}
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

#include "HarpSyncInput.h"

HarpSyncInput::HarpSyncInput(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, BREAKOUT_BOARD_NAME, OnixDeviceType::HARPSYNCINPUT, deviceIdx_, oni_ctx)
{
	setEnabled(false);

	StreamInfo harpTimeStream = StreamInfo(
		OnixDevice::createStreamName({ getHeadstageName(), getName(), "HarpTime" }),
		"Harp clock time corresponding to the local acquisition ONIX clock count",
		"onix-harpsyncinput.data.harptime",
		1,
		1,
		"HarpTime",
		ContinuousChannel::Type::AUX,
		1.0f,
		"s",
		{""});
	streamInfos.add(harpTimeStream);

	for (int i = 0; i < numFrames; i++)
		eventCodes[i] = 0;
}

int HarpSyncInput::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -1;

	return deviceContext->writeRegister(deviceIdx, (uint32_t)HarpSyncInputRegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0));
}

bool HarpSyncInput::updateSettings()
{
	return deviceContext->writeRegister(deviceIdx, (oni_reg_addr_t)HarpSyncInputRegisters::SOURCE, (oni_reg_val_t)HarpSyncSource::Breakout);
}

void HarpSyncInput::startAcquisition()
{
	currentFrame = 0;
	sampleNumber = 0;
}

void HarpSyncInput::stopAcquisition()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}
}

void HarpSyncInput::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void HarpSyncInput::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	for (StreamInfo streamInfo : streamInfos)
	{
		sourceBuffers.add(new DataBuffer(streamInfo.getNumChannels(), (int)streamInfo.getSampleRate() * bufferSizeInSeconds));

		if (streamInfo.getChannelPrefix().equalsIgnoreCase("HarpTime"))
			harpTimeBuffer = sourceBuffers.getLast();
	}
}

void HarpSyncInput::processFrames()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		uint32_t* dataPtr = (uint32_t*)frame->data;

		timestamps[currentFrame] = deviceContext->convertTimestampToSeconds(frame->time);

		harpTimeSamples[currentFrame] = *(dataPtr + 2) + 1;

		oni_destroy_frame(frame);

		sampleNumbers[currentFrame] = sampleNumber++;

		currentFrame++;

		if (currentFrame >= numFrames)
		{
			shouldAddToBuffer = true;
			currentFrame = 0;
		}

		if (shouldAddToBuffer)
		{
			shouldAddToBuffer = false;
			harpTimeBuffer->addToBuffer(harpTimeSamples, sampleNumbers, timestamps, eventCodes, numFrames);
		}
	}
}

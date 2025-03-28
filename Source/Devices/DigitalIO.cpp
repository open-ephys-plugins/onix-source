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

#include "DigitalIO.h"

DigitalIO::DigitalIO(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, OnixDeviceType::DIGITALIO, deviceIdx_, oni_ctx)
{
}

int DigitalIO::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized()) return -1;

	deviceContext->writeRegister(deviceIdx, (uint32_t)DigitalIORegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0));

	return deviceContext->getLastResult();
}

bool DigitalIO::updateSettings()
{
	return true;
}

void DigitalIO::startAcquisition()
{
}

void DigitalIO::stopAcquisition()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}
}

EventChannel::Settings DigitalIO::getEventChannelSettings()
{
	// NB: The stream must be assigned before adding the channel
	EventChannel::Settings settings{
		EventChannel::Type::TTL,
		"DigitalIO Event Channel",
		"Digital inputs and breakout button states coming from a DigitalIO device",
		"onix-digitalio.events",
		nullptr,
		numButtons + numDigitalInputs
	};

	return settings;
}

void DigitalIO::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void DigitalIO::processFrames()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		const GenericScopedLock<CriticalSection> digitalInputsLock(eventWords.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		uint16_t* dataPtr = (uint16_t*)frame->data;
		uint64_t timestamp = frame->time;

		int dataOffset = 4;

		uint64_t portState = *(dataPtr + dataOffset);
		uint64_t buttonState = *(dataPtr + dataOffset + 1);

		if (portState != 0 || buttonState != 0)
		{
			uint64_t ttlEventWord = (portState & 255) << 6 | (buttonState & 63);
			eventWords.add(ttlEventWord);
		}

		oni_destroy_frame(frame);
	}
}

uint64_t DigitalIO::getEventWord()
{
	const GenericScopedLock<CriticalSection> digitalInputsLock(eventWords.getLock());

	if (eventWords.size() != 0)
		return eventWords.removeAndReturn(0);

	return 0;
}

bool DigitalIO::hasEventWord()
{
	const GenericScopedLock<CriticalSection> digitalInputsLock(eventWords.getLock());

	return eventWords.size() > 0;
}

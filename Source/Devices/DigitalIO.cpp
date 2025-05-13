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

#include "DigitalIO.h"

using namespace OnixSourcePlugin;

DigitalIO::DigitalIO(std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, hubName, DigitalIO::getDeviceType(), deviceIdx_, oni_ctx)
{
}

OnixDeviceType DigitalIO::getDeviceType()
{
	return OnixDeviceType::DIGITALIO;
}

int DigitalIO::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized())
		throw error_str("Device context is not initialized properly for	" + getName());

	return deviceContext->writeRegister(deviceIdx, (uint32_t)DigitalIORegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0));
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
		OnixDevice::createStreamName({getHubName(), getName(), "Events"}),
		"Digital inputs and breakout button states coming from a DigitalIO device",
		getStreamIdentifier() + ".event.digital",
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
		uint64_t timestamp = deviceContext->convertTimestampToSeconds(frame->time);

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

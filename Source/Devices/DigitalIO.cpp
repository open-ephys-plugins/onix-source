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
#include "AnalogIO.h"

using namespace OnixSourcePlugin;

DigitalIO::DigitalIO(std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
	: OnixDevice(name, hubName, DigitalIO::getDeviceType(), deviceIdx_, oni_ctx), eventWords(64)
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

	int rc = deviceContext->writeRegister(deviceIdx, (uint32_t)DigitalIORegisters::ENABLE, (oni_reg_val_t)(isEnabled() ? 1 : 0));
	if (rc != ONI_ESUCCESS)
		throw error_str("Failed to enable the DigitalIO device.");

	oni_reg_val_t baseFreqHz;
	rc = deviceContext->readRegister(deviceIdx, (uint32_t)DigitalIORegisters::BASE_FREQ_HZ, &baseFreqHz);
	if (rc != ONI_ESUCCESS)
		throw error_str("Could not read the base frequency register on the DigitalIO device.");

	uint32_t periodTicks = baseFreqHz / (uint32_t)AnalogIO::getSampleRate();
	rc = deviceContext->writeRegister(deviceIdx, (uint32_t)DigitalIORegisters::SAMPLE_PERIOD, periodTicks);
	if (rc != ONI_ESUCCESS)
		throw error_str("Could not write the sample rate for polling to the DigitalIO device.");

	return rc;
}

bool DigitalIO::updateSettings()
{
	return true;
}

void DigitalIO::startAcquisition()
{
}

EventChannel::Settings DigitalIO::getEventChannelSettings(DataStream* stream)
{
	EventChannel::Settings settings{
		EventChannel::Type::TTL,
		OnixDevice::createStreamName({getHubName(), getName(), "Events"}),
		"Digital inputs and breakout button states coming from a DigitalIO device",
		getStreamIdentifier() + ".event.digital",
		stream,
		numButtons + numDigitalInputs
	};

	return settings;
}

int DigitalIO::getNumberOfWords()
{
	return eventWords.size_approx();
}

void DigitalIO::processFrames()
{
	oni_frame_t* frame;
	while (frameQueue.try_dequeue(frame))
	{

		uint16_t* dataPtr = (uint16_t*)frame->data;
		uint64_t timestamp = deviceContext->convertTimestampToSeconds(frame->time);

		int dataOffset = 4;

		uint64_t portState = *(dataPtr + dataOffset);
		uint64_t buttonState = *(dataPtr + dataOffset + 1);

		uint64_t ttlEventWord = (buttonState & 0x3F) << 8 | (portState & 0xFF);
		eventWords.enqueue(ttlEventWord);

		oni_destroy_frame(frame);
	}
}

uint64_t DigitalIO::getEventWord()
{
	uint64_t eventWord;
	if (eventWords.try_dequeue(eventWord))
		return eventWord;

	return 0;
}

bool DigitalIO::hasEventWord()
{
	return eventWords.peek() != nullptr;
}

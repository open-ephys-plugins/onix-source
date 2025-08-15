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

DigitalIO::DigitalIO (std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
    : OnixDevice (name, hubName, DigitalIO::getDeviceType(), deviceIdx_, oni_ctx)
{
    StreamInfo digitalInputStream = StreamInfo (
        createStreamName ({ getHubName(), name, "DigitalInputs" }),
        "Digital Inputs data",
        getStreamIdentifier(),
        NumDigitalInputs,
        AnalogIO::getSampleRate(),
        "CH",
        ContinuousChannel::Type::AUX,
        1.0,
        "u", // NB: Digital data is unitless by definition
        {},
        { "input" });
    streamInfos.add (digitalInputStream);

    StreamInfo digitalButtonStream = StreamInfo (
        createStreamName ({ getHubName(), name, "DigitalButtons" }),
        "Digital Buttons data",
        getStreamIdentifier(),
        NumButtons,
        AnalogIO::getSampleRate(),
        "",
        ContinuousChannel::Type::AUX,
        1.0,
        "u", // NB: Digital data is unitless by definition
        { "Moon", "Triangle", "X", "Check", "Circle", "Square" },
        { "input" });
    streamInfos.add (digitalButtonStream);

    eventCodes.fill (0);
}

OnixDeviceType DigitalIO::getDeviceType()
{
    return OnixDeviceType::DIGITALIO;
}

int DigitalIO::configureDevice()
{
    if (deviceContext == nullptr || ! deviceContext->isInitialized())
        throw error_str ("Device context is not initialized properly for	" + getName());

    int rc = deviceContext->writeRegister (deviceIdx, (uint32_t) DigitalIORegisters::ENABLE, (oni_reg_val_t) (isEnabled() ? 1 : 0));
    if (rc != ONI_ESUCCESS)
        throw error_str ("Failed to enable the DigitalIO device.");

    oni_reg_val_t baseFreqHz;
    rc = deviceContext->readRegister (deviceIdx, (uint32_t) DigitalIORegisters::BASE_FREQ_HZ, &baseFreqHz);
    if (rc != ONI_ESUCCESS)
        throw error_str ("Could not read the base frequency register on the DigitalIO device.");

    // NB: Two states are not accounted for when comparing clock ticks on the hardware,
    //	   therefore the periodTicks variable must be decreased by 2 to get the correct sample rate.
    uint32_t periodTicks = (baseFreqHz / (uint32_t) AnalogIO::getSampleRate()) - 2u;
    rc = deviceContext->writeRegister (deviceIdx, (uint32_t) DigitalIORegisters::SAMPLE_PERIOD, periodTicks);
    if (rc != ONI_ESUCCESS)
        throw error_str ("Could not write the sample rate for polling to the DigitalIO device.");

    return rc;
}

bool DigitalIO::updateSettings()
{
    return true;
}

void DigitalIO::startAcquisition()
{
    currentFrame = 0;
    sampleNumber = 0;

    digitalSamples.fill (0);
}

void DigitalIO::addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers)
{
    sourceBuffers.add (new DataBuffer (NumChannels, (int) streamInfos.getFirst().getSampleRate() * bufferSizeInSeconds));
    digitalBuffer = sourceBuffers.getLast();
}

EventChannel::Settings DigitalIO::getEventChannelSettings (DataStream* stream)
{
    EventChannel::Settings settings {
        EventChannel::Type::TTL,
        createStreamName ("Events", false),
        "Digital inputs and breakout button states coming from a DigitalIO device",
        getStreamIdentifier() + ".event.digital",
        stream,
        NumChannels
    };

    return settings;
}

float DigitalIO::getChannelState (uint8_t state, int channel)
{
    return (state & (1 << channel)) >> channel; // NB: Return the state of the specified channel
};

void DigitalIO::processFrames()
{
    oni_frame_t* frame;
    while (frameQueue.try_dequeue (frame))
    {
        size_t offset = 0;

        uint16_t* dataPtr = (uint16_t*) frame->data;

        timestamps[currentFrame] = deviceContext->convertTimestampToSeconds (frame->time);
        sampleNumbers[currentFrame] = sampleNumber++;

        constexpr int inputDataOffset = 4;
        constexpr int buttonDataOffset = inputDataOffset + 1;

        uint64_t inputState = *(dataPtr + inputDataOffset);
        uint64_t buttonState = *(dataPtr + buttonDataOffset);

        for (int i = 0; i < NumDigitalInputs; i++)
        {
            digitalSamples[currentFrame + offset++ * NumFrames] = getChannelState (inputState, i);
        }

        for (int i = 0; i < NumButtons; i++)
        {
            digitalSamples[currentFrame + offset++ * NumFrames] = getChannelState (buttonState, i);
        }

        eventCodes[currentFrame] = (buttonState & 0x3F) << 8 | (inputState & 0xFF);

        oni_destroy_frame (frame);

        if (++currentFrame >= NumFrames)
        {
            digitalBuffer->addToBuffer (digitalSamples.data(), sampleNumbers.data(), timestamps.data(), eventCodes.data(), NumFrames);

            currentFrame = 0;
        }
    }
}

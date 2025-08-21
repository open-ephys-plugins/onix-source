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

#include "Bno055.h"

using namespace OnixSourcePlugin;

Bno055::Bno055 (std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx)
    : OnixDevice (name, hubName, Bno055::getDeviceType(), deviceIdx_, ctx)
{
    auto streamIdentifier = getStreamIdentifier();

    std::string port = getPortName (deviceIdx);
    StreamInfo eulerAngleStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Euler" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Euler angle",
        streamIdentifier,
        3,
        sampleRate,
        "Eul",
        ContinuousChannel::Type::AUX,
        eulerAngleScale,
        "Degrees",
        { "Y", "R", "P" },
        "euler",
        { "y", "r", "p" });
    streamInfos.add (eulerAngleStream);

    StreamInfo quaternionStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Quaternion" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Quaternion",
        streamIdentifier,
        4,
        sampleRate,
        "Quat",
        ContinuousChannel::Type::AUX,
        quaternionScale,
        "u", // NB: Quaternion data is unitless by definition
        { "W", "X", "Y", "Z" },
        "quaternion",
        { "w", "x", "y", "z" });
    streamInfos.add (quaternionStream);

    StreamInfo accelerationStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Acceleration" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Acceleration",
        streamIdentifier,
        3,
        sampleRate,
        "Acc",
        ContinuousChannel::Type::AUX,
        accelerationScale,
        "m/s^2",
        { "X", "Y", "Z" },
        "acceleration",
        { "x", "y", "z" });
    streamInfos.add (accelerationStream);

    StreamInfo gravityStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Gravity" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Gravity",
        streamIdentifier,
        3,
        sampleRate,
        "Grav",
        ContinuousChannel::Type::AUX,
        accelerationScale,
        "m/s^2",
        { "X", "Y", "Z" },
        "gravity",
        { "x", "y", "z" });
    streamInfos.add (gravityStream);

    StreamInfo temperatureStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Temperature" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Temperature",
        streamIdentifier,
        1,
        sampleRate,
        "Temp",
        ContinuousChannel::Type::AUX,
        1.0f,
        "Celsius",
        { "" },
        "temperature");
    streamInfos.add (temperatureStream);

    StreamInfo calibrationStatusStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Calibration" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Calibration status",
        streamIdentifier,
        4,
        sampleRate,
        "Cal",
        ContinuousChannel::Type::AUX,
        1.0f,
        "",
        { "Mag", "Acc", "Gyr", "Sys" },
        "calibration",
        { "magnetometer", "acceleration", "gyroscope", "system" });
    streamInfos.add (calibrationStatusStream);

    for (int i = 0; i < numFrames; i++)
        eventCodes[i] = 0;
}

OnixDeviceType Bno055::getDeviceType()
{
    return OnixDeviceType::BNO;
}

int Bno055::configureDevice()
{
    if (deviceContext == nullptr || ! deviceContext->isInitialized())
        throw error_str ("Device context is not initialized properly for	" + getName());

    return deviceContext->writeRegister (deviceIdx, (uint32_t) Bno055Registers::ENABLE, isEnabled() ? 1 : 0);
}

bool Bno055::updateSettings()
{
    return true;
}

void Bno055::startAcquisition()
{
    currentFrame = 0;
    sampleNumber = 0;
}

void Bno055::addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers)
{
    sourceBuffers.add (new DataBuffer (numberOfChannels, (int) sampleRate * bufferSizeInSeconds));
    bnoBuffer = sourceBuffers.getLast();
}

void Bno055::processFrames()
{
    oni_frame_t* frame;
    while (frameQueue.try_dequeue (frame))
    {
        int16_t* dataPtr = ((int16_t*) frame->data) + 4;

        bnoTimestamps[currentFrame] = deviceContext->convertTimestampToSeconds (frame->time);

        size_t offset = 0;

        // Euler
        for (int i = 0; i < 3; i++)
        {
            bnoSamples[currentFrame + offset * numFrames] = float (*(dataPtr + offset)) * eulerAngleScale;
            offset++;
        }

        // Quaternion
        for (int i = 0; i < 4; i++)
        {
            bnoSamples[currentFrame + offset * numFrames] = float (*(dataPtr + offset)) * quaternionScale;
            offset++;
        }

        // Acceleration
        for (int i = 0; i < 3; i++)
        {
            bnoSamples[currentFrame + offset * numFrames] = float (*(dataPtr + offset)) * accelerationScale;
            offset++;
        }

        // Gravity
        for (int i = 0; i < 3; i++)
        {
            bnoSamples[currentFrame + offset * numFrames] = float (*(dataPtr + offset)) * accelerationScale;
            offset++;
        }

        // Temperature
        bnoSamples[currentFrame + offset * numFrames] = *((uint8_t*) (dataPtr + offset));

        // Calibration
        auto calibrationStatus = *((uint8_t*) (dataPtr + offset) + 1);

        constexpr uint8_t statusMask = 0b11;

        for (int i = 0; i < 4; i++)
        {
            bnoSamples[currentFrame + (offset + i + 1) * numFrames] = (calibrationStatus & (statusMask << (2 * i))) >> (2 * i);
        }

        oni_destroy_frame (frame);

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
            bnoBuffer->addToBuffer (bnoSamples.data(), sampleNumbers, bnoTimestamps, eventCodes, numFrames);
        }
    }
}

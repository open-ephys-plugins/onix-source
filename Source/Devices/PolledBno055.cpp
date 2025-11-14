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

#include "PolledBno055.h"

using namespace OnixSourcePlugin;

PolledBno055::PolledBno055 (std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx)
    : OnixDevice (name, hubName, PolledBno055::getDeviceType(), deviceIdx_, ctx, true),
      I2CRegisterContext (Bno055Address, deviceIdx_, ctx),
      Thread ("Polled BNO: " + name)
{
    auto streamIdentifier = getStreamIdentifier();

    const ContinuousChannel::InputRange eulerYawRange { -360.0f, 360.0f };
    const ContinuousChannel::InputRange eulerRollRange { -180.0f, 180.0f };
    const ContinuousChannel::InputRange eulerPitchRange { -90.0f, 90.0f };

    std::string port = getPortName (deviceIdx);
    StreamInfo eulerAngleStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Euler" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Euler angle",
        streamIdentifier,
        3,
        SampleRate,
        "Eul",
        ContinuousChannel::Type::AUX,
        EulerAngleScale,
        "Deg.",
        { "Y", "R", "P" },
        "euler",
        { "y", "r", "p" },
        { eulerYawRange, eulerRollRange, eulerPitchRange });
    streamInfos.add (eulerAngleStream);

    const ContinuousChannel::InputRange quaternionRange { -1.0f, 1.0f };

    StreamInfo quaternionStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Quaternion" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Quaternion",
        streamIdentifier,
        4,
        SampleRate,
        "Quat",
        ContinuousChannel::Type::AUX,
        QuaternionScale,
        "", // NB: Quaternion data is unitless by definition
        { "W", "X", "Y", "Z" },
        "quaternion",
        { "w", "x", "y", "z" },
        { quaternionRange });
    streamInfos.add (quaternionStream);

    const ContinuousChannel::InputRange accelerationRange { -100.0f, 100.0f };

    StreamInfo accelerationStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Acceleration" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Acceleration",
        streamIdentifier,
        3,
        SampleRate,
        "Acc",
        ContinuousChannel::Type::AUX,
        AccelerationScale,
        "m/s^2",
        { "X", "Y", "Z" },
        "acceleration",
        { "x", "y", "z" },
        { accelerationRange });
    streamInfos.add (accelerationStream);

    const ContinuousChannel::InputRange gravityRange { -10.0f, 10.0f };

    StreamInfo gravityStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Gravity" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Gravity",
        streamIdentifier,
        3,
        SampleRate,
        "Grav",
        ContinuousChannel::Type::AUX,
        AccelerationScale,
        "m/s^2",
        { "X", "Y", "Z" },
        "gravity",
        { "x", "y", "z" },
        { gravityRange });
    streamInfos.add (gravityStream);

    const ContinuousChannel::InputRange temperatureRange { -100.0f, 100.0f };

    StreamInfo temperatureStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Temperature" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Temperature",
        streamIdentifier,
        1,
        SampleRate,
        "Temp",
        ContinuousChannel::Type::AUX,
        1.0f,
        String::fromUTF8 ("\xc2\xb0") + String ("C"), // NB: "\xc2\xb0" --> degree symbol
        { "" },
        "temperature",
        {},
        { temperatureRange });
    streamInfos.add (temperatureStream);

    const ContinuousChannel::InputRange calibrationRange { -3.0f, 3.0f };

    StreamInfo calibrationStatusStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), "Calibration" }),
        "Bosch Bno055 9-axis inertial measurement unit (IMU) Calibration status",
        streamIdentifier,
        4,
        SampleRate,
        "Cal",
        ContinuousChannel::Type::AUX,
        1.0f,
        "",
        { "Mag", "Acc", "Gyr", "Sys" },
        "calibration",
        { "magnetometer", "acceleration", "gyroscope", "system" },
        { calibrationRange });
    streamInfos.add (calibrationStatusStream);

    for (int i = 0; i < NumFrames; i++)
        eventCodes[i] = 0;
}

PolledBno055::~PolledBno055()
{
    stopThread (500);
}

OnixDeviceType PolledBno055::getDeviceType()
{
    return OnixDeviceType::POLLEDBNO;
}

int PolledBno055::configureDevice()
{
    if (deviceContext == nullptr || ! deviceContext->isInitialized())
        throw error_str ("Device context is not initialized properly for " + getName());

    deserializer = std::make_unique<I2CRegisterContext> (DS90UB9x::DES_ADDR, deviceIdx, deviceContext);
    uint32_t alias = Bno055Address << 1;
    int rc = deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID4, alias);
    if (rc != ONI_ESUCCESS)
        throw error_str ("Error while writing bytes for " + getName());
    rc = deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias4, alias);
    if (rc != ONI_ESUCCESS)
        throw error_str ("Error while writing bytes for " + getName());

    return ONI_ESUCCESS;
}

bool PolledBno055::updateSettings()
{
    if (! isEnabled())
        return true;

    int rc = WriteByte (0x3E, 0x00); // Power mode normal
    if (rc != ONI_ESUCCESS)
        return false;

    rc = WriteByte (0x07, 0x00); // Page ID address 0
    if (rc != ONI_ESUCCESS)
        return false;

    rc = WriteByte (0x3F, 0x00); // Internal oscillator
    if (rc != ONI_ESUCCESS)
        return false;

    rc = WriteByte (0x41, (uint32_t) axisMap); // Axis map config
    if (rc != ONI_ESUCCESS)
        return false;

    rc = WriteByte (0x42, (uint32_t) axisSign); // Axis sign
    if (rc != ONI_ESUCCESS)
        return false;

    rc = WriteByte (0x3D, 0x0C); // Operation mode is NDOF
    if (rc != ONI_ESUCCESS)
        return false;

    rc = set933I2cRate (I2cRate);

    return rc == ONI_ESUCCESS;
}

void PolledBno055::startAcquisition()
{
    sampleNumber = 0;
    currentFrame = 0;

    previousTime = std::chrono::steady_clock::now();

    if (isEnabled())
        startThread();
}

void PolledBno055::run()
{
    while (! threadShouldExit())
    {
        time_point now = std::chrono::steady_clock::now();

        std::chrono::milliseconds dur = std::chrono::duration_cast<std::chrono::milliseconds> (now - previousTime);

        // NB: If the interval has not passed yet, wait for the remaining duration
        if (dur < TimerIntervalInMilliseconds)
        {
            std::this_thread::sleep_for (TimerIntervalInMilliseconds - dur);
        }

        pollFrame();

        previousTime += TimerIntervalInMilliseconds;
    }
}

void PolledBno055::stopAcquisition()
{
    if (isThreadRunning())
        stopThread (500);
}

void PolledBno055::addFrame (oni_frame_t* frame)
{
    oni_destroy_frame (frame);
}

void PolledBno055::addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers)
{
    sourceBuffers.add (new DataBuffer (NumberOfChannels, (int) SampleRate * bufferSizeInSeconds));
    bnoBuffer = sourceBuffers.getLast();
}

void PolledBno055::processFrames()
{
}

int16_t PolledBno055::getInt16FromUint32 (uint32_t value, bool getLowerValue)
{
    return getLowerValue ? static_cast<int16_t> (value & 0xFFFFu) : static_cast<int16_t> ((value & 0xFFFF0000u) >> 16);
}

void PolledBno055::pollFrame()
{
    size_t offset = 0;
    uint32_t value;

    // Euler
    ReadWord (EulerHeadingLsbAddress, 4, &value);
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, true) * EulerAngleScale;
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, false) * EulerAngleScale;

    ReadWord (EulerHeadingLsbAddress + 4, 4, &value);
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, true) * EulerAngleScale;

    // Quaternion
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, false) * QuaternionScale;

    ReadWord (EulerHeadingLsbAddress + 8, 4, &value);
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, true) * QuaternionScale;
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, false) * QuaternionScale;

    ReadWord (EulerHeadingLsbAddress + 12, 4, &value);
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, true) * QuaternionScale;

    // Acceleration

    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, false) * AccelerationScale;

    ReadWord (EulerHeadingLsbAddress + 16, 4, &value);
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, true) * AccelerationScale;
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, false) * AccelerationScale;

    // Gravity

    ReadWord (EulerHeadingLsbAddress + 20, 4, &value);
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, true) * AccelerationScale;
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, false) * AccelerationScale;

    ReadWord (EulerHeadingLsbAddress + 24, 4, &value);
    bnoSamples[offset++ * NumFrames + currentFrame] = getInt16FromUint32 (value, true) * AccelerationScale;

    // Temperature

    bnoSamples[offset++ * NumFrames + currentFrame] = static_cast<uint8_t> ((value & 0xFF0000u) >> 16);

    // Calibration Status

    oni_reg_val_t byte = static_cast<uint8_t> ((value & 0xFF000000u) >> 24);

    constexpr uint8_t statusMask = 0b11;

    for (int i = 0; i < 4; i++)
    {
        bnoSamples[currentFrame + (offset + i) * NumFrames] = (byte & (statusMask << (2 * i))) >> (2 * i);
    }

    oni_reg_val_t timestampL = 0, timestampH = 0;
    int rc = deviceContext->readRegister (deviceIdx, DS90UB9x::LASTI2CL, &timestampL);
    if (rc != ONI_ESUCCESS)
        return;
    rc = (uint64_t) deviceContext->readRegister (deviceIdx, DS90UB9x::LASTI2CH, &timestampH);
    if (rc != ONI_ESUCCESS)
        return;

    bnoTimestamps[currentFrame] = deviceContext->convertTimestampToSeconds ((uint64_t (timestampH) << 32) | uint64_t (timestampL));

    sampleNumbers[currentFrame] = sampleNumber++;

    currentFrame++;

    if (currentFrame >= NumFrames)
    {
        bnoBuffer->addToBuffer (bnoSamples.data(), sampleNumbers, bnoTimestamps, eventCodes, NumFrames);
        currentFrame = 0;
    }
}

int16_t PolledBno055::readInt16 (uint32_t startAddress)
{
    uint32_t value = 0;
    int rc = ReadWord (startAddress, 2, &value);

    if (rc != ONI_ESUCCESS)
        return 0;

    return static_cast<int16_t> (value);
}

void PolledBno055::setBnoAxisMap (Bno055AxisMap map)
{
    axisMap = map;
}

void PolledBno055::setBnoAxisSign (uint32_t sign)
{
    axisSign = sign;
}

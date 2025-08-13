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

#pragma once

#include "../OnixDevice.h"

namespace OnixSourcePlugin
{
enum class DigitalIORegisters : uint32_t
{
    ENABLE = 0x0,
    BASE_FREQ_HZ = 0x5,
    DEAD_TICKS = 0x6,
    SAMPLE_PERIOD = 0x7,
};

enum class DigitalPortState : uint16_t
{
    Pin0 = 0x1,
    Pin1 = 0x2,
    Pin2 = 0x4,
    Pin3 = 0x8,
    Pin4 = 0x10,
    Pin5 = 0x20,
    Pin6 = 0x40,
    Pin7 = 0x80
};

enum class BreakoutButtonState : uint16_t
{
    None = 0x0,
    Moon = 0x1,
    Triangle = 0x2,
    X = 0x4,
    Check = 0x8,
    Circle = 0x10,
    Square = 0x20,
    Reserved0 = 0x40,
    Reserved1 = 0x80,
    PortDOn = 0x100,
    PortCOn = 0x200,
    PortBOn = 0x400,
    PortAOn = 0x800
};

/*
    Configures and streams data from a DigitalIO device on a Breakout Board
*/
class DigitalIO : public OnixDevice
{
public:
    DigitalIO (std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

    int configureDevice() override;
    bool updateSettings() override;
    void startAcquisition() override;
    void addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers) override;
    void processFrames() override;

    EventChannel::Settings getEventChannelSettings (DataStream* stream);

    static OnixDeviceType getDeviceType();

private:
    DataBuffer* digitalBuffer = nullptr;

    static float getChannelState (uint8_t state, int channel);

    static constexpr int NumFrames = 25;

    static constexpr int NumDigitalInputs = 8;
    static constexpr int NumButtons = 6;
    static constexpr int NumChannels = NumDigitalInputs + NumButtons;

    static constexpr int NumSamples = NumFrames * NumChannels;

    unsigned short currentFrame = 0;
    int64_t sampleNumber = 0;

    std::array<float, NumSamples> digitalSamples;

    std::array<double, NumFrames> timestamps;
    std::array<int64_t, NumFrames> sampleNumbers;
    std::array<uint64_t, NumFrames> eventCodes;

    JUCE_LEAK_DETECTOR (DigitalIO);
};
} // namespace OnixSourcePlugin

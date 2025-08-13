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
enum class OutputClockRegisters : uint32_t
{
    NULL_REGISTER = 0,
    CLOCK_GATE = 1,
    HIGH_CYCLES = 2,
    LOW_CYCLES = 3,
    DELAY_CYCLES = 4,
    GATE_RUN = 5,
    BASE_FREQ_HZ = 6
};

/*
    Configures an OutputClock device on a Breakout Board
*/
class OutputClock : public OnixDevice
{
public:
    OutputClock (std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1> oni_ctx);

    int configureDevice() override;
    bool updateSettings() override;
    void startAcquisition() override;
    void addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers) override;
    void addFrame (oni_frame_t* frame) override;
    void processFrames() override;

    double getFrequencyHz() const;
    void setFrequencyHz (double frequency);

    int32_t getDutyCycle() const;
    void setDutyCycle (int32_t dutyCycle_);

    int32_t getDelay() const;
    void setDelay (int32_t delay_);

    bool getGateRun() const;
    void setGateRun (bool gate, bool writeToRegister = false);

    static OnixDeviceType getDeviceType();

private:
    double frequencyHz = 1e6;
    int32_t dutyCycle = 50;
    int32_t delay = 0;

    bool gateRun = true;

    void writeGateRunRegister();

    JUCE_LEAK_DETECTOR (OutputClock);
};
} // namespace OnixSourcePlugin

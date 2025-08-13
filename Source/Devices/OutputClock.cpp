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

#include "OutputClock.h"

using namespace OnixSourcePlugin;

OutputClock::OutputClock (std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
    : OnixDevice (name, hubName, OutputClock::getDeviceType(), deviceIdx_, oni_ctx)
{
}

OnixDeviceType OutputClock::getDeviceType()
{
    return OnixDeviceType::OUTPUTCLOCK;
}

int OutputClock::configureDevice()
{
    if (deviceContext == nullptr || ! deviceContext->isInitialized())
        throw error_str ("Device context is not initialized properly for	" + getName());

    setEnabled (true);
    return ONI_ESUCCESS;
};

bool OutputClock::updateSettings()
{
    oni_reg_val_t baseFreqHz;
    int rc = deviceContext->readRegister (deviceIdx, (oni_reg_addr_t) OutputClockRegisters::BASE_FREQ_HZ, &baseFreqHz);

    auto periodCycles = (uint32_t) (baseFreqHz / frequencyHz);
    auto h = (uint32_t) (periodCycles * ((double) dutyCycle / 100.0));
    auto l = periodCycles - h;
    auto delayCycles = (uint32_t) (delay * baseFreqHz);

    rc = deviceContext->writeRegister (deviceIdx, (oni_reg_addr_t) OutputClockRegisters::CLOCK_GATE, 1);
    if (rc != ONI_ESUCCESS)
        return false;

    rc = deviceContext->writeRegister (deviceIdx, (oni_reg_addr_t) OutputClockRegisters::HIGH_CYCLES, h);
    if (rc != ONI_ESUCCESS)
        return false;
    rc = deviceContext->writeRegister (deviceIdx, (oni_reg_addr_t) OutputClockRegisters::LOW_CYCLES, l);
    if (rc != ONI_ESUCCESS)
        return false;
    rc = deviceContext->writeRegister (deviceIdx, (oni_reg_addr_t) OutputClockRegisters::DELAY_CYCLES, delayCycles);
    if (rc != ONI_ESUCCESS)
        return false;

    rc = deviceContext->writeRegister (deviceIdx, (oni_reg_addr_t) OutputClockRegisters::GATE_RUN, gateRun ? 1 : 0);
    if (rc != ONI_ESUCCESS)
        return false;

    return true;
}

void OutputClock::startAcquisition()
{
}

void OutputClock::addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers)
{
}

void OutputClock::addFrame (oni_frame_t* frame)
{
    oni_destroy_frame (frame);
}

void OutputClock::processFrames()
{
}

double OutputClock::getFrequencyHz() const
{
    return frequencyHz;
}

void OutputClock::setFrequencyHz (double frequency)
{
    frequencyHz = frequency;
}

int32_t OutputClock::getDutyCycle() const
{
    return dutyCycle;
}

void OutputClock::setDutyCycle (int32_t dutyCycle_)
{
    dutyCycle = dutyCycle_;
}

int32_t OutputClock::getDelay() const
{
    return delay;
}

void OutputClock::setDelay (int32_t delay_)
{
    delay = delay_;
}

bool OutputClock::getGateRun() const
{
    return gateRun;
}

void OutputClock::setGateRun (bool gate, bool writeToRegister)
{
    gateRun = gate;
    if (writeToRegister)
        writeGateRunRegister();
}

void OutputClock::writeGateRunRegister()
{
    deviceContext->writeRegister (deviceIdx, (oni_reg_addr_t) OutputClockRegisters::GATE_RUN, gateRun ? 1 : 0);
}

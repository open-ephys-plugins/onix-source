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

#include "MemoryMonitor.h"
#include "DigitalIO.h"

using namespace OnixSourcePlugin;

MemoryMonitorUsage::MemoryMonitorUsage (GenericProcessor* p)
    : LevelMonitor (p)
{
    device = nullptr;
    setPassiveTooltip();
}

void MemoryMonitorUsage::timerCallback()
{
    if (device != nullptr)
    {
        auto memoryUsedPercent = device->getLastPercentUsedValue();
        auto logMemoryUsed = std::log (memoryUsedPercent + 1) / maxLogarithmicValue;
        setFillPercentage (logMemoryUsed);
        setTooltip (getNewTooltip (memoryUsedPercent));
        repaint();
    }
}

std::string MemoryMonitorUsage::getNewTooltip (float memoryUsage)
{
    std::stringstream ss;
    ss << "Memory Used: " << std::setprecision (3) << memoryUsage << "%";
    return ss.str();
}

void MemoryMonitorUsage::setPassiveTooltip()
{
    setTooltip ("Monitors the percent of the hardware memory buffer used.");
}

void MemoryMonitorUsage::setMemoryMonitor (std::shared_ptr<MemoryMonitor> memoryMonitor)
{
    device = memoryMonitor;
}

void MemoryMonitorUsage::startAcquisition()
{
    startTimerHz (TimerFrequencyHz);
}

void MemoryMonitorUsage::stopAcquisition()
{
    stopTimer();
    setFillPercentage (0.0f);
    setPassiveTooltip();
    repaint();
}

MemoryMonitor::MemoryMonitor (std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> oni_ctx)
    : OnixDevice (name, hubName, MemoryMonitor::getDeviceType(), deviceIdx_, oni_ctx)
{
    StreamInfo percentUsedStream = StreamInfo (
        createStreamName ("PercentUsed", false),
        "Percent of available memory that is currently used",
        getStreamIdentifier(),
        1,
        samplesPerSecond,
        "Percent",
        ContinuousChannel::Type::AUX,
        1.0f,
        "%",
        { "" },
        "percent");
    streamInfos.add (percentUsedStream);
}

OnixDeviceType MemoryMonitor::getDeviceType()
{
    return OnixDeviceType::MEMORYMONITOR;
}

int MemoryMonitor::configureDevice()
{
    if (deviceContext == nullptr || ! deviceContext->isInitialized())
        throw error_str ("Device context is not initialized properly for	" + getName());

    setEnabled (true);

    int rc = deviceContext->writeRegister (deviceIdx, (uint32_t) MemoryMonitorRegisters::ENABLE, 1);
    if (rc != ONI_ESUCCESS)
        throw error_str ("Unable to enable " + getName());

    rc = deviceContext->readRegister (deviceIdx, (oni_reg_addr_t) MemoryMonitorRegisters::TOTAL_MEM, &totalMemory);

    if (rc != ONI_ESUCCESS)
        throw error_str ("Unable to find the total memory used for " + getName());

    return rc;
}

bool MemoryMonitor::updateSettings()
{
    oni_reg_val_t clkHz;
    int rc = deviceContext->readRegister (deviceIdx, (oni_reg_addr_t) MemoryMonitorRegisters::CLK_HZ, &clkHz);
    if (rc != ONI_ESUCCESS)
        return rc;

    rc = deviceContext->writeRegister (deviceIdx, (oni_reg_addr_t) MemoryMonitorRegisters::CLK_DIV, clkHz / samplesPerSecond);

    return rc == ONI_ESUCCESS;
}

void MemoryMonitor::startAcquisition()
{
    sampleNumber = 0;
    lastPercentUsedValue = 0.0f;
}

void MemoryMonitor::addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers)
{
    sourceBuffers.add (new DataBuffer (streamInfos.getFirst().getNumChannels(), (int) streamInfos.getFirst().getSampleRate() * bufferSizeInSeconds));
    percentUsedBuffer = sourceBuffers.getLast();
}

float MemoryMonitor::getLastPercentUsedValue()
{
    return lastPercentUsedValue;
}

void MemoryMonitor::processFrames()
{
    static uint64_t ec = 0;
    oni_frame_t* frame;

    while (frameQueue.try_dequeue (frame))
    {
        uint32_t* dataPtr = (uint32_t*) frame->data;
        auto t = deviceContext->convertTimestampToSeconds (frame->time);
        auto p = 100.0f * float (*(dataPtr + 2)) / totalMemory;
        lastPercentUsedValue = p;
        oni_destroy_frame (frame);
        auto sn = sampleNumber++;
        percentUsedBuffer->addToBuffer (&p, &sn, &t, &ec, 1);
    }
}

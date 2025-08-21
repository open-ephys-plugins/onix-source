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

#include "Neuropixels1.h"

namespace OnixSourcePlugin
{
/**
    Configures and streams data from a Neuropixels 1.0f device
*/
class Neuropixels1f : public Neuropixels1
{
public:
    Neuropixels1f (std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1>);

    int configureDevice() override;
    bool updateSettings() override;
    void startAcquisition() override;
    void stopAcquisition() override;
    void addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers) override;
    void processFrames() override;

    void writeShiftRegisters();

    static OnixDeviceType getDeviceType();

private:
    static constexpr char* STREAM_NAME_AP = "AP";
    static constexpr char* STREAM_NAME_LFP = "LFP";

    // ADC number to frame index mapping
    static constexpr int adcToFrameIndex[] = {
        0,
        7,
        14,
        21,
        28,
        1,
        8,
        15,
        22,
        29,
        2,
        9,
        16,
        23,
        30,
        3,
        10,
        17,
        24,
        31,
        4,
        11,
        18,
        25,
        32,
        5,
        12,
        19,
        26,
        33,
        6,
        13
    };

    // ADC to muxed channel mapping
    static constexpr size_t adcToChannel[] = {
        0,
        1,
        24,
        25,
        48,
        49,
        72,
        73,
        96,
        97,
        120,
        121,
        144,
        145,
        168,
        169,
        192,
        193,
        216,
        217,
        240,
        241,
        264,
        265,
        288,
        289,
        312,
        313,
        336,
        337,
        360,
        361
    };

    JUCE_LEAK_DETECTOR (Neuropixels1f);
};

/*

    A thread that updates probe settings in the background and shows a progress bar

*/
class NeuropixelsV1fBackgroundUpdater : public NeuropixelsV1BackgroundUpdater
{
public:
    NeuropixelsV1fBackgroundUpdater (Neuropixels1f* d);

    void run() override;

private:
    JUCE_LEAK_DETECTOR (NeuropixelsV1fBackgroundUpdater);
};
} // namespace OnixSourcePlugin

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

#include <DataThreadHeaders.h>
#include <bitset>
#include <stdio.h>
#include <string.h>

#include "Onix1.h"

namespace OnixSourcePlugin
{
enum class ProbeType
{
    NONE = 1,
    NPX_V1,
    NPX_V2_SINGLE_SHANK,
    NPX_V2_QUAD_SHANK,
};

const std::map<ProbeType, std::string> ProbeTypeString = {
    {               ProbeType::NONE,                           "None"},
    {             ProbeType::NPX_V1,                "Neuropixels 1.0"},
    {ProbeType::NPX_V2_SINGLE_SHANK, "Neuropixels 2.0 - single shank"},
    {  ProbeType::NPX_V2_QUAD_SHANK,   "Neuropixels 2.0 - multishank"},
};

enum class Bank
{
    NONE = -1,
    A = 0,
    B = 1,
    C = 2,
    D = 3,
    E = 4,
    F = 5,
    G = 6,
    H = 7,
    I = 8,
    J = 9,
    K = 10,
    L = 11,
    M = 12,
    A1 = 13, // used for quad base
    A2 = 14,
    A3 = 15,
    A4 = 16,
    B1 = 17,
    B2 = 18,
    B3 = 19,
    B4 = 20,
    C1 = 21,
    C2 = 22,
    C3 = 23,
    C4 = 24,
    D1 = 25,
    D2 = 26,
    D3 = 27,
    D4 = 28,
    OFF = 255 // used in v1 API
};

enum class ElectrodeStatus
{
    CONNECTED,
    DISCONNECTED
};

enum class ElectrodeType
{
    ELECTRODE,
    REFERENCE
};

struct ElectrodeMetadata
{
    int global_index;
    int shank_local_index;
    int shank;
    int column_index;
    int channel;
    int row_index;
    float xpos; // position on shank, in microns
    float ypos; // position on shank, in microns
    float site_width; // in microns
    Bank bank;
    ElectrodeStatus status;
    ElectrodeType type;
    bool isSelected;
    Colour colour;
};

struct ProbeMetadata
{
    int shank_count;
    int electrodes_per_shank;
    int num_adcs;
    int adc_bits;
    std::vector<std::array<float, 2>> shankOutline;
    std::vector<std::array<float, 2>> probeContour;
    int columns_per_shank;
    int rows_per_shank;
    std::string name;
    bool switchable;
};

enum class NeuropixelsV1Registers : uint32_t
{
    OP_MODE = 0x00,
    REC_MOD = 0x01,
    CAL_MOD = 0x02,
    TEST_CONFIG1 = 0x03,
    TEST_CONFIG2 = 0x04,
    TEST_CONFIG3 = 0x05,
    TEST_CONFIG4 = 0x06,
    TEST_CONFIG5 = 0x07,
    STATUS = 0X08,
    SYNC = 0X09
};

enum class NeuropixelsV1ShiftRegisters : uint32_t
{
    SR_CHAIN1 = 0X0E,
    SR_CHAIN3 = 0X0C,
    SR_CHAIN2 = 0X0D,
    SR_LENGTH2 = 0X0F,
    SR_LENGTH1 = 0X10,
    SOFT_RESET = 0X11
};

enum class NeuropixelsV1CalibrationRegisterValues : uint32_t
{
    CAL_OFF = 0,
    OSC_ACTIVE = 1 << 4, // 0 = external osc inactive, 1 = activate the external calibration oscillator
    ADC_CAL = 1 << 5, // Enable ADC calibration
    CH_CAL = 1 << 6, // Enable channel gain calibration
    PIX_CAL = 1 << 7, // Enable pixel + channel gain calibration

    // Useful combinations
    OSC_ACTIVE_AND_ADC_CAL = OSC_ACTIVE | ADC_CAL,
    OSC_ACTIVE_AND_CH_CAL = OSC_ACTIVE | CH_CAL,
    OSC_ACTIVE_AND_PIX_CAL = OSC_ACTIVE | PIX_CAL,
};

enum class NeuropixelsV1OperationRegisterValues : uint32_t
{
    TEST = 1 << 3, // Enable Test mode
    DIG_TEST = 1 << 4, // Enable Digital Test mode
    CALIBRATE = 1 << 5, // Enable calibration mode
    RECORD = 1 << 6, // Enable recording mode
    POWER_DOWN = 1 << 7, // Enable power down mode

    // Useful combinations
    RECORD_AND_DIG_TEST = RECORD | DIG_TEST,
    RECORD_AND_CALIBRATE = RECORD | CALIBRATE,
};

enum class NeuropixelsV1RecordRegisterValues : uint32_t
{
    RESET_ALL = 1 << 5, // 1 = Set analog SR chains to default values
    DIG_ENABLE = 1 << 6, // 0 = Reset the MUX, ADC, and PSB counter, 1 = Disable reset
    CH_ENABLE = 1 << 7, // 0 = Reset channel pseudo-registers, 1 = Disable reset

    // Useful combinations
    SR_RESET = RESET_ALL | CH_ENABLE | DIG_ENABLE,
    DIG_CH_RESET = 0, // Yes, this is actually correct
    ACTIVE = DIG_ENABLE | CH_ENABLE,
};

enum class NeuropixelsV1Reference : unsigned char
{
    External = 0b001,
    Tip = 0b010
};

enum class NeuropixelsV1Gain : unsigned char
{
    Gain50 = 0b000,
    Gain125 = 0b001,
    Gain250 = 0b010,
    Gain500 = 0b011,
    Gain1000 = 0b100,
    Gain1500 = 0b101,
    Gain2000 = 0b110,
    Gain3000 = 0b111
};

struct NeuropixelsV1Adc
{
public:
    const int compP;
    const int compN;
    const int slope;
    const int coarse;
    const int fine;
    const int cfix;
    const int offset;
    const int threshold;

    NeuropixelsV1Adc (int compP_ = 16, int compN_ = 16, int slope_ = 0, int coarse_ = 0, int fine_ = 0, int cfix_ = 0, int offset_ = 0, int threshold_ = 512)
        : compP (compP_),
          compN (compN_),
          slope (slope_),
          coarse (coarse_),
          fine (fine_),
          cfix (cfix_),
          offset (offset_),
          threshold (threshold_)
    {
    }
};

struct NeuropixelsV1Values
{
    static constexpr int numberOfChannels = 384;
    static constexpr int numberOfElectrodes = 960;
    static constexpr int numberOfShanks = 1;
    static constexpr int numberOfSettings = 1;
    static constexpr int AdcCount = 32;
    static constexpr int FrameWordsV1e = 40;
    static constexpr int FrameWordsV1f = 36;
};

struct NeuropixelsV2eValues
{
    static constexpr int numberOfChannels = 384;
    static constexpr int electrodesPerShank = 1280;
    static constexpr int quadShankCount = 4;
    static constexpr int numberOfQuadShankElectrodes = electrodesPerShank * quadShankCount;
    static constexpr int singleShankCount = 1;
    static constexpr int numberOfSingleShankElectrodes = electrodesPerShank;
    static constexpr int numberOfSettings = 2;
};

struct ProbeSettings
{
    ProbeSettings (int numChannels, int numElectrodes, ProbeType type) : numberOfChannels (numChannels),
                                                                         numberOfElectrodes (numElectrodes)
    {
        selectedBank = std::vector<Bank> (numChannels, Bank::A);
        selectedShank = std::vector<int> (numChannels, 0);
        selectedElectrode = std::vector<int> (numChannels);

        for (int i = 0; i < numChannels; i++)
        {
            selectedElectrode[i] = i;
        }

        electrodeMetadata = std::vector<ElectrodeMetadata> (numElectrodes);

        probeType = type;
    }

    const int numberOfChannels;
    const int numberOfElectrodes;

    void updateProbeSettings (ProbeSettings* newSettings)
    {
        availableElectrodeConfigurations = newSettings->availableElectrodeConfigurations;
        availableApGains = newSettings->availableApGains;
        availableLfpGains = newSettings->availableLfpGains;
        availableReferences = newSettings->availableReferences;

        electrodeConfigurationIndex = newSettings->electrodeConfigurationIndex;
        apGainIndex = newSettings->apGainIndex;
        lfpGainIndex = newSettings->lfpGainIndex;
        referenceIndex = newSettings->referenceIndex;
        apFilterState = newSettings->apFilterState;

        selectedBank = newSettings->selectedBank;
        selectedShank = newSettings->selectedShank;
        selectedElectrode = newSettings->selectedElectrode;
        electrodeMetadata = newSettings->electrodeMetadata;

        probeType = newSettings->probeType;

        probeMetadata = newSettings->probeMetadata;
    };

    void selectElectrodes (std::vector<int> electrodes)
    {
        for (int i = 0; i < electrodes.size(); i++)
        {
            selectElectrode (electrodes[i]);
        }
    }

    void selectElectrode (int electrode)
    {
        Bank bank = electrodeMetadata[electrode].bank;
        int channel = electrodeMetadata[electrode].channel;
        int shank = electrodeMetadata[electrode].shank;
        int global_index = electrodeMetadata[electrode].global_index;

        for (int j = 0; j < electrodeMetadata.size(); j++)
        {
            if (electrodeMetadata[j].channel == channel)
            {
                if (electrodeMetadata[j].bank == bank && electrodeMetadata[j].shank == shank)
                {
                    electrodeMetadata[j].status = ElectrodeStatus::CONNECTED;
                }

                else
                {
                    electrodeMetadata[j].status = ElectrodeStatus::DISCONNECTED;
                }
            }
        }

        selectedBank[channel] = bank;
        selectedShank[channel] = shank;
        selectedElectrode[channel] = global_index;
    }

    Array<std::string> availableElectrodeConfigurations;
    Array<float> availableApGains; // Available AP gain values for each channel (if any)
    Array<float> availableLfpGains; // Available LFP gain values for each channel (if any)
    Array<std::string> availableReferences; // reference types

    int electrodeConfigurationIndex = 0;
    int apGainIndex = 0;
    int lfpGainIndex = 0;
    int referenceIndex = 0;
    bool apFilterState = false;

    std::vector<Bank> selectedBank;
    std::vector<int> selectedShank;
    std::vector<int> selectedElectrode;
    std::vector<ElectrodeMetadata> electrodeMetadata;

    ProbeType probeType = ProbeType::NONE;

    ProbeMetadata probeMetadata;
};

template <int N>
std::vector<unsigned char> toBitReversedBytes (std::bitset<N> bits)
{
    std::vector<unsigned char> bytes ((bits.size() - 1) / 8 + 1);

    for (int i = 0; i < bytes.size(); i++)
    {
        for (int j = 0; j < 8; j++)
        {
            bytes[i] |= bits[i * 8 + j] << (8 - j - 1);
        }
    }

    return bytes;
}

static const std::string ProbeString = "Probe";

template <int ch, int e>
class INeuropixel
{
public:
    INeuropixel (int numSettings, int numShanks) : numberOfShanks (numShanks)
    {
        for (int i = 0; i < numSettings; i++)
        {
            settings.emplace_back (std::make_unique<ProbeSettings> (ch, e, ProbeType::NONE));
        }
    }

    static const int numberOfChannels = ch;
    static const int numberOfElectrodes = e;
    const int numberOfShanks;

    std::vector<std::unique_ptr<ProbeSettings>> settings;

    virtual void setSettings (ProbeSettings* settings_, int index) = 0;
    virtual void defineMetadata (ProbeSettings* settings, ProbeType probeType) = 0;
    virtual uint64_t getProbeSerialNumber (int index) = 0;
    virtual std::string getProbePartNumber (int index) = 0;
    virtual std::string getFlexPartNumber (int index) = 0;
    virtual std::string getFlexVersion (int index) = 0;
    virtual std::vector<int> selectElectrodeConfiguration (int electrodeConfigurationIndex) = 0;

    bool saveProbeInterfaceFile (File recordingDirectory, std::string streamName, int probeIndex = 0)
    {
        if (streamName != "")
        {
            File filename = ProbeInterfaceJson::createFileName (recordingDirectory, streamName);

            LOGC ("Saving " + filename.getFullPathName());

            try
            {
                ProbeInterfaceJson::writeProbeSettingsToJson (filename, settings[probeIndex].get());
            }
            catch (const error_str& e)
            {
                Onix1::showWarningMessageBoxAsync ("Unable to Save Probe JSON File", e.what());
                return false;
            }
        }
        else
        {
            Onix1::showWarningMessageBoxAsync ("No Valid Stream",
                                               "Could not find a valid data stream when writing the Probe Interface file.");
            return false;
        }

        return true;
    }
};

static constexpr int shankConfigurationBitCount = 968;
static constexpr int BaseConfigurationBitCount = 2448;

using ShankBitset = std::bitset<shankConfigurationBitCount>;
using ConfigBitsArray = std::array<std::bitset<BaseConfigurationBitCount>, 2>;

static class NeuropixelsV1
{
public:
    ShankBitset static makeShankBits (NeuropixelsV1Reference reference, std::vector<int> channelMap)
    {
        constexpr int shankBitExt1 = 965;
        constexpr int shankBitExt2 = 2;
        constexpr int shankBitTip1 = 484;
        constexpr int shankBitTip2 = 483;
        constexpr int internalReferenceChannel = 191;

        std::bitset<shankConfigurationBitCount> shankBits;

        for (auto e : channelMap)
        {
            if (e == internalReferenceChannel)
                continue;

            int bitIndex = e % 2 == 0
                               ? 485 + (e / 2)
                               : 482 - (e / 2);

            shankBits[bitIndex] = true;
        }

        switch (reference)
        {
            case NeuropixelsV1Reference::External:
                shankBits[shankBitExt1] = true;
                shankBits[shankBitExt2] = true;
                break;
            case NeuropixelsV1Reference::Tip:
                shankBits[shankBitTip1] = true;
                shankBits[shankBitTip2] = true;
                break;
            default:
                break;
        }

        if (channelMap.size() != NeuropixelsV1Values::numberOfChannels)
        {
            LOGE ("Invalid number of channels connected for Neuropixels 1.0, configuration might be invalid.");
        }

        return shankBits;
    }

    ConfigBitsArray static makeConfigBits (NeuropixelsV1Reference reference, NeuropixelsV1Gain spikeAmplifierGain, NeuropixelsV1Gain lfpAmplifierGain, bool spikeFilterEnabled, std::vector<NeuropixelsV1Adc> adcs)
    {
        if (adcs.size() != NeuropixelsV1Values::AdcCount)
            throw error_str ("Invalid number of ADC values given.");

        const int BaseConfigurationConfigOffset = 576;

        ConfigBitsArray baseConfigs;

        for (size_t i = 0; i < NeuropixelsV1Values::numberOfChannels; i++)
        {
            size_t configIdx = i % 2;

            size_t refIdx = configIdx == 0 ? (382 - i) / 2 * 3 : (383 - i) / 2 * 3;

            baseConfigs[configIdx][refIdx + 0] = ((unsigned char) reference >> 0 & 0x1) == 1;
            baseConfigs[configIdx][refIdx + 1] = ((unsigned char) reference >> 1 & 0x1) == 1;
            baseConfigs[configIdx][refIdx + 2] = ((unsigned char) reference >> 2 & 0x1) == 1;

            size_t chanOptsIdx = BaseConfigurationConfigOffset + ((i - configIdx) * 4);

            baseConfigs[configIdx][chanOptsIdx + 0] = ((unsigned char) spikeAmplifierGain >> 0 & 0x1) == 1;
            baseConfigs[configIdx][chanOptsIdx + 1] = ((unsigned char) spikeAmplifierGain >> 1 & 0x1) == 1;
            baseConfigs[configIdx][chanOptsIdx + 2] = ((unsigned char) spikeAmplifierGain >> 2 & 0x1) == 1;

            baseConfigs[configIdx][chanOptsIdx + 3] = ((unsigned char) lfpAmplifierGain >> 0 & 0x1) == 1;
            baseConfigs[configIdx][chanOptsIdx + 4] = ((unsigned char) lfpAmplifierGain >> 1 & 0x1) == 1;
            baseConfigs[configIdx][chanOptsIdx + 5] = ((unsigned char) lfpAmplifierGain >> 2 & 0x1) == 1;

            baseConfigs[configIdx][chanOptsIdx + 6] = false;
            baseConfigs[configIdx][chanOptsIdx + 7] = ! spikeFilterEnabled;
        }

        int k = 0;

        for (const auto& adc : adcs)
        {
            auto configIdx = k % 2;
            size_t d = k++ / 2;

            size_t compOffset = 2406 - 42 * (d / 2) + (d % 2) * 10;
            size_t slopeOffset = compOffset + 20 + (d % 2);

            auto compP = std::bitset<8> { (unsigned char) (adc.compP) };
            auto compN = std::bitset<8> { (unsigned char) (adc.compN) };
            auto cfix = std::bitset<8> { (unsigned char) (adc.cfix) };
            auto slope = std::bitset<8> { (unsigned char) (adc.slope) };
            auto coarse = std::bitset<8> { (unsigned char) (adc.coarse) };
            auto fine = std::bitset<8> { (unsigned char) (adc.fine) };

            baseConfigs[configIdx][compOffset + 0] = compP[0];
            baseConfigs[configIdx][compOffset + 1] = compP[1];
            baseConfigs[configIdx][compOffset + 2] = compP[2];
            baseConfigs[configIdx][compOffset + 3] = compP[3];
            baseConfigs[configIdx][compOffset + 4] = compP[4];

            baseConfigs[configIdx][compOffset + 5] = compN[0];
            baseConfigs[configIdx][compOffset + 6] = compN[1];
            baseConfigs[configIdx][compOffset + 7] = compN[2];
            baseConfigs[configIdx][compOffset + 8] = compN[3];
            baseConfigs[configIdx][compOffset + 9] = compN[4];

            baseConfigs[configIdx][slopeOffset + 0] = slope[0];
            baseConfigs[configIdx][slopeOffset + 1] = slope[1];
            baseConfigs[configIdx][slopeOffset + 2] = slope[2];

            baseConfigs[configIdx][slopeOffset + 3] = fine[0];
            baseConfigs[configIdx][slopeOffset + 4] = fine[1];

            baseConfigs[configIdx][slopeOffset + 5] = coarse[0];
            baseConfigs[configIdx][slopeOffset + 6] = coarse[1];

            baseConfigs[configIdx][slopeOffset + 7] = cfix[0];
            baseConfigs[configIdx][slopeOffset + 8] = cfix[1];
            baseConfigs[configIdx][slopeOffset + 9] = cfix[2];
            baseConfigs[configIdx][slopeOffset + 10] = cfix[3];
        }

        return baseConfigs;
    }
};
} // namespace OnixSourcePlugin

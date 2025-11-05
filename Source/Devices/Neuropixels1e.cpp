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

#include "Neuropixels1e.h"

using namespace OnixSourcePlugin;

NeuropixelsV1eBackgroundUpdater::NeuropixelsV1eBackgroundUpdater (Neuropixels1e* d)
    : NeuropixelsV1BackgroundUpdater (d)
{
}

void NeuropixelsV1eBackgroundUpdater::run()
{
    setProgress (0);

    ((Neuropixels1e*) device)->resetProbe();

    if (! device->parseGainCalibrationFile())
    {
        result = false;
        return;
    }

    if (! device->parseAdcCalibrationFile())
    {
        result = false;
        return;
    }

    setProgress (0.5);

    device->WriteByte ((uint32_t) NeuropixelsV1Registers::CAL_MOD, (uint32_t) NeuropixelsV1CalibrationRegisterValues::CAL_OFF);
    device->WriteByte ((uint32_t) NeuropixelsV1Registers::TEST_CONFIG1, 0);
    device->WriteByte ((uint32_t) NeuropixelsV1Registers::TEST_CONFIG2, 0);
    device->WriteByte ((uint32_t) NeuropixelsV1Registers::TEST_CONFIG3, 0);
    device->WriteByte ((uint32_t) NeuropixelsV1Registers::TEST_CONFIG4, 0);
    device->WriteByte ((uint32_t) NeuropixelsV1Registers::TEST_CONFIG5, 0);
    device->WriteByte ((uint32_t) NeuropixelsV1Registers::SYNC, 0);
    device->WriteByte ((uint32_t) NeuropixelsV1Registers::REC_MOD, (uint32_t) NeuropixelsV1RecordRegisterValues::ACTIVE);
    device->WriteByte ((uint32_t) NeuropixelsV1Registers::OP_MODE, (uint32_t) NeuropixelsV1OperationRegisterValues::RECORD);

    try
    {
        ((Neuropixels1e*) device)->writeShiftRegisters();
    }
    catch (const error_str& e)
    {
        Onix1::showWarningMessageBoxAsync ("Error Writing Shift Registers", e.what());
        result = false;
        return;
    }

    setProgress (1);

    result = true;
}

Neuropixels1e::Neuropixels1e (std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx_)
    : Neuropixels1 (name, hubName, OnixDeviceType::NEUROPIXELSV1E, deviceIdx_, ctx_)
{
    std::string port = getPortName (getDeviceIdx());

    StreamInfo apStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), STREAM_NAME_AP }),
        "Neuropixels 1.0 AP band data stream",
        getStreamIdentifier(),
        numberOfChannels,
        apSampleRate,
        STREAM_NAME_AP,
        ContinuousChannel::Type::ELECTRODE,
        0.195f,
        "uV",
        {},
        "ap",
        {},
        {});
    streamInfos.add (apStream);

    StreamInfo lfpStream = StreamInfo (
        OnixDevice::createStreamName ({ port, getHubName(), getName(), STREAM_NAME_LFP }),
        "Neuropixels 1.0 LFP band data stream",
        getStreamIdentifier(),
        numberOfChannels,
        lfpSampleRate,
        STREAM_NAME_LFP,
        ContinuousChannel::Type::ELECTRODE,
        0.195f,
        "uV",
        {},
        "lfp",
        {},
        {});
    streamInfos.add (lfpStream);

    defineMetadata (settings[0].get());

    adcCalibrationFilePath = "None";
    gainCalibrationFilePath = "None";

    for (int i = 0; i < numUltraFrames; i++)
    {
        apEventCodes[i] = 0;
        lfpEventCodes[i] = 0;
    }

    probeNumber = 0;
}

int Neuropixels1e::configureDevice()
{
    if (deviceContext == nullptr || ! deviceContext->isInitialized())
        throw error_str ("Device context is not initialized properly for " + getName());

    configureSerDes();

    int rc = serializer->set933I2cRate (400e3);
    if (rc != ONI_ESUCCESS)
        throw error_str ("Unable to set I2C rate for " + getName());

    // Get Probe SN

    int errorCode = 0;

    for (int i = 0; i < 8; i++)
    {
        oni_reg_val_t reg_val;
        rc = flex->ReadByte (OFFSET_ID + i, &reg_val);

        if (rc != ONI_ESUCCESS)
            throw error_str ("Unable to read the probe serial number for device at address " + getDeviceIdx());

        if (reg_val <= 0xFF)
        {
            probeNumber |= (((uint64_t) reg_val) << (i * 8));
        }
    }

    LOGD ("Probe SN: ", probeNumber);

    return ONI_ESUCCESS;
}

OnixDeviceType Neuropixels1e::getDeviceType()
{
    return OnixDeviceType::NEUROPIXELSV1E;
}

void Neuropixels1e::configureSerDes()
{
    deviceContext->writeRegister (deviceIdx, DS90UB9x::ENABLE, 1);

    deviceContext->writeRegister (deviceIdx, DS90UB9x::TRIGGEROFF, 0);
    deviceContext->writeRegister (deviceIdx, DS90UB9x::TRIGGER, (uint32_t) DS90UB9x::DS90UB9xTriggerMode::Continuous);
    deviceContext->writeRegister (deviceIdx, DS90UB9x::SYNCBITS, 0);
    deviceContext->writeRegister (deviceIdx, DS90UB9x::DATAGATE, 0b00000001000100110000000000000001);
    deviceContext->writeRegister (deviceIdx, DS90UB9x::MARK, (uint32_t) DS90UB9x::DS90UB9xMarkMode::Disabled);

    // configure one magic word-triggered stream for the PSB bus
    deviceContext->writeRegister (deviceIdx, DS90UB9x::READSZ, 851973); // 13 frames/superframe,  7x 140-bit words on each serial line per frame
    deviceContext->writeRegister (deviceIdx, DS90UB9x::MAGIC_MASK, 0b11000000000000000000001111111111); // Enable inverse, wait for non-inverse, 10-bit magic word
    deviceContext->writeRegister (deviceIdx, DS90UB9x::MAGIC, 816); // Super-frame sync word
    deviceContext->writeRegister (deviceIdx, DS90UB9x::MAGIC_WAIT, 0);
    deviceContext->writeRegister (deviceIdx, DS90UB9x::DATAMODE, 913);
    deviceContext->writeRegister (deviceIdx, DS90UB9x::DATALINES0, 0x3245106B); // Sync, psb[0], psb[1], psb[2], psb[3], psb[4], psb[5], psb[6],
    deviceContext->writeRegister (deviceIdx, DS90UB9x::DATALINES1, 0xFFFFFFFF);

    std::this_thread::sleep_for (100ms); // Empirical. The gateware seems to need some milliseconds to get i2c initialized.

    deserializer = std::make_unique<I2CRegisterContext> (DS90UB9x::DES_ADDR, deviceIdx, deviceContext);
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::PortSel, 0x01); // Enable port 0
    int coaxMode = 0x4 + (uint32_t) (DS90UB9x::DS90UB9xMode::Raw12BitHighFrequency); // 0x4 maintains coax mode
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::PortMode, coaxMode); // 0x4 maintains coax mode
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::I2CConfig, 0b01011000); // 7: i2c pass all (0), 6: i2c pass (1), 5: auto_ack (0), 4: BC enable (1), 3: BC crc en (1), 2: reserved (0) 1:0: bc freq (00) 2.5Mbps
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SerAlias, DS90UB9x::SER_ADDR << 1);
    // Enable backchannel GPIO on deserializer. It is then the serializer task to decide if using them or use manual output
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::GpioCtrl0, 0x10);
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::GpioCtrl1, 0x32);

    auto alias = ProbeI2CAddress << 1;
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID1, alias);
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias1, alias);

    alias = FlexEepromI2CAddress << 1;
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID2, alias);
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias2, alias);

    serializer = std::make_unique<I2CRegisterContext> (DS90UB9x::SER_ADDR, deviceIdx, deviceContext);
    flex = std::make_unique<I2CRegisterContext> (FlexEepromI2CAddress, deviceIdx, deviceContext);
}

void Neuropixels1e::resetProbe()
{
    auto gpo10Config = DefaultGPO10Config & ~Gpo10ResetMask;
    serializer->WriteByte ((uint32_t) DS90UB9x::DS90UB933SerializerI2CRegister::Gpio10, gpo10Config);
    std::this_thread::sleep_for (1ms);
    gpo10Config |= Gpo10ResetMask;
    serializer->WriteByte ((uint32_t) DS90UB9x::DS90UB933SerializerI2CRegister::Gpio10, gpo10Config);
}

bool Neuropixels1e::updateSettings()
{
    auto updater = NeuropixelsV1eBackgroundUpdater (this);

    return updater.updateSettings() && adcValues.size() == NeuropixelsV1Values::AdcCount;
}

void Neuropixels1e::startAcquisition()
{
    apGain = getGainValue (getGainEnum (settings[0]->apGainIndex));
    lfpGain = getGainValue (getGainEnum (settings[0]->lfpGainIndex));

    apOffsetValues.clear();
    apOffsetValues.reserve (numberOfChannels);
    lfpOffsetValues.clear();
    lfpOffsetValues.reserve (numberOfChannels);

    for (int i = 0; i < numberOfChannels; i++)
    {
        apOffsets[i] = 0;
        lfpOffsets[i] = 0;

        apOffsetValues.emplace_back (std::vector<float> {});
        lfpOffsetValues.emplace_back (std::vector<float> {});
    }

    lfpOffsetCalculated = false;
    apOffsetCalculated = false;

    // WONTFIX: Soft reset inside settings.WriteShiftRegisters() above puts probe in reset set that
    // needs to be undone here
    WriteByte ((uint32_t) NeuropixelsV1Registers::OP_MODE, (uint32_t) NeuropixelsV1OperationRegisterValues::RECORD);
    WriteByte ((uint32_t) NeuropixelsV1Registers::REC_MOD, (uint32_t) NeuropixelsV1RecordRegisterValues::ACTIVE);

    if (ledEnabled)
    {
        auto gpo23Config = DefaultGPO32Config & ~Gpo32LedMask;
        serializer->WriteByte ((uint32_t) DS90UB9x::DS90UB933SerializerI2CRegister::Gpio32, gpo23Config);
    }

    superFrameCount = 0;
    ultraFrameCount = 0;
    apSampleNumber = 0;
    lfpSampleNumber = 0;
}

void Neuropixels1e::stopAcquisition()
{
    serializer->WriteByte ((uint32_t) DS90UB9x::DS90UB933SerializerI2CRegister::Gpio10, DefaultGPO10Config);
    serializer->WriteByte ((uint32_t) DS90UB9x::DS90UB933SerializerI2CRegister::Gpio32, DefaultGPO32Config);

    OnixDevice::stopAcquisition();
}

void Neuropixels1e::addSourceBuffers (OwnedArray<DataBuffer>& sourceBuffers)
{
    for (StreamInfo streamInfo : streamInfos)
    {
        sourceBuffers.add (new DataBuffer (streamInfo.getNumChannels(), (int) streamInfo.getSampleRate() * bufferSizeInSeconds));

        if (streamInfo.getChannelPrefix() == STREAM_NAME_AP)
            apBuffer = sourceBuffers.getLast();
        else if (streamInfo.getChannelPrefix() == STREAM_NAME_LFP)
            lfpBuffer = sourceBuffers.getLast();
    }
}

void Neuropixels1e::processFrames()
{
    const float apConversion = (1171.875 / apGain) * -1.0f;
    const float lfpConversion = (1171.875 / lfpGain) * -1.0f;

    oni_frame_t* frame;
    while (frameQueue.try_dequeue (frame))
    {
        uint16_t* dataPtr = (uint16_t*) frame->data;
        dataPtr += dataOffset;

        apTimestamps[superFrameCount] = deviceContext->convertTimestampToSeconds (frame->time);
        apSampleNumbers[superFrameCount] = apSampleNumber++;

        for (size_t i = 0; i < framesPerSuperFrame; i++)
        {
            if (i == 0) // LFP data
            {
                size_t superCountOffset = superFrameCount % superFramesPerUltraFrame;
                if (superCountOffset == 0)
                {
                    lfpTimestamps[ultraFrameCount] = apTimestamps[superFrameCount];
                    lfpSampleNumbers[ultraFrameCount] = lfpSampleNumber++;
                }

                for (int adc = 0; adc < NeuropixelsV1Values::AdcCount; adc++)
                {
                    auto sample = *(dataPtr + adcToFrameIndex[adc]);
                    sample = sample > adcValues.at (adc).threshold ? sample - adcValues.at (adc).offset : sample;
                    lfpSamples[(rawToChannel[adc][superCountOffset] * numUltraFrames) + ultraFrameCount] =
                        lfpConversion * (lfpGainCorrection * sample - DataMidpoint) - lfpOffsets.at (rawToChannel[adc][superCountOffset]);
                }
            }
            else // AP data
            {
                for (int adc = 0; adc < NeuropixelsV1Values::AdcCount; adc++)
                {
                    auto sample = *(dataPtr + adcToFrameIndex[adc] + i * NeuropixelsV1Values::FrameWordsV1e);
                    sample = sample > adcValues.at (adc).threshold ? sample - adcValues.at (adc).offset : sample;
                    apSamples[(rawToChannel[adc][i - 1] * superFramesPerUltraFrame * numUltraFrames) + superFrameCount] =
                        apConversion * (apGainCorrection * sample - DataMidpoint) - apOffsets.at (rawToChannel[adc][i - 1]);
                }
            }
        }

        oni_destroy_frame (frame);

        superFrameCount++;

        if (superFrameCount % superFramesPerUltraFrame == 0)
        {
            ultraFrameCount++;
        }

        if (ultraFrameCount >= numUltraFrames)
        {
            ultraFrameCount = 0;
            superFrameCount = 0;

            lfpBuffer->addToBuffer (lfpSamples.data(), lfpSampleNumbers, lfpTimestamps, lfpEventCodes, numUltraFrames);
            apBuffer->addToBuffer (apSamples.data(), apSampleNumbers, apTimestamps, apEventCodes, numUltraFrames * superFramesPerUltraFrame);

            if (! lfpOffsetCalculated)
                updateLfpOffsets (lfpSamples, lfpSampleNumbers[0]);
            if (! apOffsetCalculated)
                updateApOffsets (apSamples, apSampleNumbers[0]);
        }
    }
}

void Neuropixels1e::writeShiftRegisters()
{
    if (adcValues.size() != NeuropixelsV1Values::AdcCount)
        throw error_str ("Invalid number of ADC values found.");

    auto shankBits = NeuropixelsV1::makeShankBits (getReference (settings[0]->referenceIndex), settings[0]->selectedElectrode);
    auto configBits = NeuropixelsV1::makeConfigBits (getReference (settings[0]->referenceIndex), getGainEnum (settings[0]->apGainIndex), getGainEnum (settings[0]->lfpGainIndex), true, adcValues);

    auto shankBytes = toBitReversedBytes<shankConfigurationBitCount> (shankBits);

    int rc = WriteByte ((uint32_t) NeuropixelsV1ShiftRegisters::SR_LENGTH1, (uint32_t) shankBytes.size() % 0x100);
    if (rc != ONI_ESUCCESS)
        throw error_str ("Could not set shift register length.");

    rc = WriteByte ((uint32_t) NeuropixelsV1ShiftRegisters::SR_LENGTH2, (uint32_t) shankBytes.size() / 0x100);
    if (rc != ONI_ESUCCESS)
        throw error_str ("Could not set shift register length.");

    for (auto b : shankBytes)
    {
        rc = WriteByte ((uint32_t) NeuropixelsV1ShiftRegisters::SR_CHAIN1, b);
        if (rc != ONI_ESUCCESS)
            throw error_str ("Could not write byte for shift register chain for shank configuration.");
    }

    const uint32_t shiftRegisterSuccess = 1 << 7;

    for (int i = 0; i < configBits.size(); i++)
    {
        auto srAddress = i == 0 ? (uint32_t) NeuropixelsV1ShiftRegisters::SR_CHAIN2 : (uint32_t) NeuropixelsV1ShiftRegisters::SR_CHAIN3;

        for (int j = 0; j < 2; j++)
        {
            // WONTFIX: Without this reset, the ShiftRegisterSuccess check below will always fail
            // on whatever the second shift register write sequence regardless of order or
            // contents. Could be increased current draw during internal process causes MCLK
            // to droop and mess up internal state. Or that MCLK is just not good enough to
            // prevent metastability in some logic in the ASIC that is only entered in between
            // SR accesses.
            WriteByte ((uint32_t) NeuropixelsV1ShiftRegisters::SOFT_RESET, 0xFF);
            WriteByte ((uint32_t) NeuropixelsV1ShiftRegisters::SOFT_RESET, 0x00);

            auto baseBytes = toBitReversedBytes<BaseConfigurationBitCount> (configBits[i]);

            rc = WriteByte ((uint32_t) NeuropixelsV1ShiftRegisters::SR_LENGTH1, (uint32_t) baseBytes.size() % 0x100);
            if (rc != ONI_ESUCCESS)
                throw error_str ("Could not set shift register length.");

            rc = WriteByte ((uint32_t) NeuropixelsV1ShiftRegisters::SR_LENGTH2, (uint32_t) baseBytes.size() / 0x100);
            if (rc != ONI_ESUCCESS)
                throw error_str ("Could not set shift register length.");

            for (auto b : baseBytes)
            {
                rc = WriteByte (srAddress, b);
                if (rc != ONI_ESUCCESS)
                    throw error_str ("Could not set write byte to shift register for base configuration.");
            }
        }

        oni_reg_val_t value;
        rc = ReadByte ((uint32_t) NeuropixelsV1Registers::STATUS, &value);

        if (rc != ONI_ESUCCESS || value != shiftRegisterSuccess)
        {
            LOGE ("Shift register ", srAddress, " status check failed.");
            return;
        }
    }
}

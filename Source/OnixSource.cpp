/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2023 Allen Institute for Brain Science and Open Ephys

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

#include "OnixSource.h"

#include "OnixSourceEditor.h"

OnixSource::OnixSource(SourceNode* sn) :
    DataThread(sn),
    ctx(NULL),
    devicesFound(false)
{
    LOGD("ONIX Source creating ONI context.");

    ctx = oni_create_ctx("riffa"); // "riffa" is the PCIe driver name

    if (!ctx) { LOGE("Failed to create context."); return; }

    // Initialize context and discover hardware
    int errorCode = oni_init_ctx(ctx, 0);
    
    if (errorCode) { LOGE(oni_error_str(errorCode)); return; }

    initializeDevices();

    if(!devicesFound) { return; }

    oni_size_t frame_size = 0;
    size_t frame_size_sz = sizeof(frame_size);
    oni_get_opt(ctx, ONI_OPT_MAXREADFRAMESIZE, &frame_size, &frame_size_sz);
    printf("Max. read frame size: %u bytes\n", frame_size);

    oni_get_opt(ctx, ONI_OPT_MAXWRITEFRAMESIZE, &frame_size, &frame_size_sz);
    printf("Max. write frame size: %u bytes\n", frame_size);

    size_t block_size_sz = sizeof(block_read_size);
    
    // set block read size
    oni_set_opt(ctx, ONI_OPT_BLOCKREADSIZE, &block_read_size, block_size_sz);
}

DataThread* OnixSource::createDataThread(SourceNode *sn)
{
	return new OnixSource(sn);
}


std::unique_ptr<GenericEditor> OnixSource::createEditor(SourceNode* sn)
{
    std::unique_ptr<OnixSourceEditor> e = std::make_unique<OnixSourceEditor>(sn, this);
    ed = e.get();

    return e;
}


void OnixSource::initializeDevices()
{
    oni_size_t num_devs = 0;
    oni_device_t* devices = NULL;

    // Examine device table
    size_t num_devs_sz = sizeof(num_devs);
    oni_get_opt(ctx, ONI_OPT_NUMDEVICES, &num_devs, &num_devs_sz);

    size_t devices_sz = sizeof(oni_device_t) * num_devs;
    devices = (oni_device_t *)realloc(devices, devices_sz);
    if (devices == NULL) { LOGE("No devices found."); return; }
    oni_get_opt(ctx, ONI_OPT_DEVICETABLE, devices, &devices_sz);

    devicesFound = true;

#ifdef DEBUG

    // print device info
    printf("   +--------------------+-------+-------+-------+-------+---------------------\n");
    printf("   |        \t\t|  \t|Firm.\t|Read\t|Wrt. \t|     \n");
    printf("   |Dev. idx\t\t|ID\t|ver. \t|size\t|size \t|Desc.\n");
    printf("   +--------------------+-------+-------+-------+-------+---------------------\n");

    for (size_t dev_idx = 0; dev_idx < num_devs; dev_idx++) {
        if (devices[dev_idx].id != ONIX_NULL) {

            const char* dev_str = onix_device_str(devices[dev_idx].id);

            printf("%02zd |%05zd: 0x%02x.0x%02x\t|%d\t|%d\t|%u\t|%u\t|%s\n",
                dev_idx,
                devices[dev_idx].idx,
                (uint8_t)(devices[dev_idx].idx >> 8),
                (uint8_t)devices[dev_idx].idx,
                devices[dev_idx].id,
                devices[dev_idx].version,
                devices[dev_idx].read_size,
                devices[dev_idx].write_size,
                dev_str);
        }
    }
#endif

    static const String probeLetters = "ABCDEFGHI";
    int npxProbeIdx = 0;

    for (size_t dev_idx = 0; dev_idx < num_devs; dev_idx++)
    {
        if (devices[dev_idx].id == ONIX_NEUROPIX1R0)
        {
            Neuropixels_1* np1 = new Neuropixels_1("Probe-" + String::charToString(probeLetters[npxProbeIdx]), devices[dev_idx].idx, ctx);

            if(np1->enableDevice() != 0)
            {
                LOGE("Error enabling device data stream.");
                delete np1;
                continue;
            }

            sources.add(np1);
            
            for (auto streamInfo : np1->streams)
            {
                sourceBuffers.add(new DataBuffer(streamInfo.numChannels, (int)streamInfo.sampleRate * 10));

                if (streamInfo.channelPrefix.equalsIgnoreCase("AP"))
                    np1->apBuffer = sourceBuffers.getLast();
                else if (streamInfo.channelPrefix.equalsIgnoreCase("LFP"))
                    np1->lfpBuffer = sourceBuffers.getLast();
            }

            // // Enable device stream
            // const oni_reg_addr_t enable_device_stream = 0x8000;
            // oni_write_reg(ctx, DEVICE_NPX1_1, enable_device_stream, 1);

            // oni_write_reg(ctx, DEVICE_NPX1_1, Neuropixels_Registers::REC_MOD , (uint32_t)1 << 5);

            // oni_write_reg(ctx, DEVICE_NPX1_1, Neuropixels_Registers::OP_MODE , (uint32_t)1 << 6);

            npxProbeIdx++;
        }
    }
}

void OnixSource::updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
		OwnedArray<EventChannel>* eventChannels,
		OwnedArray<SpikeChannel>* spikeChannels,
		OwnedArray<DataStream>* dataStreams,
		OwnedArray<DeviceInfo>* deviceInfos,
		OwnedArray<ConfigurationObject>* configurationObjects)
{

    LOGD("ONIX Source initializing data streams.");

    dataStreams->clear();
    eventChannels->clear();
    continuousChannels->clear();
    spikeChannels->clear();
    deviceInfos->clear();
    configurationObjects->clear();

    if (devicesFound)
    {
        DataStream* currentStream;

        for (auto source : sources)
        {
            // create device info object
            if(source->type == OnixDeviceType::NEUROPIXELS_1)
            {
                DeviceInfo::Settings deviceSettings{
                    source->getName(), // device name
                    "Neuropixels 1.0 Probe",
                    "neuropixels1.probe",
                    "0000000",
                    "imec"
                };

                DeviceInfo* device = new DeviceInfo(deviceSettings);
                deviceInfos->add(device); // unique device object owned by SourceNode
            }

            // add data streams and channels
            for (auto streamInfo : source->streams)
            {
                DataStream::Settings apStreamSettings
                {
                    streamInfo.name, // stream name
                    streamInfo.description, // stream description
                    streamInfo.identifier, // stream identifier
                    streamInfo.sampleRate // sample rate

                };

                DataStream* stream = new DataStream(apStreamSettings);
                dataStreams->add(stream);
                currentStream = stream;
                currentStream->device = deviceInfos->getLast();

                // Add continuous channels
                for (int chan = 0; chan < streamInfo.numChannels; chan++)
                {
                    ContinuousChannel::Settings channelSettings{
                        streamInfo.channelType,
                        streamInfo.channelPrefix + String(chan + 1),
                        streamInfo.description,
                        streamInfo.identifier,
                        streamInfo.bitVolts, // bitVolts
                        currentStream
                    };
                    continuousChannels->add(new ContinuousChannel(channelSettings));
                }
            }

        }
    }
            
}


bool OnixSource::foundInputSource()
{
    return devicesFound;
}

bool OnixSource::startAcquisition()
{

    // for (auto source : sources)
    //     source->buffer->clear();

    // for (auto source : sources)
    //     source->startThread();
    if (!devicesFound)
        return false;
    
    for (auto source : sources)
    {
        source->startAcquisition();
    }

    oni_reg_val_t reg = 2;
    int res = oni_set_opt(ctx, ONI_OPT_RESETACQCOUNTER, &reg, sizeof(oni_size_t));
    if (res < ONI_ESUCCESS)
    {
        LOGE("Error starting acquisition: ", oni_error_str(res), " code ", res);
        return false;
    }

    startThread();

    return true;
}

bool OnixSource::stopAcquisition()
{
    if (isThreadRunning())
        signalThreadShouldExit();
    
    waitForThreadToExit(2000);

    for (auto source : sources)
    {
        source->stopAcquisition();
    }
    
    if (devicesFound)
    {
        oni_size_t reg = 0;
        int res = oni_set_opt(ctx, ONI_OPT_RUNNING, &reg, sizeof(reg));
        if (res < ONI_ESUCCESS)
        {
            LOGE("Error stopping acquisition: ", oni_error_str(res), " code ", res);
            return false;
        }

        uint32_t val = 1;
        oni_set_opt(ctx, ONI_OPT_RESET, &val, sizeof(val));
        oni_set_opt(ctx, ONI_OPT_BLOCKREADSIZE, &block_read_size, sizeof(block_read_size));
    }
    
    
    for (auto buffers : sourceBuffers)
        buffers->clear();

    return true;
}

bool OnixSource::updateBuffer()
{
    const int nSamps = 120;
    oni_frame_t* frame;
    unsigned char* bufferPtr;

    for (int samp = 0; samp < nSamps; samp++)
    {
        int res = oni_read_frame(ctx, &frame);

        if (res < ONI_ESUCCESS)
        {
            LOGE("Error reading ONI frame: ", oni_error_str(res), " code ", res);
            return false;
        }

        for (auto source : sources)
        {
            if (frame->dev_idx == source->getDeviceIdx())
            {
                source->addFrame(frame);
            }
        }

        oni_destroy_frame(frame);
    }

    return true;
}

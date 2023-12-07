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

void OnixSource::updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
		OwnedArray<EventChannel>* eventChannels,
		OwnedArray<SpikeChannel>* spikeChannels,
		OwnedArray<DataStream>* dataStreams,
		OwnedArray<DeviceInfo>* devices,
		OwnedArray<ConfigurationObject>* configurationObjects)
{

    LOGD("ONIX Source initializing data streams.");

    dataStreams->clear();
    eventChannels->clear();
    continuousChannels->clear();
    spikeChannels->clear();
    devices->clear();
    configurationObjects->clear();

    sources.clear();
    sourceBuffers.clear();

    if (ctx == NULL)
    {
        LOGD("ONIX Source creating ONI context.");

        ctx = oni_create_ctx("riffa"); // "riffa" is the PCIe driver name

        if (!ctx) { LOGE("Failed to create context."); return; }

        // Initialize context and discover hardware
        int errorCode = oni_init_ctx(ctx, 0);
        
        if (errorCode) { LOGE(oni_error_str(errorCode)); return; }

        oni_size_t num_devs = 0;
        oni_device_t* devices = NULL;

        // Examine device table
        size_t num_devs_sz = sizeof(num_devs);
        oni_get_opt(ctx, ONI_OPT_NUMDEVICES, &num_devs, &num_devs_sz);

        size_t devices_sz = sizeof(oni_device_t) * num_devs;
        devices = (oni_device_t *)realloc(devices, devices_sz);
        if (devices == NULL) { LOGE("No devices found."); return; }
        oni_get_opt(ctx, ONI_OPT_DEVICETABLE, devices, &devices_sz);
        
        // print device info
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
    }
        



    // detect available devices and create source objects for each
    
}


bool OnixSource::foundInputSource()
{
    return false;
}

bool OnixSource::startAcquisition()
{

    for (auto source : sources)
        source->buffer->clear();

    for (auto source : sources)
        source->startThread();

    return true;
}

bool OnixSource::stopAcquisition()
{

    for (auto source : sources)
        source->signalThreadShouldExit();

    return true;
}

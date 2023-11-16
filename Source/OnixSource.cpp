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

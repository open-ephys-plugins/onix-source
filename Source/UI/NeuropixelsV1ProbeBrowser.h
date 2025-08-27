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

#include "../Devices/Neuropixels1e.h"
#include "../Devices/Neuropixels1f.h"
#include "ProbeBrowser.h"

namespace OnixSourcePlugin
{
class NeuropixelsV1ProbeBrowser : public ProbeBrowser<NeuropixelsV1Values::numberOfChannels, NeuropixelsV1Values::numberOfElectrodes>
{
public:
    NeuropixelsV1ProbeBrowser (SettingsInterface* parent_, int probeIndex_) : ProbeBrowser (parent_, probeIndex_)
    {
        setDrawingSettings();
    }

    ProbeSettings<NeuropixelsV1Values::numberOfChannels, NeuropixelsV1Values::numberOfElectrodes>* getSettings() override
    {
        if (parent->getDevice()->getDeviceType() == OnixDeviceType::NEUROPIXELSV1E)
            return std::static_pointer_cast<Neuropixels1e> (parent->getDevice())->settings[0].get();
        else if (parent->getDevice()->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F)
            return std::static_pointer_cast<Neuropixels1f> (parent->getDevice())->settings[0].get();
        else
            return nullptr;
    }
};
} // namespace OnixSourcePlugin

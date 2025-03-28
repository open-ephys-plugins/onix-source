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

#include "../NeuropixComponents.h"

#include "../Devices/Neuropixels_1.h"
#include "ProbeBrowser.h"

class NeuropixelsV1fProbeBrowser : public ProbeBrowser<NeuropixelsV1fValues::numberOfChannels, NeuropixelsV1fValues::numberOfElectrodes>
{
public:
	NeuropixelsV1fProbeBrowser(SettingsInterface* parent_, int probeIndex_) :
		ProbeBrowser(parent_, probeIndex_)
	{
		setDrawingSettings();
	}

	ProbeSettings<NeuropixelsV1fValues::numberOfChannels, NeuropixelsV1fValues::numberOfElectrodes>* getSettings() override
	{
		return std::static_pointer_cast<Neuropixels_1>(parent->device)->settings[0].get();
	}
};

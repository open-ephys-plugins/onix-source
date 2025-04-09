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

#include "NeuropixelsV2eProbeInterface.h"

class OnixSourceCanvas;
class OnixSourceEditor;

class NeuropixelsV2eInterface : public SettingsInterface,
	public Button::Listener
{
public:
	NeuropixelsV2eInterface(std::shared_ptr<Neuropixels2e> d, OnixSourceEditor* e, OnixSourceCanvas* c);

	void startAcquisition() override {};

	void stopAcquisition() override {};

	void saveParameters(XmlElement* xml) override {};

	void loadParameters(XmlElement* xml) override {};

	void updateInfoString() override;

	void resized() override;

	void buttonClicked(Button* b) override;

	void updateSettings() override;

	void updateDevice(std::shared_ptr<Neuropixels2e> d);

private:

	void setInterfaceEnabledState(bool newState) override;

	std::unique_ptr<CustomTabComponent> topLevelTabComponent;

	std::unique_ptr<UtilityButton> probeEnableButton;
	std::unique_ptr<Component> deviceComponent;

	static const int numProbes = 2;

	std::array<std::unique_ptr<NeuropixelsV2eProbeInterface>, numProbes> probeInterfaces;
};

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

#include <VisualizerEditorHeaders.h>

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

#include "../Devices/PolledBno055.h"

class PolledBno055Interface : public SettingsInterface,
	public Button::Listener
{
public:
	PolledBno055Interface(std::shared_ptr<PolledBno055> d, OnixSourceEditor* e, OnixSourceCanvas* c);

	void startAcquisition() override {};

	void stopAcquisition() override {};

	void saveParameters(XmlElement* xml) override {};

	void loadParameters(XmlElement* xml) override {};

	void updateInfoString() override {};

	void buttonClicked(Button*) override;

	void updateSettings() override;

private:

	std::unique_ptr<UtilityButton> deviceEnableButton;

	JUCE_LEAK_DETECTOR(PolledBno055Interface);
};

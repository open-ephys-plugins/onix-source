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

#pragma

#include <VisualizerEditorHeaders.h>

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

#include "../Devices/DigitalIO.h"

class DigitalIOInterface : public SettingsInterface,
	public Button::Listener
{
public:
	/** Constructor */
	DigitalIOInterface(std::shared_ptr<DigitalIO> d, OnixSourceEditor* e, OnixSourceCanvas* c);

	/** Disables buttons and starts animation if necessary */
	void startAcquisition() override {};

	/** Enables buttons and start animation if necessary */
	void stopAcquisition() override {};

	/** Save parameters to XML */
	void saveParameters(XmlElement* xml) override {};

	/** Load parameters from XML */
	void loadParameters(XmlElement* xml) override {};

	/** Updates the info string on the right-hand side of the component */
	void updateInfoString() override {};

	/** Listener methods*/
	void buttonClicked(Button*) override;

private:

	std::unique_ptr<UtilityButton> deviceEnableButton;

	JUCE_LEAK_DETECTOR(DigitalIOInterface);
};

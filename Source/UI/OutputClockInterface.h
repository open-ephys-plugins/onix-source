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

#include "../Devices/OutputClock.h"

class OutputClockInterface : public SettingsInterface,
	public Label::Listener,
	public Button::Listener
{
public:

	OutputClockInterface(std::shared_ptr<OutputClock> d, OnixSourceEditor* e, OnixSourceCanvas* c);

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

	void updateSettings() override;

	/** Listener methods*/
	void buttonClicked(Button*) override;
	void labelTextChanged(Label* l) override;

private:

	std::unique_ptr<Label> frequencyHzLabel;
	std::unique_ptr<Label> frequencyHzValue;

	std::unique_ptr<Label> dutyCycleLabel;
	std::unique_ptr<Label> dutyCycleValue;

	std::unique_ptr<Label> delayLabel;
	std::unique_ptr<Label> delayValue;

	std::unique_ptr<ToggleButton> gateRunButton;

	const float MinFrequencyHz = 0.1f;
	const float MaxFrequencyHz = 10e6;

	const int MinDutyCyclePercent = 10;
	const int MaxDutyCyclePercent = 90;

	const int MinDelaySeconds = 0;
	const int MaxDelaySeconds = 3600;

	JUCE_LEAK_DETECTOR(OutputClockInterface);
};

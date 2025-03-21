/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2020 Allen Institute for Brain Science and Open Ephys

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

#include "HeartbeatInterface.h"

HeartbeatInterface::HeartbeatInterface(std::shared_ptr<Heartbeat> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	if (device != nullptr)
	{
		nameLabel = std::make_unique<Label>("MAIN", "NAME");
		nameLabel->setFont(FontOptions("Fira Code", "Medium", 30.0f));
		nameLabel->setBounds(50, 40, 370, 45);
		addAndMakeVisible(nameLabel.get());

		deviceEnableButton = std::make_unique<UtilityButton>("ENABLED");
		deviceEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(nameLabel->getX(), nameLabel->getBottom() + 3, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setTooltip("If disabled, Heartbeat will not stream data during acquisition");
		deviceEnableButton->setToggleState(true, dontSendNotification);
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());
		deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);

		infoLabel = std::make_unique<Label>("INFO", "INFO");
		infoLabel->setFont(FontOptions(15.0f));
		infoLabel->setBounds(deviceEnableButton->getX(), deviceEnableButton->getBottom() + 10, 300, 20);
		infoLabel->setJustificationType(Justification::topLeft);
		addAndMakeVisible(infoLabel.get());

		beatsPerSecondLabel = std::make_unique<Label>("beatsPerSecondLabel", "Beats Per Second [Hz]");
		beatsPerSecondLabel->setBounds(infoLabel->getX(), infoLabel->getBottom() + 10, 170, 20);
		addAndMakeVisible(beatsPerSecondLabel.get());

		beatsPerSecondValue = std::make_unique<Label>("beatsPerSecondValue", String(std::static_pointer_cast<Heartbeat>(device)->getBeatsPerSecond()));
		beatsPerSecondValue->setEditable(true);
		beatsPerSecondValue->setBounds(beatsPerSecondLabel->getRight() + 3, beatsPerSecondLabel->getY(), 50, 20);
		beatsPerSecondValue->setTooltip("Sets the rate at which beats are produced. Must be between 1 and 10 MHz.");
		beatsPerSecondValue->setColour(Label::ColourIds::backgroundColourId, Colours::lightgrey);
		beatsPerSecondValue->setJustificationType(Justification::centred);
		beatsPerSecondValue->addListener(this);
		addAndMakeVisible(beatsPerSecondValue.get());

		updateInfoString();
	}

	type = SettingsInterface::Type::HEARTBEAT_SETTINGS_INTERFACE;
}

void HeartbeatInterface::buttonClicked(Button* button)
{
	if (button == deviceEnableButton.get())
	{
		device->setEnabled(deviceEnableButton->getToggleState());
		device->configureDevice();
		if (canvas->foundInputSource()) canvas->resetContext();

		if (device->isEnabled())
		{
			deviceEnableButton->setLabel("ENABLED");
		}
		else
		{
			deviceEnableButton->setLabel("DISABLED");
		}

		CoreServices::updateSignalChain(editor);
	}
}

void HeartbeatInterface::labelTextChanged(Label* l)
{
	if (l == beatsPerSecondValue.get())
	{
		auto d = std::static_pointer_cast<Heartbeat>(device);

		int rate = l->getText().getIntValue();

		if (rate < 1 || rate > 10e6)
		{
			l->setText(String(d->getBeatsPerSecond()), dontSendNotification);
			return;
		}

		d->setBeatsPerSecond((uint32_t)rate, editor->acquisitionIsActive);
	}
}

void HeartbeatInterface::updateInfoString()
{
	String nameString, infoString;

	nameString = "Breakout Board";

	if (device != nullptr)
	{
		infoString = "Device: Heartbeat";
		infoString += "\n";
		infoString += "\n";
	}

	infoLabel->setText(infoString, dontSendNotification);
	nameLabel->setText(nameString, dontSendNotification);
}

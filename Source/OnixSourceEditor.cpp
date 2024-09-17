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

#include "OnixSourceEditor.h"

using namespace Onix;

OnixSourceEditor::OnixSourceEditor(GenericProcessor* parentNode, OnixSource* onixSource)
	: GenericEditor(parentNode), thread(onixSource)
{
	desiredWidth = 180;

	portVoltageLabel = new Label("Voltage", "VOLTAGE [V]");
	portVoltageLabel->setBounds(5, 20, 75, 20);
	portVoltageLabel->setFont(Font("Small Text", 11, Font::plain));
	portVoltageLabel->setColour(Label::textColourId, Colours::black);
	addAndMakeVisible(portVoltageLabel);

	portVoltage = 5.0f;
	portVoltageValue = new Label("VoltageValue", String(portVoltage));
	portVoltageValue->setBounds(10, 38, 30, 13);
	portVoltageValue->setFont(Font("Small Text", 11, Font::plain));
	portVoltageValue->setEditable(true);
	portVoltageValue->setColour(Label::textColourId, Colours::black);
	portVoltageValue->setColour(Label::backgroundColourId, Colours::lightgrey);
	portVoltageValue->addListener(this);
	addAndMakeVisible(portVoltageValue);
	
	portVoltageOverrideButton = new UtilityButton("Apply Voltage [NPX1]", Font("Small Text", 8, Font::plain));
	portVoltageOverrideButton->setBounds(48, 35, 125, 17);
	portVoltageOverrideButton->setRadius(2.0f);
	portVoltageOverrideButton->addListener(this);
	addAndMakeVisible(portVoltageOverrideButton);

	refreshDevicesButton = new UtilityButton("Refresh Devices", Font("Small Text", 9, Font::plain));
	refreshDevicesButton->setBounds(10, 110, 110, 17);
	refreshDevicesButton->setRadius(3.0f);
	refreshDevicesButton->addListener(this);
	addAndMakeVisible(refreshDevicesButton);
}

void OnixSourceEditor::labelTextChanged(Label* l)
{
	if (l == portVoltageValue)
	{
		const float minVoltage = 4.5;
		const float maxVoltage = 6.5;

		float voltage = portVoltageValue->getText().getFloatValue();

		if (voltage >= minVoltage && voltage <= maxVoltage)
		{
			portVoltage = voltage;
		}
		else if (voltage < minVoltage)
		{
			portVoltage = minVoltage;
			portVoltageValue->setText(String(portVoltage), NotificationType::dontSendNotification);
		}
		else if (voltage > maxVoltage)
		{
			portVoltage = maxVoltage;
			portVoltageValue->setText(String(portVoltage), NotificationType::dontSendNotification);
		}
	}
}

void OnixSourceEditor::buttonClicked(Button* b)
{
	if (b == portVoltageOverrideButton)
	{
		thread->setPortVoltage(OnixDeviceType::NEUROPIXELS_1, (oni_dev_idx_t)PortName::PortA, (int)(portVoltage * 10));
	}
	else if (b == refreshDevicesButton)
	{
		thread->initializeDevices();
	}
}

void OnixSourceEditor::updateSettings()
{

}

void OnixSourceEditor::startAcquisition()
{
	portVoltageOverrideButton->setEnabled(false);
	portVoltageOverrideButton->setAlpha(0.3f);

	refreshDevicesButton->setEnabled(false);
	refreshDevicesButton->setAlpha(0.3f);
}

void OnixSourceEditor::stopAcquisition()
{
	portVoltageOverrideButton->setEnabled(true);
	portVoltageOverrideButton->setAlpha(1.0f);

	refreshDevicesButton->setEnabled(true);
	refreshDevicesButton->setAlpha(1.0f);
}

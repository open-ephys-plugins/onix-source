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
#include "OnixSource.h"

OnixSourceEditor::OnixSourceEditor(GenericProcessor* parentNode, OnixSource* onixSource)
	: VisualizerEditor(parentNode, "Onix Source"), thread(onixSource)
{
	canvas = nullptr;

	desiredWidth = 200;

	portVoltageLabel = std::make_unique<Label>("Voltage", "PORT VOLTAGE [V]");
	portVoltageLabel->setBounds(5, 20, 85, 20);
	portVoltageLabel->setFont(FontOptions("Fira Code", "Regular", 11.0f));
	portVoltageLabel->setColour(Label::textColourId, Colours::black);
	addAndMakeVisible(portVoltageLabel.get());

	portVoltage = 5.0f;
	portVoltageValue = std::make_unique<Label>("VoltageValue", String(portVoltage));
	portVoltageValue->setBounds(10, 38, 30, 13);
	portVoltageValue->setFont(FontOptions("Fira Code", "Regular", 11.0f));
	portVoltageValue->setEditable(true);
	portVoltageValue->setColour(Label::textColourId, Colours::black);
	portVoltageValue->setColour(Label::backgroundColourId, Colours::lightgrey);
	portVoltageValue->addListener(this);
	addAndMakeVisible(portVoltageValue.get());

	connectButton = std::make_unique<UtilityButton>("Connect");
	connectButton->setFont(FontOptions("Fira Code", "Regular", 11.0f));
	connectButton->setBounds(10, 95, 65, 20);
	connectButton->setRadius(3.0f);
	connectButton->setClickingTogglesState(true);
	connectButton->setToggleState(false, dontSendNotification);
	connectButton->setTooltip("Press to connect or disconnect from Onix hardware");
	connectButton->addListener(this);
	addAndMakeVisible(connectButton.get());

	passthroughEditor = std::make_unique<ToggleParameterEditor>(onixSource->getParameter("is_passthrough_A"), 20, 95);
	passthroughEditor->setLayout(ParameterEditor::nameOnTop);
	passthroughEditor->setBounds(80, 90, 100, 36);
	addAndMakeVisible(passthroughEditor.get());
}

void OnixSourceEditor::labelTextChanged(Label* l)
{
	if (l == portVoltageValue.get())
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
	if (b == connectButton.get())
	{
		if (connectButton->getToggleState() == true)
		{
			int result = thread->setPortVoltage((oni_dev_idx_t)PortName::PortA, (int)(portVoltage * 10));

			if (result != 0) { CoreServices::sendStatusMessage("Unable to set port voltage to " + String(portVoltage) + " for Port A."); return; }

			thread->initializeDevices(true);
			canvas->refreshTabs();

			connectButton->setLabel("Disconnect");

			if (!thread->foundInputSource())
			{
				CoreServices::sendStatusMessage("No Onix hardware found.");
				connectButton->setToggleState(false, NotificationType::dontSendNotification);
				connectButton->setLabel("Connect");
			}
		}
		else
		{
			int result = thread->setPortVoltage((oni_dev_idx_t)PortName::PortA, 0);

			if (result != 0) { CoreServices::sendStatusMessage("Unable to set port voltage to 0 for Port A."); return; }

			canvas->removeTabs();
			thread->disconnectDevices(true);
			connectButton->setLabel("Connect");
		}
	}
}

void OnixSourceEditor::updateSettings()
{
	if (canvas != nullptr)
		canvas->update();
}

void OnixSourceEditor::startAcquisition()
{
	connectButton->setEnabled(false);
}

void OnixSourceEditor::stopAcquisition()
{
	connectButton->setEnabled(true);
}

Visualizer* OnixSourceEditor::createNewCanvas(void)
{
	GenericProcessor* processor = (GenericProcessor*)getProcessor();
	canvas = new OnixSourceCanvas(processor, this, thread);

	if (acquisitionIsActive)
	{
		canvas->startAcquisition();
	}

	return canvas;
}

void OnixSourceEditor::resetCanvas()
{
	if (canvas != nullptr)
	{
		VisualizerEditor::canvas.reset();

		if (isOpenInTab)
		{
			removeTab();
			addTab();
		}
		else
		{
			checkForCanvas();

			if (dataWindow != nullptr)
				dataWindow->setContentNonOwned(VisualizerEditor::canvas.get(), false);
		}
	}
}

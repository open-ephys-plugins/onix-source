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
	: VisualizerEditor(parentNode, "Onix Source", 220), thread(onixSource)
{
	canvas = nullptr;

	FontOptions fontOptionSmall = FontOptions("Fira Code", "Regular", 12.0f);
	FontOptions fontOptionTitle = FontOptions("Fira Code", "Bold", 15.0f);

	memoryUsage = std::make_unique<MemoryMonitorUsage>(parentNode);
	memoryUsage->setBounds(10, 30, 15, 95);
	memoryUsage->setTooltip("Monitors the percent of the hardware memory buffer used.");
	addAndMakeVisible(memoryUsage.get());

	portLabelA = std::make_unique<Label>("portLabelA", "Port A:");
	portLabelA->setBounds(memoryUsage->getRight() + 5, memoryUsage->getY(), 60, 16);
	portLabelA->setFont(fontOptionTitle);
	addAndMakeVisible(portLabelA.get());

	headstageComboBoxA = std::make_unique<ComboBox>("headstageComboBoxA");
	headstageComboBoxA->setBounds(portLabelA->getRight() + 2, portLabelA->getY(), 120, portLabelA->getHeight());
	headstageComboBoxA->addListener(this);
	headstageComboBoxA->setTooltip("Select the headstage connected to port A.");
	headstageComboBoxA->addItem("Select headstage...", 1);
	headstageComboBoxA->setItemEnabled(1, false);
	headstageComboBoxA->addSeparator();
	// TODO: Add list of available devices here
	headstageComboBoxA->addItem("Neuropixels 1.0f", 2);

	headstageComboBoxA->setSelectedId(1, dontSendNotification);
	addAndMakeVisible(headstageComboBoxA.get());

	portVoltageOverrideLabelA = std::make_unique<Label>("voltageOverrideLabelA", "Voltage");
	portVoltageOverrideLabelA->setBounds(headstageComboBoxA->getX(), headstageComboBoxA->getBottom() + 4, 50, headstageComboBoxA->getHeight());
	portVoltageOverrideLabelA->setFont(fontOptionSmall);
	addAndMakeVisible(portVoltageOverrideLabelA.get());

	portVoltageValueA = std::make_unique<Label>("voltageValueA", "");
	portVoltageValueA->setBounds(portVoltageOverrideLabelA->getRight() + 3, portVoltageOverrideLabelA->getY(), 25, portVoltageOverrideLabelA->getHeight());
	portVoltageValueA->setFont(fontOptionSmall);
	portVoltageValueA->setEditable(true);
	portVoltageValueA->setColour(Label::textColourId, Colours::black);
	portVoltageValueA->setColour(Label::backgroundColourId, Colours::lightgrey);
	portVoltageValueA->setTooltip("Voltage override. If set, overrides the automated voltage discovery algorithm.");
	portVoltageValueA->addListener(this);
	addAndMakeVisible(portVoltageValueA.get());

	portLabelB = std::make_unique<Label>("portLabelB", "Port B:");
	portLabelB->setBounds(portLabelA->getX(), portVoltageOverrideLabelA->getBottom() + 3, portLabelA->getWidth(), portLabelA->getHeight());
	portLabelB->setFont(fontOptionTitle);
	addAndMakeVisible(portLabelB.get());

	headstageComboBoxB = std::make_unique<ComboBox>("headstageComboBoxB");
	headstageComboBoxB->setBounds(portLabelB->getRight(), portLabelB->getY(), headstageComboBoxA->getWidth(), portLabelB->getHeight());
	headstageComboBoxB->addListener(this);
	headstageComboBoxB->setTooltip("Select the headstage connected to port B.");
	headstageComboBoxB->addItem("Select headstage...", 1);
	headstageComboBoxB->setItemEnabled(1, false);
	headstageComboBoxB->addSeparator();
	// TODO: Add list of available devices here
	headstageComboBoxB->addItem("Neuropixels 1.0f", 2);

	headstageComboBoxB->setSelectedId(1, dontSendNotification);
	addAndMakeVisible(headstageComboBoxB.get());

	portVoltageOverrideLabelB = std::make_unique<Label>("voltageOverrideLabelB", "Voltage");
	portVoltageOverrideLabelB->setBounds(headstageComboBoxB->getX(), headstageComboBoxB->getBottom() + 4, portVoltageOverrideLabelA->getWidth(), portVoltageOverrideLabelA->getHeight());
	portVoltageOverrideLabelB->setFont(fontOptionSmall);
	addAndMakeVisible(portVoltageOverrideLabelB.get());

	portVoltageValueB = std::make_unique<Label>("voltageValueB", "");
	portVoltageValueB->setBounds(portVoltageValueA->getX(), portVoltageOverrideLabelB->getY(), portVoltageValueA->getWidth(), portVoltageValueA->getHeight());
	portVoltageValueB->setFont(fontOptionSmall);
	portVoltageValueB->setEditable(true);
	portVoltageValueB->setColour(Label::textColourId, Colours::black);
	portVoltageValueB->setColour(Label::backgroundColourId, Colours::lightgrey);
	portVoltageValueB->setTooltip("Voltage override. If set, overrides the automated voltage discovery algorithm.");
	portVoltageValueB->addListener(this);
	addAndMakeVisible(portVoltageValueB.get());

	connectButton = std::make_unique<UtilityButton>("CONNECT");
	connectButton->setFont(fontOptionSmall);
	connectButton->setBounds(portLabelB->getX() + 5, portVoltageOverrideLabelB->getBottom() + 3, 70, 18);
	connectButton->setRadius(3.0f);
	connectButton->setClickingTogglesState(true);
	connectButton->setToggleState(false, dontSendNotification);
	connectButton->setTooltip("Press to connect or disconnect from Onix hardware");
	connectButton->addListener(this);
	addAndMakeVisible(connectButton.get());
}

void OnixSourceEditor::labelTextChanged(Label* l)
{
	if (l == portVoltageValueA.get())
	{
	}
}

void OnixSourceEditor::buttonClicked(Button* b)
{
	if (b == connectButton.get())
	{
		if (connectButton->getToggleState() == true)
		{
			// NB: Configure port voltages, using either the automated voltage discovery algorithm, or the explicit voltage value given
			if (isHeadstageSelected(PortName::PortA))
			{
				if (!thread->configurePortVoltage(PortName::PortA, portVoltageValueA->getText()))
				{
					CoreServices::sendStatusMessage("Unable to set port voltage for Port A.");
					connectButton->setToggleState(false, true);
					return;
				}
			}
			
			if (isHeadstageSelected(PortName::PortB))
			{
				if (!thread->configurePortVoltage(PortName::PortB, portVoltageValueB->getText()))
				{
					CoreServices::sendStatusMessage("Unable to set port voltage for Port B.");
					connectButton->setToggleState(false, true);
					return;
				}
			}

			thread->initializeDevices(true);
			canvas->refreshTabs();

			connectButton->setLabel("DISCONNECT");

			if (!thread->foundInputSource())
			{
				CoreServices::sendStatusMessage("No Onix hardware found.");
				connectButton->setToggleState(false, true);
			}
		}
		else
		{
			if (!thread->setPortVoltage(PortName::PortA, 0))
			{
				CoreServices::sendStatusMessage("Unable to set port voltage to 0 for Port A.");
				return;
			}

			if (!thread->setPortVoltage(PortName::PortB, 0))
			{
				CoreServices::sendStatusMessage("Unable to set port voltage to 0 for Port B.");
				return;
			}

			canvas->removeTabs();
			thread->disconnectDevices(true);
			connectButton->setLabel("CONNECT");
		}
	}
}

void OnixSourceEditor::comboBoxChanged(ComboBox* cb)
{
	if (cb == headstageComboBoxA.get())
	{
		// TODO: Call canvas to remove / add tabs as needed depending on what is chosen
		String headstage = headstageComboBoxA->getText();

		thread->updateDiscoveryParameters(PortName::PortA, PortController::getHeadstageDiscoveryParameters(headstage));

		if (headstage == "Neuropixels 1.0f")
		{
			thread->getParameter("passthroughA")->setNextValue(false);
		}
		else
		{
			thread->getParameter("passthroughA")->setNextValue(true);
		}
	}
	else if (cb == headstageComboBoxB.get())
	{
		// TODO: Call canvas to remove / add tabs as needed depending on what is chosen
		String headstage = headstageComboBoxB->getText();

		thread->updateDiscoveryParameters(PortName::PortB, PortController::getHeadstageDiscoveryParameters(headstage));

		if (headstage == "Neuropixels 1.0f")
		{
			thread->getParameter("passthroughB")->setNextValue(false);
		}
		else
		{
			thread->getParameter("passthroughB")->setNextValue(true);
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
	// TODO: Disable all UI elements that should not be changed during acquisition...
	connectButton->setEnabled(false);

	for (const auto& source : thread->getDataSources())
	{
		if (source->type == OnixDeviceType::MEMORYMONITOR)
		{
			memoryUsage->setMemoryMonitor(std::static_pointer_cast<MemoryMonitor>(source));
			memoryUsage->startAcquisition();
			break;
		}
	}
}

void OnixSourceEditor::stopAcquisition()
{
  // TODO: Re-enable all of the UI elements 
	connectButton->setEnabled(true);

	for (const auto& source : thread->getDataSources())
	{
		if (source->type == OnixDeviceType::MEMORYMONITOR)
		{
			memoryUsage->stopAcquisition();
			break;
		}
	}
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

bool OnixSourceEditor::isHeadstageSelected(PortName port)
{
	switch (port)
	{
	case PortName::PortA:
		return headstageComboBoxA->getSelectedItemIndex() > 0;
	case PortName::PortB:
		return headstageComboBoxB->getSelectedItemIndex() > 0;
	default:
		return false;
	}
}

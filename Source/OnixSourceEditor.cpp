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

OnixSourceEditor::OnixSourceEditor(GenericProcessor* parentNode, OnixSource* source_)
	: VisualizerEditor(parentNode, "Onix Source", 200), source(source_)
{
	canvas = nullptr;

	FontOptions fontOptionSmall = FontOptions("Fira Code", "Regular", 12.0f);
	FontOptions fontOptionTitle = FontOptions("Fira Code", "Bold", 15.0f);

	portLabelA = std::make_unique<Label>("portLabelA", "Port A:");
	portLabelA->setBounds(4, 25, 60, 16);
	portLabelA->setFont(fontOptionTitle);
	addAndMakeVisible(portLabelA.get());

	headstageComboBoxA = std::make_unique<ComboBox>("headstageComboBoxA");
	headstageComboBoxA->setBounds(portLabelA->getRight() + 2, portLabelA->getY(), 120, portLabelA->getHeight());
	headstageComboBoxA->addListener(this);
	headstageComboBoxA->setTooltip("Select the headstage connected to port A.");
	addHeadstageComboBoxOptions(headstageComboBoxA.get());

	headstageComboBoxA->setSelectedId(1, dontSendNotification);
	addAndMakeVisible(headstageComboBoxA.get());

	passthroughEditorA = std::make_unique<ToggleParameterEditor>(source->getParameter("passthroughA"));
	passthroughEditorA->setLayout(ParameterEditor::nameHidden);
	passthroughEditorA->setBounds(headstageComboBoxA->getX(), headstageComboBoxA->getBottom() + 4, 60, headstageComboBoxA->getHeight());
	addAndMakeVisible(passthroughEditorA.get());

	portVoltageValueA = std::make_unique<Label>("voltageValueA", "");
	portVoltageValueA->setBounds(passthroughEditorA->getRight() + 10, passthroughEditorA->getY(), 35, passthroughEditorA->getHeight());
	portVoltageValueA->setFont(fontOptionSmall);
	portVoltageValueA->setEditable(true);
	portVoltageValueA->setColour(Label::textColourId, Colours::black);
	portVoltageValueA->setColour(Label::backgroundColourId, Colours::lightgrey);
	portVoltageValueA->setTooltip("Voltage override. If set, overrides the automated voltage discovery algorithm.");
	portVoltageValueA->addListener(this);
	addAndMakeVisible(portVoltageValueA.get());

	portLabelB = std::make_unique<Label>("portLabelB", "Port B:");
	portLabelB->setBounds(portLabelA->getX(), passthroughEditorA->getBottom() + 5, portLabelA->getWidth(), portLabelA->getHeight());
	portLabelB->setFont(fontOptionTitle);
	addAndMakeVisible(portLabelB.get());

	headstageComboBoxB = std::make_unique<ComboBox>("headstageComboBoxB");
	headstageComboBoxB->setBounds(portLabelB->getRight(), portLabelB->getY(), headstageComboBoxA->getWidth(), portLabelB->getHeight());
	headstageComboBoxB->addListener(this);
	headstageComboBoxB->setTooltip("Select the headstage connected to port B.");
	addHeadstageComboBoxOptions(headstageComboBoxB.get());

	headstageComboBoxB->setSelectedId(1, dontSendNotification);
	addAndMakeVisible(headstageComboBoxB.get());

	passthroughEditorB = std::make_unique<ToggleParameterEditor>(source->getParameter("passthroughB"));
	passthroughEditorB->setLayout(ParameterEditor::nameHidden);
	passthroughEditorB->setBounds(headstageComboBoxB->getX(), headstageComboBoxB->getBottom() + 4, passthroughEditorA->getWidth(), passthroughEditorA->getHeight());
	addAndMakeVisible(passthroughEditorB.get());

	portVoltageValueB = std::make_unique<Label>("voltageValueB", "");
	portVoltageValueB->setBounds(passthroughEditorB->getRight() + 10, passthroughEditorB->getY(), portVoltageValueA->getWidth(), passthroughEditorB->getHeight());
	portVoltageValueB->setFont(fontOptionSmall);
	portVoltageValueB->setEditable(true);
	portVoltageValueB->setColour(Label::textColourId, Colours::black);
	portVoltageValueB->setColour(Label::backgroundColourId, Colours::lightgrey);
	portVoltageValueB->setTooltip("Voltage override. If set, overrides the automated voltage discovery algorithm.");
	portVoltageValueB->addListener(this);
	addAndMakeVisible(portVoltageValueB.get());

	connectButton = std::make_unique<UtilityButton>("CONNECT");
	connectButton->setFont(fontOptionSmall);
	connectButton->setBounds(portLabelB->getX() + 5, portLabelB->getBottom() + 25, 70, 18);
	connectButton->setRadius(3.0f);
	connectButton->setClickingTogglesState(true);
	connectButton->setToggleState(false, dontSendNotification);
	connectButton->setTooltip("Press to connect or disconnect from Onix hardware");
	connectButton->addListener(this);
	addAndMakeVisible(connectButton.get());
}

void OnixSourceEditor::addHeadstageComboBoxOptions(ComboBox* comboBox)
{
	comboBox->addItem("Select headstage...", 1);
	comboBox->addSeparator();
	comboBox->addItem("Neuropixels 1.0f", 2);
	comboBox->addItem("TEST HEADSTAGE", 99);
	// TODO: Add list of available devices here
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
			if (isHeadstageSelected(PortName::PortA) || !portVoltageValueA->getText().isEmpty())
			{
				if (!source->configurePortVoltage(PortName::PortA, portVoltageValueA->getText()))
				{
					CoreServices::sendStatusMessage("Unable to set port voltage for Port A.");
					connectButton->setToggleState(false, true);
					return;
				}
			}

			if (isHeadstageSelected(PortName::PortB) || !portVoltageValueB->getText().isEmpty())
			{
				if (!source->configurePortVoltage(PortName::PortB, portVoltageValueB->getText()))
				{
					CoreServices::sendStatusMessage("Unable to set port voltage for Port B.");
					connectButton->setToggleState(false, true);
					return;
				}
			}

			source->initializeDevices(false);
			canvas->refreshTabs();

			connectButton->setLabel("DISCONNECT");

			if (!source->foundInputSource())
			{
				CoreServices::sendStatusMessage("No Onix hardware found.");
				connectButton->setToggleState(false, true);
			}
		}
		else
		{
			if (!source->setPortVoltage(PortName::PortA, 0))
			{
				CoreServices::sendStatusMessage("Unable to set port voltage to 0 for Port A.");
				return;
			}

			if (!source->setPortVoltage(PortName::PortB, 0))
			{
				CoreServices::sendStatusMessage("Unable to set port voltage to 0 for Port B.");
				return;
			}

			source->disconnectDevices(true);
			connectButton->setLabel("CONNECT");
		}
	}
}

void OnixSourceEditor::comboBoxChanged(ComboBox* cb)
{
	auto deviceMap = source->createDeviceMap(source->getDataSources());
	auto tabMap = canvas->createTabMap(canvas->settingsInterfaces);
	std::vector<int> deviceIndices;
	std::vector<int> tabIndices;

	for (auto& [key, _] : deviceMap) { deviceIndices.push_back(key); }
	for (auto& [key, _] : tabMap) { tabIndices.push_back(key); }

	auto devicePorts = PortController::getUniquePortsFromIndices(deviceIndices);
	auto tabPorts = PortController::getUniquePortsFromIndices(tabIndices);

	if (cb == headstageComboBoxA.get())
	{
		if (tabPorts.contains(PortName::PortA) && devicePorts.contains(PortName::PortA) && source->foundInputSource())
		{
			AlertWindow::showMessageBox(MessageBoxIconType::WarningIcon, "Devices Connected", "Cannot replace settings tabs while devices are conncted. Disconnect from hardware before changing the selected headstage.");

			refreshComboBoxSelection();
			return;
		}

		if (headstageComboBoxB->getSelectedId() > 1)
			canvas->removeTabs(PortName::PortA);
		else
			canvas->removeAllTabs();

		if (headstageComboBoxA->getSelectedId() > 1)
		{
			String headstage = headstageComboBoxA->getText();

			source->updateDiscoveryParameters(PortName::PortA, PortController::getHeadstageDiscoveryParameters(headstage));
			canvas->addHeadstage(headstage, PortName::PortA);
		}
	}
	else if (cb == headstageComboBoxB.get())
	{
		if (tabPorts.contains(PortName::PortB) && devicePorts.contains(PortName::PortB) && source->foundInputSource())
		{
			AlertWindow::showMessageBox(MessageBoxIconType::WarningIcon, "Devices Connected", "Cannot replace settings tabs while devices are conncted. Disconnect from hardware before changing the selected headstage.");

			refreshComboBoxSelection();
			return;
		}

		if (headstageComboBoxA->getSelectedId() > 1)
			canvas->removeTabs(PortName::PortB);
		else
			canvas->removeAllTabs();

		if (headstageComboBoxB->getSelectedId() > 1)
		{
			String headstage = headstageComboBoxB->getText();

			source->updateDiscoveryParameters(PortName::PortB, PortController::getHeadstageDiscoveryParameters(headstage));
			canvas->addHeadstage(headstage, PortName::PortB);
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
	canvas = new OnixSourceCanvas(processor, this, source);

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

void OnixSourceEditor::setComboBoxSelection(ComboBox* comboBox, String headstage)
{
	for (int i = 0; i < comboBox->getNumItems(); i += 1)
	{
		if (headstage.contains(comboBox->getItemText(i)))
		{
			comboBox->setSelectedItemIndex(i, dontSendNotification);
		}
	}
}

void OnixSourceEditor::refreshComboBoxSelection()
{
	Array<CustomTabComponent*> headstageTabs = canvas->getHeadstageTabs();

	bool resetPortA = true, resetPortB = true;

	for (const auto tab : headstageTabs)
	{
		if (tab->getName().contains(PortController::getPortName(PortName::PortA)))
		{
			setComboBoxSelection(headstageComboBoxA.get(), tab->getName());
			source->updateDiscoveryParameters(PortName::PortA, PortController::getHeadstageDiscoveryParameters(headstageComboBoxA->getText()));
			resetPortA = false;
		}
		else if (tab->getName().contains(PortController::getPortName(PortName::PortB)))
		{
			setComboBoxSelection(headstageComboBoxB.get(), tab->getName());
			source->updateDiscoveryParameters(PortName::PortB, PortController::getHeadstageDiscoveryParameters(headstageComboBoxB->getText()));
			resetPortB = false;
		}
	}

	if (resetPortA) headstageComboBoxA->setSelectedItemIndex(0, dontSendNotification);
	if (resetPortB) headstageComboBoxB->setSelectedItemIndex(0, dontSendNotification);
}

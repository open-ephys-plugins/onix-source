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

	portVoltageOverrideLabelA = std::make_unique<Label>("voltageOverrideLabelA", "Voltage:");
	portVoltageOverrideLabelA->setBounds(portLabelA->getX() + 15, headstageComboBoxA->getBottom() + 4, 62, headstageComboBoxA->getHeight());
	portVoltageOverrideLabelA->setFont(fontOptionSmall);
	addAndMakeVisible(portVoltageOverrideLabelA.get());

	portVoltageValueA = std::make_unique<Label>("voltageValueA", "Auto");
	portVoltageValueA->setBounds(portVoltageOverrideLabelA->getRight() + 3, portVoltageOverrideLabelA->getY(), 40, portVoltageOverrideLabelA->getHeight());
	portVoltageValueA->setFont(fontOptionSmall);
	portVoltageValueA->setEditable(true);
	portVoltageValueA->setColour(Label::textColourId, Colours::black);
	portVoltageValueA->setColour(Label::backgroundColourId, Colours::lightgrey);
	portVoltageValueA->setTooltip("Voltage override. If set, overrides the automated voltage discovery algorithm.");
	portVoltageValueA->addListener(this);
	addAndMakeVisible(portVoltageValueA.get());

	portLabelB = std::make_unique<Label>("portLabelB", "Port B:");
	portLabelB->setBounds(portLabelA->getX(), portVoltageOverrideLabelA->getBottom() + 5, portLabelA->getWidth(), portLabelA->getHeight());
	portLabelB->setFont(fontOptionTitle);
	addAndMakeVisible(portLabelB.get());

	headstageComboBoxB = std::make_unique<ComboBox>("headstageComboBoxB");
	headstageComboBoxB->setBounds(portLabelB->getRight(), portLabelB->getY(), headstageComboBoxA->getWidth(), portLabelB->getHeight());
	headstageComboBoxB->addListener(this);
	headstageComboBoxB->setTooltip("Select the headstage connected to port B.");
	addHeadstageComboBoxOptions(headstageComboBoxB.get());

	headstageComboBoxB->setSelectedId(1, dontSendNotification);
	addAndMakeVisible(headstageComboBoxB.get());

	portVoltageOverrideLabelB = std::make_unique<Label>("voltageOverrideLabelB", "Voltage:");
	portVoltageOverrideLabelB->setBounds(portVoltageOverrideLabelA->getX(), headstageComboBoxB->getBottom() + 4, portVoltageOverrideLabelA->getWidth(), portVoltageOverrideLabelA->getHeight());
	portVoltageOverrideLabelB->setFont(fontOptionSmall);
	addAndMakeVisible(portVoltageOverrideLabelB.get());

	portVoltageValueB = std::make_unique<Label>("voltageValueB", "Auto");
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
	comboBox->addItem(NEUROPIXELSV1F_HEADSTAGE_NAME, 2);
	// TODO: Add list of available devices here
	// TODO: Create const char* for the headstage names so they are shared across the plugin
}

void OnixSourceEditor::labelTextChanged(Label* l)
{
	// TODO: Add headstage specific parameters to limit voltage within safe levels
	if (l == portVoltageValueA.get())
	{
		if (l->getText() == "")
		{
			l->setText("Auto", dontSendNotification);
			return;
		}

		float input = l->getText().getFloatValue();

		if (input < 0.0f)
		{
			l->setText("0.0", dontSendNotification);
		}
		else if (input > 7.0f)
		{
			l->setText("7.0", dontSendNotification);
		}
	}
	else if (l == portVoltageValueB.get())
	{
		if (l->getText() == "")
		{
			l->setText("Auto", dontSendNotification);
			return;
		}

		float input = l->getText().getFloatValue();

		if (input < 0.0f)
		{
			l->setText("0.0", dontSendNotification);
		}
		else if (input > 7.0f)
		{
			l->setText("7.0", dontSendNotification);
		}
	}
}

void OnixSourceEditor::buttonClicked(Button* b)
{
	if (b == connectButton.get())
	{
		setConnectedStatus(connectButton->getToggleState());
	}
}

void OnixSourceEditor::setConnectedStatus(bool connected)
{
	connectButton->setToggleState(connected, dontSendNotification);

	if (connected)
	{
		// NB: Configure port voltages, using either the automated voltage discovery algorithm, or the explicit voltage value given
		if (isHeadstageSelected(PortName::PortA) || portVoltageValueA->getText() != "Auto")
		{
			if (!source->configurePortVoltage(PortName::PortA, portVoltageValueA->getText()))
			{
				CoreServices::sendStatusMessage("Unable to acquire communication lock on Port A.");
			}
		}
		else
		{
			source->setPortVoltage(PortName::PortA, 0);
		}

		if (isHeadstageSelected(PortName::PortB) || portVoltageValueB->getText() != "Auto")
		{
			if (!source->configurePortVoltage(PortName::PortB, portVoltageValueB->getText()))
			{
				CoreServices::sendStatusMessage("Unable to acquire communication lock on Port B.");
			}
		}
		else
		{
			source->setPortVoltage(PortName::PortB, 0);
		}

		source->initializeDevices(false);
		canvas->refreshTabs();

		connectButton->setLabel("DISCONNECT");

		headstageComboBoxA->setEnabled(false);
		headstageComboBoxB->setEnabled(false);

		if (!source->foundInputSource())
		{
			CoreServices::sendStatusMessage("No Onix hardware found.");
			connectButton->setToggleState(false, sendNotification);
		}
	}
	else
	{
		source->setPortVoltage(PortName::PortA, 0);
		source->setPortVoltage(PortName::PortB, 0);

		source->disconnectDevices(true);
		connectButton->setLabel("CONNECT");

		headstageComboBoxA->setEnabled(true);
		headstageComboBoxB->setEnabled(true);
	}
}

void OnixSourceEditor::comboBoxChanged(ComboBox* cb)
{
	if (cb == headstageComboBoxA.get())
	{
		updateComboBox(cb);
	}
	else if (cb == headstageComboBoxB.get())
	{
		updateComboBox(cb);
	}
}

void OnixSourceEditor::updateComboBox(ComboBox* cb)
{
	auto deviceMap = source->createDeviceMap(source->getDataSources());
	auto tabMap = canvas->createSelectedMap(canvas->settingsInterfaces);
	std::vector<int> deviceIndices;
	std::vector<int> tabIndices;

	for (auto& [key, _] : deviceMap) { deviceIndices.push_back(key); }
	for (auto& [key, _] : tabMap) { tabIndices.push_back(key); }

	auto devicePorts = PortController::getUniquePortsFromIndices(deviceIndices);
	auto tabPorts = PortController::getUniquePortsFromIndices(tabIndices);

	bool isPortA = cb == headstageComboBoxA.get();

	PortName currentPort = isPortA ? PortName::PortA : PortName::PortB;

	if (tabPorts.contains(currentPort) && devicePorts.contains(currentPort) && source->foundInputSource())
	{
		AlertWindow::showMessageBox(
			MessageBoxIconType::WarningIcon,
			"Devices Connected",
			"Cannot select a different headstage on " + PortController::getPortName(currentPort) + " when connected. \n\nPress disconnect before changing the selected headstage.");

		refreshComboBoxSelection();
		return;
	}

	bool otherHeadstageSelected = isPortA ? headstageComboBoxB->getSelectedId() > 1 : headstageComboBoxA->getSelectedId() > 1;
	bool currentHeadstageSelected = isPortA ? headstageComboBoxA->getSelectedId() > 1 : headstageComboBoxB->getSelectedId() > 1;

	if (otherHeadstageSelected)
		canvas->removeTabs(currentPort);
	else
		canvas->removeAllTabs();

	if (currentHeadstageSelected)
	{
		String headstage = isPortA ? headstageComboBoxA->getText() : headstageComboBoxB->getText();

		source->updateDiscoveryParameters(currentPort, PortController::getHeadstageDiscoveryParameters(headstage));
		canvas->addHeadstage(headstage, currentPort);

		String passthroughName = isPortA ? "passthroughA" : "passthroughB";

		if (headstage == NEUROPIXELSV1F_HEADSTAGE_NAME)
		{
			source->getParameter(passthroughName)->setNextValue(false);
		}
		else
		{
			source->getParameter(passthroughName)->setNextValue(true);
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

String OnixSourceEditor::getHeadstageSelected(PortName port)
{
	switch (port)
	{
	case PortName::PortA:
		return headstageComboBoxA->getText();
	case PortName::PortB:
		return headstageComboBoxB->getText();
	default:
		return "";
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

std::map<int, OnixDeviceType> OnixSourceEditor::createTabMapFromCanvas()
{
	return canvas->createSelectedMap(canvas->settingsInterfaces);
}

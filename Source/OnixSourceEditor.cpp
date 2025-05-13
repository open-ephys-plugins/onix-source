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

#include "OnixSourceEditor.h"
#include "OnixSource.h"
#include "OnixSourceCanvas.h"

using namespace OnixSourcePlugin;

OnixSourceEditor::OnixSourceEditor(GenericProcessor* parentNode, OnixSource* source_)
	: VisualizerEditor(parentNode, "Onix Source", 250), source(source_)
{
	canvas = nullptr;

	FontOptions fontOptionSmall = FontOptions("Fira Code", "Regular", 12.0f);
	FontOptions fontOptionTitle = FontOptions("Fira Code", "Bold", 15.0f);

	memoryUsage = std::make_unique<MemoryMonitorUsage>(parentNode);
	memoryUsage->setBounds(10, 30, 15, 95);
	memoryUsage->setTooltip("Monitors the percent of the hardware memory buffer used.");
	addAndMakeVisible(memoryUsage.get());

	if (source->isContextInitialized())
	{
		portStatusA = std::make_unique<DrawableRectangle>();
		portStatusA->setRectangle(Rectangle<float>(memoryUsage->getRight() + 4, memoryUsage->getY(), 10, 10));
		portStatusA->setCornerSize(Point<float>(5,5));
		portStatusA->setFill(fillDisconnected);
		portStatusA->setStrokeFill(statusIndicatorStrokeColor);
		portStatusA->setStrokeThickness(statusIndicatorStrokeThickness);
		addAndMakeVisible(portStatusA.get());

		portLabelA = std::make_unique<Label>("portLabelA", "Port A:");
		portLabelA->setBounds(portStatusA->getRight(), portStatusA->getY(), 60, 16);
		portLabelA->setFont(fontOptionTitle);
		addAndMakeVisible(portLabelA.get());

		headstageComboBoxA = std::make_unique<ComboBox>("headstageComboBoxA");
		headstageComboBoxA->setBounds(portLabelA->getRight() + 2, portLabelA->getY(), 140, portLabelA->getHeight());
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

		lastVoltageSetA = std::make_unique<Label>("lastVoltageSetA", "0 V");
		lastVoltageSetA->setBounds(portVoltageValueA->getRight() + 5, portVoltageValueA->getY(), portVoltageValueA->getWidth(), portVoltageValueA->getHeight());
		lastVoltageSetA->setFont(fontOptionSmall);
		lastVoltageSetA->setEditable(false);
		lastVoltageSetA->setTooltip("Records the last voltage set for Port A. Useful for displaying what the automated voltage discovery algorithm settled on.");
		addAndMakeVisible(lastVoltageSetA.get());

		portStatusB = std::make_unique<DrawableRectangle>();
		portStatusB->setRectangle(Rectangle<float>(portStatusA->getX(), portVoltageOverrideLabelA->getBottom() + 3, 10, 10));
		portStatusB->setCornerSize(portStatusA->getCornerSize());
		portStatusB->setFill(portStatusA->getFill());
		portStatusB->setStrokeFill(portStatusA->getStrokeFill());
		portStatusB->setStrokeThickness(statusIndicatorStrokeThickness);
		addAndMakeVisible(portStatusB.get());

		portLabelB = std::make_unique<Label>("portLabelB", "Port B:");
		portLabelB->setBounds(portStatusB->getRight(), portStatusB->getY(), portLabelA->getWidth(), portLabelA->getHeight());
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

		lastVoltageSetB = std::make_unique<Label>("lastVoltageSetB", "0 V");
		lastVoltageSetB->setBounds(portVoltageValueB->getRight() + 5, portVoltageValueB->getY(), portVoltageValueB->getWidth(), portVoltageValueB->getHeight());
		lastVoltageSetB->setFont(fontOptionSmall);
		lastVoltageSetB->setEditable(false);
		lastVoltageSetB->setTooltip("Records the last voltage set for Port B. Useful for displaying what the automated voltage discovery algorithm settled on.");
		addAndMakeVisible(lastVoltageSetB.get());

		connectButton = std::make_unique<UtilityButton>("CONNECT");
		connectButton->setFont(fontOptionSmall);
		connectButton->setBounds(portLabelB->getX() + 5, portVoltageOverrideLabelB->getBottom() + 3, 70, 18);
		connectButton->setRadius(3.0f);
		connectButton->setClickingTogglesState(true);
		connectButton->setToggleState(false, dontSendNotification);
		connectButton->setTooltip("Press to connect or disconnect from Onix hardware");
		connectButton->addListener(this);
		addAndMakeVisible(connectButton.get());

		liboniVersionLabel = std::make_unique<Label>("liboniVersion", "liboni: v" + source->getLiboniVersion());
		liboniVersionLabel->setFont(fontOptionSmall);
		liboniVersionLabel->setBounds(desiredWidth - 100, 110, 95, 22);
		liboniVersionLabel->setEnabled(false);
		liboniVersionLabel->setTooltip("Displays the version of liboni running.");
		addAndMakeVisible(liboniVersionLabel.get());
	}
}

void OnixSourceEditor::addHeadstageComboBoxOptions(ComboBox* comboBox)
{
	comboBox->addItem("Select headstage...", 1);
	comboBox->addSeparator();
	comboBox->addItem(NEUROPIXELSV1F_HEADSTAGE_NAME, 2);
	comboBox->addItem(NEUROPIXELSV2E_HEADSTAGE_NAME, 3);
}

void OnixSourceEditor::labelTextChanged(Label* l)
{
	if (l == portVoltageValueA.get())
	{
		if (l->getText() == "" || l->getText() == "Auto")
		{
			l->setText("Auto", dontSendNotification);
			return;
		}

		auto input = l->getText().getDoubleValue();

		if (input < 0.0)
		{
			l->setText("0.0", dontSendNotification);
		}
		else if (input > 7.0)
		{
			l->setText("7.0", dontSendNotification);
		}
	}
	else if (l == portVoltageValueB.get())
	{
		if (l->getText() == "" || l->getText() == "Auto")
		{
			l->setText("Auto", dontSendNotification);
			return;
		}

		auto input = l->getText().getDoubleValue();

		if (input < 0.0)
		{
			l->setText("0.0", dontSendNotification);
		}
		else if (input > 7.0)
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
		lastVoltageSetA->setText("---", dontSendNotification);

		// NB: Configure port voltages, using either the automated voltage discovery algorithm, or the explicit voltage value given
		if (isHeadstageSelected(PortName::PortA) || portVoltageValueA->getText() != "Auto")
		{
			portStatusA->setFill(fillSearching);

			if (!source->configurePortVoltage(PortName::PortA, portVoltageValueA->getText()))
			{
				CoreServices::sendStatusMessage("Unable to acquire communication lock on Port A.");
				portStatusA->setFill(fillDisconnected);
			}
			else
			{
				portStatusA->setFill(fillConnected);
			}
		}
		else
		{
			source->setPortVoltage(PortName::PortA, 0);
			portStatusA->setFill(fillDisconnected);
		}

		lastVoltageSetA->setText(String(source->getLastVoltageSet(PortName::PortA)) + " V", dontSendNotification);

		lastVoltageSetB->setText("---", dontSendNotification);

		if (isHeadstageSelected(PortName::PortB) || portVoltageValueB->getText() != "Auto")
		{
			portStatusB->setFill(fillSearching);

			if (!source->configurePortVoltage(PortName::PortB, portVoltageValueB->getText()))
			{
				CoreServices::sendStatusMessage("Unable to acquire communication lock on Port B.");
				portStatusB->setFill(fillDisconnected);
			}
			else
			{
				portStatusB->setFill(fillConnected);
			}
		}
		else
		{
			source->setPortVoltage(PortName::PortB, 0);
			portStatusB->setFill(fillDisconnected);
		}

		lastVoltageSetB->setText(String(source->getLastVoltageSet(PortName::PortB)) + " V", dontSendNotification);

		source->initializeDevices(false);

		if (source->foundInputSource())
			canvas->refreshTabs();

		// NB: Check if headstages were not discovered, and then removed
		if (!isHeadstageSelected(PortName::PortA) && source->getLastVoltageSet(PortName::PortA) > 0)
		{
			source->setPortVoltage(PortName::PortA, 0);
			portStatusA->setFill(fillDisconnected);
			lastVoltageSetA->setText(String(source->getLastVoltageSet(PortName::PortA)) + " V", dontSendNotification);
		}

		if (!isHeadstageSelected(PortName::PortB) && source->getLastVoltageSet(PortName::PortB) > 0)
		{
			source->setPortVoltage(PortName::PortB, 0);
			portStatusB->setFill(fillDisconnected);
			lastVoltageSetB->setText(String(source->getLastVoltageSet(PortName::PortB)) + " V", dontSendNotification);
		}

		connectButton->setLabel("DISCONNECT");

		headstageComboBoxA->setEnabled(false);
		headstageComboBoxB->setEnabled(false);
		portVoltageValueA->setEnabled(false);
		portVoltageValueB->setEnabled(false);

		if (!source->foundInputSource())
		{
			CoreServices::sendStatusMessage("Error configuring hardware. Check logs for more details.");
			connectButton->setToggleState(false, sendNotification);
		}
	}
	else
	{
		source->setPortVoltage(PortName::PortA, 0);
		source->setPortVoltage(PortName::PortB, 0);

		lastVoltageSetA->setText(String(source->getLastVoltageSet(PortName::PortA)) + " V", dontSendNotification);
		lastVoltageSetB->setText(String(source->getLastVoltageSet(PortName::PortB)) + " V", dontSendNotification);

		portStatusA->setFill(fillDisconnected);
		portStatusB->setFill(fillDisconnected);

		source->disconnectDevices(true);
		connectButton->setLabel("CONNECT");

		headstageComboBoxA->setEnabled(true);
		headstageComboBoxB->setEnabled(true);
		portVoltageValueA->setEnabled(true);
		portVoltageValueB->setEnabled(true);
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
	auto deviceMap = source->createDeviceMap();
	auto tabMap = canvas->createSelectedMap(canvas->settingsInterfaces);
	std::vector<int> deviceIndices;
	std::vector<int> tabIndices;

	for (auto& [key, _] : deviceMap) { deviceIndices.emplace_back(key); }
	for (auto& [key, _] : tabMap) { tabIndices.emplace_back(key); }

	auto devicePorts = OnixDevice::getUniquePortsFromIndices(deviceIndices);
	auto tabPorts = PortController::getUniquePortsFromIndices(tabIndices);

	bool isPortA = cb == headstageComboBoxA.get();

	PortName currentPort = isPortA ? PortName::PortA : PortName::PortB;

	if (tabPorts.contains(currentPort) && devicePorts.contains(currentPort) && source->foundInputSource())
	{
		AlertWindow::showMessageBox(
			MessageBoxIconType::WarningIcon,
			"Devices Connected",
			"Cannot select a different headstage on " + OnixDevice::getPortName(currentPort) + " when connected. \n\nPress disconnect before changing the selected headstage.");

		refreshComboBoxSelection();
		return;
	}

	bool otherHeadstageSelected = isPortA ? headstageComboBoxB->getSelectedId() > 1 : headstageComboBoxA->getSelectedId() > 1;
	bool currentHeadstageSelected = isPortA ? headstageComboBoxA->getSelectedId() > 1 : headstageComboBoxB->getSelectedId() > 1;

	canvas->removeTabs(currentPort);

	String passthroughName = isPortA ? "passthroughA" : "passthroughB";
	bool passthroughValue = false;

	if (currentHeadstageSelected)
	{
		std::string headstage = isPortA ? headstageComboBoxA->getText().toStdString() : headstageComboBoxB->getText().toStdString();

		source->updateDiscoveryParameters(currentPort, PortController::getHeadstageDiscoveryParameters(headstage));
		canvas->addHub(headstage, PortController::getPortOffset(currentPort));

		if (headstage == NEUROPIXELSV2E_HEADSTAGE_NAME)
		{
			passthroughValue = true;
		}
	}

	source->getParameter(passthroughName)->setNextValue(passthroughValue);

	String voltage = isPortA ? portVoltageValueA->getText() : portVoltageValueB->getText();

	if (voltage != "Auto")
	{
		if (!currentHeadstageSelected)
		{
			if (isPortA)
				portVoltageValueA->setText("", sendNotification);
			else
				portVoltageValueB->setText("", sendNotification);
		}
		else
		{
			int result = AlertWindow::show(MessageBoxOptions()
				.withIconType(MessageBoxIconType::InfoIcon)
				.withTitle("Voltage Override")
				.withMessage(String("There is currently a voltage override selected for this port. Would you like to keep this voltage [Keep Voltage] ") +
					"or allow the automated voltage discovery algorithm [Automated Discovery] to determine the best voltage for the new headstage selected?")
				.withButton("Keep Voltage")
				.withButton("Automated Discovery"));

			switch (result)
			{
			case 1: // Keep Voltage
				break;
			case 0: // Automated Discovery
				if (isPortA)
					portVoltageValueA->setText("", sendNotification);
				else
					portVoltageValueB->setText("", sendNotification);
				break;
			default:
				break;
			}
		}
	}
}

void OnixSourceEditor::updateSettings()
{
	if (canvas != nullptr)
		canvas->update();
}

void OnixSourceEditor::setInterfaceEnabledState(bool newState)
{
	connectButton->setEnabled(newState);

	portVoltageValueA->setEnabled(newState);
	portVoltageValueB->setEnabled(newState);
}

void OnixSourceEditor::startAcquisition()
{
	setInterfaceEnabledState(false);

	for (const auto& source : source->getDataSources())
	{
		if (source->getDeviceType() == OnixDeviceType::MEMORYMONITOR)
		{
			memoryUsage->setMemoryMonitor(std::static_pointer_cast<MemoryMonitor>(source));
			memoryUsage->startAcquisition();
			break;
		}
	}

	if (canvas != nullptr)
		canvas->startAcquisition();
}

void OnixSourceEditor::stopAcquisition()
{
	setInterfaceEnabledState(true);

	for (const auto& source : source->getDataSources())
	{
		if (source->getDeviceType() == OnixDeviceType::MEMORYMONITOR)
		{
			memoryUsage->stopAcquisition();
			break;
		}
	}

	if (canvas != nullptr)
		canvas->stopAcquisition();
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

OnixSourceCanvas* OnixSourceEditor::getCanvas()
{
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

std::string OnixSourceEditor::getHeadstageSelected(int offset)
{
	switch (offset)
	{
	case 0:
		return "Breakout Board";
	case OnixDevice::HubAddressPortA:
		return headstageComboBoxA->getText().toStdString();
	case OnixDevice::HubAddressPortB:
		return headstageComboBoxB->getText().toStdString();
	default:
		return "";
	}
}

std::string OnixSourceEditor::getHeadstageSelected(PortName port)
{
	switch (port)
	{
	case PortName::PortA:
		return getHeadstageSelected(PortController::getPortOffset(port));
	case PortName::PortB:
		return getHeadstageSelected(PortController::getPortOffset(port));
	default:
		return "";
	}
}

void OnixSourceEditor::setComboBoxSelection(ComboBox* comboBox, String headstage)
{
	for (int i = 0; i < comboBox->getNumItems(); i++)
	{
		if (headstage.contains(comboBox->getItemText(i)))
		{
			comboBox->setSelectedItemIndex(i, dontSendNotification);
			return;
		}
	}

	comboBox->setSelectedItemIndex(0, dontSendNotification);
}

void OnixSourceEditor::refreshComboBoxSelection()
{
	Array<CustomTabComponent*> hubTabs = canvas->getHubTabs();

	bool resetPortA = true, resetPortB = true;

	for (const auto tab : hubTabs)
	{
		if (tab->getName().contains(OnixDevice::getPortName(PortName::PortA)))
		{
			setComboBoxSelection(headstageComboBoxA.get(), tab->getName());
			source->updateDiscoveryParameters(PortName::PortA, PortController::getHeadstageDiscoveryParameters(headstageComboBoxA->getText()));
			resetPortA = false;
		}
		else if (tab->getName().contains(OnixDevice::getPortName(PortName::PortB)))
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

void OnixSourceEditor::saveVisualizerEditorParameters(XmlElement* xml)
{
	LOGD("Saving OnixSourceEditor settings.");

	xml->setAttribute("headstagePortA", headstageComboBoxA->getText());
	xml->setAttribute("headstagePortB", headstageComboBoxB->getText());

	xml->setAttribute("portVoltageA", portVoltageValueA->getText());
	xml->setAttribute("portVoltageB", portVoltageValueB->getText());
}

void OnixSourceEditor::loadVisualizerEditorParameters(XmlElement* xml)
{
	LOGD("Loading OnixSourceEditor settings.");

	setComboBoxSelection(headstageComboBoxA.get(), xml->getStringAttribute("headstagePortA"));
	updateComboBox(headstageComboBoxA.get());

	setComboBoxSelection(headstageComboBoxB.get(), xml->getStringAttribute("headstagePortB"));
	updateComboBox(headstageComboBoxB.get());

	portVoltageValueA->setText(xml->getStringAttribute("portVoltageA"), sendNotification);
	portVoltageValueB->setText(xml->getStringAttribute("portVoltageB"), sendNotification);
}

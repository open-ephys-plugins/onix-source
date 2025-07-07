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
#include "Devices/MemoryMonitor.h"

using namespace OnixSourcePlugin;

OnixSourceEditor::OnixSourceEditor(GenericProcessor* parentNode, OnixSource* source_)
	: VisualizerEditor(parentNode, "Onix Source", 250), source(source_)
{
	canvas = nullptr;

	FontOptions fontOptionRegular = FontOptions("Fira Code", 12.0f, Font::plain);
	FontOptions fontOptionTitle = FontOptions("Fira Code", 15.0f, Font::bold);

	if (source->isContextInitialized())
	{
		memoryUsage = std::make_unique<MemoryMonitorUsage>(parentNode);
		memoryUsage->setBounds(8, 28, 14, 80);
		addAndMakeVisible(memoryUsage.get());

		blockReadSizeValue = std::make_unique<Label>("blockReadSizeValue", String(source->getBlockReadSize()));
		blockReadSizeValue->setBounds(memoryUsage->getX() - 4, memoryUsage->getBottom() + 4, 26, 14);
		blockReadSizeValue->setFont(fontOptionRegular);
		blockReadSizeValue->setEditable(true);
		blockReadSizeValue->setColour(Label::textColourId, Colours::black);
		blockReadSizeValue->setTooltip("Number of bytes read per cycle of the acquisition thread. Smaller values provide lower latency, but can cause the memory monitor to fill up. Larger values may improve processing performance for high-bandwidth data sources.");
		blockReadSizeValue->addListener(this);
		blockReadSizeValue->setBorderSize(BorderSize<int>(1));
		blockReadSizeValue->setJustificationType(Justification::centred);
		addAndMakeVisible(blockReadSizeValue.get());

		portStatusA = std::make_unique<DrawableRectangle>();
		portStatusA->setRectangle(Rectangle<float>(memoryUsage->getRight() + 4, memoryUsage->getY(), 10, 10));
		portStatusA->setCornerSize(Point<float>(5, 5));
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
		addAndMakeVisible(headstageComboBoxA.get());

		portVoltageOverrideLabelA = std::make_unique<Label>("voltageOverrideLabelA", "Voltage:");
		portVoltageOverrideLabelA->setBounds(portLabelA->getX() + 15, headstageComboBoxA->getBottom() + 4, 62, headstageComboBoxA->getHeight());
		portVoltageOverrideLabelA->setFont(fontOptionRegular);
		addAndMakeVisible(portVoltageOverrideLabelA.get());

		portVoltageValueA = std::make_unique<Label>("voltageValueA", "Auto");
		portVoltageValueA->setBounds(portVoltageOverrideLabelA->getRight() + 3, portVoltageOverrideLabelA->getY(), 40, portVoltageOverrideLabelA->getHeight());
		portVoltageValueA->setFont(fontOptionRegular);
		portVoltageValueA->setEditable(true);
		portVoltageValueA->setColour(Label::textColourId, Colours::black);
		portVoltageValueA->setColour(Label::backgroundColourId, Colours::lightgrey);
		portVoltageValueA->setTooltip("Voltage override. If set, overrides the automated voltage discovery algorithm.");
		portVoltageValueA->addListener(this);
		addAndMakeVisible(portVoltageValueA.get());

		lastVoltageSetA = std::make_unique<Label>("lastVoltageSetA", "0 V");
		lastVoltageSetA->setBounds(portVoltageValueA->getRight() + 5, portVoltageValueA->getY(), portVoltageValueA->getWidth(), portVoltageValueA->getHeight());
		lastVoltageSetA->setFont(fontOptionRegular);
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
		addAndMakeVisible(headstageComboBoxB.get());

		portVoltageOverrideLabelB = std::make_unique<Label>("voltageOverrideLabelB", "Voltage:");
		portVoltageOverrideLabelB->setBounds(portVoltageOverrideLabelA->getX(), headstageComboBoxB->getBottom() + 4, portVoltageOverrideLabelA->getWidth(), portVoltageOverrideLabelA->getHeight());
		portVoltageOverrideLabelB->setFont(fontOptionRegular);
		addAndMakeVisible(portVoltageOverrideLabelB.get());

		portVoltageValueB = std::make_unique<Label>("voltageValueB", "Auto");
		portVoltageValueB->setBounds(portVoltageValueA->getX(), portVoltageOverrideLabelB->getY(), portVoltageValueA->getWidth(), portVoltageValueA->getHeight());
		portVoltageValueB->setFont(fontOptionRegular);
		portVoltageValueB->setEditable(true);
		portVoltageValueB->setColour(Label::textColourId, Colours::black);
		portVoltageValueB->setColour(Label::backgroundColourId, Colours::lightgrey);
		portVoltageValueB->setTooltip("Voltage override. If set, overrides the automated voltage discovery algorithm.");
		portVoltageValueB->addListener(this);
		addAndMakeVisible(portVoltageValueB.get());

		lastVoltageSetB = std::make_unique<Label>("lastVoltageSetB", "0 V");
		lastVoltageSetB->setBounds(portVoltageValueB->getRight() + 5, portVoltageValueB->getY(), portVoltageValueB->getWidth(), portVoltageValueB->getHeight());
		lastVoltageSetB->setFont(fontOptionRegular);
		lastVoltageSetB->setEditable(false);
		lastVoltageSetB->setTooltip("Records the last voltage set for Port B. Useful for displaying what the automated voltage discovery algorithm settled on.");
		addAndMakeVisible(lastVoltageSetB.get());

		liboniVersionLabel = std::make_unique<Label>("liboniVersion", "liboni: v" + source->getLiboniVersion());
		liboniVersionLabel->setFont(fontOptionRegular);
		liboniVersionLabel->setBounds(portLabelB->getX() + 5, portVoltageOverrideLabelB->getBottom() + 5, 95, 22);
		liboniVersionLabel->setEnabled(false);
		liboniVersionLabel->setTooltip("Displays the version of liboni running.");
		addAndMakeVisible(liboniVersionLabel.get());

		const int connectWidth = 70;

		connectButton = std::make_unique<UtilityButton>("CONNECT");
		connectButton->setFont(fontOptionRegular);
		connectButton->setBounds(headstageComboBoxB->getRight() - connectWidth, liboniVersionLabel->getY(), connectWidth, 18);
		connectButton->setRadius(3.0f);
		connectButton->setClickingTogglesState(true);
		connectButton->setToggleState(false, dontSendNotification);
		connectButton->setTooltip("Press to connect or disconnect from Onix hardware");
		connectButton->addListener(this);
		addAndMakeVisible(connectButton.get());
	}
}

void OnixSourceEditor::addHeadstageComboBoxOptions(ComboBox* comboBox)
{
	comboBox->setTextWhenNothingSelected("Select Headstage...");
	comboBox->addItem("None", 1);
	comboBox->addSeparator();
	comboBox->addItem(NEUROPIXELSV1F_HEADSTAGE_NAME, 2);
	comboBox->addItem(NEUROPIXELSV1E_HEADSTAGE_NAME, 3);
	comboBox->addItem(NEUROPIXELSV2E_HEADSTAGE_NAME, 4);
}

void OnixSourceEditor::labelTextChanged(Label* l)
{
	if (l == portVoltageValueA.get())
	{
		if (l->getText() == "")
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
		if (l->getText() == "")
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
	else if (l == blockReadSizeValue.get())
	{
		auto readSize = l->getText().getIntValue();

		const int32_t minReadSize = 512;
		const int32_t maxReadSize = 20e3;

		if (readSize < minReadSize)
		{
			Onix1::showWarningMessageBoxAsync("Read Size Too Small", "The given read size is too small, it should be greater than " + std::to_string(minReadSize));
			readSize = minReadSize;
		}
		if (readSize > maxReadSize)
		{
			Onix1::showWarningMessageBoxAsync("Read Size Too Big", "The given read size is too big, it should be less than " + std::to_string(maxReadSize));
			readSize = maxReadSize;
		}

		source->setBlockReadSize(readSize);
		l->setText(String(source->getBlockReadSize()), dontSendNotification);
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
		if (!source->configurePort(PortName::PortA) || !source->configurePort(PortName::PortB))
			return;

		if (!configurePortVoltage(PortName::PortA, lastVoltageSetA.get(), portVoltageValueA.get(), portStatusA.get()))
		{
			setPortStatusAndVoltageValue(PortName::PortA, 0.0, fillDisconnected, lastVoltageSetA.get(), portStatusA.get());
			connectButton->setToggleState(false, sendNotification);
			return;
		}

		if (!configurePortVoltage(PortName::PortB, lastVoltageSetB.get(), portVoltageValueB.get(), portStatusB.get()))
		{
			setPortStatusAndVoltageValue(PortName::PortA, 0.0, fillDisconnected, lastVoltageSetA.get(), portStatusA.get());
			setPortStatusAndVoltageValue(PortName::PortB, 0.0, fillDisconnected, lastVoltageSetB.get(), portStatusB.get());
			connectButton->setToggleState(false, sendNotification);
			return;
		}

		if (!configureAllDevices())
		{
			setPortStatusAndVoltageValue(PortName::PortA, 0.0, fillDisconnected, lastVoltageSetA.get(), portStatusA.get());
			setPortStatusAndVoltageValue(PortName::PortB, 0.0, fillDisconnected, lastVoltageSetB.get(), portStatusB.get());
			connectButton->setToggleState(false, dontSendNotification);
			return;
		}

		// NB: Check if headstages were not discovered, and then removed
		if (!isHeadstageSelected(PortName::PortA) && source->getLastVoltageSet(PortName::PortA) > 0)
			setPortStatusAndVoltageValue(PortName::PortA, 0.0, fillDisconnected, lastVoltageSetA.get(), portStatusA.get());

		if (!isHeadstageSelected(PortName::PortB) && source->getLastVoltageSet(PortName::PortB) > 0)
			setPortStatusAndVoltageValue(PortName::PortB, 0.0, fillDisconnected, lastVoltageSetB.get(), portStatusB.get());

		connectButton->setLabel("DISCONNECT");

		enableEditorElements(false);
	}
	else
	{
		setPortStatusAndVoltageValue(PortName::PortA, 0.0, fillDisconnected, lastVoltageSetA.get(), portStatusA.get());
		setPortStatusAndVoltageValue(PortName::PortB, 0.0, fillDisconnected, lastVoltageSetB.get(), portStatusB.get());

		source->disconnectDevices(true);
		connectButton->setLabel("CONNECT");

		enableEditorElements(true);
	}
}

bool OnixSourceEditor::configurePortVoltage(PortName port, Label* lastVoltageSet, Label* portVoltageValue, DrawableRectangle* portStatus)
{
	bool result = false;

	lastVoltageSet->setText("---", dontSendNotification);

	// NB: Configure port voltages, using either the automated voltage discovery algorithm, or the explicit voltage value given
	if (isHeadstageSelected(port) || portVoltageValue->getText() != "Auto")
	{
		portStatus->setFill(fillSearching);

		if (!source->configurePortVoltage(port, portVoltageValue->getText().toStdString()))
		{
			Onix1::showWarningMessageBoxAsync("Communication Error", "Unable to acquire communication lock on " + OnixDevice::getPortName(port) + ".");
			portStatus->setFill(fillDisconnected);
			result = false;
		}
		else
		{
			portStatus->setFill(fillConnected);
			result = true;
		}
	}
	else
	{
		source->setPortVoltage(port, 0);
		portStatus->setFill(fillDisconnected);
		result = true;
	}

	lastVoltageSet->setText(String(source->getLastVoltageSet(port)) + " V", dontSendNotification);

	return result;
}

void OnixSourceEditor::setPortStatusAndVoltageValue(PortName port, double voltage, FillType fill, Label* lastVoltageSet, DrawableRectangle* portStatus)
{
	source->setPortVoltage(port, 0);
	portStatus->setFill(fill);
	lastVoltageSet->setText(String(source->getLastVoltageSet(port)) + " V", dontSendNotification);
}

bool OnixSourceEditor::configureAllDevices()
{
	if (source->foundInputSource() && !source->disconnectDevices(false))
		return false;

	if (!OnixSource::enablePassthroughMode(source->getContext(), source->getParameter("passthroughA")->getValue(), source->getParameter("passthroughB")->getValue()))
	{
		Onix1::showWarningMessageBoxAsync("Passthrough Configuration Error", "Unable to set passthrough mode. Check logs for more details.");
		return false;
	}

	device_map_t deviceTable;
	if (!source->getDeviceTable(&deviceTable))
	{
		Onix1::showWarningMessageBoxAsync("Device Table Error", "An error occurred when trying to get the device table. Check logs for more details.");
		return false;
	}

	if (!OnixSource::checkHubFirmwareCompatibility(source->getContext(), deviceTable))
		return false;

	if (!source->initializeDevices(deviceTable, false) || !canvas->verifyHeadstageSelection())
	{
		setConnectedStatus(false);
		return false;
	}
	
	if (!source->configureBlockReadSize(source->getContext(), blockReadSizeValue->getText().getIntValue()))
	{
		connectButton->setToggleState(false, sendNotification);
		return false;
	}

	return true;
}

void OnixSourceEditor::enableEditorElements(bool enable)
{
	headstageComboBoxA->setEnabled(enable);
	headstageComboBoxB->setEnabled(enable);
	portVoltageValueA->setEnabled(enable);
	portVoltageValueB->setEnabled(enable);
	blockReadSizeValue->setEnabled(enable);
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
	bool isPortA = cb == headstageComboBoxA.get();

	PortName currentPort = isPortA ? PortName::PortA : PortName::PortB;

	bool currentHeadstageSelected = isPortA ? headstageComboBoxA->getSelectedId() > 1 : headstageComboBoxB->getSelectedId() > 1;

	canvas->removeTabs(currentPort);

	std::string passthroughName = isPortA ? "passthroughA" : "passthroughB";
	bool passthroughValue = false;

	if (currentHeadstageSelected)
	{
		std::string headstage = isPortA ? headstageComboBoxA->getText().toStdString() : headstageComboBoxB->getText().toStdString();

		source->updateDiscoveryParameters(currentPort, PortController::getHeadstageDiscoveryParameters(headstage));
		canvas->addHub(headstage, PortController::getPortOffset(currentPort));

		if (headstage == NEUROPIXELSV2E_HEADSTAGE_NAME || headstage == NEUROPIXELSV1E_HEADSTAGE_NAME)
		{
			passthroughValue = true;
		}
	}
	else
	{
		cb->setSelectedId(0, dontSendNotification);
	}

	source->getParameter(passthroughName)->setNextValue(passthroughValue);

	std::string voltage = isPortA ? portVoltageValueA->getText().toStdString() : portVoltageValueB->getText().toStdString();

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
				.withMessage(std::string("There is currently a voltage override selected for this port. Would you like to keep this voltage [Keep Voltage] ") +
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

void OnixSourceEditor::setComboBoxSelection(ComboBox* comboBox, std::string headstage)
{
	String headstage_ = headstage;

	for (int i = 0; i < comboBox->getNumItems(); i++)
	{
		if (headstage_.contains(comboBox->getItemText(i)))
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
			setComboBoxSelection(headstageComboBoxA.get(), tab->getName().toStdString());
			source->updateDiscoveryParameters(PortName::PortA, PortController::getHeadstageDiscoveryParameters(headstageComboBoxA->getText().toStdString()));
			resetPortA = false;
		}
		else if (tab->getName().contains(OnixDevice::getPortName(PortName::PortB)))
		{
			setComboBoxSelection(headstageComboBoxB.get(), tab->getName().toStdString());
			source->updateDiscoveryParameters(PortName::PortB, PortController::getHeadstageDiscoveryParameters(headstageComboBoxB->getText().toStdString()));
			resetPortB = false;
		}
	}

	if (resetPortA) headstageComboBoxA->setSelectedItemIndex(0, dontSendNotification);
	if (resetPortB) headstageComboBoxB->setSelectedItemIndex(0, dontSendNotification);
}

OnixDeviceMap OnixSourceEditor::createTabMapFromCanvas()
{
	return canvas->getSelectedDevices(canvas->settingsInterfaces);
}

void OnixSourceEditor::saveVisualizerEditorParameters(XmlElement* xml)
{
	LOGD("Saving OnixSourceEditor settings.");

	xml->setAttribute("headstagePortA", headstageComboBoxA->getText());
	xml->setAttribute("headstagePortB", headstageComboBoxB->getText());

	xml->setAttribute("portVoltageA", portVoltageValueA->getText());
	xml->setAttribute("portVoltageB", portVoltageValueB->getText());

	xml->setAttribute("blockReadSize", String(source->getBlockReadSize()));
}

void OnixSourceEditor::loadVisualizerEditorParameters(XmlElement* xml)
{
	LOGD("Loading OnixSourceEditor settings.");

	if (xml->hasAttribute("headstagePortA"))
	{
		setComboBoxSelection(headstageComboBoxA.get(), xml->getStringAttribute("headstagePortA").toStdString());
		updateComboBox(headstageComboBoxA.get());
	}

	if (xml->hasAttribute("headstagePortB"))
	{
		setComboBoxSelection(headstageComboBoxB.get(), xml->getStringAttribute("headstagePortB").toStdString());
		updateComboBox(headstageComboBoxB.get());
	}

	if (xml->hasAttribute("portVoltageA"))
		portVoltageValueA->setText(xml->getStringAttribute("portVoltageA"), sendNotification);

	if (xml->hasAttribute("portVoltageB"))
		portVoltageValueB->setText(xml->getStringAttribute("portVoltageB"), sendNotification);

	if (xml->hasAttribute("blockReadSize"))
		blockReadSizeValue->setText(xml->getStringAttribute("blockReadSize"), sendNotification);
}

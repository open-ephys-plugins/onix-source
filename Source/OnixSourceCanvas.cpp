/*
	------------------------------------------------------------------

	Copyright(C) Open Ephys

	------------------------------------------------------------------

	This program is free software : you can redistribute it and /or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < http://www.gnu.org/licenses/>.

*/

#include "OnixSourceCanvas.h"
#include "OnixSource.h"

OnixSourceCanvas::OnixSourceCanvas(GenericProcessor* processor_, OnixSourceEditor* editor_, OnixSource* onixSource_) :
	Visualizer(processor_),
	editor(editor_),
	source(onixSource_)
{
	topLevelTabComponent = std::make_unique<CustomTabComponent>(true);
	addAndMakeVisible(topLevelTabComponent.get());

	addHub(BREAKOUT_BOARD_NAME, 0);
}

CustomTabComponent* OnixSourceCanvas::addTopLevelTab(String tabName, int index)
{
	CustomTabComponent* tab = new CustomTabComponent(false);

	topLevelTabComponent->addTab(tabName, Colours::grey, tab, true, index);
	tab->setName(tabName);

	headstageTabs.add(tab);

	return tab;
}

void OnixSourceCanvas::addHub(String hubName, int offset)
{
	CustomTabComponent* tab = nullptr;
	OnixDeviceVector devices;
	PortName port = PortController::getPortFromIndex(offset);

	if (hubName == NEUROPIXELSV1F_HEADSTAGE_NAME)
	{
		tab = addTopLevelTab(getTopLevelTabName(port, hubName), (int)port);

		devices.emplace_back(std::make_shared<Neuropixels_1>("Probe-A", offset, nullptr));
		devices.emplace_back(std::make_shared<Neuropixels_1>("Probe-B", offset + 1, nullptr));
		devices.emplace_back(std::make_shared<Bno055>("BNO055", offset + 2, nullptr));
	}
	else if (hubName == BREAKOUT_BOARD_NAME)
	{
		tab = addTopLevelTab(hubName, 0);

		devices.emplace_back(std::make_shared<OutputClock>("Output Clock", 5, nullptr));
		devices.emplace_back(std::make_shared<AnalogIO>("Analog IO", 6, nullptr));
		devices.emplace_back(std::make_shared<DigitalIO>("Digital IO", 7, nullptr));
		devices.emplace_back(std::make_shared<HarpSyncInput>("Harp Sync Input", 12, nullptr));
	}
	else if (hubName == NEUROPIXELSV2E_HEADSTAGE_NAME)
	{
		const int passthroughIndex = (offset >> 8) + 7;

		tab = addTopLevelTab(getTopLevelTabName(port, hubName), (int)port);

		devices.emplace_back(std::make_shared<Neuropixels2e>("Neuropixels 2e-A", passthroughIndex, nullptr));
		devices.emplace_back(std::make_shared<PolledBno055>("BNO055", passthroughIndex, nullptr));
	}

	if (tab != nullptr && devices.size() > 0)
	{
		populateSourceTabs(tab, devices);
	}
}

void OnixSourceCanvas::populateSourceTabs(CustomTabComponent* tab, OnixDeviceVector devices)
{
	int portTabNumber = 0;

	for (const auto& device : devices)
	{
		if (device->type == OnixDeviceType::NEUROPIXELS_1)
		{
			auto neuropixInterface = std::make_shared<NeuropixV1Interface>(std::static_pointer_cast<Neuropixels_1>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device), tab, neuropixInterface);
		}
		else if (device->type == OnixDeviceType::BNO)
		{
			auto bno055Interface = std::make_shared<Bno055Interface>(std::static_pointer_cast<Bno055>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device), tab, bno055Interface);
		}
		else if (device->type == OnixDeviceType::OUTPUTCLOCK)
		{
			auto outputClockInterface = std::make_shared<OutputClockInterface>(std::static_pointer_cast<OutputClock>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device), tab, outputClockInterface);
		}
		else if (device->type == OnixDeviceType::HARPSYNCINPUT)
		{
			auto harpSyncInputInterface = std::make_shared<HarpSyncInputInterface>(std::static_pointer_cast<HarpSyncInput>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device), tab, harpSyncInputInterface);
		}
		else if (device->type == OnixDeviceType::ANALOGIO)
		{
			auto analogIOInterface = std::make_shared<AnalogIOInterface>(std::static_pointer_cast<AnalogIO>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device), tab, analogIOInterface);
		}
		else if (device->type == OnixDeviceType::DIGITALIO)
		{
			auto digitalIOInterface = std::make_shared<DigitalIOInterface>(std::static_pointer_cast<DigitalIO>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device), tab, digitalIOInterface);
		}
		else if (device->type == OnixDeviceType::NEUROPIXELSV2E)
		{
			auto npxv2eInterface = std::make_shared<NeuropixelsV2eInterface>(std::static_pointer_cast<Neuropixels2e>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device), tab, npxv2eInterface);
		}
		else if (device->type == OnixDeviceType::POLLEDBNO)
		{
			auto polledBnoInterface = std::make_shared<PolledBno055Interface>(std::static_pointer_cast<PolledBno055>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device), tab, polledBnoInterface);
		}
	}
}

void OnixSourceCanvas::addInterfaceToTab(String tabName, CustomTabComponent* tab, std::shared_ptr<SettingsInterface> interface_)
{
	settingsInterfaces.emplace_back(interface_);
	tab->addTab(tabName, Colours::darkgrey, CustomViewport::createCustomViewport(interface_.get()), true);
}

bool OnixSourceCanvas::compareDeviceNames(String dev1, String dev2)
{
	StringRef charsToTrim = "-ABCD";

	if (dev1 == dev2)
		return true;
	else if (dev1.trimCharactersAtEnd(charsToTrim) == dev2.trimCharactersAtEnd(charsToTrim))
		return true;
	else
		return false;
}

void OnixSourceCanvas::updateSettingsInterfaceDataSource(std::shared_ptr<OnixDevice> device)
{
	int ind = -1;

	for (int j = 0; j < settingsInterfaces.size(); j++)
	{
		if (device->getDeviceIdx() == settingsInterfaces[j]->device->getDeviceIdx() &&
			compareDeviceNames(device->getName(), settingsInterfaces[j]->device->getName()))
		{
			ind = j;
			break;
		}
	}

	if (ind == -1)
	{
		if (device->type != OnixDeviceType::MEMORYMONITOR && device->type != OnixDeviceType::HEARTBEAT)
			LOGD("Unable to match " + device->getName() + " to an open tab.");

		return;
	}

	if (device->type == OnixDeviceType::NEUROPIXELS_1)
	{
		auto npx1Found = std::static_pointer_cast<Neuropixels_1>(device);
		auto npx1Selected = std::static_pointer_cast<Neuropixels_1>(settingsInterfaces[ind]->device);
		npx1Found->setSettings(npx1Selected->settings[0].get());
		npx1Found->adcCalibrationFilePath = npx1Selected->adcCalibrationFilePath;
		npx1Found->gainCalibrationFilePath = npx1Selected->gainCalibrationFilePath;
		npx1Found->setCorrectOffset(npx1Selected->getCorrectOffset());
		npx1Found->setInvertSignal(npx1Selected->getInvertSignal());
	}
	else if (device->type == OnixDeviceType::OUTPUTCLOCK)
	{
		auto outputClockFound = std::static_pointer_cast<OutputClock>(device);
		auto outputClockSelected = std::static_pointer_cast<OutputClock>(settingsInterfaces[ind]->device);
		outputClockFound->setDelay(outputClockSelected->getDelay());
		outputClockFound->setDutyCycle(outputClockSelected->getDutyCycle());
		outputClockFound->setFrequencyHz(outputClockSelected->getFrequencyHz());
		outputClockFound->setGateRun(outputClockSelected->getGateRun());
	}
	else if (device->type == OnixDeviceType::ANALOGIO)
	{
		auto analogIOFound = std::static_pointer_cast<AnalogIO>(device);
		auto analogIOSelected = std::static_pointer_cast<AnalogIO>(settingsInterfaces[ind]->device);
		for (int i = 0; i < analogIOFound->getNumChannels(); i++)
		{
			analogIOFound->setChannelDirection(i, analogIOSelected->getChannelDirection(i));
		}
	}
	else if (device->type == OnixDeviceType::NEUROPIXELSV2E)
	{
		auto npx2Found = std::static_pointer_cast<Neuropixels2e>(device);
		auto npx2Selected = std::static_pointer_cast<Neuropixels2e>(settingsInterfaces[ind]->device);
		npx2Found->setSettings(npx2Selected->settings[0].get(), 0);
		npx2Found->setSettings(npx2Selected->settings[1].get(), 1);
		npx2Found->setGainCorrectionFile(0, npx2Selected->getGainCorrectionFile(0));
		npx2Found->setGainCorrectionFile(1, npx2Selected->getGainCorrectionFile(1));
		npx2Found->setCorrectOffset(npx2Selected->getCorrectOffset());
		npx2Found->setInvertSignal(npx2Selected->getInvertSignal());

		std::static_pointer_cast<NeuropixelsV2eInterface>(settingsInterfaces[ind])->updateDevice(npx2Found);
	}

	device->setEnabled(settingsInterfaces[ind]->device->isEnabled());
	settingsInterfaces[ind]->device.reset();
	settingsInterfaces[ind]->device = device;
}

String OnixSourceCanvas::getTopLevelTabName(PortName port, String headstage)
{
	return PortController::getPortName(port) + ": " + headstage;
}

String OnixSourceCanvas::getDeviceTabName(std::shared_ptr<OnixDevice> device)
{
	return device->getName();
}

Array<CustomTabComponent*> OnixSourceCanvas::getHeadstageTabs()
{
	Array<CustomTabComponent*> tabs;

	for (const auto headstage : headstageTabs)
	{
		tabs.add(headstage);
	}

	return tabs;
}

void OnixSourceCanvas::refresh()
{
	repaint();
}

void OnixSourceCanvas::refreshState()
{
	resized();
}

void OnixSourceCanvas::removeTabs(PortName port)
{
	bool tabExists = false;

	for (int i = headstageTabs.size() - 1; i >= 0; i -= 1)
	{
		if (headstageTabs[i]->getName().contains(PortController::getPortName(port)))
		{
			headstageTabs.remove(i, true);
			tabExists = true;
			break;
		}
	}

	int offset = PortController::getPortOffset(port);

	for (int i = settingsInterfaces.size() - 1; i >= 0; i -= 1)
	{
		if ((settingsInterfaces[i]->device->getDeviceIdx() & offset) > 0)
		{
			settingsInterfaces.erase(settingsInterfaces.begin() + i);
			tabExists = true;
		}
	}

	if (tabExists)
	{
		if (port == PortName::PortB && headstageTabs.size() == 1 && headstageTabs[0]->getName().contains(BREAKOUT_BOARD_NAME))
			topLevelTabComponent->removeTab((int)port - 1); // NB: If only one headstage is selected in the editor, the index needs to be corrected here.
		else
			topLevelTabComponent->removeTab((int)port);
	}
}

void OnixSourceCanvas::removeAllTabs()
{
	headstageTabs.clear(true);
	settingsInterfaces.clear();

	topLevelTabComponent->clearTabs();
}

std::map<int, OnixDeviceType> OnixSourceCanvas::createSelectedMap(std::vector<std::shared_ptr<SettingsInterface>> interfaces)
{
	std::map<int, OnixDeviceType> tabMap;

	for (const auto& settings : interfaces)
	{
		tabMap.insert({ settings->device->getDeviceIdx(), settings->device->type });
	}

	return tabMap;
}

void OnixSourceCanvas::askKeepRemove(int offset)
{
	String selectedHeadstage = editor->getHeadstageSelected(offset);

	String msg = "Headstage " + selectedHeadstage + " is selected on " + PortController::getPortName(offset) + ", but was not discovered there.\n\n";
	msg += "Select one of the options below to continue:\n";
	msg += " [Keep Current] to keep " + selectedHeadstage + " selected.\n";
	msg += " [Remove] to remove " + selectedHeadstage + ".\n - Note: this will delete any settings that were modified.";

	int result = AlertWindow::show(
		MessageBoxOptions()
		.withIconType(MessageBoxIconType::WarningIcon)
		.withTitle("Headstage Not Found")
		.withMessage(msg)
		.withButton("Keep Current")
		.withButton("Remove")
	);

	switch (result)
	{
	case 0: // Remove
		removeTabs(PortController::getPortFromIndex(offset));
		break;
	case 1: // Keep Current
		break;
	default:
		break;
	}
}

void OnixSourceCanvas::askKeepUpdate(int offset, String foundHeadstage, OnixDeviceVector devices)
{
	String selectedHeadstage = editor->getHeadstageSelected(offset);

	if (selectedHeadstage == foundHeadstage) return;

	String msg = "Headstage " + selectedHeadstage + " is selected on " + PortController::getPortName(offset) + ". ";
	msg += "However, headstage " + foundHeadstage + " was found on " + PortController::getPortName(offset) + ". \n\n";
	msg += "Select one of the options below to continue:\n";
	msg += " [Keep Current] to keep " + selectedHeadstage + " selected.\n";
	msg += " [Update] to change the selected headstage to " + foundHeadstage + ".\n - Note: this will delete any settings that were modified.";

	int result = AlertWindow::show(
		MessageBoxOptions()
		.withIconType(MessageBoxIconType::WarningIcon)
		.withTitle("Mismatched Headstages")
		.withMessage(msg)
		.withButton("Keep Current")
		.withButton("Update")
	);

	switch (result)
	{
	case 0: // Update
	{
		PortName port = PortController::getPortFromIndex(offset);
		removeTabs(port);

		CustomTabComponent* tab = addTopLevelTab(getTopLevelTabName(port, foundHeadstage), (int)port);
		populateSourceTabs(tab, devices);
	}
	break;
	case 1: // Keep Current
		break;
	default:
		break;
	}
}

void OnixSourceCanvas::refreshTabs()
{
	auto selectedMap = createSelectedMap(settingsInterfaces);
	auto foundMap = source->createDeviceMap(true);

	if (selectedMap != foundMap)
	{
		std::vector<int> selectedIndices, foundIndices;

		for (const auto& [key, _] : selectedMap) { selectedIndices.emplace_back(key); }
		for (const auto& [key, _] : foundMap) { foundIndices.emplace_back(key); }

		auto selectedOffsets = PortController::getUniqueOffsetsFromIndices(selectedIndices);
		auto foundOffsets = PortController::getUniqueOffsetsFromIndices(foundIndices);

		if (foundIndices.size() == 0) // NB: No devices found, inform the user if they were expecting to find something
		{
			if (selectedMap.size() != 0)
			{
				AlertWindow::showMessageBox(
					MessageBoxIconType::WarningIcon,
					"No Headstages Found",
					"No headstages were found when connecting. Double check that the correct headstage is selected. " +
					String("If the correct headstage is selected, try pressing disconnect / connect again.\n\n") +
					String("If the port voltage is manually set, try clearing the value and letting the automated voltage discovery algorithm run.")
				);
			}
		}
		else if (selectedIndices.size() == 0) // NB: No headstages selected, add all found headstages
		{
			for (auto& [offset, headstageName] : source->getHeadstageMap())
			{
				addHub(headstageName, offset);
			}
		}
		else if (selectedOffsets.size() == foundOffsets.size()) // NB: Same number of ports selected and found
		{
			auto headstages = source->getHeadstageMap();

			if (selectedOffsets.size() == 1)
			{
				if (headstages.size() != 1)
				{
					LOGE("Wrong number of headstages found in the source node.");
					return;
				}

				if (selectedOffsets[0] == foundOffsets[0]) // NB: Selected headstage is different from the found headstage on the same port
				{
					askKeepUpdate(selectedOffsets[0], headstages[foundOffsets[0]], source->getDataSources());
				}
				else // NB: Selected headstage on one port is not found, and the found headstage is not selected on the other port
				{
					askKeepRemove(selectedOffsets[0]);

					addHub(headstages[foundOffsets[0]], foundOffsets[0]);
				}
			}
			else // NB: Two headstages are selected on different ports, and at least one of those headstages does not match the found headstages
			{
				for (auto offset : foundOffsets)
				{
					if (headstages[offset] != editor->getHeadstageSelected(offset))
					{
						askKeepUpdate(offset, headstages[offset], source->getDataSourcesFromOffset(offset));
					}
				}
			}
		}
		else // NB: Different number of ports selected versus found
		{
			auto headstages = source->getHeadstageMap();

			if (selectedOffsets.size() > foundOffsets.size()) // NB: More headstages selected than found
			{
				for (auto offset : selectedOffsets)
				{
					if (offset == foundOffsets[0])
					{
						if (headstages[offset] != editor->getHeadstageSelected(offset))
						{
							askKeepUpdate(offset, headstages[offset], source->getDataSourcesFromOffset(offset));
						}
					}
					else
					{
						askKeepRemove(offset);
					}
				}
			}
			else // NB: More headstages found than selected
			{
				for (auto offset : foundOffsets)
				{
					if (offset == selectedOffsets[0])
					{
						if (headstages[offset] != editor->getHeadstageSelected(offset))
						{
							askKeepUpdate(offset, headstages[offset], source->getDataSourcesFromOffset(offset));
						}
					}
					else
					{
						addHub(headstages[offset], offset);
					}
				}
			}
		}
	}

	for (const auto& device : source->getDataSources())
	{
		updateSettingsInterfaceDataSource(device);
	}

	CoreServices::updateSignalChain(editor);
	editor->refreshComboBoxSelection();
}

void OnixSourceCanvas::update() const
{
	for (int i = 0; i < settingsInterfaces.size(); i++)
		settingsInterfaces[i]->updateInfoString();
}

void OnixSourceCanvas::resized()
{
	topLevelTabComponent->setBounds(0, -3, getWidth(), getHeight() + 3);
}

void OnixSourceCanvas::resetContext()
{
	source->resetContext();
}

bool OnixSourceCanvas::foundInputSource()
{
	return source->foundInputSource();
}

void OnixSourceCanvas::startAcquisition()
{
	for (const auto& settingsInterface : settingsInterfaces)
	{
		if (settingsInterface->device != nullptr && settingsInterface->device->isEnabled())
		{
			settingsInterface->startAcquisition();
		}
	}
}

void OnixSourceCanvas::stopAcquisition()
{
	for (const auto& settingsInterface : settingsInterfaces)
	{
		if (settingsInterface->device != nullptr && settingsInterface->device->isEnabled())
		{
			settingsInterface->stopAcquisition();
		}
	}
}

void OnixSourceCanvas::saveCustomParametersToXml(XmlElement* xml)
{
	//for (int i = 0; i < settingsInterfaces.size(); i++)
	//	settingsInterfaces[i]->saveParameters(xml);
}

void OnixSourceCanvas::loadCustomParametersFromXml(XmlElement* xml)
{
	//for (int i = 0; i < settingsInterfaces.size(); i++)
	//	settingsInterfaces[i]->loadParameters(xml);
}

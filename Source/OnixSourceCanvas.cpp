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
#include "OnixSourceEditor.h"

using namespace OnixSourcePlugin;

OnixSourceCanvas::OnixSourceCanvas(GenericProcessor* processor_, OnixSourceEditor* editor_, OnixSource* onixSource_) :
	Visualizer(processor_),
	editor(editor_),
	source(onixSource_)
{
	topLevelTabComponent = std::make_unique<CustomTabComponent>(true);
	addAndMakeVisible(topLevelTabComponent.get());

	addHub(BREAKOUT_BOARD_NAME, 0);
}

CustomTabComponent* OnixSourceCanvas::addTopLevelTab(std::string tabName, int index)
{
	CustomTabComponent* tab = new CustomTabComponent(false);

	topLevelTabComponent->addTab(tabName, Colours::grey, tab, true, index);
	tab->setName(tabName);

	hubTabs.add(tab);

	return tab;
}

void OnixSourceCanvas::addHub(std::string hubName, int offset)
{
	CustomTabComponent* tab = nullptr;
	OnixDeviceVector devices;
	PortName port = PortController::getPortFromIndex(offset);

	if (hubName == NEUROPIXELSV1E_HEADSTAGE_NAME)
	{
		tab = addTopLevelTab(getTopLevelTabName(port, hubName), (int)port);

		const int passthroughIndex = (offset >> 8) + 7;

		devices.emplace_back(std::make_shared<Neuropixels1e>("Probe", hubName, passthroughIndex, source->getContext()));
		devices.emplace_back(std::make_shared<PolledBno055>("BNO055", hubName, passthroughIndex, source->getContext()));
	}
	else if (hubName == NEUROPIXELSV1F_HEADSTAGE_NAME)
	{
		tab = addTopLevelTab(getTopLevelTabName(port, hubName), (int)port);

		devices.emplace_back(std::make_shared<Neuropixels1f>("Probe0", hubName, offset, source->getContext()));
		devices.emplace_back(std::make_shared<Neuropixels1f>("Probe1", hubName, offset + 1, source->getContext()));
		devices.emplace_back(std::make_shared<Bno055>("BNO055", hubName, offset + 2, source->getContext()));
	}
	else if (hubName == BREAKOUT_BOARD_NAME)
	{
		tab = addTopLevelTab(hubName, 0);

		devices.emplace_back(std::make_shared<OutputClock>("Output Clock", hubName, 5, source->getContext()));
		devices.emplace_back(std::make_shared<AnalogIO>("Analog IO", hubName, 6, source->getContext()));
		devices.emplace_back(std::make_shared<DigitalIO>("Digital IO", hubName, 7, source->getContext()));
		devices.emplace_back(std::make_shared<HarpSyncInput>("Harp Sync Input", hubName, 12, source->getContext()));
	}
	else if (hubName == NEUROPIXELSV2E_HEADSTAGE_NAME)
	{
		const int passthroughIndex = (offset >> 8) + 7;

		tab = addTopLevelTab(getTopLevelTabName(port, hubName), (int)port);

		devices.emplace_back(std::make_shared<Neuropixels2e>("", hubName, passthroughIndex, source->getContext()));
		devices.emplace_back(std::make_shared<PolledBno055>("BNO055", hubName, passthroughIndex, source->getContext()));
	}

	if (tab != nullptr && devices.size() > 0)
	{
		populateSourceTabs(tab, devices);
	}
}

std::shared_ptr<OnixDevice> OnixSourceCanvas::getDevicePtr(OnixDeviceType deviceType, int deviceIndex)
{
	for (const auto& settingsInterface : settingsInterfaces)
	{
		auto device = settingsInterface->getDevice();

		if (device->getDeviceType() == deviceType && device->getDeviceIdx() == deviceIndex)
		{
			return device;
		}
	}

	return nullptr;
}

void OnixSourceCanvas::populateSourceTabs(CustomTabComponent* tab, OnixDeviceVector devices)
{
	int portTabNumber = 0;

	for (const auto& device : devices)
	{
		if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F)
		{
			auto neuropixInterface = std::make_shared<NeuropixelsV1Interface>(std::static_pointer_cast<Neuropixels1>(device), editor, this);
			addInterfaceToTab(device->getName(), tab, neuropixInterface);
		}
		else if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1E)
		{
			auto neuropixInterface = std::make_shared<NeuropixelsV1Interface>(std::static_pointer_cast<Neuropixels1>(device), editor, this);
			addInterfaceToTab(device->getName(), tab, neuropixInterface);
		}
		else if (device->getDeviceType() == OnixDeviceType::BNO)
		{
			auto bno055Interface = std::make_shared<Bno055Interface>(std::static_pointer_cast<Bno055>(device), editor, this);
			addInterfaceToTab(device->getName(), tab, bno055Interface);
		}
		else if (device->getDeviceType() == OnixDeviceType::OUTPUTCLOCK)
		{
			auto outputClockInterface = std::make_shared<OutputClockInterface>(std::static_pointer_cast<OutputClock>(device), editor, this);
			addInterfaceToTab(device->getName(), tab, outputClockInterface);
		}
		else if (device->getDeviceType() == OnixDeviceType::HARPSYNCINPUT)
		{
			auto harpSyncInputInterface = std::make_shared<HarpSyncInputInterface>(std::static_pointer_cast<HarpSyncInput>(device), editor, this);
			addInterfaceToTab(device->getName(), tab, harpSyncInputInterface);
		}
		else if (device->getDeviceType() == OnixDeviceType::ANALOGIO)
		{
			auto analogIOInterface = std::make_shared<AnalogIOInterface>(std::static_pointer_cast<AnalogIO>(device), editor, this);
			addInterfaceToTab(device->getName(), tab, analogIOInterface);
		}
		else if (device->getDeviceType() == OnixDeviceType::DIGITALIO)
		{
			auto digitalIOInterface = std::make_shared<DigitalIOInterface>(std::static_pointer_cast<DigitalIO>(device), editor, this);
			addInterfaceToTab(device->getName(), tab, digitalIOInterface);
		}
		else if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV2E)
		{
			auto npxv2eInterface = std::make_shared<NeuropixelsV2eInterface>(std::static_pointer_cast<Neuropixels2e>(device), editor, this);
			std::string substring = " Headstage";
			std::string hubName = device->getHubName();
			addInterfaceToTab(hubName.erase(hubName.find(substring), substring.size()), tab, npxv2eInterface);
		}
		else if (device->getDeviceType() == OnixDeviceType::POLLEDBNO)
		{
			auto polledBnoInterface = std::make_shared<PolledBno055Interface>(std::static_pointer_cast<PolledBno055>(device), editor, this);
			addInterfaceToTab(device->getName(), tab, polledBnoInterface);
		}
	}
}

void OnixSourceCanvas::addInterfaceToTab(std::string tabName, CustomTabComponent* tab, std::shared_ptr<SettingsInterface> interface_)
{
	settingsInterfaces.emplace_back(interface_);
	tab->addTab(tabName, Colours::darkgrey, CustomViewport::createCustomViewport(interface_.get()), true);
}

std::string OnixSourceCanvas::getTopLevelTabName(PortName port, std::string headstage)
{
	return OnixDevice::getPortName(port) + ": " + headstage;
}

Array<CustomTabComponent*> OnixSourceCanvas::getHubTabs()
{
	Array<CustomTabComponent*> tabs;

	for (const auto hub : hubTabs)
	{
		tabs.add(hub);
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

	for (int i = hubTabs.size() - 1; i >= 0; i -= 1)
	{
		if (hubTabs[i]->getName().contains(OnixDevice::getPortName(port)))
		{
			hubTabs.remove(i, true);
			tabExists = true;
			break;
		}
	}

	int offset = PortController::getPortOffset(port);

	for (int i = settingsInterfaces.size() - 1; i >= 0; i -= 1)
	{
		auto selectedDevice = settingsInterfaces[i]->getDevice();

		if ((selectedDevice->getDeviceIdx() & offset) > 0)
		{
			settingsInterfaces.erase(settingsInterfaces.begin() + i);
			tabExists = true;
		}
	}

	if (tabExists)
	{
		if (port == PortName::PortB && hubTabs.size() == 1 && hubTabs[0]->getName().contains(BREAKOUT_BOARD_NAME))
			topLevelTabComponent->removeTab((int)port - 1); // NB: If only one headstage is selected in the editor, the index needs to be corrected here.
		else
			topLevelTabComponent->removeTab((int)port);
	}
}

void OnixSourceCanvas::removeAllTabs()
{
	hubTabs.clear(true);
	settingsInterfaces.clear();

	topLevelTabComponent->clearTabs();
}

OnixDeviceMap OnixSourceCanvas::getSelectedDevices(std::vector<std::shared_ptr<SettingsInterface>> interfaces)
{
	OnixDeviceMap tabMap;

	for (const auto& settings : interfaces)
	{
		auto device = settings->getDevice();

		if (!device->isEnabled())
			continue;

		tabMap.insert({ device->getDeviceIdx(), device->getDeviceType() });
	}

	return tabMap;
}

void OnixSourceCanvas::askKeepRemove(int offset)
{
	std::string selectedHeadstage = editor->getHeadstageSelected(offset);

	std::string msg = "Headstage " + selectedHeadstage + " is selected on " + OnixDevice::getPortName(offset) + ", but was not discovered there.\n\n";
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

void OnixSourceCanvas::askKeepUpdate(int offset, std::string foundHeadstage, OnixDeviceVector devices)
{
	std::string selectedHeadstage = editor->getHeadstageSelected(offset);

	if (selectedHeadstage == foundHeadstage) return;

	std::string msg = "Headstage " + selectedHeadstage + " is selected on " + OnixDevice::getPortName(offset) + ". ";
	msg += "However, headstage " + foundHeadstage + " was found on " + OnixDevice::getPortName(offset) + ". \n\n";
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

		auto selectedOffsets = OnixDevice::getUniqueOffsetsFromIndices(selectedIndices);
		auto foundOffsets = OnixDevice::getUniqueOffsetsFromIndices(foundIndices);

		if (foundIndices.size() == 0 || foundOffsets.size() == 0) // NB: No devices found
		{
			if (selectedMap.size() != 0)
			{
				for (const auto& offset : selectedOffsets)
				{
					askKeepRemove(offset);
				}
			}
		}
		else if (selectedIndices.size() == 0) // NB: No headstages selected, add all found headstages
		{
			for (auto& [offset, hubName] : source->getHubNames())
			{
				addHub(hubName, offset);
			}
		}
		else if (selectedOffsets.size() == foundOffsets.size()) // NB: Same number of ports selected and found
		{
			auto hubNames = source->getHubNames();

			if (selectedOffsets.size() == 1)
			{
				if (hubNames.size() != 2)
				{
					LOGE("Wrong number of headstages found in the source node.");
					return;
				}

				if (selectedOffsets[0] == foundOffsets[0]) // NB: Selected headstage is different from the found headstage on the same port
				{
					askKeepUpdate(selectedOffsets[0], hubNames[foundOffsets[0]], source->getDataSources());
				}
				else // NB: Selected headstage on one port is not found, and the found headstage is not selected on the other port
				{
					askKeepRemove(selectedOffsets[0]);

					addHub(hubNames[foundOffsets[0]], foundOffsets[0]);
				}
			}
			else // NB: Two headstages are selected on different ports, and at least one of those headstages does not match the found headstages
			{
				for (const auto& offset : foundOffsets)
				{
					if (hubNames[offset] != editor->getHeadstageSelected(offset))
					{
						askKeepUpdate(offset, hubNames[offset], source->getDataSourcesFromOffset(offset));
					}
				}
			}
		}
		else // NB: Different number of ports selected versus found
		{
			auto hubNames = source->getHubNames();

			if (selectedOffsets.size() > foundOffsets.size()) // NB: More headstages selected than found
			{
				for (const auto& offset : selectedOffsets)
				{
					if (offset == foundOffsets[0])
					{
						if (hubNames[offset] != editor->getHeadstageSelected(offset))
						{
							askKeepUpdate(offset, hubNames[offset], source->getDataSourcesFromOffset(offset));
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
				for (const auto& offset : foundOffsets)
				{
					if (offset == selectedOffsets[0])
					{
						if (hubNames[offset] != editor->getHeadstageSelected(offset))
						{
							askKeepUpdate(offset, hubNames[offset], source->getDataSourcesFromOffset(offset));
						}
					}
					else
					{
						addHub(hubNames[offset], offset);
					}
				}
			}
		}
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
		settingsInterface->startAcquisition();
	}
}

void OnixSourceCanvas::stopAcquisition()
{
	for (const auto& settingsInterface : settingsInterfaces)
	{
		settingsInterface->stopAcquisition();
	}
}

void OnixSourceCanvas::saveCustomParametersToXml(XmlElement* xml)
{
	for (int i = 0; i < settingsInterfaces.size(); i++)
		settingsInterfaces[i]->saveParameters(xml);
}

void OnixSourceCanvas::loadCustomParametersFromXml(XmlElement* xml)
{
	for (int i = 0; i < settingsInterfaces.size(); i++)
		settingsInterfaces[i]->loadParameters(xml);
}

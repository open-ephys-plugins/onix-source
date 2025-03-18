/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright(C) 2020 Allen Institute for Brain Science and Open Ephys

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

CustomTabButton::CustomTabButton(const String& name, TabbedComponent* parent, bool isTopLevel_) :
	TabBarButton(name, parent->getTabbedButtonBar()),
	isTopLevel(isTopLevel_)
{
}

void CustomTabButton::paintButton(Graphics& g,
	bool isMouseOver,
	bool isMouseDown)
{
	getTabbedButtonBar().setTabBackgroundColour(getIndex(), Colours::grey);

	getLookAndFeel().drawTabButton(*this, g, isMouseOver, isMouseDown);
}

CustomTabComponent::CustomTabComponent(OnixSourceEditor* editor_, bool isTopLevel_) :
	TabbedComponent(TabbedButtonBar::TabsAtTop),
	editor(editor_),
	isTopLevel(isTopLevel_)
{
	setTabBarDepth(26);
	setOutline(0);
	setIndent(0);
}

OnixSourceCanvas::OnixSourceCanvas(GenericProcessor* processor_, OnixSourceEditor* editor_, OnixSource* onixSource_) :
	Visualizer(processor_),
	editor(editor_),
	source(onixSource_)
{
	topLevelTabComponent = std::make_unique<CustomTabComponent>(editor, true);
	addAndMakeVisible(topLevelTabComponent.get());
}

CustomTabComponent* OnixSourceCanvas::addTopLevelTab(String tabName, int index)
{
	CustomTabComponent* tab = new CustomTabComponent(editor, false);

	topLevelTabComponent->addTab(tabName, Colours::grey, tab, true, index);
	tab->setName(tabName);

	headstageTabs.add(tab);

	return tab;
}

Parameter* OnixSourceCanvas::getSourceParameter(String name)
{
	return source->getParameter(name);
}

void OnixSourceCanvas::addHeadstage(String headstage, PortName port)
{
	int offset = PortController::getPortOffset(port);
	CustomTabComponent* tab = nullptr;
	OnixDeviceVector devices;

	if (headstage == NEUROPIXELSV1F_HEADSTAGE_NAME)
	{
		tab = addTopLevelTab(getTopLevelTabName(0, port, headstage), (int)port - 1);

		devices.push_back(std::make_shared<Neuropixels_1>("Probe-A", offset, nullptr));
		devices.push_back(std::make_shared<Neuropixels_1>("Probe-B", offset + 1, nullptr));
		devices.push_back(std::make_shared<Bno055>("BNO055", offset + 2, nullptr));
	}
	else if (headstage == "TEST HEADSTAGE")
	{
		// NOTE: This is only a temporary testing headstage used to confirm that the port logic works correctly. Remove before merging
		tab = addTopLevelTab(getTopLevelTabName(0, port, headstage), (int)port - 1);

		devices.push_back(std::make_shared<Neuropixels_1>("Probe-TEST", offset, nullptr));
		devices.push_back(std::make_shared<Bno055>("BNO055-TEST", offset + 1, nullptr));
		devices.push_back(std::make_shared<Neuropixels_1>("Probe-TEST2", offset + 2, nullptr));
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
	}
}

void OnixSourceCanvas::addInterfaceToTab(String tabName, CustomTabComponent* tab, std::shared_ptr<SettingsInterface> interface_)
{
	settingsInterfaces.push_back(interface_);
	tab->addTab(tabName, Colours::darkgrey, createCustomViewport(interface_.get()), true);
}

void OnixSourceCanvas::updateSettingsInterfaceDataSource(std::shared_ptr<OnixDevice> device)
{
	int ind = -1;

	for (int j = 0; j < settingsInterfaces.size(); j += 1)
	{
		if (device->getDeviceIdx() == settingsInterfaces[j]->device->getDeviceIdx() &&
			device->getName() == settingsInterfaces[j]->device->getName())
		{
			ind = j;
			break;
		}
	}

	if (ind == -1) { LOGD("Unable to match " + device->getName() + " to an open tab."); return; }

	if (device->type == OnixDeviceType::NEUROPIXELS_1)
	{
		// NB: Neuropixels-specific settings need to be updated
		auto npx1 = std::static_pointer_cast<Neuropixels_1>(device);
		npx1->setSettings(std::static_pointer_cast<Neuropixels_1>(settingsInterfaces[ind]->device)->settings.get());
	}
	// TODO: Add more devices, since they will have device-specific settings to be updated

	device->setEnabled(settingsInterfaces[ind]->device->isEnabled());
	settingsInterfaces[ind]->device.reset();
	settingsInterfaces[ind]->device = device;
}

String OnixSourceCanvas::getTopLevelTabName(int hub, PortName port, String headstage)
{
	return "Hub " + String(hub) + ": " + PortController::getPortName(port) + ": " + headstage;
}

String OnixSourceCanvas::getDeviceTabName(std::shared_ptr<OnixDevice> device)
{
	return String(device->getDeviceIdx()) + ": " + device->getName();
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

CustomViewport* OnixSourceCanvas::createCustomViewport(SettingsInterface* settingsInterface)
{
	Rectangle bounds = settingsInterface->getBounds();

	return new CustomViewport(settingsInterface, bounds.getWidth(), bounds.getHeight());
}

OnixSourceCanvas::~OnixSourceCanvas()
{
}

void OnixSourceCanvas::paint(Graphics& g)
{
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
		topLevelTabComponent->removeTab((int)port - 1);
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

void OnixSourceCanvas::askKeepRemove(PortName port)
{
	String selectedHeadstage = editor->getHeadstageSelected(port);

	String msg = "Headstage " + selectedHeadstage + " is selected on " + PortController::getPortName(port) + ", but was not discovered there.\n\n";
	msg += "Select one of the options below to continue:\n";
	msg += " - [Keep Current] to keep " + selectedHeadstage + " selected.\n";
	msg += " - [Remove] to remove " + selectedHeadstage + ". Note: this will delete any settings that were modified.";

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
		removeTabs(port);
		break;
	case 1: // Keep Current
		break;
	default:
		break;
	}
}

void OnixSourceCanvas::askKeepUpdate(PortName port, String foundHeadstage, OnixDeviceVector devices)
{
	String selectedHeadstage = editor->getHeadstageSelected(port);

	String msg = "Headstage " + selectedHeadstage + " is selected on " + PortController::getPortName(port) + ". ";
	msg += "However, headstage " + foundHeadstage + " was found on " + PortController::getPortName(port) + ". \n\n";
	msg += "Select one of the options below to continue:\n";
	msg += " - [Keep Current] to keep " + selectedHeadstage + " selected.\n";
	msg += " - [Update] to change the selected headstage to " + foundHeadstage + ". Note: this will delete any settings that were modified.";

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
		removeTabs(port);

		{
			CustomTabComponent* tab = addTopLevelTab(getTopLevelTabName(0, port, foundHeadstage), (int)port);
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
	auto foundMap = source->createDeviceMap();

	if (selectedMap != foundMap)
	{
		std::vector<int> selectedIndices, foundIndices;

		for (const auto& [key, _] : selectedMap) { selectedIndices.push_back(key); }
		for (const auto& [key, _] : foundMap) { foundIndices.push_back(key); }

		auto selectedPorts = PortController::getUniquePortsFromIndices(selectedIndices);
		auto foundPorts = PortController::getUniquePortsFromIndices(foundIndices);

		if (foundIndices.size() == 0) // NB: No devices found, inform the user if they were expecting to find something
		{
			if (selectedMap.size() != 0)
			{
				AlertWindow::showMessageBox(
					MessageBoxIconType::WarningIcon,
					"No Headstages Found",
					"No headstages were found when connecting. Double check that the correct headstage is selected. If this is unexpected, try pressing disconnect / connect again.\n\n"
					+ String("If the port voltage is manually set, try clearing the value and letting the automated voltage discovery algorithm run.")
				);
			}
		}
		else if (selectedIndices.size() == 0) // NB: No headstages selected, add all found headstages
		{
			for (auto& [port, headstageName] : source->getHeadstageMap())
			{
				addHeadstage(headstageName, port);
			}
		}
		else if (selectedPorts.size() == foundPorts.size()) // NB: Same number of ports selected and found
		{
			auto headstages = source->getHeadstageMap();

			if (selectedPorts.size() == 1)
			{
				if (headstages.size() != 1)
				{
					LOGE("Wrong number of headstages found in the source node.");
					return;
				}

				if (selectedPorts[0] == foundPorts[0]) // NB: Selected headstage is different from the found headstage on the same port
				{
					askKeepUpdate(selectedPorts[0], headstages[foundPorts[0]], source->getDataSources());
				}
				else // NB: Selected headstage on one port is not found, and the found headstage is not selected on the other port
				{
					askKeepRemove(selectedPorts[0]);

					addHeadstage(headstages[foundPorts[0]], foundPorts[0]);
				}
			}
			else // NB: Two headstages are selected on different ports, and at least one of those headstages does not match the found headstages
			{
				for (auto port : foundPorts)
				{
					if (headstages[port] != editor->getHeadstageSelected(port))
					{
						askKeepUpdate(port, headstages[port], source->getDataSourcesFromPort(port));
					}
				}
			}
		}
		else // NB: Different number of ports selected versus found
		{
			auto headstages = source->getHeadstageMap();

			if (selectedPorts.size() > foundPorts.size()) // NB: More headstages selected than found
			{
				for (auto port : selectedPorts)
				{
					if (port == foundPorts[0])
					{
						if (headstages[port] != editor->getHeadstageSelected(port))
						{
							askKeepUpdate(port, headstages[port], source->getDataSourcesFromPort(port));
						}
					}
					else
					{
						askKeepRemove(port);
					}
				}
			}
			else // NB: More headstages found than selected
			{
				for (auto port : foundPorts)
				{
					if (port == selectedPorts[0])
					{
						if (headstages[port] != editor->getHeadstageSelected(port))
						{
							askKeepUpdate(port, headstages[port], source->getDataSourcesFromPort(port));
						}
					}
					else
					{
						addHeadstage(headstages[port], port);
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

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
	std::vector<std::shared_ptr<OnixDevice>> devices;

	if (headstage == "Neuropixels 1.0f")
	{
		tab = addTopLevelTab(getTopLevelTabName(0, port, headstage), (int)port - 1);

		devices.push_back(std::make_shared<Neuropixels_1>("Probe-A", offset, source->getContext()));
		devices.push_back(std::make_shared<Neuropixels_1>("Probe-B", offset + 1, source->getContext()));
		devices.push_back(std::make_shared<Bno055>("BNO055", offset + 2, source->getContext()));
	}
	else if (headstage == "TEST HEADSTAGE")
	{
		// NOTE: This is only a temporary testing headstage used to confirm that the port logic works correctly. Remove before merging
		tab = addTopLevelTab(getTopLevelTabName(0, port, headstage), (int)port - 1);

		devices.push_back(std::make_shared<Neuropixels_1>("Probe-TEST", offset, source->getContext()));
		devices.push_back(std::make_shared<Bno055>("BNO055-TEST", offset + 1, source->getContext()));
		devices.push_back(std::make_shared<Neuropixels_1>("Probe-TEST2", offset + 2, source->getContext()));
	}

	if (tab != nullptr && devices.size() > 0)
	{
		populateSourceTabs(tab, devices);
	}
}

void OnixSourceCanvas::populateSourceTabs(CustomTabComponent* tab, std::vector<std::shared_ptr<OnixDevice>> devices)
{
	int portTabNumber = 0;

	for (const auto& device : devices)
	{
		if (device->type == OnixDeviceType::NEUROPIXELS_1)
		{
			auto neuropixInterface = std::make_shared<NeuropixV1Interface>(std::static_pointer_cast<Neuropixels_1>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device.get()), tab, neuropixInterface);
		}
		else if (device->type == OnixDeviceType::BNO)
		{
			auto bno055Interface = std::make_shared<Bno055Interface>(std::static_pointer_cast<Bno055>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device.get()), tab, bno055Interface);
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
		if (device->getDeviceIdx() == settingsInterfaces[j]->device->getDeviceIdx())
		{
			ind = j;
			break;
		}
	}

	if (ind == -1) { LOGD("Unable to match the device to a settings interface."); return; }

	if (device->type == OnixDeviceType::NEUROPIXELS_1)
	{
		// NB: Neuropixels-specific settings need to be updated
		auto npx1 = std::static_pointer_cast<Neuropixels_1>(device);
		npx1->setSettings(std::static_pointer_cast<Neuropixels_1>(settingsInterfaces[ind]->device)->settings.get());
	}

	settingsInterfaces[ind]->device.reset();
	settingsInterfaces[ind]->device = device;
}

String OnixSourceCanvas::getTopLevelTabName(int hub, PortName port, String headstage)
{
	return "Hub " + String(hub) + ": " + PortController::getPortName(port) + ": " + headstage;
}

String OnixSourceCanvas::getDeviceTabName(OnixDevice* device)
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

std::map<int, OnixDeviceType> OnixSourceCanvas::createTabMap(std::vector<std::shared_ptr<SettingsInterface>> interfaces)
{
	std::map<int, OnixDeviceType> tabMap;

	for (const auto& settings : interfaces)
	{
		tabMap.insert({ settings->device->getDeviceIdx(), settings->device->type });
	}

	return tabMap;
}

void OnixSourceCanvas::refreshTabs()
{
	auto devices = source->getDataSources();

	auto tabMap = createTabMap(settingsInterfaces);
	auto sourceMap = source->createDeviceMap(devices);

	if (tabMap == sourceMap)
	{
		for (int i = 0; i < devices.size(); i += 1)
		{
			updateSettingsInterfaceDataSource(devices[i]);
		}
	}
	else
	{
		std::vector<int> tabIndices, sourceIndices;

		for (const auto& [key, _] : tabMap) { tabIndices.push_back(key); }
		for (const auto& [key, _] : sourceMap) { sourceIndices.push_back(key); }

		auto tabPorts = PortController::getUniquePortsFromIndices(tabIndices);
		auto sourcePorts = PortController::getUniquePortsFromIndices(sourceIndices);

		auto headstages = source->getHeadstageMap();

		if (sourceIndices.size() == 0)
		{
		}
		else if (tabIndices.size() == 0)
		{
			for (auto& [port, headstageName] : headstages)
			{
				addHeadstage(headstageName, port);
			}
		}
		else if (tabPorts.size() == sourcePorts.size())
		{
			if (tabPorts.size() == 1)
			{
				if (headstages.size() == 0)
				{
					LOGE("Expected to find headstages in the source node.");
					return;
				}

				if (tabPorts[0] == sourcePorts[0])
				{
					int result = AlertWindow::show(
						MessageBoxOptions()
						.withIconType(MessageBoxIconType::WarningIcon)
						.withTitle("Mismatched Devices")
						.withMessage("The selected headstage does not match the hardware connected." +
							String("\n\n[Keep Current] to keep the current selection, ") + 
							String("[Create New] to close selected tabs and create new tabs from hardware connected, or ") + 
							String("[Cancel] to do nothing."))
						.withButton("Keep Current")
						.withButton("Create New")
						.withButton("Cancel")
					);

					switch (result)
					{
					case 1: // Keep Current
						break;
					case 2: // Create New
						removeTabs(tabPorts[0]);

						{
							CustomTabComponent* tab = addTopLevelTab(getTopLevelTabName(0, sourcePorts[0], headstages[sourcePorts[0]]), (int)sourcePorts[0]);
							populateSourceTabs(tab, devices);
						}
						break;
					case 3: // Cancel
						break;
					default:
						break;
					}
				}
				else
				{
					int result = AlertWindow::show(
						MessageBoxOptions()
						.withIconType(MessageBoxIconType::WarningIcon)
						.withTitle("Devices Not Found")
						.withMessage("The selected headstage does not exist in " + PortController::getPortName(tabPorts[0]) +
							String(".\n\n[Keep Tabs] to keep the current selected tabs open, ") +
							String("[Remove Tabs] to close selected tabs, or ") +
							String("[Cancel] to do nothing."))
						.withButton("Keep Tabs")
						.withButton("Remove Tabs")
						.withButton("Cancel")
					);

					switch (result)
					{
					case 1: // Keep Tabs
						break;
					case 2: // Remove Tabs
						removeTabs(tabPorts[0]);
						break;
					case 3: // Cancel
						break;
					default:
						break;
					}

					addHeadstage(headstages[sourcePorts[0]], sourcePorts[0]);
				}
			}
			else
			{
				int result = AlertWindow::show(
					MessageBoxOptions()
					.withIconType(MessageBoxIconType::WarningIcon)
					.withTitle("Mismatched Devices")
					.withMessage("The selected headstages do not match the hardware connected." +
						String("\n\n[Keep Current] to keep the current selection, ") +
						String("[Create New] to close selected tabs and create new tabs from hardware connected, or ") +
						String("[Cancel] to do nothing."))
					.withButton("Keep Current")
					.withButton("Create New")
					.withButton("Cancel")
				);

				switch (result)
				{
				case 1: // Keep Current
					break;
				case 2: // Create New
					removeAllTabs();
					for (auto& [port, headstageName] : headstages)
					{
						addHeadstage(headstageName, port);
					}
					break;
				case 3: // Cancel
					break;
				default:
					break;
				}
			}
		}
		else
		{
			if (tabPorts.size() > sourcePorts.size())
			{
				bool hardwareMatch = true;

				for (const auto& [index, type] : sourceMap)
				{
					if (tabMap[index] != type) hardwareMatch = false;
				}

				if (hardwareMatch)
				{
					int result = AlertWindow::show(
						MessageBoxOptions()
						.withIconType(MessageBoxIconType::WarningIcon)
						.withTitle("Extra Tabs")
						.withMessage("There are more selected headstages than hardware connected." +
							String("\n\n[Keep Current] to keep the current selection, ") +
							String("[Remove Extra] to close tabs not found in hardware, or ") +
							String("[Cancel] to do nothing."))
						.withButton("Keep Current")
						.withButton("Remove Extra")
						.withButton("Cancel")
					);

					switch (result)
					{
					case 1: // Keep Current
						break;
					case 2: // Remove Extra
						{
							PortName portToRemove = tabPorts[0] == sourcePorts[0] ? tabPorts[1] : tabPorts[0];
							removeTabs(portToRemove);
						}
						break;
					case 3: // Cancel
						break;
					default:
						break;
					}
				}
				else
				{
					int result = AlertWindow::show(
						MessageBoxOptions()
						.withIconType(MessageBoxIconType::WarningIcon)
						.withTitle("Extra Tabs")
						.withMessage("There are more selected headstages than hardware connected." +
							String("\n\n[Keep Current] to keep the current selection, ") +
							String("[Remove All] to close all tabs not found in hardware and add new ones, or ") +
							String("[Cancel] to do nothing."))
						.withButton("Keep Current")
						.withButton("Remove All")
						.withButton("Cancel")
					);

					switch (result)
					{
					case 1: // Keep Current
						break;
					case 2: // Remove All
						removeAllTabs();
						addHeadstage(headstages[sourcePorts[0]], sourcePorts[0]);

						break;
					case 3: // Cancel
						break;
					default:
						break;
					}
				}
			}
			else
			{
				bool hardwareMatch = true;

				for (const auto& [index, type] : tabMap)
				{
					if (sourceMap[index] != type) hardwareMatch = false;
				}

				if (hardwareMatch)
				{
					PortName portToAdd = sourcePorts[0] == tabPorts[0] ? sourcePorts[1] : sourcePorts[0];
					addHeadstage(headstages[portToAdd], portToAdd);
				}
				else
				{
					int result = AlertWindow::show(
						MessageBoxOptions()
						.withIconType(MessageBoxIconType::WarningIcon)
						.withTitle("Device Mismatch")
						.withMessage("The selected headstage does not match the hardware connected." +
							String("\n\n[Keep Current] to keep the current selection, ") +
							String("[Remove All] to close all tabs not found in hardware and add new ones, or ") +
							String("[Cancel] to do nothing."))
						.withButton("Keep Current")
						.withButton("Remove All")
						.withButton("Cancel")
					);

					switch (result)
					{
					case 1: // Keep Current
						break;
					case 2: // Remove All
						removeAllTabs();
						for (auto& [port, headstageName] : headstages)
						{
							addHeadstage(headstageName, port);
						}

						break;
					case 3: // Cancel
						break;
					default:
						break;
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

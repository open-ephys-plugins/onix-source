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
	onixSource(onixSource_)
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
	return onixSource->getParameter(name);
}

void OnixSourceCanvas::addHeadstage(String headstage, PortName port)
{
	int offset = PortController::getPortOffset(port);
	CustomTabComponent* tab = nullptr;
	std::vector<std::shared_ptr<OnixDevice>> devices;

	if (headstage == "Neuropixels 1.0f")
	{
		tab = addTopLevelTab(getTopLevelTabName(0, port, headstage), (int)port - 1);

		devices.push_back(std::make_shared<Neuropixels_1>("Neuropixels 1.0 Probe A", offset, onixSource->getContext()));
		devices.push_back(std::make_shared<Neuropixels_1>("Neuropixels 1.0 Probe B", offset + 1, onixSource->getContext()));
		devices.push_back(std::make_shared<Bno055>("BNO055", offset + 2, onixSource->getContext()));
	}

	if (tab != nullptr && devices.size() > 0)
	{
		populateSourceTabs(tab, devices);
	}
}

void OnixSourceCanvas::populateSourceTabs(CustomTabComponent* tab, std::vector<std::shared_ptr<OnixDevice>> devices)
{
	int portTabNumber = 0;

	for (std::shared_ptr<OnixDevice> device : devices)
	{
		if (device->type == OnixDeviceType::NEUROPIXELS_1)
		{
			NeuropixV1Interface* neuropixInterface = new NeuropixV1Interface(std::static_pointer_cast<Neuropixels_1>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device.get()), tab, neuropixInterface);
		}
		else if (device->type == OnixDeviceType::BNO)
		{
			Bno055Interface* bno055Interface = new Bno055Interface(std::static_pointer_cast<Bno055>(device), editor, this);
			addInterfaceToTab(getDeviceTabName(device.get()), tab, bno055Interface);
		}
	}
}

void OnixSourceCanvas::addInterfaceToTab(String tabName, CustomTabComponent* tab, SettingsInterface* interface_)
{
	settingsInterfaces.add(interface_);
	tab->addTab(tabName, Colours::darkgrey, createCustomViewport(interface_), true);
}

String OnixSourceCanvas::getTopLevelTabName(int hub, PortName port, String headstage)
{
	return "Hub " + String(hub) + ": " + PortController::getPortName(port) + ": " + headstage;
}

String OnixSourceCanvas::getDeviceTabName(OnixDevice* device)
{
	return String(device->getDeviceIdx()) + ": " + device->getName();
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
	for (int i = headstageTabs.size() - 1; i >= 0 ; i -= 1)
	{
		if (headstageTabs[i]->getName().contains(PortController::getPortName(port)))
		{
			headstageTabs.remove(i, true);
			break;
		}
	}

	int offset = PortController::getPortOffset(port);
	
	for (int i = settingsInterfaces.size() - 1; i >= 0; i -= 1)
	{
		if ((settingsInterfaces[i]->dataSource->getDeviceIdx() & offset) > 0)
		{
			settingsInterfaces.remove(i, true);
		}
	}

	topLevelTabComponent->removeTab((int)port - 1);
}

void OnixSourceCanvas::removeAllTabs()
{
	headstageTabs.clear(true);
	settingsInterfaces.clear(true);

	topLevelTabComponent->clearTabs();
}

void OnixSourceCanvas::refreshTabs()
{
}

void OnixSourceCanvas::update()
{
	for (int i = 0; i < settingsInterfaces.size(); i++)
		settingsInterfaces[i]->updateInfoString();

	for (int i = 0; i < topLevelTabComponent->getNumTabs(); i++)
	{
		CustomTabComponent* t = (CustomTabComponent*)topLevelTabComponent->getTabContentComponent(i);

		for (int j = 0; j < t->getNumTabs(); j++)
		{
			if (t->getTabContentComponent(j) != nullptr)
			{
				CustomViewport* v = (CustomViewport*)t->getTabContentComponent(j);

				if (v != nullptr)
				{
					if (v->settingsInterface->dataSource != nullptr)
						t->setTabName(j, " " + v->settingsInterface->dataSource->getName() + " ");
					else
						t->setTabName(j, "");
				}
			}
		}
	}
}

void OnixSourceCanvas::resized()
{
	topLevelTabComponent->setBounds(0, -3, getWidth(), getHeight() + 3);
}

void OnixSourceCanvas::startAcquisition()
{
	for (auto settingsInterface : settingsInterfaces)
	{
		if (settingsInterface->dataSource != nullptr && settingsInterface->dataSource->isEnabled())
		{
			settingsInterface->startAcquisition();
		}
	}
}

void OnixSourceCanvas::stopAcquisition()
{
	for (auto settingsInterface : settingsInterfaces)
	{
		if (settingsInterface->dataSource != nullptr && settingsInterface->dataSource->isEnabled())
		{
			settingsInterface->stopAcquisition();
		}
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

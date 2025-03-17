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

	CustomTabComponent* portTab = new CustomTabComponent(editor, false);

	topLevelTabComponent->addTab("Devices", Colours::grey, portTab, true);

	portTab->setTabBarDepth(26);
	portTab->setIndent(0);
	portTab->setOutline(0);

	portTabs.add(portTab);

	refreshTabs();
	update();
}

Parameter* OnixSourceCanvas::getSourceParameter(String name)
{
	return onixSource->getParameter(name);
}

void OnixSourceCanvas::populateSourceTabs(CustomTabComponent* portTab)
{
	auto availableDataSources = onixSource->getDataSources();

	int portTabNumber = 0;

	for (const auto& source : availableDataSources)
	{
		if (source->type == OnixDeviceType::NEUROPIXELS_1)
		{
			NeuropixV1Interface* neuropixInterface = new NeuropixV1Interface(std::static_pointer_cast<Neuropixels_1>(source), editor, this);
			settingsInterfaces.add(neuropixInterface);
			portTab->addTab(source->getName(), Colours::darkgrey, createCustomViewport(neuropixInterface), true);

			portTabIndex.add(portTabNumber++);
		}
		else if (source->type == OnixDeviceType::BNO)
		{
			auto bno055Interface = new Bno055Interface(std::static_pointer_cast<Bno055>(source), editor, this);
			settingsInterfaces.add(bno055Interface);
			portTab->addTab(source->getName(), Colours::darkgrey, createCustomViewport(bno055Interface), true);

			portTabIndex.add(portTabNumber++);
		}
		else if (source->type == OnixDeviceType::MEMORYMONITOR)
		{
			auto memoryMonitorInterface = new MemoryMonitorInterface(std::static_pointer_cast<MemoryMonitor>(source), editor, this);
			settingsInterfaces.add(memoryMonitorInterface);
			portTab->addTab(source->getName(), Colours::darkgrey, createCustomViewport(memoryMonitorInterface), true);

			portTabIndex.add(portTabNumber++);
		}
		else if (source->type == OnixDeviceType::OUTPUTCLOCK)
		{
			auto outputClockInterface = new OutputClockInterface(std::static_pointer_cast<OutputClock>(source), editor, this);
			settingsInterfaces.add(outputClockInterface);
			portTab->addTab(source->getName(), Colours::darkgrey, createCustomViewport(outputClockInterface), true);

			portTabIndex.add(portTabNumber++);
		}
		else if (source->type == OnixDeviceType::HEARTBEAT)
		{
			auto heartbeatInterface = new HeartbeatInterface(std::static_pointer_cast<Heartbeat>(source), editor, this);
			settingsInterfaces.add(heartbeatInterface);
			portTab->addTab(source->getName(), Colours::darkgrey, createCustomViewport(heartbeatInterface), true);

			portTabIndex.add(portTabNumber++);
		}
	}
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

void OnixSourceCanvas::removeTabs()
{
	for (auto tab : portTabs)
	{
		tab->clearTabs();
	}

	settingsInterfaces.clear(true);
}

void OnixSourceCanvas::refreshTabs()
{
	removeTabs();

	CustomTabComponent* portTab = portTabs.getFirst();

	populateSourceTabs(portTab);

	topLevelTabComponent->setCurrentTabIndex(0);
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
					if (v->settingsInterface->device != nullptr)
						t->setTabName(j, " " + v->settingsInterface->device->getName() + " ");
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
		if (settingsInterface->device != nullptr && settingsInterface->device->isEnabled())
		{
			settingsInterface->startAcquisition();
		}
	}
}

void OnixSourceCanvas::stopAcquisition()
{
	for (auto settingsInterface : settingsInterfaces)
	{
		if (settingsInterface->device != nullptr && settingsInterface->device->isEnabled())
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

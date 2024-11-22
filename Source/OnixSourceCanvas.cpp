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

#include "UI/NeuropixV1Interface.h"

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

void CustomTabComponent::currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName)
{
	if (isTopLevel)
	{
		TabbedComponent* currentTab = (TabbedComponent*)getCurrentContentComponent();

		if (currentTab != nullptr)
		{
			CustomViewport* viewport = (CustomViewport*)currentTab->getCurrentContentComponent();
		}
	}
	else
	{
		CustomViewport* viewport = (CustomViewport*)getCurrentContentComponent();
	}
}

OnixSourceCanvas::OnixSourceCanvas(GenericProcessor* processor_, OnixSourceEditor* editor_, OnixSource* onixSource_) :
	Visualizer(processor_),
	editor(editor_),
	onixSource(onixSource_)
{
	topLevelTabComponent = std::make_unique<CustomTabComponent>(editor, true);
	addAndMakeVisible(topLevelTabComponent.get());

	int topLevelTabNumber = 0;

	for (int i = 0; i < 2; i++)
	{
		CustomTabComponent* portTab = new CustomTabComponent(editor, false);

		String name = i == 0 ? "Port A" : "Port B";

		topLevelTabComponent->addTab(name, Colours::grey, portTab, true);

		portTab->setTabBarDepth(26);
		portTab->setIndent(0);
		portTab->setOutline(0);

		portTabs.add(portTab);

		populateSourceTabs(portTab, topLevelTabNumber);
	}

	topLevelTabComponent->setCurrentTabIndex(topLevelTabNumber - 1);

	savedSettings.probeType = ProbeType::NONE;
}

void OnixSourceCanvas::populateSourceTabs(CustomTabComponent* portTab, int& topLevelTabNumber)
{
	Array<OnixDevice*> availableDataSources = onixSource->getDataSources();

	int portTabNumber = 0;

	for (auto source : availableDataSources)
	{
		if (source->type == OnixDeviceType::NEUROPIXELS_1)
		{
			NeuropixV1Interface* neuropixInterface = new NeuropixV1Interface(source, editor, this);
			settingsInterfaces.add((SettingsInterface*)neuropixInterface);
			portTab->addTab(source->getName(), Colours::darkgrey, neuropixInterface->viewport.get(), false);

			topLevelTabIndex.add(topLevelTabNumber);
			portTabIndex.add(portTabNumber++);
		}
	}

	if (availableDataSources.isEmpty() && DEBUG)
	{
		OnixDevice* source = new Neuropixels_1("test", 0.0f, "", "", NULL, NULL);
		NeuropixV1Interface* neuropixInterface = new NeuropixV1Interface(source, editor, this);
		settingsInterfaces.add((SettingsInterface*)neuropixInterface);
		portTab->addTab("TEST", Colours::darkgrey, neuropixInterface->viewport.get(), false);

		topLevelTabIndex.add(topLevelTabNumber);
		portTabIndex.add(portTabNumber++);
	}

	topLevelTabNumber += 1;
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
					OnixDevice* dataSource = v->settingsInterface->dataSource;

					if (dataSource != nullptr)
						t->setTabName(j, " " + dataSource->getName() + " ");
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
		if (settingsInterface->dataSource != nullptr && settingsInterface->dataSource->settings.isEnabled)
		{
			settingsInterface->startAcquisition();
		}
	}
}

void OnixSourceCanvas::stopAcquisition()
{
	for (auto settingsInterface : settingsInterfaces)
	{
		if (settingsInterface->dataSource != nullptr && settingsInterface->dataSource->settings.isEnabled)
		{
			settingsInterface->stopAcquisition();
		}
	}
}

void OnixSourceCanvas::setSelectedInterface(OnixDevice* dataSource)
{
	if (dataSource != nullptr)
	{
		int index = dataSources.indexOf(dataSource);

		topLevelTabComponent->setCurrentTabIndex(topLevelTabIndex[index], false);
		portTabs[topLevelTabIndex[index]]->setCurrentTabIndex(portTabIndex[index], false);
	}
}

void OnixSourceCanvas::storeProbeSettings(ProbeSettings p)
{
	savedSettings = p;
}

ProbeSettings OnixSourceCanvas::getProbeSettings()
{
	return savedSettings;
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

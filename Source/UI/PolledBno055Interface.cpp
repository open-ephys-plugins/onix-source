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

#include "PolledBno055Interface.h"

using namespace OnixSourcePlugin;

PolledBno055Interface::PolledBno055Interface(std::shared_ptr<PolledBno055> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	device = d;

	if (device != nullptr)
	{
		deviceEnableButton = std::make_unique<UtilityButton>("ENABLED");
		deviceEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(50, 40, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setToggleState(device->isEnabled(), dontSendNotification);
		deviceEnableButton->setTooltip("If disabled, PolledBNO055 device will not stream data during acquisition");
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());
	}

	type = SettingsInterface::Type::POLLEDBNO055_SETTINGS_INTERFACE;
}

void PolledBno055Interface::buttonClicked(Button* button)
{
	if (button == deviceEnableButton.get())
	{
		device->setEnabled(deviceEnableButton->getToggleState());

		if (canvas->foundInputSource())
		{
			device->configureDevice();
			canvas->resetContext();
		}

		if (device->isEnabled())
		{
			deviceEnableButton->setLabel("ENABLED");
		}
		else
		{
			deviceEnableButton->setLabel("DISABLED");
		}

		CoreServices::updateSignalChain(editor);
	}
}

void PolledBno055Interface::updateSettings()
{
	if (device == nullptr) return;

	deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);
}

void PolledBno055Interface::setInterfaceEnabledState(bool newState)
{
	if (deviceEnableButton != nullptr)
		deviceEnableButton->setEnabled(newState);
}

void PolledBno055Interface::saveParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Saving PolledBno055 settings.");

	XmlElement* xmlNode = xml->createNewChildElement("POLLEDBNO055");

	xmlNode->setAttribute("name", device->getName());
	xmlNode->setAttribute("idx", (int)device->getDeviceIdx());

	xmlNode->setAttribute("isEnabled", device->isEnabled());
}

void PolledBno055Interface::loadParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Loading PolledBno055 settings.");

	XmlElement* xmlNode = nullptr;

	for (auto* node : xml->getChildIterator())
	{
		if (node->hasTagName("POLLEDBNO055"))
		{
			if (node->getStringAttribute("name") == device->getName() &&
				node->getIntAttribute("idx") == device->getDeviceIdx())
			{
				xmlNode = node;
				break;
			}
		}
	}

	if (xmlNode == nullptr)
	{
		LOGD("No PolledBno055 element found.");
		return;
	}

	device->setEnabled(xmlNode->getBoolAttribute("isEnabled"));

	updateSettings();
}

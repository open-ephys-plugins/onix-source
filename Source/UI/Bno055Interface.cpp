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

#include "Bno055Interface.h"

Bno055Interface::Bno055Interface(std::shared_ptr<Bno055> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	if (device != nullptr)
	{
		deviceEnableButton = std::make_unique<UtilityButton>("ENABLED");
		deviceEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(50, 40, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setTooltip("If disabled, BNO055 device will not stream data during acquisition");
		deviceEnableButton->setToggleState(true, dontSendNotification);
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());
		deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);
	}

	type = SettingsInterface::Type::BNO055_SETTINGS_INTERFACE;
}

void Bno055Interface::updateSettings()
{
	if (device == nullptr) return;

	deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);
}

void Bno055Interface::buttonClicked(Button* button)
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

void Bno055Interface::saveParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Saving Bno055 settings.");

	XmlElement* xmlNode = xml->createNewChildElement("BNO055");

	auto bno055 = std::static_pointer_cast<Bno055>(device);

	xmlNode->setAttribute("name", bno055->getName());
	xmlNode->setAttribute("idx", (int)bno055->getDeviceIdx());

	xmlNode->setAttribute("isEnabled", bno055->isEnabled());
}

void Bno055Interface::loadParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Loading Bno055 settings.");

	auto bno055 = std::static_pointer_cast<Bno055>(device);

	XmlElement* xmlNode = nullptr;

	for (auto* node : xml->getChildIterator())
	{
		if (node->hasTagName("BNO055"))
		{
			if (node->getStringAttribute("name") == bno055->getName() &&
				node->getIntAttribute("idx") == bno055->getDeviceIdx())
			{
				xmlNode = node;
				break;
			}
		}
	}

	if (xmlNode == nullptr)
	{
		LOGD("No Bno055 element found.");
		return;
	}

	device->setEnabled(xmlNode->getBoolAttribute("isEnabled"));

	updateSettings();
}

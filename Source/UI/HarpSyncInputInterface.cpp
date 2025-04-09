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

#include "HarpSyncInputInterface.h"

HarpSyncInputInterface::HarpSyncInputInterface(std::shared_ptr<HarpSyncInput> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	if (device != nullptr)
	{
		deviceEnableButton = std::make_unique<UtilityButton>("ENABLED");
		deviceEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(50, 40, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setTooltip("If disabled, Harp Sync Input will not stream data during acquisition");
		deviceEnableButton->setToggleState(true, dontSendNotification);
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());
		deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);
	}

	type = SettingsInterface::Type::HARPSYNCINPUT_SETTINGS_INTERFACE;
}

void HarpSyncInputInterface::setInterfaceEnabledState(bool newState)
{
	if (deviceEnableButton != nullptr)
		deviceEnableButton->setEnabled(newState);
}

void HarpSyncInputInterface::updateSettings()
{
	if (device == nullptr) return;

	deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);
}

void HarpSyncInputInterface::buttonClicked(Button* button)
{
	if (button == deviceEnableButton.get())
	{
		device->setEnabled(deviceEnableButton->getToggleState());
		device->configureDevice();

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

void HarpSyncInputInterface::saveParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Saving HarpSyncInput settings.");

	XmlElement* xmlNode = xml->createNewChildElement("HARPSYNCINPUT");

	xmlNode->setAttribute("isEnabled", device->isEnabled());
}

void HarpSyncInputInterface::loadParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Loading HarpSyncInput settings.");

	auto xmlNode = xml->getChildByName("HARPSYNCINPUT");

	if (xmlNode == nullptr)
	{
		LOGE("No HARPSYNCINPUT element found.");
		return;
	}

	device->setEnabled(xmlNode->getBoolAttribute("isEnabled"));

	updateSettings();
}

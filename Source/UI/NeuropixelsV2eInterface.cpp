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

#include "NeuropixelsV2eInterface.h"
#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

using namespace OnixSourcePlugin;

NeuropixelsV2eInterface::NeuropixelsV2eInterface(std::shared_ptr<Neuropixels2e> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	topLevelTabComponent = std::make_unique<CustomTabComponent>(true);
	addAndMakeVisible(topLevelTabComponent.get());

	probeInterfaces[0] = std::make_unique<NeuropixelsV2eProbeInterface>(d, 0, e, c);
	topLevelTabComponent->addTab("Probe0", Colours::grey, CustomViewport::createCustomViewport(probeInterfaces[0].get()), true, 0);

	probeInterfaces[1] = std::make_unique<NeuropixelsV2eProbeInterface>(d, 1, e, c);
	topLevelTabComponent->addTab("Probe1", Colours::grey, CustomViewport::createCustomViewport(probeInterfaces[1].get()), true, 1);

	deviceComponent = std::make_unique<Component>();
	deviceComponent->setBounds(0, 0, 600, 40);

	deviceEnableButton = std::make_unique<UtilityButton>(enabledButtonText);
	deviceEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
	deviceEnableButton->setRadius(3.0f);
	deviceEnableButton->setBounds(10, 15, 100, 22);
	deviceEnableButton->setClickingTogglesState(true);
	deviceEnableButton->setToggleState(device->isEnabled(), dontSendNotification);
	deviceEnableButton->setTooltip("If disabled, probe will not stream data during acquisition");
	deviceEnableButton->addListener(this);
	deviceComponent->addAndMakeVisible(deviceEnableButton.get());

	addAndMakeVisible(deviceComponent.get());

	setBounds(0, 0, 1000, 800);

	type = Type::NEUROPIXELS2E_SETTINGS_INTERFACE;
}

void NeuropixelsV2eInterface::resized()
{
	topLevelTabComponent->setBounds(0, 50, canvas->getWidth() * 0.99, canvas->getHeight() - 50);
}

void NeuropixelsV2eInterface::updateInfoString()
{
	for (const auto& probeInterface : probeInterfaces)
	{
		probeInterface->updateInfoString();
	}
}

void NeuropixelsV2eInterface::buttonClicked(Button* button)
{
	if (button == deviceEnableButton.get())
	{
		device->setEnabled(deviceEnableButton->getToggleState());

		if (canvas->foundInputSource())
		{
			try
			{
				device->configureDevice();
			}
			catch (const error_str& e)
			{
				LOGE(e.what());
				button->setToggleState(!button->getToggleState(), dontSendNotification);
				return;
			}

			canvas->resetContext();
		}

		if (device->isEnabled())
		{
			deviceEnableButton->setLabel(enabledButtonText);
		}
		else
		{
			deviceEnableButton->setLabel(disabledButtonText);
		}

		updateInfoString();
		repaint();

		CoreServices::updateSignalChain(editor);
	}
}

void NeuropixelsV2eInterface::updateSettings()
{
	if (device == nullptr) return;

	deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);

	for (const auto& probeInterface : probeInterfaces)
	{
		probeInterface->updateSettings();
	}
}

void NeuropixelsV2eInterface::setInterfaceEnabledState(bool newState)
{
	if (device == nullptr) return;

	if (deviceEnableButton != nullptr)
		deviceEnableButton->setEnabled(newState);

	for (const auto& probeInterface : probeInterfaces)
	{
		probeInterface->setInterfaceEnabledState(newState);
	}
}

void NeuropixelsV2eInterface::saveParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Saving NeuropixelsV2e settings.");

	XmlElement* xmlNode = xml->createNewChildElement("NEUROPIXELSV2E");

	xmlNode->setAttribute("idx", (int)device->getDeviceIdx());

	xmlNode->setAttribute("isEnabled", device->isEnabled());

	for (const auto& probeInterface : probeInterfaces)
	{
		probeInterface->saveParameters(xmlNode);
	}
}

void NeuropixelsV2eInterface::loadParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Loading NeuropixelsV2e settings.");

	XmlElement* xmlNode = nullptr;

	for (auto* node : xml->getChildIterator())
	{
		if (node->hasTagName("NEUROPIXELSV2E"))
		{
			if (node->getIntAttribute("idx") == device->getDeviceIdx())
			{
				xmlNode = node;
				break;
			}
		}
	}

	if (xmlNode == nullptr)
	{
		LOGD("No NeuropixelsV2e element found.");
		return;
	}

	device->setEnabled(xmlNode->getBoolAttribute("isEnabled"));

	for (const auto& probeInterface : probeInterfaces)
	{
		probeInterface->loadParameters(xmlNode);
	}

	updateSettings();
}

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

#include "AnalogIOInterface.h"
#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

using namespace OnixSourcePlugin;

AnalogIOInterface::AnalogIOInterface(std::shared_ptr<AnalogIO> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	if (device != nullptr)
	{
		FontOptions font = FontOptions("Fira Code", "Regular", 12.0f);

		deviceEnableButton = std::make_unique<UtilityButton>(enabledButtonText);
		deviceEnableButton->setFont(font);
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(50, 40, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setTooltip("If disabled, AnalogIO will not stream or receive data during acquisition");
		deviceEnableButton->setToggleState(true, dontSendNotification);
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());
		deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);

		auto prevLabelRectangle = deviceEnableButton->getBounds().translated(0, 20);
		prevLabelRectangle.setWidth(140);
		auto prevComboBoxRectangle = deviceEnableButton->getBounds().translated(100, 20);
		prevComboBoxRectangle.setWidth(80);
		prevComboBoxRectangle.setX(prevLabelRectangle.getRight() + 5);

		StringArray directionList;
		directionList.add("Input");
		directionList.add("Output");

		for (int i = 0; i < numChannels; i++)
		{
			channelDirectionLabels[i] = std::make_unique<Label>("channelDirectionLabel" + std::to_string(i), "Channel " + std::to_string(i) + ": Direction");
			channelDirectionLabels[i]->setBounds(prevLabelRectangle.getX(), prevLabelRectangle.getBottom() + 4, prevLabelRectangle.getWidth(), prevLabelRectangle.getHeight());
			channelDirectionLabels[i]->setFont(font);
			addAndMakeVisible(channelDirectionLabels[i].get());

			prevLabelRectangle = channelDirectionLabels[i]->getBounds();

			channelDirectionComboBoxes[i] = std::make_unique<ComboBox>("channelDirectionComboBox" + std::to_string(i));
			channelDirectionComboBoxes[i]->setBounds(prevComboBoxRectangle.getX(), prevLabelRectangle.getY(), prevComboBoxRectangle.getWidth(), prevComboBoxRectangle.getHeight());
			channelDirectionComboBoxes[i]->addListener(this);
			channelDirectionComboBoxes[i]->setTooltip("Sets the direction of Channel " + std::to_string(i));
			channelDirectionComboBoxes[i]->addItemList(directionList, 1);
			channelDirectionComboBoxes[i]->setSelectedId(1, dontSendNotification);
			channelDirectionComboBoxes[i]->setItemEnabled(2, false);
			addAndMakeVisible(channelDirectionComboBoxes[i].get());

			prevComboBoxRectangle = channelDirectionComboBoxes[i]->getBounds();
		}

		saveSettingsButton = std::make_unique<UtilityButton>("Save Settings");
		saveSettingsButton->setRadius(3.0f);
		saveSettingsButton->setBounds(deviceEnableButton->getX() + 3, channelDirectionLabels[numChannels - 1]->getBottom() + 20, 120, 22);
		saveSettingsButton->addListener(this);
		saveSettingsButton->setTooltip("Save all AnalogIO settings to file.");
		addAndMakeVisible(saveSettingsButton.get());

		loadSettingsButton = std::make_unique<UtilityButton>("Load Settings");
		loadSettingsButton->setRadius(3.0f);
		loadSettingsButton->setBounds(saveSettingsButton->getRight() + 5, saveSettingsButton->getY(), saveSettingsButton->getWidth(), saveSettingsButton->getHeight());
		loadSettingsButton->addListener(this);
		loadSettingsButton->setTooltip("Load all AnalogIO settings from a file.");
		addAndMakeVisible(loadSettingsButton.get());

		updateInfoString();

		updateSettings();
	}

	type = SettingsInterface::Type::ANALOGIO_SETTINGS_INTERFACE;
}

void AnalogIOInterface::setInterfaceEnabledState(bool newState)
{
	if (deviceEnableButton != nullptr)
		deviceEnableButton->setEnabled(newState);

	for (int i = 0; i < numChannels; i++)
	{
		if (channelDirectionComboBoxes[i] != nullptr)
			channelDirectionComboBoxes[i]->setEnabled(newState);
	}

	if (saveSettingsButton != nullptr)
		saveSettingsButton->setEnabled(newState);

	if (loadSettingsButton != nullptr)
		loadSettingsButton->setEnabled(newState);
}

void AnalogIOInterface::updateSettings()
{
	if (device == nullptr) return;

	deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);

	auto analogIO = std::static_pointer_cast<AnalogIO>(device);

	for (int i = 0; i < numChannels; i++)
	{
		channelDirectionComboBoxes[i]->setSelectedId(getChannelDirectionId(analogIO, i), dontSendNotification);
	}
}

void AnalogIOInterface::hideEnableButton()
{
	deviceEnableButton->setVisible(false);
}

void AnalogIOInterface::buttonClicked(Button* button)
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

		CoreServices::updateSignalChain(editor);
	}
	else if (button == saveSettingsButton.get())
	{
		FileChooser fileChooser("Save AnalogIO settings to an XML file.", File(), "*.xml");

		if (fileChooser.browseForFileToSave(true))
		{
			XmlElement rootElement("DEVICE");

			saveParameters(&rootElement);

			writeToXmlFile(&rootElement, fileChooser.getResult());
		}
	}
	else if (button == loadSettingsButton.get())
	{
		FileChooser fileChooser("Load AnalogIO settings from an XML file.", File(), "*.xml");

		if (fileChooser.browseForFileToOpen())
		{
			auto rootElement = readFromXmlFile(fileChooser.getResult());

			loadParameters(rootElement);

			delete rootElement;
		}
	}
}

void AnalogIOInterface::comboBoxChanged(ComboBox* cb)
{
	for (int i = 0; i < numChannels; i++)
	{
		if (cb == channelDirectionComboBoxes[i].get())
		{
			AnalogIODirection newDirection = getChannelDirectionFromString(channelDirectionComboBoxes[i]->getText().toStdString());
			std::static_pointer_cast<AnalogIO>(device)->setChannelDirection(i, newDirection);
			return;
		}
	}
}

AnalogIODirection AnalogIOInterface::getChannelDirectionFromString(std::string direction)
{
	if (direction == "Input")
		return AnalogIODirection::Input;
	else if (direction == "Output")
		return AnalogIODirection::Output;
	else
		LOGE("Invalid direction given.");

	return AnalogIODirection::Input;
}

int AnalogIOInterface::getChannelDirectionId(std::shared_ptr<AnalogIO> device, int channelNumber)
{
	switch (device->getChannelDirection(channelNumber))
	{
	case AnalogIODirection::Input:
		return 1;
	case AnalogIODirection::Output:
		return 2;
	default:
		return 0;
	}
}

int AnalogIOInterface::getChannelVoltageRangeId(std::shared_ptr<AnalogIO> device, int channelNumber)
{
	switch (device->getChannelVoltageRange(channelNumber))
	{
	case AnalogIOVoltageRange::TwoPointFiveVolts:
		return 1;
	case AnalogIOVoltageRange::FiveVolts:
		return 2;
	case AnalogIOVoltageRange::TenVolts:
		return 3;
	default:
		break;
	}
}

int AnalogIOInterface::getDataTypeId(std::shared_ptr<AnalogIO> device)
{
	switch (device->getDataType())
	{
	case AnalogIODataType::S16:
		return 1;
	case AnalogIODataType::Volts:
		return 2;
	default:
		return 0;
	}
}

void AnalogIOInterface::saveParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	auto analogIO = std::static_pointer_cast<AnalogIO>(device);

	LOGD("Saving AnalogIO settings.");

	XmlElement* xmlNode = xml->createNewChildElement("ANALOGIO");

	xmlNode->setAttribute("isEnabled", device->isEnabled());

	XmlElement* channelDirectionNode = xmlNode->createNewChildElement("CHANNEL_DIRECTION");

	for (int i = 0; i < numChannels; i++)
	{
		channelDirectionNode->setAttribute(String("CH" + std::to_string(i)), AnalogIO::getChannelDirection(analogIO->getChannelDirection(i)));
	}
}

void AnalogIOInterface::loadParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Loading AnalogIO settings.");

	auto analogIO = std::static_pointer_cast<AnalogIO>(device);

	auto xmlNode = xml->getChildByName("ANALOGIO");

	if (xmlNode == nullptr)
	{
		LOGE("No ANALOGIO element found.");
		return;
	}

	auto enabled = xmlNode->getBoolAttribute("isEnabled");
	analogIO->setEnabled(enabled);

	auto chDirectionNode = xmlNode->getChildByName("CHANNEL_DIRECTION");

	if (chDirectionNode == nullptr)
	{
		LOGE("No CHANNEL_DIRECTION element found");
		return;
	}

	for (int i = 0; i < numChannels; i++)
	{
		AnalogIODirection direction = getChannelDirectionFromString(chDirectionNode->getStringAttribute("CH" + std::to_string(i)).toStdString());
		analogIO->setChannelDirection(i, direction);
	}

	updateSettings();
}

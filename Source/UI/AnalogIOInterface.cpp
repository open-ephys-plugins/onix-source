/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2020 Allen Institute for Brain Science and Open Ephys

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

AnalogIOInterface::AnalogIOInterface(std::shared_ptr<AnalogIO> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	if (device != nullptr)
	{
		FontOptions font = FontOptions("Fira Code", "Regular", 12.0f);

		deviceEnableButton = std::make_unique<UtilityButton>("ENABLED");
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

		StringArray voltageRangeList;
		voltageRangeList.add("+/- 2.5 V");
		voltageRangeList.add("+/- 5.0 V");
		voltageRangeList.add("+/- 10.0 V");

		for (int i = 0; i < numChannels; i += 1)
		{
			channelDirectionLabels[i] = std::make_unique<Label>("channelDirectionLabel" + String(i), "Channel " + String(i) + String(": Direction"));
			channelDirectionLabels[i]->setBounds(prevLabelRectangle.getX(), prevLabelRectangle.getBottom() + 4, prevLabelRectangle.getWidth(), prevLabelRectangle.getHeight());
			channelDirectionLabels[i]->setFont(font);
			addAndMakeVisible(channelDirectionLabels[i].get());

			prevLabelRectangle = channelDirectionLabels[i]->getBounds();

			channelDirectionComboBoxes[i] = std::make_unique<ComboBox>("channelDirectionComboBox" + String(i));
			channelDirectionComboBoxes[i]->setBounds(prevComboBoxRectangle.getX(), prevLabelRectangle.getY(), prevComboBoxRectangle.getWidth(), prevComboBoxRectangle.getHeight());
			channelDirectionComboBoxes[i]->addListener(this);
			channelDirectionComboBoxes[i]->setTooltip("Sets the direction of Channel " + String(i));
			channelDirectionComboBoxes[i]->addItemList(directionList, 1);
			channelDirectionComboBoxes[i]->setSelectedId(getChannelDirectionId(std::static_pointer_cast<AnalogIO>(device), i), dontSendNotification);
			addAndMakeVisible(channelDirectionComboBoxes[i].get());

			prevComboBoxRectangle = channelDirectionComboBoxes[i]->getBounds();
		}

		updateInfoString();
	}

	type = SettingsInterface::Type::ANALOGIO_SETTINGS_INTERFACE;
}

void AnalogIOInterface::buttonClicked(Button* button)
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

void AnalogIOInterface::comboBoxChanged(ComboBox* cb)
{
	for (int i = 0; i < numChannels; i += 1)
	{
		if (cb == channelDirectionComboBoxes[i].get())
		{
			AnalogIODirection newDirection = channelDirectionComboBoxes[i]->getText() == "Input" ? AnalogIODirection::Input : AnalogIODirection::Output;
			std::static_pointer_cast<AnalogIO>(device)->setChannelDirection(i, newDirection);
			return;
		}
	}
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

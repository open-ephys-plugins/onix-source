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

		dataTypeLabel = std::make_unique<Label>("dataTypeLabel", "Data Type");
		dataTypeLabel->setFont(font);
		dataTypeLabel->setBounds(deviceEnableButton->getX(), deviceEnableButton->getBottom() + 5, 80, 20);
		addAndMakeVisible(dataTypeLabel.get());

		dataTypeComboBox = std::make_unique<ComboBox>("dataTypeComboBox");
		dataTypeComboBox->setBounds(dataTypeLabel->getRight() + 5, dataTypeLabel->getY(), 65, dataTypeLabel->getHeight());
		dataTypeComboBox->addListener(this);
		dataTypeComboBox->setTooltip("Set the data type output by this device. S16 outputs signed 16-bit data, while Volts outputs 32-bit floating point data.");
		dataTypeComboBox->addItem("S16", 1);
		dataTypeComboBox->addItem("Volts", 2);
		dataTypeComboBox->setSelectedId(getDataTypeId(std::static_pointer_cast<AnalogIO>(device)), dontSendNotification);
		addAndMakeVisible(dataTypeComboBox.get());

		auto prevLabelRectangle = dataTypeLabel->getBounds().translated(0, 10);
		prevLabelRectangle.setWidth(140);
		auto prevComboBoxRectangle = dataTypeComboBox->getBounds().translated(0, 10);
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

			voltageRangeLabels[i] = std::make_unique<Label>("voltageRangeLabel" + String(i), "Voltage Range");
			voltageRangeLabels[i]->setBounds(prevComboBoxRectangle.getRight() + 10, prevComboBoxRectangle.getY(), 110, prevComboBoxRectangle.getHeight());
			voltageRangeLabels[i]->setFont(font);
			addAndMakeVisible(voltageRangeLabels[i].get());

			voltageRangeComboBoxes[i] = std::make_unique<ComboBox>("voltageRangeComboBox" + String(i));
			voltageRangeComboBoxes[i]->setBounds(voltageRangeLabels[i]->getRight() + 4, voltageRangeLabels[i]->getY(), prevComboBoxRectangle.getWidth(), prevComboBoxRectangle.getHeight());
			voltageRangeComboBoxes[i]->addListener(this);
			voltageRangeComboBoxes[i]->setTooltip("Sets the voltage range of channel " + String(i));
			voltageRangeComboBoxes[i]->addItemList(voltageRangeList, 1);
			voltageRangeComboBoxes[i]->setSelectedId(getChannelVoltageRangeId(std::static_pointer_cast<AnalogIO>(device), i), dontSendNotification);
			addAndMakeVisible(voltageRangeComboBoxes[i].get());
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
		if (canvas->foundInputSource()) canvas->resetContext();

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
	if (cb == dataTypeComboBox.get())
	{
		AnalogIODataType newType = dataTypeComboBox->getText() == "S16" ? AnalogIODataType::S16 : AnalogIODataType::Volts;
		std::static_pointer_cast<AnalogIO>(device)->setDataType(newType);
		return;
	}

	for (int i = 0; i < numChannels; i += 1)
	{
		if (cb == channelDirectionComboBoxes[i].get())
		{
			AnalogIODirection newDirection = channelDirectionComboBoxes[i]->getText() == "Input" ? AnalogIODirection::Input : AnalogIODirection::Output;
			std::static_pointer_cast<AnalogIO>(device)->setChannelDirection(i, newDirection);
			return;
		}
		else if (cb == voltageRangeComboBoxes[i].get())
		{
			AnalogIOVoltageRange newVoltageRange = AnalogIOVoltageRange::TwoPointFiveVolts;
			
			if (voltageRangeComboBoxes[i]->getText() == "+/- 2.5 V") newVoltageRange = AnalogIOVoltageRange::TwoPointFiveVolts;
			else if (voltageRangeComboBoxes[i]->getText() == "+/- 5.0 V") newVoltageRange = AnalogIOVoltageRange::FiveVolts;
			else if (voltageRangeComboBoxes[i]->getText() == "+/- 10.0 V") newVoltageRange = AnalogIOVoltageRange::TenVolts;

			std::static_pointer_cast<AnalogIO>(device)->setChannelVoltageRange(i, newVoltageRange);
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

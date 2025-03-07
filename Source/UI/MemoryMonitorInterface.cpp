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

#include "MemoryMonitorInterface.h"

MemoryMonitorInterface::MemoryMonitorInterface(OnixDevice* d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c),
	device((MemoryMonitor*)d)
{
	if (device != nullptr)
	{
		nameLabel = std::make_unique<Label>("MAIN", "NAME");
		nameLabel->setFont(FontOptions("Fira Code", "Medium", 30.0f));
		nameLabel->setBounds(625, 40, 370, 45);
		addAndMakeVisible(nameLabel.get());

		deviceEnableButton = std::make_unique<UtilityButton>("ENABLED");
		deviceEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(nameLabel->getX(), nameLabel->getBottom() + 3, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setToggleState(device->isEnabled(), dontSendNotification);
		deviceEnableButton->setTooltip("If disabled, Memory Monitor will not stream data during acquisition");
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());

		infoLabel = std::make_unique<Label>("INFO", "INFO");
		infoLabel->setFont(FontOptions(15.0f));
		infoLabel->setBounds(deviceEnableButton->getX(), deviceEnableButton->getBottom() + 10, 300, 20);
		infoLabel->setJustificationType(Justification::topLeft);
		addAndMakeVisible(infoLabel.get());

		sampleRateLabel = std::make_unique<Label>("sampleRateLabel", "Sample Rate [Hz]");
		sampleRateLabel->setBounds(infoLabel->getX(), infoLabel->getBottom() + 10, 130, 20);
		addAndMakeVisible(sampleRateLabel.get());

		sampleRateValue = std::make_unique<Label>("sampleRateValue", String(device->getSamplesPerSecond()));
		sampleRateValue->setEditable(true);
		sampleRateValue->setBounds(sampleRateLabel->getRight() + 3, sampleRateLabel->getY(), 50, 20);
		sampleRateValue->setTooltip("Sets the sample rate of the Memory Monitor device in Hz. Must be between 1 and 1000 Hz.");
		sampleRateValue->setColour(Label::ColourIds::backgroundColourId, Colours::lightgrey);
		sampleRateValue->setJustificationType(Justification::centred);
		sampleRateValue->addListener(this);
		addAndMakeVisible(sampleRateValue.get());

		updateInfoString();
	}

	type = SettingsInterface::Type::MEMORYMONITOR_SETTINGS_INTERFACE;
}

MemoryMonitorInterface::~MemoryMonitorInterface()
{
}

void MemoryMonitorInterface::buttonClicked(Button* button)
{
	if (button == deviceEnableButton.get())
	{
		device->setEnabled(deviceEnableButton->getToggleState());
		device->enableDevice();
		//canvas->resetContext(); // TODO: Once issue-23 is merged, uncomment this line

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

void MemoryMonitorInterface::labelTextChanged(Label* l)
{
	if (l == sampleRateValue.get())
	{
		int rate = l->getText().getIntValue();

		if (rate < 1 || rate > 1000)
		{
			l->setText(String(device->getSamplesPerSecond()), dontSendNotification);
			return;
		}

		device->setSamplesPerSecond((uint32_t)rate);
	}
}

void MemoryMonitorInterface::startAcquisition()
{
}

void MemoryMonitorInterface::stopAcquisition()
{
}

void MemoryMonitorInterface::saveParameters(XmlElement* xml)
{
}

void MemoryMonitorInterface::loadParameters(XmlElement* xml)
{
}

void MemoryMonitorInterface::updateInfoString()
{
	String nameString, infoString;

	nameString = "Breakout Board";

	if (device != nullptr)
	{
		infoString = "Device: Memory Monitor";
		infoString += "\n";
		infoString += "\n";
	}

	infoLabel->setText(infoString, dontSendNotification);
	nameLabel->setText(nameString, dontSendNotification);
}

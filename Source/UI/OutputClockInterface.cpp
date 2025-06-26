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

#include "OutputClockInterface.h"

using namespace OnixSourcePlugin;

OutputClockInterface::OutputClockInterface(std::shared_ptr<OutputClock> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	if (device != nullptr)
	{
		auto outputClock = std::static_pointer_cast<OutputClock>(device);

		frequencyHzLabel = std::make_unique<Label>("frequencyHz", "Frequency [Hz]");
		frequencyHzLabel->setBounds(50, 40, 130, 20);
		addAndMakeVisible(frequencyHzLabel.get());

		frequencyHzValue = std::make_unique<Label>("frequencyHzValue", String(outputClock->getFrequencyHz()));
		frequencyHzValue->setEditable(true);
		frequencyHzValue->setBounds(frequencyHzLabel->getRight() + 3, frequencyHzLabel->getY(), 50, 20);
		frequencyHzValue->setTooltip("Sets the output clock frequency in Hz. Must be between 0.1 Hz and 10 MHz.");
		frequencyHzValue->setColour(Label::ColourIds::backgroundColourId, Colours::lightgrey);
		frequencyHzValue->setJustificationType(Justification::centred);
		frequencyHzValue->addListener(this);
		addAndMakeVisible(frequencyHzValue.get());

		dutyCycleLabel = std::make_unique<Label>("dutyCycle", "Duty Cycle [%]");
		dutyCycleLabel->setBounds(frequencyHzLabel->getX(), frequencyHzLabel->getBottom() + 10, 130, 20);
		addAndMakeVisible(dutyCycleLabel.get());

		dutyCycleValue = std::make_unique<Label>("dutyCycleValue", "50");
		dutyCycleValue->setEditable(true);
		dutyCycleValue->setBounds(dutyCycleLabel->getRight() + 3, dutyCycleLabel->getY(), 50, 20);
		dutyCycleValue->setTooltip("Sets the output clock duty cycle in percent. Values must be between 10 and 90.");
		dutyCycleValue->setColour(Label::ColourIds::backgroundColourId, Colours::lightgrey);
		dutyCycleValue->setJustificationType(Justification::centred);
		dutyCycleValue->addListener(this);
		addAndMakeVisible(dutyCycleValue.get());

		delayLabel = std::make_unique<Label>("delay", "Delay [s]");
		delayLabel->setBounds(dutyCycleLabel->getX(), dutyCycleLabel->getBottom() + 10, 130, 20);
		addAndMakeVisible(delayLabel.get());

		delayValue = std::make_unique<Label>("delayValue", "0");
		delayValue->setEditable(true);
		delayValue->setBounds(delayLabel->getRight() + 3, delayLabel->getY(), 50, 20);
		delayValue->setTooltip("Sets the delay after acquisition commences before the clock becomes active, in seconds. Values must be between 0 and 3600 seconds.");
		delayValue->setColour(Label::ColourIds::backgroundColourId, Colours::lightgrey);
		delayValue->setJustificationType(Justification::centred);
		delayValue->addListener(this);
		addAndMakeVisible(delayValue.get());

		gateRunButton = std::make_unique<ToggleButton>("Synchronize clock to acquisition status");
		gateRunButton->setBounds(delayLabel->getX(), delayLabel->getBottom() + 5, 250, 22);
		gateRunButton->setClickingTogglesState(true);
		gateRunButton->setToggleState(std::static_pointer_cast<OutputClock>(device)->getGateRun(), dontSendNotification);
		gateRunButton->setTooltip("Toggles the output clock gate. If checked, the clock output will follow the acquisition status." +
			std::string(" If unchecked, the clock output will run continuously."));
		gateRunButton->addListener(this);
		addAndMakeVisible(gateRunButton.get());

		saveSettingsButton = std::make_unique<UtilityButton>("Save Settings");
		saveSettingsButton->setRadius(3.0f);
		saveSettingsButton->setBounds(gateRunButton->getX() + 3, gateRunButton->getBottom() + 20, 120, 22);
		saveSettingsButton->addListener(this);
		saveSettingsButton->setTooltip("Save all OutputClock settings to file.");
		addAndMakeVisible(saveSettingsButton.get());

		loadSettingsButton = std::make_unique<UtilityButton>("Load Settings");
		loadSettingsButton->setRadius(3.0f);
		loadSettingsButton->setBounds(saveSettingsButton->getRight() + 5, saveSettingsButton->getY(), saveSettingsButton->getWidth(), saveSettingsButton->getHeight());
		loadSettingsButton->addListener(this);
		loadSettingsButton->setTooltip("Load all OutputClock settings from a file.");
		addAndMakeVisible(loadSettingsButton.get());

		updateSettings();
	}

	type = Type::OUTPUTCLOCK_SETTINGS_INTERFACE;
}

void OutputClockInterface::setInterfaceEnabledState(bool newState)
{
	if (frequencyHzValue != nullptr)
		frequencyHzValue->setEnabled(newState);

	if (dutyCycleValue != nullptr)
		dutyCycleValue->setEnabled(newState);

	if (delayValue != nullptr)
		delayValue->setEnabled(newState);

	if (gateRunButton != nullptr)
		gateRunButton->setEnabled(newState);
}

void OutputClockInterface::updateSettings()
{
	if (device == nullptr) return;

	auto outputClock = std::static_pointer_cast<OutputClock>(device);

	frequencyHzValue->setText(String(outputClock->getFrequencyHz()), dontSendNotification);
	dutyCycleValue->setText(std::to_string(outputClock->getDutyCycle()), dontSendNotification);
	delayValue->setText(std::to_string(outputClock->getDelay()), dontSendNotification);
	gateRunButton->setToggleState(outputClock->getGateRun(), dontSendNotification);
}

void OutputClockInterface::buttonClicked(Button* b)
{
	auto outputClock = std::static_pointer_cast<OutputClock>(device);

	if (b == gateRunButton.get())
	{
		outputClock->setGateRun(gateRunButton->getToggleState(), editor->acquisitionIsActive);
	}
	else if (b == saveSettingsButton.get())
	{
		FileChooser fileChooser("Save OutputClock settings to an XML file.", File(), "*.xml");

		if (fileChooser.browseForFileToSave(true))
		{
			XmlElement rootElement("DEVICE");

			saveParameters(&rootElement);

			writeToXmlFile(&rootElement, fileChooser.getResult());
		}
	}
	else if (b == loadSettingsButton.get())
	{
		FileChooser fileChooser("Load OutputClock settings from an XML file.", File(), "*.xml");

		if (fileChooser.browseForFileToOpen())
		{
			auto rootElement = readFromXmlFile(fileChooser.getResult());

			loadParameters(rootElement);

			delete rootElement;
		}
	}
}

void OutputClockInterface::labelTextChanged(Label* l)
{
	auto outputClock = std::static_pointer_cast<OutputClock>(device);

	if (l == frequencyHzValue.get())
	{
		auto rate = l->getText().getFloatValue();

		if (rate < MinFrequencyHz || rate > MaxFrequencyHz)
		{
			l->setText(String(outputClock->getFrequencyHz()), dontSendNotification);
			return;
		}

		outputClock->setFrequencyHz(rate);
	}
	else if (l == dutyCycleValue.get())
	{
		uint32_t rate = l->getText().getIntValue();

		if (rate < MinDutyCyclePercent || rate > MaxDutyCyclePercent)
		{
			l->setText(std::to_string(outputClock->getDutyCycle()), dontSendNotification);
			return;
		}

		outputClock->setDutyCycle(rate);
	}
	else if (l == delayValue.get())
	{
		uint32_t rate = l->getText().getIntValue();

		if (rate < MinDelaySeconds || rate > MaxDelaySeconds)
		{
			l->setText(std::to_string(outputClock->getDelay()), dontSendNotification);
			return;
		}

		outputClock->setDelay(rate);
	}
}

void OutputClockInterface::saveParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	auto outputClock = std::static_pointer_cast<OutputClock>(device);

	LOGD("Saving Output Clock settings.");

	XmlElement* xmlNode = xml->createNewChildElement("OUTPUTCLOCK");

	xmlNode->setAttribute("frequencyHz", outputClock->getFrequencyHz());
	xmlNode->setAttribute("dutyCycle", outputClock->getDutyCycle());
	xmlNode->setAttribute("delay", outputClock->getDelay());
	xmlNode->setAttribute("gateRun", outputClock->getGateRun());
}

void OutputClockInterface::loadParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Loading OutputClock settings.");

	auto outputClock = std::static_pointer_cast<OutputClock>(device);

	auto xmlNode = xml->getChildByName("OUTPUTCLOCK");

	if (xmlNode == nullptr)
	{
		LOGE("No OUTPUTCLOCK element found.");
		return;
	}

	outputClock->setEnabled(true);

	outputClock->setFrequencyHz(xmlNode->getDoubleAttribute("frequencyHz"));
	outputClock->setDutyCycle(xmlNode->getIntAttribute("dutyCycle"));
	outputClock->setDelay(xmlNode->getIntAttribute("delay"));
	outputClock->setGateRun(xmlNode->getIntAttribute("gateRun"));

	updateSettings();
}

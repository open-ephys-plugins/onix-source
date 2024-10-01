/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2023 Allen Institute for Brain Science and Open Ephys

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

#include "OnixSourceEditor.h"

using namespace Onix;

OnixSourceEditor::OnixSourceEditor(GenericProcessor* parentNode, OnixSource* onixSource)
	: GenericEditor(parentNode), thread(onixSource)
{
	desiredWidth = 200;

	portVoltageLabel = new Label("Voltage", "PORT VOLTAGE [V]");
	portVoltageLabel->setBounds(5, 20, 75, 20);
	portVoltageLabel->setFont(Font("Small Text", 11, Font::plain));
	portVoltageLabel->setColour(Label::textColourId, Colours::black);
	addAndMakeVisible(portVoltageLabel);

	portVoltage = 6.0f;
	portVoltageValue = new Label("VoltageValue", String(portVoltage));
	portVoltageValue->setBounds(10, 38, 30, 13);
	portVoltageValue->setFont(Font("Small Text", 11, Font::plain));
	portVoltageValue->setEditable(true);
	portVoltageValue->setColour(Label::textColourId, Colours::black);
	portVoltageValue->setColour(Label::backgroundColourId, Colours::lightgrey);
	portVoltageValue->addListener(this);
	addAndMakeVisible(portVoltageValue);

	rescanButton = new UtilityButton("Rescan");
	rescanButton->setFont(FontOptions("Small Text", 9, Font::plain));
	rescanButton->setBounds(10, 100, 50, 17);
	rescanButton->setRadius(3.0f);
	rescanButton->addListener(this);
	addAndMakeVisible(rescanButton);

	adcCalibrationLabel = new Label("ADC CAL LABEL", "ADC CAL.");
	adcCalibrationLabel->setFont(FontOptions("Small Text", 11, Font::plain));
	adcCalibrationLabel->setBounds(80, 25, 50, 15);
	adcCalibrationLabel->setColour(Label::textColourId, Colours::black);
	addAndMakeVisible(adcCalibrationLabel);

	adcCalibrationFile = new TextEditor("ADC CAL FILE");
	adcCalibrationFile->setFont(FontOptions("Small Text", 11, Font::plain));
	adcCalibrationFile->setBounds(80, 40, 90, 16);
	adcCalibrationFile->setColour(Label::textColourId, Colours::black);
	adcCalibrationFile->addListener(this);
	addAndMakeVisible(adcCalibrationFile);

	chooseAdcCalibrationFileButton = new UtilityButton("...");
	chooseAdcCalibrationFileButton->setFont(FontOptions("Small Text", 9, Font::bold));
	chooseAdcCalibrationFileButton->setBounds(172, 40, 18, 16);
	chooseAdcCalibrationFileButton->setRadius(1.0f);
	chooseAdcCalibrationFileButton->addListener(this);
	addAndMakeVisible(chooseAdcCalibrationFileButton);

	adcCalibrationFileChooser = std::make_unique<FileChooser>("Select ADC Calibration file.", File::getSpecialLocation(File::userHomeDirectory), "*_ADCCalibration.csv");

	gainCalibrationLabel = new Label("GAIN CAL LABEL", "GAIN CAL.");
	gainCalibrationLabel->setFont(FontOptions("Small Text", 11, Font::plain));
	gainCalibrationLabel->setBounds(80, 57, 50, 15);
	gainCalibrationLabel->setColour(Label::textColourId, Colours::black);
	addAndMakeVisible(gainCalibrationLabel);

	gainCalibrationFile = new TextEditor("GAIN CAL FILE");
	gainCalibrationFile->setFont(FontOptions("Small Text", 11, Font::plain));
	gainCalibrationFile->setBounds(80, 72, 90, 16);
	gainCalibrationFile->setColour(Label::textColourId, Colours::black);
	gainCalibrationFile->addListener(this);
	addAndMakeVisible(gainCalibrationFile);

	chooseGainCalibrationFileButton = new UtilityButton("...");
	chooseGainCalibrationFileButton->setFont(FontOptions("Small Text", 9, Font::bold));
	chooseGainCalibrationFileButton->setBounds(172, 72, 18, 16);
	chooseGainCalibrationFileButton->setRadius(1.0f);
	chooseGainCalibrationFileButton->addListener(this);
	addAndMakeVisible(chooseGainCalibrationFileButton);

	gainCalibrationFileChooser = std::make_unique<FileChooser>("Select Gain Calibration file.", File::getSpecialLocation(File::userHomeDirectory), "*_gainCalValues.csv");
}

void OnixSourceEditor::labelTextChanged(Label* l)
{
	if (l == portVoltageValue)
	{
		const float minVoltage = 4.5;
		const float maxVoltage = 6.5;

		float voltage = portVoltageValue->getText().getFloatValue();

		if (voltage >= minVoltage && voltage <= maxVoltage)
		{
			portVoltage = voltage;
		}
		else if (voltage < minVoltage)
		{
			portVoltage = minVoltage;
			portVoltageValue->setText(String(portVoltage), NotificationType::dontSendNotification);
		}
		else if (voltage > maxVoltage)
		{
			portVoltage = maxVoltage;
			portVoltageValue->setText(String(portVoltage), NotificationType::dontSendNotification);
		}
	}
}

void OnixSourceEditor::buttonClicked(Button* b)
{
	if (b == portVoltageOverrideButton)
	{
		thread->setPortVoltage((oni_dev_idx_t)PortName::PortA, (int)(portVoltage * 10));
	}
	else if (b == rescanButton)
	{
		thread->setPortVoltage((oni_dev_idx_t)PortName::PortA, (int)(portVoltage * 10));
		thread->initializeDevices(true);
	}
	else if (b == chooseAdcCalibrationFileButton)
	{
		if (adcCalibrationFileChooser->browseForFileToOpen())
		{
			adcCalibrationFile->setText(adcCalibrationFileChooser->getResult().getFullPathName(), false);
		}
	}
	else if (b == chooseGainCalibrationFileButton)
	{
		if (gainCalibrationFileChooser->browseForFileToOpen())
		{
			gainCalibrationFile->setText(gainCalibrationFileChooser->getResult().getFullPathName(), false);
		}
	}
}

void OnixSourceEditor::updateSettings()
{
}

void OnixSourceEditor::startAcquisition()
{
	rescanButton->setEnabled(false);
	rescanButton->setAlpha(0.3f);

	chooseAdcCalibrationFileButton->setEnabled(false);
	chooseAdcCalibrationFileButton->setAlpha(0.3f);

	chooseGainCalibrationFileButton->setEnabled(false);
	chooseGainCalibrationFileButton->setAlpha(0.3f);
}

void OnixSourceEditor::stopAcquisition()
{
	rescanButton->setEnabled(true);
	rescanButton->setAlpha(1.0f);

	chooseAdcCalibrationFileButton->setEnabled(true);
	chooseAdcCalibrationFileButton->setAlpha(1.0f);

	chooseGainCalibrationFileButton->setEnabled(true);
	chooseGainCalibrationFileButton->setAlpha(1.0f);
}

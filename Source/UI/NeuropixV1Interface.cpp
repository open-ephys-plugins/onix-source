/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright(C) 2020 Allen Institute for Brain Science and Open Ephys

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

#include "NeuropixV1Interface.h"
#include "ProbeBrowser.h"

#include "../Formats/ProbeInterface.h"

NeuropixV1Interface::NeuropixV1Interface(std::shared_ptr<Neuropixels_1> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c),
	neuropix_info("INFO")
{
	ColourScheme::setColourScheme(ColourSchemeId::PLASMA);

	if (device != nullptr)
	{
		type = SettingsInterface::Type::PROBE_SETTINGS_INTERFACE;

		mode = VisualizationMode::ENABLE_VIEW;

		probeBrowser = std::make_unique<ProbeBrowser>(this);
		probeBrowser->setBounds(0, 0, 600, 600);
		addAndMakeVisible(probeBrowser.get());

		int currentHeight = 55;

		FontOptions fontName = FontOptions("Fira Code", "Medium", 30.0f);
		FontOptions fontRegularButton = FontOptions("Fira Code", "Regular", 12.0f);
		FontOptions fontRegularLabel = FontOptions("Fira Code", "Regular", 13.0f);

		nameLabel = std::make_unique<Label>("MAIN", "NAME");
		nameLabel->setFont(fontName);
		nameLabel->setBounds(625, 40, 370, 45);
		addAndMakeVisible(nameLabel.get());

		deviceEnableButton = std::make_unique<UtilityButton>("ENABLED");
		deviceEnableButton->setFont(fontRegularButton);
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(nameLabel->getX(), nameLabel->getBottom() + 3, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setTooltip("If disabled, probe will not stream data during acquisition");
		deviceEnableButton->setToggleState(true, dontSendNotification);
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());
		deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);

		infoLabel = std::make_unique<Label>("INFO", "INFO");
		infoLabel->setFont(FontOptions(15.0f));
		infoLabel->setBounds(deviceEnableButton->getX(), deviceEnableButton->getBottom() + 10, nameLabel->getWidth(), 50);
		infoLabel->setJustificationType(Justification::topLeft);
		addAndMakeVisible(infoLabel.get());

		adcCalibrationFileLabel = std::make_unique<Label>("adcCalibrationFileLabel", "ADC Calibration File");
		adcCalibrationFileLabel->setBounds(infoLabel->getX() + 2, infoLabel->getBottom() + 5, 240, 16);
		adcCalibrationFileLabel->setColour(Label::textColourId, Colours::black);
		addAndMakeVisible(adcCalibrationFileLabel.get());

		adcCalibrationFile = std::make_unique<TextEditor>("ADC CAL FILE");
		adcCalibrationFile->setEnabled(false);
		adcCalibrationFile->setBounds(adcCalibrationFileLabel->getX(), adcCalibrationFileLabel->getBottom() + 2, adcCalibrationFileLabel->getWidth(), 20);
		adcCalibrationFile->setColour(Label::textColourId, Colours::black);
		adcCalibrationFile->addListener(this);
		addAndMakeVisible(adcCalibrationFile.get());

		adcCalibrationFileButton = std::make_unique<UtilityButton>("...");
		adcCalibrationFileButton->setBounds(adcCalibrationFile->getRight() + 3, adcCalibrationFile->getY(), 26, adcCalibrationFile->getHeight() + 2);
		adcCalibrationFileButton->setRadius(1.0f);
		adcCalibrationFileButton->addListener(this);
		adcCalibrationFileButton->setTooltip("Open a file dialog to choose the ADC calibration file for this probe.");
		addAndMakeVisible(adcCalibrationFileButton.get());

		adcCalibrationFileChooser = std::make_unique<FileChooser>("Select ADC Calibration file.", File::getSpecialLocation(File::userHomeDirectory), "*_ADCCalibration.csv");

		gainCalibrationFileLabel = std::make_unique<Label>("gainCalibrationFileLabel", "Gain Calibration File");
		gainCalibrationFileLabel->setBounds(adcCalibrationFile->getX(), adcCalibrationFile->getBottom() + 3, adcCalibrationFile->getWidth(), adcCalibrationFile->getHeight());
		gainCalibrationFileLabel->setColour(Label::textColourId, Colours::black);
		addAndMakeVisible(gainCalibrationFileLabel.get());

		gainCalibrationFile = std::make_unique<TextEditor>("GAIN CAL FILE");
		gainCalibrationFile->setEnabled(false);
		gainCalibrationFile->setBounds(gainCalibrationFileLabel->getX(), gainCalibrationFileLabel->getBottom() + 2, gainCalibrationFileLabel->getWidth(), gainCalibrationFileLabel->getHeight());
		gainCalibrationFile->setColour(Label::textColourId, Colours::black);
		gainCalibrationFile->addListener(this);
		addAndMakeVisible(gainCalibrationFile.get());

		gainCalibrationFileButton = std::make_unique<UtilityButton>("...");
		gainCalibrationFileButton->setBounds(adcCalibrationFileButton->getX(), gainCalibrationFile->getY(), adcCalibrationFileButton->getWidth(), adcCalibrationFileButton->getHeight());
		gainCalibrationFileButton->setRadius(3.0f);
		gainCalibrationFileButton->addListener(this);
		gainCalibrationFileButton->setTooltip("Open a file dialog to choose the Gain calibration file for this probe.");
		addAndMakeVisible(gainCalibrationFileButton.get());

		gainCalibrationFileChooser = std::make_unique<FileChooser>("Select Gain Calibration file.", File::getSpecialLocation(File::userHomeDirectory), "*_gainCalValues.csv");

		saveJsonButton = std::make_unique<UtilityButton>("SAVE TO JSON");
		saveJsonButton->setRadius(3.0f);
		saveJsonButton->setBounds(gainCalibrationFile->getX(), gainCalibrationFile->getBottom() + 4, 120, 22);
		saveJsonButton->addListener(this);
		saveJsonButton->setTooltip("Save channel map to probeinterface .json file");
		addAndMakeVisible(saveJsonButton.get());

		loadJsonButton = std::make_unique<UtilityButton>("LOAD FROM JSON");
		loadJsonButton->setRadius(3.0f);
		loadJsonButton->setBounds(saveJsonButton->getRight() + 5, saveJsonButton->getY(), 120, 22);
		loadJsonButton->addListener(this);
		loadJsonButton->setTooltip("Load channel map from probeinterface .json file");
		addAndMakeVisible(loadJsonButton.get());

		electrodesLabel = std::make_unique<Label>("ELECTRODES", "ELECTRODES");
		electrodesLabel->setFont(FontOptions("Inter", "Regular", 13.0f));
		electrodesLabel->setBounds(446, currentHeight - 20, 100, 20);
		addAndMakeVisible(electrodesLabel.get());

		enableViewButton = std::make_unique<UtilityButton>("VIEW");
		enableViewButton->setFont(fontRegularButton);
		enableViewButton->setRadius(3.0f);
		enableViewButton->setBounds(530, currentHeight + 2, 45, 18);
		enableViewButton->addListener(this);
		enableViewButton->setTooltip("View electrode enabled state");
		addAndMakeVisible(enableViewButton.get());

		enableButton = std::make_unique<UtilityButton>("ENABLE");
		enableButton->setFont(fontRegularButton);
		enableButton->setRadius(3.0f);
		enableButton->setBounds(450, currentHeight, 65, 22);
		enableButton->addListener(this);
		enableButton->setTooltip("Enable selected electrodes");
		addAndMakeVisible(enableButton.get());

		currentHeight += 58;

		electrodePresetLabel = std::make_unique<Label>("ELECTRODE PRESET", "ELECTRODE PRESET");
		electrodePresetLabel->setFont(fontRegularLabel);
		electrodePresetLabel->setBounds(446, currentHeight - 20, 150, 20);
		addAndMakeVisible(electrodePresetLabel.get());

		electrodeConfigurationComboBox = std::make_unique<ComboBox>("electrodeConfigurationComboBox");
		electrodeConfigurationComboBox->setBounds(450, currentHeight, 135, 22);
		electrodeConfigurationComboBox->addListener(this);
		electrodeConfigurationComboBox->setTooltip("Enable a pre-configured set of electrodes");
		electrodeConfigurationComboBox->addItem("Select a preset...", 1);
		electrodeConfigurationComboBox->setItemEnabled(1, false);
		electrodeConfigurationComboBox->addSeparator();

		auto npx = std::static_pointer_cast<Neuropixels_1>(device);

		for (int i = 0; i < npx->settings->availableElectrodeConfigurations.size(); i++)
		{
			electrodeConfigurationComboBox->addItem(npx->settings->availableElectrodeConfigurations[i], i + 2);
		}

		checkForExistingChannelPreset();

		addAndMakeVisible(electrodeConfigurationComboBox.get());

		currentHeight += 55;

		if (npx->settings->availableApGains.size() > 0)
		{
			apGainComboBox = std::make_unique<ComboBox>("apGainComboBox");
			apGainComboBox->setBounds(450, currentHeight, 65, 22);
			apGainComboBox->addListener(this);

			for (int i = 0; i < npx->settings->availableApGains.size(); i++)
				apGainComboBox->addItem(String(npx->settings->availableApGains[i]) + "x", i + 1);

			apGainComboBox->setSelectedId(npx->settings->apGainIndex + 1, dontSendNotification);
			addAndMakeVisible(apGainComboBox.get());

			apGainViewButton = std::make_unique<UtilityButton>("VIEW");
			apGainViewButton->setFont(fontRegularButton);
			apGainViewButton->setRadius(3.0f);
			apGainViewButton->setBounds(530, currentHeight + 2, 45, 18);
			apGainViewButton->addListener(this);
			apGainViewButton->setTooltip("View AP gain of each channel");
			addAndMakeVisible(apGainViewButton.get());

			apGainLabel = std::make_unique<Label>("AP GAIN", "AP GAIN");
			apGainLabel->setFont(fontRegularLabel);
			apGainLabel->setBounds(446, currentHeight - 20, 100, 20);
			addAndMakeVisible(apGainLabel.get());

			currentHeight += 55;
		}

		if (npx->settings->availableLfpGains.size() > 0)
		{
			lfpGainComboBox = std::make_unique<ComboBox>("lfpGainComboBox");
			lfpGainComboBox->setBounds(450, currentHeight, 65, 22);
			lfpGainComboBox->addListener(this);

			for (int i = 0; i < npx->settings->availableLfpGains.size(); i++)
				lfpGainComboBox->addItem(String(npx->settings->availableLfpGains[i]) + "x", i + 1);

			lfpGainComboBox->setSelectedId(npx->settings->lfpGainIndex + 1, dontSendNotification);
			addAndMakeVisible(lfpGainComboBox.get());

			lfpGainViewButton = std::make_unique<UtilityButton>("VIEW");
			lfpGainViewButton->setFont(fontRegularButton);
			lfpGainViewButton->setRadius(3.0f);
			lfpGainViewButton->setBounds(530, currentHeight + 2, 45, 18);
			lfpGainViewButton->addListener(this);
			lfpGainViewButton->setTooltip("View LFP gain of each channel");
			addAndMakeVisible(lfpGainViewButton.get());

			lfpGainLabel = std::make_unique<Label>("LFP GAIN", "LFP GAIN");
			lfpGainLabel->setFont(fontRegularLabel);
			lfpGainLabel->setBounds(446, currentHeight - 20, 100, 20);
			addAndMakeVisible(lfpGainLabel.get());

			currentHeight += 55;
		}

		if (npx->settings->availableReferences.size() > 0)
		{
			referenceComboBox = std::make_unique<ComboBox>("ReferenceComboBox");
			referenceComboBox->setBounds(450, currentHeight, 65, 22);
			referenceComboBox->addListener(this);

			for (int i = 0; i < npx->settings->availableReferences.size(); i++)
			{
				referenceComboBox->addItem(npx->settings->availableReferences[i], i + 1);
			}

			referenceComboBox->setSelectedId(npx->settings->referenceIndex + 1, dontSendNotification);
			addAndMakeVisible(referenceComboBox.get());

			referenceViewButton = std::make_unique<UtilityButton>("VIEW");
			referenceViewButton->setFont(fontRegularButton);
			referenceViewButton->setRadius(3.0f);
			referenceViewButton->setBounds(530, currentHeight + 2, 45, 18);
			referenceViewButton->addListener(this);
			referenceViewButton->setTooltip("View reference of each channel");
			addAndMakeVisible(referenceViewButton.get());

			referenceLabel = std::make_unique<Label>("REFERENCE", "REFERENCE");
			referenceLabel->setFont(fontRegularLabel);
			referenceLabel->setBounds(446, currentHeight - 20, 100, 20);
			addAndMakeVisible(referenceLabel.get());

			currentHeight += 55;
		}

		filterComboBox = std::make_unique<ComboBox>("FilterComboBox");
		filterComboBox->setBounds(450, currentHeight, 75, 22);
		filterComboBox->addListener(this);
		filterComboBox->addItem("ON", 1);
		filterComboBox->addItem("OFF", 2);
		filterComboBox->setSelectedId(1, dontSendNotification);
		addAndMakeVisible(filterComboBox.get());

		filterLabel = std::make_unique<Label>("FILTER", "AP FILTER CUT");
		filterLabel->setFont(fontRegularLabel);
		filterLabel->setBounds(446, currentHeight - 20, 200, 20);
		addAndMakeVisible(filterLabel.get());

		currentHeight += 55;

		activityViewButton = std::make_unique<UtilityButton>("VIEW");
		activityViewButton->setFont(fontRegularButton);
		activityViewButton->setRadius(3.0f);

		activityViewButton->addListener(this);
		activityViewButton->setTooltip("View peak-to-peak amplitudes for each channel");
		addAndMakeVisible(activityViewButton.get());

		activityViewComboBox = std::make_unique<ComboBox>("ActivityView Combo Box");

		if (npx->settings->availableLfpGains.size() > 0)
		{
			activityViewComboBox->setBounds(450, currentHeight, 65, 22);
			activityViewComboBox->addListener(this);
			activityViewComboBox->addItem("AP", 1);
			activityViewComboBox->addItem("LFP", 2);
			activityViewComboBox->setSelectedId(1, dontSendNotification);
			addAndMakeVisible(activityViewComboBox.get());
			activityViewButton->setBounds(530, currentHeight + 2, 45, 18);
		}
		else
		{
			activityViewButton->setBounds(450, currentHeight + 2, 45, 18);
		}

		activityViewLabel = std::make_unique<Label>("PROBE SIGNAL", "PROBE SIGNAL");
		activityViewLabel->setFont(fontRegularLabel);
		activityViewLabel->setBounds(446, currentHeight - 20, 180, 20);
		addAndMakeVisible(activityViewLabel.get());

		/// Draw Legends

		// ENABLE View
		Colour colour = Colour(55, 55, 55);
		float fontSize = 16.0f;

		enableViewComponent = std::make_unique<Component>("enableViewComponent");
		enableViewComponent->setBounds(450, 430, 120, 200);

		enableViewLabels.push_back(std::make_unique<Label>("enableViewLabel", "ENABLED?"));
		enableViewLabels[0]->setJustificationType(Justification::centredLeft);
		enableViewLabels[0]->setFont(FontOptions(fontSize));
		enableViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		enableViewLabels[0]->setBounds(0, 0, 110, 15);
		enableViewComponent->addAndMakeVisible(enableViewLabels[0].get());

		std::vector<Colour> colors = { Colours::yellow, Colours::darkgrey, Colours::black };
		StringArray legendLabels = { "YES", "NO", "REFERENCE" };

		for (int i = 0; i < colors.size(); i += 1)
		{
			enableViewRectangles.push_back(std::make_unique<DrawableRectangle>());
			enableViewRectangles[i]->setFill(colors[i]);
			enableViewRectangles[i]->setRectangle(Rectangle<float>(enableViewLabels[0]->getX() + 6, enableViewLabels[i]->getBottom() + 1, 12, 12));
			enableViewComponent->addAndMakeVisible(enableViewRectangles[i].get());

			enableViewLabels.push_back(std::make_unique<Label>("enableViewLabel", legendLabels[i]));
			int labelInd = i + 1;
			enableViewLabels[labelInd]->setJustificationType(Justification::centredLeft);
			enableViewLabels[labelInd]->setFont(FontOptions(fontSize));
			enableViewLabels[labelInd]->setColour(Label::ColourIds::textColourId, colour);
			enableViewLabels[labelInd]->setBounds(enableViewRectangles[i]->getRight() + 2, enableViewRectangles[i]->getY(), 100, 17);
			enableViewComponent->addAndMakeVisible(enableViewLabels[labelInd].get());
		}

		addAndMakeVisible(enableViewComponent.get());

		// AP GAIN View
		apGainViewComponent = std::make_unique<Component>("apGainViewComponent");
		apGainViewComponent->setBounds(enableViewComponent->getX(), enableViewComponent->getY(), 120, 300);

		apGainViewLabels.push_back(std::make_unique<Label>("apGainViewLabel", "AP GAIN"));
		apGainViewLabels[0]->setJustificationType(Justification::centredLeft);
		apGainViewLabels[0]->setFont(FontOptions(fontSize));
		apGainViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		apGainViewLabels[0]->setBounds(0, 0, 110, 15);
		apGainViewComponent->addAndMakeVisible(apGainViewLabels[0].get());

		colors.clear();
		legendLabels.clear();

		for (int i = 0; i < apGainComboBox->getNumItems(); i += 1)
		{
			colors.push_back(Colour(25 * i, 25 * i, 50));
			legendLabels.add(apGainComboBox->getItemText(i));
		}

		for (int i = 0; i < colors.size(); i += 1)
		{
			apGainViewRectangles.push_back(std::make_unique<DrawableRectangle>());
			apGainViewRectangles[i]->setFill(colors[i]);
			apGainViewRectangles[i]->setRectangle(Rectangle<float>(apGainViewLabels[0]->getX() + 6, apGainViewLabels[i]->getBottom() + 1, 12, 12));
			apGainViewComponent->addAndMakeVisible(apGainViewRectangles[i].get());

			apGainViewLabels.push_back(std::make_unique<Label>("apGainViewLabel", legendLabels[i]));
			int labelInd = i + 1;
			apGainViewLabels[labelInd]->setJustificationType(Justification::centredLeft);
			apGainViewLabels[labelInd]->setFont(FontOptions(fontSize));
			apGainViewLabels[labelInd]->setColour(Label::ColourIds::textColourId, colour);
			apGainViewLabels[labelInd]->setBounds(apGainViewRectangles[i]->getRight() + 2, apGainViewRectangles[i]->getY(), 100, 17);
			apGainViewComponent->addAndMakeVisible(apGainViewLabels[labelInd].get());
		}

		addAndMakeVisible(apGainViewComponent.get());

		// LFP GAIN View
		lfpGainViewComponent = std::make_unique<Component>("lfpGainViewComponent");
		lfpGainViewComponent->setBounds(enableViewComponent->getX(), enableViewComponent->getY(), 120, 300);

		lfpGainViewLabels.push_back(std::make_unique<Label>("lfpGainViewLabel", "LFP GAIN"));
		lfpGainViewLabels[0]->setJustificationType(Justification::centredLeft);
		lfpGainViewLabels[0]->setFont(FontOptions(fontSize));
		lfpGainViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		lfpGainViewLabels[0]->setBounds(0, 0, 110, 15);
		lfpGainViewComponent->addAndMakeVisible(lfpGainViewLabels[0].get());

		colors.clear();
		legendLabels.clear();

		for (int i = 0; i < lfpGainComboBox->getNumItems(); i += 1)
		{
			colors.push_back(Colour(66, 25 * i, 35 * i));
			legendLabels.add(lfpGainComboBox->getItemText(i));
		}

		for (int i = 0; i < colors.size(); i += 1)
		{
			lfpGainViewRectangles.push_back(std::make_unique<DrawableRectangle>());
			lfpGainViewRectangles[i]->setFill(colors[i]);
			lfpGainViewRectangles[i]->setRectangle(Rectangle<float>(lfpGainViewLabels[0]->getX() + 6, lfpGainViewLabels[i]->getBottom() + 1, 12, 12));
			lfpGainViewComponent->addAndMakeVisible(lfpGainViewRectangles[i].get());

			lfpGainViewLabels.push_back(std::make_unique<Label>("lfpGainViewLabel", legendLabels[i]));
			int labelInd = i + 1;
			lfpGainViewLabels[labelInd]->setJustificationType(Justification::centredLeft);
			lfpGainViewLabels[labelInd]->setFont(FontOptions(fontSize));
			lfpGainViewLabels[labelInd]->setColour(Label::ColourIds::textColourId, colour);
			lfpGainViewLabels[labelInd]->setBounds(lfpGainViewRectangles[i]->getRight() + 2, lfpGainViewRectangles[i]->getY(), 100, 17);
			lfpGainViewComponent->addAndMakeVisible(lfpGainViewLabels[labelInd].get());
		}

		addAndMakeVisible(lfpGainViewComponent.get());

		// REFERENCE View
		referenceViewComponent = std::make_unique<Component>("referenceViewComponent");
		referenceViewComponent->setBounds(enableViewComponent->getX(), enableViewComponent->getY(), 120, 300);

		referenceViewLabels.push_back(std::make_unique<Label>("referenceViewLabel", "REFERENCE"));
		referenceViewLabels[0]->setJustificationType(Justification::centredLeft);
		referenceViewLabels[0]->setFont(FontOptions(fontSize));
		referenceViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		referenceViewLabels[0]->setBounds(0, 0, 110, 15);
		referenceViewComponent->addAndMakeVisible(referenceViewLabels[0].get());

		colors.clear();
		legendLabels.clear();

		for (int i = 0; i < referenceComboBox->getNumItems(); i += 1)
		{
			String ref = referenceComboBox->getItemText(i);

			if (ref.contains("Ext"))
				colors.push_back(Colours::pink);
			else if (ref.contains("Tip"))
				colors.push_back(Colours::orange);
			else
				colors.push_back(Colours::purple);

			legendLabels.add(referenceComboBox->getItemText(i));
		}

		for (int i = 0; i < colors.size(); i += 1)
		{
			referenceViewRectangles.push_back(std::make_unique<DrawableRectangle>());
			referenceViewRectangles[i]->setFill(colors[i]);
			referenceViewRectangles[i]->setRectangle(Rectangle<float>(referenceViewLabels[0]->getX() + 6, referenceViewLabels[i]->getBottom() + 1, 12, 12));
			referenceViewComponent->addAndMakeVisible(referenceViewRectangles[i].get());

			referenceViewLabels.push_back(std::make_unique<Label>("lfpGainViewLabel", legendLabels[i]));
			int labelInd = i + 1;
			referenceViewLabels[labelInd]->setJustificationType(Justification::centredLeft);
			referenceViewLabels[labelInd]->setFont(FontOptions(fontSize));
			referenceViewLabels[labelInd]->setColour(Label::ColourIds::textColourId, colour);
			referenceViewLabels[labelInd]->setBounds(referenceViewRectangles[i]->getRight() + 2, referenceViewRectangles[i]->getY(), 100, 17);
			referenceViewComponent->addAndMakeVisible(referenceViewLabels[labelInd].get());
		}

		addAndMakeVisible(referenceViewComponent.get());

		// ACTIVITY View
		activityViewComponent = std::make_unique<Component>("activityViewComponent");
		activityViewComponent->setBounds(enableViewComponent->getX(), enableViewComponent->getY(), 120, 300);

		activityViewLabels.push_back(std::make_unique<Label>("activityViewLabel", "AMPLITUDE"));
		activityViewLabels[0]->setJustificationType(Justification::centredLeft);
		activityViewLabels[0]->setFont(FontOptions(fontSize));
		activityViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		activityViewLabels[0]->setBounds(0, 0, 110, 15);
		activityViewComponent->addAndMakeVisible(activityViewLabels[0].get());

		colors.clear();
		legendLabels.clear();

		for (int i = 0; i < 6; i += 1)
		{
			colors.push_back(ColourScheme::getColourForNormalizedValue(float(i) / 5.0f));
			legendLabels.add(String(float(probeBrowser->maxPeakToPeakAmplitude) / 5.0f * float(i)) + " uV");
		}

		for (int i = 0; i < colors.size(); i += 1)
		{
			activityViewRectangles.push_back(std::make_unique<DrawableRectangle>());
			activityViewRectangles[i]->setFill(colors[i]);
			activityViewRectangles[i]->setRectangle(Rectangle<float>(activityViewLabels[0]->getX() + 6, activityViewLabels[i]->getBottom() + 1, 12, 12));
			activityViewComponent->addAndMakeVisible(activityViewRectangles[i].get());

			activityViewLabels.push_back(std::make_unique<Label>("activityViewLabel", legendLabels[i]));
			int labelInd = i + 1;
			activityViewLabels[labelInd]->setJustificationType(Justification::centredLeft);
			activityViewLabels[labelInd]->setFont(FontOptions(fontSize));
			activityViewLabels[labelInd]->setColour(Label::ColourIds::textColourId, colour);
			activityViewLabels[labelInd]->setBounds(activityViewRectangles[i]->getRight() + 2, activityViewRectangles[i]->getY(), 100, 17);
			activityViewComponent->addAndMakeVisible(activityViewLabels[labelInd].get());
		}

		addAndMakeVisible(activityViewComponent.get());
	}
	else
	{
		type = SettingsInterface::Type::UNKNOWN_SETTINGS_INTERFACE;
	}

	drawLegend();

	updateInfoString();
}

void NeuropixV1Interface::updateInfoString()
{
	String nameString, infoString;

	nameString = "Headstage: ";

	auto npx = std::static_pointer_cast<Neuropixels_1>(device);

	if (device != nullptr)
	{
		nameString += "NeuropixelsV1f";

		infoString = "Device: Neuropixels V1 Probe";
		infoString += "\n";
		infoString += "\n";

		infoString += "Probe Number: ";
		infoString += npx->getProbeNumber();
		infoString += "\n";
		infoString += "\n";
	}

	infoLabel->setText(infoString, dontSendNotification);
	nameLabel->setText(nameString, dontSendNotification);
}

void NeuropixV1Interface::comboBoxChanged(ComboBox* comboBox)
{
	auto npx = std::static_pointer_cast<Neuropixels_1>(device);

	if (!editor->acquisitionIsActive)
	{
		auto npx = std::static_pointer_cast<Neuropixels_1>(device);

		if (comboBox == electrodeConfigurationComboBox.get())
		{
			String preset = electrodeConfigurationComboBox->getText();

			Array<int> selection = npx->selectElectrodeConfiguration(preset);

			selectElectrodes(selection);
		}
		else if ((comboBox == apGainComboBox.get()))
		{
			npx->settings->apGainIndex = apGainComboBox->getSelectedItemIndex();
		}
		else if (comboBox == lfpGainComboBox.get())
		{
			npx->settings->lfpGainIndex = lfpGainComboBox->getSelectedItemIndex();
		}
		else if (comboBox == referenceComboBox.get())
		{
			npx->settings->referenceIndex = referenceComboBox->getSelectedItemIndex();
		}
		else if (comboBox == filterComboBox.get())
		{
			npx->settings->apFilterState = filterComboBox->getSelectedId() == 1;
		}
		else if (comboBox == activityViewComboBox.get())
		{
			if (comboBox->getSelectedId() == 1)
			{
				probeBrowser->activityToView = ActivityToView::APVIEW;
				ColourScheme::setColourScheme(ColourSchemeId::PLASMA);
				probeBrowser->maxPeakToPeakAmplitude = 250.0f;
			}
			else
			{
				probeBrowser->activityToView = ActivityToView::LFPVIEW;
				ColourScheme::setColourScheme(ColourSchemeId::VIRIDIS);
				probeBrowser->maxPeakToPeakAmplitude = 500.0f;
			}
		}

		repaint();
	}
	else
	{
		if (comboBox == activityViewComboBox.get())
		{
			if (comboBox->getSelectedId() == 1)
			{
				probeBrowser->activityToView = ActivityToView::APVIEW;
				ColourScheme::setColourScheme(ColourSchemeId::PLASMA);
				probeBrowser->maxPeakToPeakAmplitude = 250.0f;
			}
			else
			{
				probeBrowser->activityToView = ActivityToView::LFPVIEW;
				ColourScheme::setColourScheme(ColourSchemeId::VIRIDIS);
				probeBrowser->maxPeakToPeakAmplitude = 500.0f;
			}

			repaint();
		}
		else
		{
			CoreServices::sendStatusMessage("Cannot update parameters while acquisition is active"); // no parameter change while acquisition is active
		}
	}
}

void NeuropixV1Interface::checkForExistingChannelPreset()
{
	auto npx = std::static_pointer_cast<Neuropixels_1>(device);

	std::set<Bank> uniqueBanks;
	std::vector<ElectrodeMetadata> electrodes;

	for (int i = 0; i < npx->settings->electrodeMetadata.size(); i += 1)
	{
		if (npx->settings->electrodeMetadata[i].status == ElectrodeStatus::CONNECTED)
		{
			uniqueBanks.insert(npx->settings->electrodeMetadata[i].bank);
			electrodes.push_back(npx->settings->electrodeMetadata[i]);
		}
	}

	if (uniqueBanks.size() == 1)
	{
		if (*uniqueBanks.begin() == Bank::A)
		{
			npx->settings->electrodeConfigurationIndex = 0;
		}
		else if (*uniqueBanks.begin() == Bank::B)
		{
			npx->settings->electrodeConfigurationIndex = 1;
		}
	}
	else if (uniqueBanks.size() == 2)
	{
		bool isBankC = true, isSingleColumn = true, isTetrode = true;

		for (int i = 0; i < electrodes.size(); i += 1)
		{
			if (electrodes[i].global_index < 576 || electrodes[i].global_index >= 960)
			{
				isBankC = false;
			}

			if ((electrodes[i].global_index % 2 != 0 && electrodes[i].bank == Bank::A) ||
				(electrodes[i].global_index % 2 != 1 && electrodes[i].bank == Bank::B) ||
				electrodes[i].bank == Bank::C)
			{
				isSingleColumn = false;
			}

			if ((electrodes[i].global_index % 8 >= 4 && electrodes[i].bank == Bank::A) ||
				(electrodes[i].global_index % 8 <= 3 && electrodes[i].bank == Bank::B) ||
				electrodes[i].bank == Bank::C)
			{
				isTetrode = false;
			}

			if (!isBankC && !isSingleColumn && !isTetrode) break;
		}

		if (isBankC)
		{
			npx->settings->electrodeConfigurationIndex = 2;
		}
		else if (isSingleColumn)
		{
			npx->settings->electrodeConfigurationIndex = 3;
		}
		else if (isTetrode)
		{
			npx->settings->electrodeConfigurationIndex = 4;
		}
		else
		{
			npx->settings->electrodeConfigurationIndex = -1;
		}
	}
	else
	{
		npx->settings->electrodeConfigurationIndex = -1;
	}

	electrodeConfigurationComboBox->setSelectedId(npx->settings->electrodeConfigurationIndex + 2, dontSendNotification);
}

void NeuropixV1Interface::buttonClicked(Button* button)
{
	auto npx = std::static_pointer_cast<Neuropixels_1>(device);

	if (button == deviceEnableButton.get())
	{
		npx->setEnabled(deviceEnableButton->getToggleState());

		if (canvas->foundInputSource())
		{
			device->configureDevice();
			canvas->resetContext();
		}

		if (npx->isEnabled())
		{
			deviceEnableButton->setLabel("ENABLED");
		}
		else
		{
			deviceEnableButton->setLabel("DISABLED");
		}

		CoreServices::updateSignalChain(editor);
	}
	else if (button == enableViewButton.get())
	{
		mode = VisualizationMode::ENABLE_VIEW;
		probeBrowser->stopTimer();
		drawLegend();
		repaint();
	}
	else if (button == apGainViewButton.get())
	{
		mode = VisualizationMode::AP_GAIN_VIEW;
		probeBrowser->stopTimer();
		drawLegend();
		repaint();
	}
	else if (button == lfpGainViewButton.get())
	{
		mode = VisualizationMode::LFP_GAIN_VIEW;
		probeBrowser->stopTimer();
		drawLegend();
		repaint();
	}
	else if (button == referenceViewButton.get())
	{
		mode = VisualizationMode::REFERENCE_VIEW;
		probeBrowser->stopTimer();
		drawLegend();
		repaint();
	}
	else if (button == activityViewButton.get())
	{
		mode = VisualizationMode::ACTIVITY_VIEW;

		if (acquisitionIsActive)
			probeBrowser->startTimer(100);

		drawLegend();
		repaint();
	}
	else if (button == enableButton.get())
	{
		Array<int> selection = getSelectedElectrodes();

		if (selection.size() > 0)
		{
			selectElectrodes(selection);
		}

		checkForExistingChannelPreset();
	}
	else if (button == loadJsonButton.get())
	{
		FileChooser fileChooser("Select an probeinterface JSON file to load.", File(), "*.json");

		if (fileChooser.browseForFileToOpen())
		{
			auto npx = std::static_pointer_cast<Neuropixels_1>(device);

			bool success = ProbeInterfaceJson::readProbeSettingsFromJson(fileChooser.getResult(), npx->settings.get());

			if (success)
			{
				applyProbeSettings(npx->settings.get());
			}
		}
	}
	else if (button == saveJsonButton.get())
	{
		FileChooser fileChooser("Save channel map to a probeinterface JSON file.", File(), "*.json");

		if (fileChooser.browseForFileToSave(true))
		{
			auto npx = std::static_pointer_cast<Neuropixels_1>(device);

			bool success = ProbeInterfaceJson::writeProbeSettingsToJson(fileChooser.getResult(), npx->settings.get());

			if (!success)
				CoreServices::sendStatusMessage("Failed to write probe channel map.");
			else
				CoreServices::sendStatusMessage("Successfully wrote probe channel map.");
		}
	}
	else if (button == adcCalibrationFileButton.get())
	{
		if (adcCalibrationFileChooser->browseForFileToOpen())
		{
			adcCalibrationFile->setText(adcCalibrationFileChooser->getResult().getFullPathName());
		}
		else
		{
			adcCalibrationFile->setText("");
		}

		npx->adcCalibrationFilePath = adcCalibrationFile->getText();
	}
	else if (button == gainCalibrationFileButton.get())
	{
		if (gainCalibrationFileChooser->browseForFileToOpen())
		{
			gainCalibrationFile->setText(gainCalibrationFileChooser->getResult().getFullPathName());
		}
		else
		{
			gainCalibrationFile->setText("");
		}

		npx->gainCalibrationFilePath = gainCalibrationFile->getText();
	}
}

Array<int> NeuropixV1Interface::getSelectedElectrodes() const
{
	Array<int> electrodeIndices;

	auto npx = std::static_pointer_cast<Neuropixels_1>(device);

	for (int i = 0; i < npx->settings->electrodeMetadata.size(); i++)
	{
		if (npx->settings->electrodeMetadata[i].isSelected)
		{
			electrodeIndices.add(i);
		}
	}

	return electrodeIndices;
}

void NeuropixV1Interface::setApGain(int index)
{
	apGainComboBox->setSelectedId(index + 1, true);
}

void NeuropixV1Interface::setLfpGain(int index)
{
	lfpGainComboBox->setSelectedId(index + 1, true);
}

void NeuropixV1Interface::setReference(int index)
{
	referenceComboBox->setSelectedId(index + 1, true);
}

void NeuropixV1Interface::setApFilterState(bool state)
{
	filterComboBox->setSelectedId(int(!state) + 1, true);
}

void NeuropixV1Interface::selectElectrodes(Array<int> electrodes)
{
	auto npx = std::static_pointer_cast<Neuropixels_1>(device);

	npx->settings->clearElectrodeSelection();

	// update selection state
	for (int i = 0; i < electrodes.size(); i++)
	{
		Bank bank = npx->settings->electrodeMetadata[electrodes[i]].bank;
		int channel = npx->settings->electrodeMetadata[electrodes[i]].channel;
		int shank = npx->settings->electrodeMetadata[electrodes[i]].shank;
		int global_index = npx->settings->electrodeMetadata[electrodes[i]].global_index;

		for (int j = 0; j < npx->settings->electrodeMetadata.size(); j++)
		{
			if (npx->settings->electrodeMetadata[j].channel == channel)
			{
				if (npx->settings->electrodeMetadata[j].bank == bank && npx->settings->electrodeMetadata[j].shank == shank)
				{
					npx->settings->electrodeMetadata.getReference(j).status = ElectrodeStatus::CONNECTED;
				}

				else
				{
					npx->settings->electrodeMetadata.getReference(j).status = ElectrodeStatus::DISCONNECTED;
				}
			}
		}

		npx->settings->selectedBank.add(bank);
		npx->settings->selectedChannel.add(channel);
		npx->settings->selectedShank.add(shank);
		npx->settings->selectedElectrode.add(global_index);
	}

	repaint();
}

void NeuropixV1Interface::setInterfaceEnabledState(bool enabledState)
{
	if (deviceEnableButton != nullptr)
		deviceEnableButton->setEnabled(enabledState);

	if (enableButton != nullptr)
		enableButton->setEnabled(enabledState);

	if (electrodeConfigurationComboBox != nullptr)
		electrodeConfigurationComboBox->setEnabled(enabledState);

	if (apGainComboBox != nullptr)
		apGainComboBox->setEnabled(enabledState);

	if (lfpGainComboBox != nullptr)
		lfpGainComboBox->setEnabled(enabledState);

	if (filterComboBox != nullptr)
		filterComboBox->setEnabled(enabledState);

	if (referenceComboBox != nullptr)
		referenceComboBox->setEnabled(enabledState);

	if (loadJsonButton != nullptr)
		loadJsonButton->setEnabled(enabledState);
}

void NeuropixV1Interface::startAcquisition()
{
	acquisitionIsActive = true;

	setInterfaceEnabledState(false);

	if (mode == VisualizationMode::ACTIVITY_VIEW)
		probeBrowser->startTimer(100);
}

void NeuropixV1Interface::stopAcquisition()
{
	acquisitionIsActive = false;

	setInterfaceEnabledState(true);
}

void NeuropixV1Interface::drawLegend()
{
	enableViewComponent->setVisible(false);
	apGainViewComponent->setVisible(false);
	lfpGainViewComponent->setVisible(false);
	referenceViewComponent->setVisible(false);
	activityViewComponent->setVisible(false);

	switch (mode)
	{
	case VisualizationMode::ENABLE_VIEW:
		enableViewComponent->setVisible(true);
		break;
	case VisualizationMode::AP_GAIN_VIEW:
		apGainViewComponent->setVisible(true);
		break;
	case VisualizationMode::LFP_GAIN_VIEW:
		lfpGainViewComponent->setVisible(true);
		break;
	case VisualizationMode::REFERENCE_VIEW:
		referenceViewComponent->setVisible(true);
		break;
	case VisualizationMode::ACTIVITY_VIEW:
		activityViewComponent->setVisible(true);
		break;
	default:
		break;
	}
}

bool NeuropixV1Interface::applyProbeSettings(ProbeSettings* p, bool shouldUpdateProbe)
{
	auto npx = std::static_pointer_cast<Neuropixels_1>(device);

	if (electrodeConfigurationComboBox != 0)
		electrodeConfigurationComboBox->setSelectedId(p->electrodeConfigurationIndex + 2, dontSendNotification);

	// update display
	if (apGainComboBox != 0)
		apGainComboBox->setSelectedId(p->apGainIndex + 1, dontSendNotification);

	if (lfpGainComboBox != 0)
		lfpGainComboBox->setSelectedId(p->lfpGainIndex + 1, dontSendNotification);

	if (filterComboBox != 0)
	{
		if (p->apFilterState)
			filterComboBox->setSelectedId(1, dontSendNotification);
		else
			filterComboBox->setSelectedId(2, dontSendNotification);
	}

	if (referenceComboBox != 0)
		referenceComboBox->setSelectedId(p->referenceIndex + 1, dontSendNotification);
	
	for (int i = 0; i < npx->settings->electrodeMetadata.size(); i++)
	{
		if (npx->settings->electrodeMetadata[i].status == ElectrodeStatus::CONNECTED)
			npx->settings->electrodeMetadata.getReference(i).status = ElectrodeStatus::DISCONNECTED;
	}

	// update selection state
	for (int i = 0; i < p->selectedChannel.size(); i++)
	{
		Bank bank = p->selectedBank[i];
		int channel = p->selectedChannel[i];
		int shank = p->selectedShank[i];

		for (int j = 0; j < npx->settings->electrodeMetadata.size(); j++)
		{
			if (npx->settings->electrodeMetadata[j].channel == channel && npx->settings->electrodeMetadata[j].bank == bank && npx->settings->electrodeMetadata[j].shank == shank)
			{
				npx->settings->electrodeMetadata.getReference(j).status = ElectrodeStatus::CONNECTED;
			}
		}
	}

	if (shouldUpdateProbe)
	{
		CoreServices::saveRecoveryConfig();
	}

	drawLegend();
	repaint();

	return true;
}

void NeuropixV1Interface::saveParameters(XmlElement* xml)
{
	if (device != nullptr)
	{
		auto npx = std::static_pointer_cast<Neuropixels_1>(device);

		LOGD("Saving Neuropix display.");

		XmlElement* xmlNode = xml->createNewChildElement("NP_PROBE");

		xmlNode->setAttribute("probe_serial_number", String(npx->getProbeNumber()));
		xmlNode->setAttribute("probe_name", npx->getName());
		xmlNode->setAttribute("num_adcs", npx->settings->probeMetadata.num_adcs);

		xmlNode->setAttribute("ZoomHeight", probeBrowser->getZoomHeight());
		xmlNode->setAttribute("ZoomOffset", probeBrowser->getZoomOffset());

		if (apGainComboBox != nullptr)
		{
			xmlNode->setAttribute("apGainValue", apGainComboBox->getText());
			xmlNode->setAttribute("apGainIndex", apGainComboBox->getSelectedId() - 1);
		}

		if (lfpGainComboBox != nullptr)
		{
			xmlNode->setAttribute("lfpGainValue", lfpGainComboBox->getText());
			xmlNode->setAttribute("lfpGainIndex", lfpGainComboBox->getSelectedId() - 1);
		}

		if (electrodeConfigurationComboBox != nullptr)
		{
			if (electrodeConfigurationComboBox->getSelectedId() > 1)
			{
				xmlNode->setAttribute("electrodeConfigurationPreset", electrodeConfigurationComboBox->getText());
			}
			else
			{
				xmlNode->setAttribute("electrodeConfigurationPreset", "NONE");
			}
		}

		if (referenceComboBox != nullptr)
		{
			if (referenceComboBox->getSelectedId() > 0)
			{
				xmlNode->setAttribute("referenceChannel", referenceComboBox->getText());
				xmlNode->setAttribute("referenceChannelIndex", referenceComboBox->getSelectedId() - 1);
			}
			else
			{
				xmlNode->setAttribute("referenceChannel", "Ext");
				xmlNode->setAttribute("referenceChannelIndex", 0);
			}
		}

		if (filterComboBox != nullptr)
		{
			xmlNode->setAttribute("filterCut", filterComboBox->getText());
			xmlNode->setAttribute("filterCutIndex", filterComboBox->getSelectedId());
		}

		XmlElement* channelNode = xmlNode->createNewChildElement("CHANNELS");
		XmlElement* xposNode = xmlNode->createNewChildElement("ELECTRODE_XPOS");
		XmlElement* yposNode = xmlNode->createNewChildElement("ELECTRODE_YPOS");

		for (int i = 0; i < npx->settings->selectedChannel.size(); i++)
		{
			int bank = int(npx->settings->selectedBank[i]);
			int shank = npx->settings->selectedShank[i];
			int channel = npx->settings->selectedChannel[i];
			int elec = npx->settings->selectedElectrode[i];

			String chString = String(bank);

			String chId = "CH" + String(channel);

			channelNode->setAttribute(chId, chString);
			xposNode->setAttribute(chId, String(npx->settings->electrodeMetadata[elec].xpos + 250 * shank));
			yposNode->setAttribute(chId, String(npx->settings->electrodeMetadata[elec].ypos));
		}

		xmlNode->setAttribute("visualizationMode", (double)mode);
		xmlNode->setAttribute("activityToView", (double)probeBrowser->activityToView);

		xmlNode->setAttribute("isEnabled", bool(device->isEnabled()));
	}
}

void NeuropixV1Interface::loadParameters(XmlElement* xml)
{
	auto npx = std::static_pointer_cast<Neuropixels_1>(device);

	if (npx != nullptr)
	{
		//auto npx = std::static_pointer_cast<Neuropixels_1>(device);

		// TODO: load parameters, put them into device->settings, and then update the interface
		//applyProbeSettings(device->settings.get(), false);
	}
}

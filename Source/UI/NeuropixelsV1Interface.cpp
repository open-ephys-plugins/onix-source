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

#include "NeuropixelsV1Interface.h"
#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"
#include "../Formats/ProbeInterface.h"

using namespace OnixSourcePlugin;
using namespace ColourScheme;

NeuropixelsV1Interface::NeuropixelsV1Interface(std::shared_ptr<Neuropixels1> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	if (d->getDeviceType() != OnixDeviceType::NEUROPIXELSV1E && d->getDeviceType() != OnixDeviceType::NEUROPIXELSV1F)
	{
		Onix1::showWarningMessageBoxAsync("Incorrect Device Given", "Incorrect device type given when creating the Neuropixels settings interface.");
		return;
	}

	ColourScheme::setColourScheme(ColourSchemeId::PLASMA);

	if (device != nullptr)
	{
		type = device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1E ? SettingsInterface::Type::NEUROPIXELS1E_SETTINGS_INTERFACE : SettingsInterface::Type::NEUROPIXELS1F_SETTINGS_INTERFACE;

		mode = VisualizationMode::ENABLE_VIEW;

		probeBrowser = std::make_unique<NeuropixelsV1ProbeBrowser>(this, 0);
		probeBrowser->setBounds(0, 0, 600, 600);
		addAndMakeVisible(probeBrowser.get());

		int currentHeight = 55;

		FontOptions fontName = FontOptions("Fira Code", "Medium", 30.0f);
		FontOptions fontRegularButton = FontOptions("Fira Code", "Regular", 12.0f);
		FontOptions fontRegularLabel = FontOptions("Fira Code", "Regular", 13.0f);

		deviceLabel = std::make_unique<Label>("deviceLabel", "Device Name");
		deviceLabel->setFont(fontName);
		deviceLabel->setBounds(625, 40, 430, 45);
		addAndMakeVisible(deviceLabel.get());

		deviceEnableButton = std::make_unique<UtilityButton>(enabledButtonText);
		deviceEnableButton->setFont(fontRegularButton);
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(deviceLabel->getX(), deviceLabel->getBottom() + 3, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setTooltip("If disabled, probe will not stream data during acquisition");
		deviceEnableButton->setToggleState(true, dontSendNotification);
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());
		deviceEnableButton->setToggleState(device->isEnabled(), sendNotification);

		infoLabel = std::make_unique<Label>("INFO", "INFO");
		infoLabel->setFont(FontOptions(15.0f));
		infoLabel->setBounds(deviceEnableButton->getX(), deviceEnableButton->getBottom() + 10, deviceLabel->getWidth(), 50);
		infoLabel->setJustificationType(Justification::topLeft);
		addAndMakeVisible(infoLabel.get());

		adcCalibrationFileLabel = std::make_unique<Label>("adcCalibrationFileLabel", "ADC Calibration File");
		adcCalibrationFileLabel->setBounds(infoLabel->getX(), infoLabel->getBottom() + 15, 240, 16);
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

		adcCalibrationFileChooser = std::make_unique<FileChooser>("Select ADC Calibration file.", File(), "*_ADCCalibration.csv");

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

		gainCalibrationFileChooser = std::make_unique<FileChooser>("Select Gain Calibration file.", File(), "*_gainCalValues.csv");

		probeInterfaceRectangle = std::make_unique<DrawableRectangle>();
		probeInterfaceRectangle->setFill(Colours::darkgrey);
		probeInterfaceRectangle->setRectangle(Rectangle<float>(gainCalibrationFile->getX(), gainCalibrationFile->getBottom() + 15, 250, 50));
		addAndMakeVisible(probeInterfaceRectangle.get());

		probeInterfaceLabel = std::make_unique<Label>("probeInterfaceLabel", "Probe Interface");
		probeInterfaceLabel->setFont(fontRegularLabel);
		probeInterfaceLabel->setBounds(probeInterfaceRectangle->getX(), probeInterfaceRectangle->getY(), 90, 18);
		probeInterfaceLabel->setColour(Label::textColourId, Colours::black);
		addAndMakeVisible(probeInterfaceLabel.get());

		saveJsonButton = std::make_unique<UtilityButton>("Save to JSON");
		saveJsonButton->setRadius(3.0f);
		saveJsonButton->setBounds(probeInterfaceRectangle->getX() + 3, probeInterfaceRectangle->getY() + 20, 120, 22);
		saveJsonButton->addListener(this);
		saveJsonButton->setTooltip("Save channel map to ProbeInterface .json file");
		addAndMakeVisible(saveJsonButton.get());

		loadJsonButton = std::make_unique<UtilityButton>("Load from JSON");
		loadJsonButton->setRadius(3.0f);
		loadJsonButton->setBounds(saveJsonButton->getRight() + 5, saveJsonButton->getY(), 120, 22);
		loadJsonButton->addListener(this);
		loadJsonButton->setTooltip("Load channel map from ProbeInterface .json file");
		addAndMakeVisible(loadJsonButton.get());

		saveSettingsButton = std::make_unique<UtilityButton>("Save Settings");
		saveSettingsButton->setRadius(3.0f);
		saveSettingsButton->setBounds(saveJsonButton->getX(), probeBrowser->getBottom() - 80, 120, 22);
		saveSettingsButton->addListener(this);
		saveSettingsButton->setTooltip("Save all Neuropixels settings to file.");
		addAndMakeVisible(saveSettingsButton.get());

		loadSettingsButton = std::make_unique<UtilityButton>("Load Settings");
		loadSettingsButton->setRadius(3.0f);
		loadSettingsButton->setBounds(saveSettingsButton->getRight() + 5, saveSettingsButton->getY(), saveSettingsButton->getWidth(), saveSettingsButton->getHeight());
		loadSettingsButton->addListener(this);
		loadSettingsButton->setTooltip("Load all Neuropixels settings from a file.");
		addAndMakeVisible(loadSettingsButton.get());

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

		selectElectrodeButton = std::make_unique<UtilityButton>("SELECT");
		selectElectrodeButton->setFont(fontRegularButton);
		selectElectrodeButton->setRadius(3.0f);
		selectElectrodeButton->setBounds(450, currentHeight, 65, 22);
		selectElectrodeButton->addListener(this);
		selectElectrodeButton->setTooltip("Enable selected electrodes");
		addAndMakeVisible(selectElectrodeButton.get());

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

		auto settings = std::static_pointer_cast<Neuropixels1>(device)->settings[0].get();

		for (int i = 0; i < settings->availableElectrodeConfigurations.size(); i++)
		{
			electrodeConfigurationComboBox->addItem(settings->availableElectrodeConfigurations[i], i + 2);
		}

		checkForExistingChannelPreset();

		addAndMakeVisible(electrodeConfigurationComboBox.get());

		currentHeight += 55;

		if (settings->availableApGains.size() > 0)
		{
			apGainComboBox = std::make_unique<ComboBox>("apGainComboBox");
			apGainComboBox->setBounds(450, currentHeight, 65, 22);
			apGainComboBox->addListener(this);

			for (int i = 0; i < settings->availableApGains.size(); i++)
				apGainComboBox->addItem(String(settings->availableApGains[i]) + "x", i + 1);

			apGainComboBox->setSelectedId(settings->apGainIndex + 1, dontSendNotification);
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

		if (settings->availableLfpGains.size() > 0)
		{
			lfpGainComboBox = std::make_unique<ComboBox>("lfpGainComboBox");
			lfpGainComboBox->setBounds(450, currentHeight, 65, 22);
			lfpGainComboBox->addListener(this);

			for (int i = 0; i < settings->availableLfpGains.size(); i++)
				lfpGainComboBox->addItem(String(settings->availableLfpGains[i]) + "x", i + 1);

			lfpGainComboBox->setSelectedId(settings->lfpGainIndex + 1, dontSendNotification);
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

		if (settings->availableReferences.size() > 0)
		{
			referenceComboBox = std::make_unique<ComboBox>("ReferenceComboBox");
			referenceComboBox->setBounds(450, currentHeight, 65, 22);
			referenceComboBox->addListener(this);

			for (int i = 0; i < settings->availableReferences.size(); i++)
			{
				referenceComboBox->addItem(settings->availableReferences[i], i + 1);
			}

			referenceComboBox->setSelectedId(settings->referenceIndex + 1, dontSendNotification);
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

#pragma region Draw Legends

		// ENABLE View
		Colour colour = Colour(55, 55, 55);
		float fontSize = 16.0f;

		enableViewComponent = std::make_unique<Component>("enableViewComponent");
		enableViewComponent->setBounds(450, 430, 120, 200);

		enableViewLabels.emplace_back(std::make_unique<Label>("enableViewLabel", "ENABLED?"));
		enableViewLabels[0]->setJustificationType(Justification::centredLeft);
		enableViewLabels[0]->setFont(FontOptions(fontSize));
		enableViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		enableViewLabels[0]->setBounds(0, 0, 110, 15);
		enableViewComponent->addAndMakeVisible(enableViewLabels[0].get());

		std::vector<Colour> colors = { Colours::yellow, Colours::darkgrey, Colours::black };
		StringArray legendLabels = { "YES", "NO", "REFERENCE" };

		for (int i = 0; i < colors.size(); i++)
		{
			enableViewRectangles.emplace_back(std::make_unique<DrawableRectangle>());
			enableViewRectangles[i]->setFill(colors[i]);
			enableViewRectangles[i]->setRectangle(Rectangle<float>(enableViewLabels[0]->getX() + 6, enableViewLabels[i]->getBottom() + 1, 12, 12));
			enableViewComponent->addAndMakeVisible(enableViewRectangles[i].get());

			enableViewLabels.emplace_back(std::make_unique<Label>("enableViewLabel", legendLabels[i]));
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

		apGainViewLabels.emplace_back(std::make_unique<Label>("apGainViewLabel", "AP GAIN"));
		apGainViewLabels[0]->setJustificationType(Justification::centredLeft);
		apGainViewLabels[0]->setFont(FontOptions(fontSize));
		apGainViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		apGainViewLabels[0]->setBounds(0, 0, 110, 15);
		apGainViewComponent->addAndMakeVisible(apGainViewLabels[0].get());

		colors.clear();
		legendLabels.clear();

		for (int i = 0; i < apGainComboBox->getNumItems(); i++)
		{
			colors.emplace_back(Colour(25 * i, 25 * i, 50));
			legendLabels.add(apGainComboBox->getItemText(i));
		}

		for (int i = 0; i < colors.size(); i++)
		{
			apGainViewRectangles.emplace_back(std::make_unique<DrawableRectangle>());
			apGainViewRectangles[i]->setFill(colors[i]);
			apGainViewRectangles[i]->setRectangle(Rectangle<float>(apGainViewLabels[0]->getX() + 6, apGainViewLabels[i]->getBottom() + 1, 12, 12));
			apGainViewComponent->addAndMakeVisible(apGainViewRectangles[i].get());

			apGainViewLabels.emplace_back(std::make_unique<Label>("apGainViewLabel", legendLabels[i]));
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

		lfpGainViewLabels.emplace_back(std::make_unique<Label>("lfpGainViewLabel", "LFP GAIN"));
		lfpGainViewLabels[0]->setJustificationType(Justification::centredLeft);
		lfpGainViewLabels[0]->setFont(FontOptions(fontSize));
		lfpGainViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		lfpGainViewLabels[0]->setBounds(0, 0, 110, 15);
		lfpGainViewComponent->addAndMakeVisible(lfpGainViewLabels[0].get());

		colors.clear();
		legendLabels.clear();

		for (int i = 0; i < lfpGainComboBox->getNumItems(); i++)
		{
			colors.emplace_back(Colour(66, 25 * i, 35 * i));
			legendLabels.add(lfpGainComboBox->getItemText(i));
		}

		for (int i = 0; i < colors.size(); i++)
		{
			lfpGainViewRectangles.emplace_back(std::make_unique<DrawableRectangle>());
			lfpGainViewRectangles[i]->setFill(colors[i]);
			lfpGainViewRectangles[i]->setRectangle(Rectangle<float>(lfpGainViewLabels[0]->getX() + 6, lfpGainViewLabels[i]->getBottom() + 1, 12, 12));
			lfpGainViewComponent->addAndMakeVisible(lfpGainViewRectangles[i].get());

			lfpGainViewLabels.emplace_back(std::make_unique<Label>("lfpGainViewLabel", legendLabels[i]));
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

		referenceViewLabels.emplace_back(std::make_unique<Label>("referenceViewLabel", "REFERENCE"));
		referenceViewLabels[0]->setJustificationType(Justification::centredLeft);
		referenceViewLabels[0]->setFont(FontOptions(fontSize));
		referenceViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		referenceViewLabels[0]->setBounds(0, 0, 110, 15);
		referenceViewComponent->addAndMakeVisible(referenceViewLabels[0].get());

		colors.clear();
		legendLabels.clear();

		for (int i = 0; i < referenceComboBox->getNumItems(); i++)
		{
			auto ref = referenceComboBox->getItemText(i);

			if (ref.contains("Ext"))
				colors.emplace_back(Colours::pink);
			else if (ref.contains("Tip"))
				colors.emplace_back(Colours::orange);
			else
				colors.emplace_back(Colours::purple);

			legendLabels.add(referenceComboBox->getItemText(i));
		}

		for (int i = 0; i < colors.size(); i++)
		{
			referenceViewRectangles.emplace_back(std::make_unique<DrawableRectangle>());
			referenceViewRectangles[i]->setFill(colors[i]);
			referenceViewRectangles[i]->setRectangle(Rectangle<float>(referenceViewLabels[0]->getX() + 6, referenceViewLabels[i]->getBottom() + 1, 12, 12));
			referenceViewComponent->addAndMakeVisible(referenceViewRectangles[i].get());

			referenceViewLabels.emplace_back(std::make_unique<Label>("referenceViewLabels", legendLabels[i]));
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

		activityViewLabels.emplace_back(std::make_unique<Label>("activityViewLabel", "AMPLITUDE"));
		activityViewLabels[0]->setJustificationType(Justification::centredLeft);
		activityViewLabels[0]->setFont(FontOptions(fontSize));
		activityViewLabels[0]->setColour(Label::ColourIds::textColourId, colour);
		activityViewLabels[0]->setBounds(0, 0, 110, 15);
		activityViewComponent->addAndMakeVisible(activityViewLabels[0].get());

		colors.clear();
		legendLabels.clear();

		for (int i = 0; i < 6; i++)
		{
			colors.emplace_back(ColourScheme::getColourForNormalizedValue(float(i) / 5.0f));
			legendLabels.add(std::to_string(float(probeBrowser->maxPeakToPeakAmplitude) / 5.0f * float(i)) + " uV");
		}

		for (int i = 0; i < colors.size(); i++)
		{
			activityViewRectangles.emplace_back(std::make_unique<DrawableRectangle>());
			activityViewRectangles[i]->setFill(colors[i]);
			activityViewRectangles[i]->setRectangle(Rectangle<float>(activityViewLabels[0]->getX() + 6, activityViewLabels[i]->getBottom() + 1, 12, 12));
			activityViewComponent->addAndMakeVisible(activityViewRectangles[i].get());

			activityViewLabels.emplace_back(std::make_unique<Label>("activityViewLabel", legendLabels[i]));
			int labelInd = i + 1;
			activityViewLabels[labelInd]->setJustificationType(Justification::centredLeft);
			activityViewLabels[labelInd]->setFont(FontOptions(fontSize));
			activityViewLabels[labelInd]->setColour(Label::ColourIds::textColourId, colour);
			activityViewLabels[labelInd]->setBounds(activityViewRectangles[i]->getRight() + 2, activityViewRectangles[i]->getY(), 100, 17);
			activityViewComponent->addAndMakeVisible(activityViewLabels[labelInd].get());
		}
#pragma endregion

		addAndMakeVisible(activityViewComponent.get());

		updateSettings();
	}

	drawLegend();

	updateInfoString();
}

void NeuropixelsV1Interface::updateSettings()
{
	if (device == nullptr) return;

	auto npx1 = std::static_pointer_cast<Neuropixels1>(device);

	applyProbeSettings(npx1->settings[0].get());
	checkForExistingChannelPreset();

	deviceEnableButton->setToggleState(npx1->isEnabled(), sendNotification);

	gainCalibrationFile->setText(npx1->getGainCalibrationFilePath() == "None" ? "" : npx1->getGainCalibrationFilePath(), dontSendNotification);
	adcCalibrationFile->setText(npx1->getAdcCalibrationFilePath() == "None" ? "" : npx1->getAdcCalibrationFilePath(), dontSendNotification);
}

void NeuropixelsV1Interface::updateInfoString()
{
	std::string deviceString, infoString;

	auto npx = std::static_pointer_cast<Neuropixels1>(device);

	if (device != nullptr)
	{
		deviceString = "Neuropixels 1.0 Probe";

		infoString += "\n";
		infoString += "Probe Number: ";
		infoString += std::to_string(npx->getProbeSerialNumber());
		infoString += "\n";
		infoString += "\n";
	}

	deviceLabel->setText(deviceString, dontSendNotification);
	infoLabel->setText(infoString, dontSendNotification);
}

void NeuropixelsV1Interface::comboBoxChanged(ComboBox* comboBox)
{
	auto npx = std::static_pointer_cast<Neuropixels1>(device);

	if (!editor->acquisitionIsActive)
	{
		auto npx = std::static_pointer_cast<Neuropixels1>(device);
		auto settings = npx->settings[0].get();

		if (comboBox == electrodeConfigurationComboBox.get())
		{
			auto selection = npx->selectElectrodeConfiguration(electrodeConfigurationComboBox->getSelectedItemIndex() - 1);

			selectElectrodes(selection);
		}
		else if ((comboBox == apGainComboBox.get()))
		{
			settings->apGainIndex = apGainComboBox->getSelectedItemIndex();
		}
		else if (comboBox == lfpGainComboBox.get())
		{
			settings->lfpGainIndex = lfpGainComboBox->getSelectedItemIndex();
		}
		else if (comboBox == referenceComboBox.get())
		{
			settings->referenceIndex = referenceComboBox->getSelectedItemIndex();
		}
		else if (comboBox == filterComboBox.get())
		{
			settings->apFilterState = filterComboBox->getSelectedId() == 1;
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

void NeuropixelsV1Interface::checkForExistingChannelPreset()
{
	auto npx = std::static_pointer_cast<Neuropixels1>(device);
	auto settings = npx->settings[0].get();

	settings->electrodeConfigurationIndex = -1;

	for (int i = 0; i < settings->availableElectrodeConfigurations.size(); i++)
	{
		auto selection = npx->selectElectrodeConfiguration(i);
		std::vector<int> channelMap;
		channelMap.assign(NeuropixelsV2eValues::numberOfChannels, 0);

		for (int j = 0; j < selection.size(); j++)
		{
			channelMap[settings->electrodeMetadata[selection[j]].channel] = selection[j];
		}

		if (std::equal(channelMap.cbegin(), channelMap.cend(), settings->selectedElectrode.cbegin(), settings->selectedElectrode.cend()))
		{
			settings->electrodeConfigurationIndex = i;
			break;
		}
	}

	electrodeConfigurationComboBox->setSelectedId(settings->electrodeConfigurationIndex + 2, dontSendNotification);
}

void NeuropixelsV1Interface::buttonClicked(Button* button)
{
	auto npx = std::static_pointer_cast<Neuropixels1>(device);

	if (button == deviceEnableButton.get())
	{
		npx->setEnabled(deviceEnableButton->getToggleState());

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
	else if (button == enableViewButton.get())
	{
		mode = SettingsInterface::VisualizationMode::ENABLE_VIEW;
		probeBrowser->stopTimer();
		drawLegend();
		repaint();
	}
	else if (button == apGainViewButton.get())
	{
		mode = SettingsInterface::VisualizationMode::AP_GAIN_VIEW;
		probeBrowser->stopTimer();
		drawLegend();
		repaint();
	}
	else if (button == lfpGainViewButton.get())
	{
		mode = SettingsInterface::VisualizationMode::LFP_GAIN_VIEW;
		probeBrowser->stopTimer();
		drawLegend();
		repaint();
	}
	else if (button == referenceViewButton.get())
	{
		mode = SettingsInterface::VisualizationMode::REFERENCE_VIEW;
		probeBrowser->stopTimer();
		drawLegend();
		repaint();
	}
	else if (button == activityViewButton.get())
	{
		mode = SettingsInterface::VisualizationMode::ACTIVITY_VIEW;

		if (acquisitionIsActive)
			probeBrowser->startTimer(100);

		drawLegend();
		repaint();
	}
	else if (button == selectElectrodeButton.get())
	{
		auto selection = getSelectedElectrodes();

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
			auto npx = std::static_pointer_cast<Neuropixels1>(device);

			if (ProbeInterfaceJson::readProbeSettingsFromJson(fileChooser.getResult(), npx->settings[0].get()))
			{
				applyProbeSettings(npx->settings[0].get());
				checkForExistingChannelPreset();
			}
		}
	}
	else if (button == saveJsonButton.get())
	{
		FileChooser fileChooser("Save channel map to a probeinterface JSON file.", File(), "*.json");

		if (fileChooser.browseForFileToSave(true))
		{
			auto npx = std::static_pointer_cast<Neuropixels1>(device);

			if (!ProbeInterfaceJson::writeProbeSettingsToJson(fileChooser.getResult(), npx->settings[0].get()))
				CoreServices::sendStatusMessage("Failed to write probe channel map.");
			else
				CoreServices::sendStatusMessage("Successfully wrote probe channel map.");
		}
	}
	else if (button == saveSettingsButton.get())
	{
		FileChooser fileChooser("Save Neuropixels settings to an XML file.", File(), "*.xml");

		if (fileChooser.browseForFileToSave(true))
		{
			XmlElement rootElement("DEVICE");

			saveParameters(&rootElement);

			writeToXmlFile(&rootElement, fileChooser.getResult());
		}
	}
	else if (button == loadSettingsButton.get())
	{
		FileChooser fileChooser("Load Neuropixels settings from an XML file.", File(), "*.xml");

		if (fileChooser.browseForFileToOpen())
		{
			auto rootElement = readFromXmlFile(fileChooser.getResult());

			loadParameters(rootElement);

			delete rootElement;
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

		std::static_pointer_cast<Neuropixels1>(device)->setAdcCalibrationFilePath(adcCalibrationFile->getText().toStdString());
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

		std::static_pointer_cast<Neuropixels1>(device)->setGainCalibrationFilePath(gainCalibrationFile->getText().toStdString());
	}
}

std::vector<int> NeuropixelsV1Interface::getSelectedElectrodes() const
{
	std::vector<int> electrodeIndices;

	auto settings = std::static_pointer_cast<Neuropixels1>(device)->settings[0].get();

	for (int i = 0; i < settings->electrodeMetadata.size(); i++)
	{
		if (settings->electrodeMetadata[i].isSelected)
		{
			electrodeIndices.emplace_back(i);
		}
	}

	return electrodeIndices;
}

int NeuropixelsV1Interface::getIndexOfComboBoxItem(ComboBox* cb, std::string item)
{
	for (int i = 0; i < cb->getNumItems(); i++)
	{
		if (item == cb->getItemText(i))
			return i;
	}

	return -1;
}

void NeuropixelsV1Interface::selectElectrodes(std::vector<int> electrodes)
{
	std::static_pointer_cast<Neuropixels1>(device)->settings[0]->selectElectrodes(electrodes);

	repaint();
}

void NeuropixelsV1Interface::setInterfaceEnabledState(bool enabledState)
{
	if (deviceEnableButton != nullptr)
		deviceEnableButton->setEnabled(enabledState);

	if (selectElectrodeButton != nullptr)
		selectElectrodeButton->setEnabled(enabledState);

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

	if (saveJsonButton != nullptr)
		saveJsonButton->setEnabled(enabledState);

	if (loadJsonButton != nullptr)
		loadJsonButton->setEnabled(enabledState);

	if (adcCalibrationFileButton != nullptr)
		adcCalibrationFileButton->setEnabled(enabledState);

	if (gainCalibrationFileButton != nullptr)
		gainCalibrationFileButton->setEnabled(enabledState);
}

void NeuropixelsV1Interface::startAcquisition()
{
	acquisitionIsActive = true;

	setInterfaceEnabledState(false);

	if (mode == SettingsInterface::VisualizationMode::ACTIVITY_VIEW)
		probeBrowser->startTimer(100);
}

void NeuropixelsV1Interface::stopAcquisition()
{
	acquisitionIsActive = false;

	setInterfaceEnabledState(true);
}

void NeuropixelsV1Interface::drawLegend()
{
	enableViewComponent->setVisible(false);
	apGainViewComponent->setVisible(false);
	lfpGainViewComponent->setVisible(false);
	referenceViewComponent->setVisible(false);
	activityViewComponent->setVisible(false);

	switch (mode)
	{
	case SettingsInterface::VisualizationMode::ENABLE_VIEW:
		enableViewComponent->setVisible(true);
		break;
	case SettingsInterface::VisualizationMode::AP_GAIN_VIEW:
		apGainViewComponent->setVisible(true);
		break;
	case SettingsInterface::VisualizationMode::LFP_GAIN_VIEW:
		lfpGainViewComponent->setVisible(true);
		break;
	case SettingsInterface::VisualizationMode::REFERENCE_VIEW:
		referenceViewComponent->setVisible(true);
		break;
	case SettingsInterface::VisualizationMode::ACTIVITY_VIEW:
		activityViewComponent->setVisible(true);
		break;
	default:
		break;
	}
}

bool NeuropixelsV1Interface::applyProbeSettings(ProbeSettings<Neuropixels1::numberOfChannels, Neuropixels1::numberOfElectrodes>* p)
{
	auto npx = std::static_pointer_cast<Neuropixels1>(device);

	if (electrodeConfigurationComboBox != 0)
		electrodeConfigurationComboBox->setSelectedId(p->electrodeConfigurationIndex + 2, dontSendNotification);

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

	auto settings = std::static_pointer_cast<Neuropixels1>(device)->settings[0].get();

	for (int i = 0; i < settings->electrodeMetadata.size(); i++)
	{
		if (settings->electrodeMetadata[i].status == ElectrodeStatus::CONNECTED)
			settings->electrodeMetadata[i].status = ElectrodeStatus::DISCONNECTED;
	}

	for (auto electrode : p->selectedElectrode)
	{
		settings->electrodeMetadata[electrode].status = ElectrodeStatus::CONNECTED;
	}

	settings->selectedBank = p->selectedBank;
	settings->selectedElectrode = p->selectedElectrode;
	settings->selectedShank = p->selectedShank;


	drawLegend();
	repaint();

	return true;
}

void NeuropixelsV1Interface::saveParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1E)
	{
		LOGD("Saving Neuropixels 1.0e settings.");
	}
	else if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F)
	{
		LOGD("Saving Neuropixels 1.0f settings.");
	}

	auto npx = std::static_pointer_cast<Neuropixels1>(device);
	auto settings = npx->settings[0].get();

	std::string deviceName = device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F ? "NEUROPIXELSV1F" : "NEUROPIXELSV1E";
	XmlElement* xmlNode = xml->createNewChildElement(deviceName);

	xmlNode->setAttribute("name", npx->getName());
	xmlNode->setAttribute("idx", (int)npx->getDeviceIdx());
	xmlNode->setAttribute("probeSerialNumber", std::to_string(npx->getProbeSerialNumber()));

	xmlNode->setAttribute("isEnabled", bool(device->isEnabled()));

	xmlNode->setAttribute("adcCalibrationFile", npx->getAdcCalibrationFilePath());
	xmlNode->setAttribute("gainCalibrationFile", npx->getGainCalibrationFilePath());

	XmlElement* probeViewerNode = xmlNode->createNewChildElement("PROBE_VIEWER");

	probeViewerNode->setAttribute("zoomHeight", probeBrowser->getZoomHeight());
	probeViewerNode->setAttribute("zoomOffset", probeBrowser->getZoomOffset());

	probeViewerNode->setAttribute("apGain", apGainComboBox->getText());
	probeViewerNode->setAttribute("lfpGain", lfpGainComboBox->getText());
	probeViewerNode->setAttribute("referenceChannel", referenceComboBox->getText());
	probeViewerNode->setAttribute("apFilter", filterComboBox->getText());

	XmlElement* channelsNode = xmlNode->createNewChildElement("SELECTED_CHANNELS");

	for (int i = 0; i < settings->selectedElectrode.size(); i++)
	{
		int globalIndex = settings->selectedElectrode[i];

		channelsNode->setAttribute("CH" + String(i), std::to_string(globalIndex));
	}
}

void NeuropixelsV1Interface::loadParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1E)
	{
		LOGD("Loading Neuropixels 1.0e settings.");
	}
	else if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F)
	{
		LOGD("Loading Neuropixels 1.0f settings.");
	}

	auto npx = std::static_pointer_cast<Neuropixels1>(device);
	auto settings = npx->settings[0].get();

	XmlElement* xmlNode = nullptr;
	std::string deviceName = device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F ? "NEUROPIXELSV1F" : "NEUROPIXELSV1E";

	for (auto* node : xml->getChildIterator())
	{
		if (node->hasTagName(deviceName))
		{
			if (node->getStringAttribute("name").toStdString() == npx->getName() &&
				node->getIntAttribute("idx") == npx->getDeviceIdx())
			{
				xmlNode = node;
				break;
			}
		}
	}

	if (xmlNode == nullptr)
	{
		LOGD("No ", deviceName, " element found matching name = " + npx->getName() + ", and idx = " + std::to_string(npx->getDeviceIdx()));
		return;
	}

	if (npx->getProbeSerialNumber() != 0 && xmlNode->getIntAttribute("probeSerialNumber") != npx->getProbeSerialNumber())
	{
		LOGC("Different serial numbers found. Current serial number is " + std::to_string(npx->getProbeSerialNumber()) + ", while the saved serial number is " + std::to_string(xmlNode->getIntAttribute("probeSerialNumber")) + ". Updating settings...");
	}

	npx->setEnabled(xmlNode->getBoolAttribute("isEnabled"));

	npx->setAdcCalibrationFilePath(xmlNode->getStringAttribute("adcCalibrationFile").toStdString());
	npx->setGainCalibrationFilePath(xmlNode->getStringAttribute("gainCalibrationFile").toStdString());

	XmlElement* probeViewerNode = xmlNode->getChildByName("PROBE_VIEWER");

	if (probeViewerNode == nullptr)
	{
		LOGE("No PROBE_VIEWER element found.");
		return;
	}

	probeBrowser->setZoomHeightAndOffset(probeViewerNode->getIntAttribute("zoomHeight"), probeViewerNode->getIntAttribute("zoomOffset"));

	int idx = -1;

	idx = getIndexOfComboBoxItem(apGainComboBox.get(), probeViewerNode->getStringAttribute("apGain").toStdString());
	if (idx == -1)
	{
		LOGE("No AP gain variable found.");
	}
	else
		settings->apGainIndex = idx;

	idx = getIndexOfComboBoxItem(lfpGainComboBox.get(), probeViewerNode->getStringAttribute("lfpGain").toStdString());
	if (idx == -1)
	{
		LOGE("No LFP gain variable found.");
	}
	else
		settings->lfpGainIndex = idx;

	idx = getIndexOfComboBoxItem(referenceComboBox.get(), probeViewerNode->getStringAttribute("referenceChannel").toStdString());
	if (idx == -1)
	{
		LOGE("No reference channel variable found.");
	}
	else
		settings->referenceIndex = idx;

	idx = getIndexOfComboBoxItem(filterComboBox.get(), probeViewerNode->getStringAttribute("apFilter").toStdString());
	if (idx == -1)
	{
		LOGE("No AP filter variable found.");
	}
	else
		settings->apFilterState = idx == 1 ? true : false;

	XmlElement* channelsNode = xmlNode->getChildByName("SELECTED_CHANNELS");

	if (channelsNode == nullptr)
	{
		LOGE("No SELECTED_CHANNELS element found.");
		return;
	}

	std::vector<int> selectedChannels{};
	selectedChannels.reserve(npx->numberOfChannels);

	for (int i = 0; i < npx->numberOfChannels; i++)
	{
		std::string chIdx = channelsNode->getStringAttribute("CH" + std::to_string(i), "").toStdString();

		if (chIdx == "")
		{
			LOGE("Channel #" + std::to_string(i) + " not found. Channel map will not be updated.");
			return;
		}

		selectedChannels.emplace_back(std::stoi(chIdx));
	}

	std::sort(selectedChannels.begin(), selectedChannels.end());
	auto last = std::unique(selectedChannels.begin(), selectedChannels.end());
	selectedChannels.erase(last, selectedChannels.end());

	if (selectedChannels.size() != npx->numberOfChannels)
	{
		LOGE("Invalid channel map. Wrong number of channels found. Channel map was not updated");
		return;
	}

	selectElectrodes(selectedChannels);

	updateSettings();
}

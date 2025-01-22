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

NeuropixV1Interface::NeuropixV1Interface(OnixDevice* d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c),
	device((Neuropixels_1*)d),
	neuropix_info("INFO")
{
	ColourScheme::setColourScheme(ColourSchemeId::PLASMA);

	if (device != nullptr)
	{
		type = SettingsInterface::PROBE_SETTINGS_INTERFACE;

		mode = VisualizationMode::ENABLE_VIEW;

		probeBrowser = std::make_unique<ProbeBrowser>(this);
		probeBrowser->setBounds(0, 0, 800, 600);
		addAndMakeVisible(probeBrowser.get());

		int currentHeight = 55;

		probeEnableButton = std::make_unique<UtilityButton>("ENABLED");
		probeEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		probeEnableButton->setRadius(3.0f);
		probeEnableButton->setBounds(630, currentHeight + 25, 100, 22);
		probeEnableButton->setClickingTogglesState(true);
		probeEnableButton->setToggleState(device->isEnabled(), dontSendNotification);
		probeEnableButton->setTooltip("If disabled, probe will not stream data during acquisition");
		probeEnableButton->addListener(this);
		addAndMakeVisible(probeEnableButton.get());

		electrodesLabel = std::make_unique<Label>("ELECTRODES", "ELECTRODES");
		electrodesLabel->setFont(FontOptions("Inter", "Regular", 13.0f));
		electrodesLabel->setBounds(446, currentHeight - 20, 100, 20);
		addAndMakeVisible(electrodesLabel.get());

		enableViewButton = std::make_unique<UtilityButton>("VIEW");
		enableViewButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		enableViewButton->setRadius(3.0f);
		enableViewButton->setBounds(530, currentHeight + 2, 45, 18);
		enableViewButton->addListener(this);
		enableViewButton->setTooltip("View electrode enabled state");
		addAndMakeVisible(enableViewButton.get());

		enableButton = std::make_unique<UtilityButton>("ENABLE");
		enableButton->setFont(FontOptions("Fira Code", "Regular", 13.0f));
		enableButton->setRadius(3.0f);
		enableButton->setBounds(450, currentHeight, 65, 22);
		enableButton->addListener(this);
		enableButton->setTooltip("Enable selected electrodes");
		addAndMakeVisible(enableButton.get());

		currentHeight += 58;

		electrodePresetLabel = std::make_unique<Label>("ELECTRODE PRESET", "ELECTRODE PRESET");
		electrodePresetLabel->setFont(FontOptions("Fira Code", "Regular", 13.0f));
		electrodePresetLabel->setBounds(446, currentHeight - 20, 150, 20);
		addAndMakeVisible(electrodePresetLabel.get());

		electrodeConfigurationComboBox = std::make_unique<ComboBox>("electrodeConfigurationComboBox");
		electrodeConfigurationComboBox->setBounds(450, currentHeight, 135, 22);
		electrodeConfigurationComboBox->addListener(this);
		electrodeConfigurationComboBox->setTooltip("Enable a pre-configured set of electrodes");

		electrodeConfigurationComboBox->addItem("Select a preset...", 1);
		electrodeConfigurationComboBox->setItemEnabled(1, false);
		electrodeConfigurationComboBox->addSeparator();

		for (int i = 0; i < device->settings.availableElectrodeConfigurations.size(); i++)
		{
			electrodeConfigurationComboBox->addItem(device->settings.availableElectrodeConfigurations[i], i + 2);
		}

		electrodeConfigurationComboBox->setSelectedId(1, dontSendNotification);

		addAndMakeVisible(electrodeConfigurationComboBox.get());

		currentHeight += 55;

		if (device->settings.availableApGains.size() > 0)
		{
			apGainComboBox = std::make_unique<ComboBox>("apGainComboBox");
			apGainComboBox->setBounds(450, currentHeight, 65, 22);
			apGainComboBox->addListener(this);

			for (int i = 0; i < device->settings.availableApGains.size(); i++)
				apGainComboBox->addItem(String(device->settings.availableApGains[i]) + "x", i + 1);

			apGainComboBox->setSelectedId(device->settings.apGainIndex + 1, dontSendNotification);
			addAndMakeVisible(apGainComboBox.get());

			apGainViewButton = std::make_unique<UtilityButton>("VIEW");
			apGainViewButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
			apGainViewButton->setRadius(3.0f);
			apGainViewButton->setBounds(530, currentHeight + 2, 45, 18);
			apGainViewButton->addListener(this);
			apGainViewButton->setTooltip("View AP gain of each channel");
			addAndMakeVisible(apGainViewButton.get());

			apGainLabel = std::make_unique<Label>("AP GAIN", "AP GAIN");
			apGainLabel->setFont(FontOptions("Fira Code", "Regular", 13.0f));
			apGainLabel->setBounds(446, currentHeight - 20, 100, 20);
			addAndMakeVisible(apGainLabel.get());

			currentHeight += 55;
		}

		if (device->settings.availableLfpGains.size() > 0)
		{
			lfpGainComboBox = std::make_unique<ComboBox>("lfpGainComboBox");
			lfpGainComboBox->setBounds(450, currentHeight, 65, 22);
			lfpGainComboBox->addListener(this);

			for (int i = 0; i < device->settings.availableLfpGains.size(); i++)
				lfpGainComboBox->addItem(String(device->settings.availableLfpGains[i]) + "x", i + 1);

			lfpGainComboBox->setSelectedId(device->settings.lfpGainIndex + 1, dontSendNotification);
			addAndMakeVisible(lfpGainComboBox.get());

			lfpGainViewButton = std::make_unique<UtilityButton>("VIEW");
			lfpGainViewButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
			lfpGainViewButton->setRadius(3.0f);
			lfpGainViewButton->setBounds(530, currentHeight + 2, 45, 18);
			lfpGainViewButton->addListener(this);
			lfpGainViewButton->setTooltip("View LFP gain of each channel");
			addAndMakeVisible(lfpGainViewButton.get());

			lfpGainLabel = std::make_unique<Label>("LFP GAIN", "LFP GAIN");
			lfpGainLabel->setFont(FontOptions("Fira Code", "Regular", 13.0f));
			lfpGainLabel->setBounds(446, currentHeight - 20, 100, 20);
			addAndMakeVisible(lfpGainLabel.get());

			currentHeight += 55;
		}

		if (device->settings.availableReferences.size() > 0)
		{
			referenceComboBox = std::make_unique<ComboBox>("ReferenceComboBox");
			referenceComboBox->setBounds(450, currentHeight, 65, 22);
			referenceComboBox->addListener(this);

			for (int i = 0; i < device->settings.availableReferences.size(); i++)
			{
				referenceComboBox->addItem(device->settings.availableReferences[i], i + 1);
			}

			referenceComboBox->setSelectedId(device->settings.referenceIndex + 1, dontSendNotification);
			addAndMakeVisible(referenceComboBox.get());

			referenceViewButton = std::make_unique<UtilityButton>("VIEW");
			referenceViewButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
			referenceViewButton->setRadius(3.0f);
			referenceViewButton->setBounds(530, currentHeight + 2, 45, 18);
			referenceViewButton->addListener(this);
			referenceViewButton->setTooltip("View reference of each channel");
			addAndMakeVisible(referenceViewButton.get());

			referenceLabel = std::make_unique<Label>("REFERENCE", "REFERENCE");
			referenceLabel->setFont(FontOptions("Fira Code", "Regular", 13.0f));
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
		filterLabel->setFont(FontOptions("Fira Code", "Regular", 13.0f));
		filterLabel->setBounds(446, currentHeight - 20, 200, 20);
		addAndMakeVisible(filterLabel.get());

		currentHeight += 55;

		activityViewButton = std::make_unique<UtilityButton>("VIEW");
		activityViewButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		activityViewButton->setRadius(3.0f);

		activityViewButton->addListener(this);
		activityViewButton->setTooltip("View peak-to-peak amplitudes for each channel");
		addAndMakeVisible(activityViewButton.get());

		activityViewComboBox = std::make_unique<ComboBox>("ActivityView Combo Box");

		if (device->settings.availableLfpGains.size() > 0)
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
		activityViewLabel->setFont(FontOptions("Fira Code", "Regular", 13.0f));
		activityViewLabel->setBounds(446, currentHeight - 20, 180, 20);
		addAndMakeVisible(activityViewLabel.get());

		currentHeight += 55;

		// COPY / PASTE / UPLOAD
		copyButton = std::make_unique<UtilityButton>("COPY");
		copyButton->setRadius(3.0f);
		copyButton->setBounds(45, 637, 60, 22);
		copyButton->addListener(this);
		copyButton->setTooltip("Copy probe settings");
		addAndMakeVisible(copyButton.get());

		pasteButton = std::make_unique<UtilityButton>("PASTE");
		pasteButton->setRadius(3.0f);
		pasteButton->setBounds(115, 637, 60, 22);
		pasteButton->addListener(this);
		pasteButton->setTooltip("Paste probe settings");
		addAndMakeVisible(pasteButton.get());

		saveJsonButton = std::make_unique<UtilityButton>("SAVE TO JSON");
		saveJsonButton->setRadius(3.0f);
		saveJsonButton->setBounds(45, 707, 120, 22);
		saveJsonButton->addListener(this);
		saveJsonButton->setTooltip("Save channel map to probeinterface .json file");
		addAndMakeVisible(saveJsonButton.get());

		loadJsonButton = std::make_unique<UtilityButton>("LOAD FROM JSON");
		loadJsonButton->setRadius(3.0f);
		loadJsonButton->setBounds(175, 707, 130, 22);
		loadJsonButton->addListener(this);
		loadJsonButton->setTooltip("Load channel map from probeinterface .json file");
		addAndMakeVisible(loadJsonButton.get());

		loadImroComboBox = std::make_unique<ComboBox>("Quick-load IMRO");
		loadImroComboBox->setBounds(175, 707, 130, 22);
		loadImroComboBox->addListener(this);
		loadImroComboBox->setTooltip("Load a favorite IMRO setting.");

		probeSettingsLabel = std::make_unique<Label>("Settings", "Probe settings:");
		probeSettingsLabel->setFont(FontOptions("Fira Code", "Regular", 13.0f));
		probeSettingsLabel->setBounds(40, 610, 300, 20);
		addAndMakeVisible(probeSettingsLabel.get());
	}
	else
	{
		type = SettingsInterface::UNKNOWN_SETTINGS_INTERFACE;
	}

	int verticalOffset = 550;

	if (device == nullptr)
		verticalOffset = 250;

	// PROBE INFO
	nameLabel = std::make_unique<Label>("MAIN", "NAME");
	nameLabel->setFont(FontOptions("Fira Code", "Medium", 30.0f));
	nameLabel->setBounds(625, 40, 500, 45);
	addAndMakeVisible(nameLabel.get());

	infoLabelView = std::make_unique<Viewport>("INFO");
	infoLabelView->setBounds(625, 110, 750, 400);

	addAndMakeVisible(infoLabelView.get());
	infoLabelView->toBack();

	infoLabel = std::make_unique<Label>("INFO", "INFO");
	infoLabelView->setViewedComponent(infoLabel.get(), false);
	infoLabel->setFont(FontOptions(15.0f));
	infoLabel = std::make_unique<Label>("INFO", "INFO");
	infoLabelView->setViewedComponent(infoLabel.get(), false);
	infoLabel->setFont(FontOptions(15.0f));
	infoLabel->setBounds(0, 0, 750, 350);
	infoLabel->setJustificationType(Justification::topLeft);

	updateInfoString();
}

NeuropixV1Interface::~NeuropixV1Interface()
{
}

void NeuropixV1Interface::updateInfoString()
{
	String nameString, infoString;

	nameString = "Headstage: ";

	if (device != nullptr)
	{
		nameString += "NeuropixelsV1e";

		infoString = "Device: Neuropixels V1e";
		infoString += "\n";
		infoString += "\n";

		infoString += "Probe Number: ";
		infoString += device->getProbeNumber();
		infoString += "\n";
		infoString += "\n";
	}

	infoLabel->setText(infoString, dontSendNotification);
	nameLabel->setText(nameString, dontSendNotification);
}

void NeuropixV1Interface::updateProbeSettings()
{
	ProbeSettings settings = getProbeSettings();

	device->updateSettings(settings);
}

void NeuropixV1Interface::comboBoxChanged(ComboBox* comboBox)
{
	if (!editor->acquisitionIsActive)
	{
		if (comboBox == electrodeConfigurationComboBox.get())
		{
			String preset = electrodeConfigurationComboBox->getText();

			Array<int> selection = device->selectElectrodeConfiguration(preset);

			selectElectrodes(selection);
		}
		else if ((comboBox == apGainComboBox.get()))
		{
			device->settings.apGainIndex = apGainComboBox->getSelectedItemIndex();
		}
		else if (comboBox == lfpGainComboBox.get())
		{
			device->settings.lfpGainIndex = lfpGainComboBox->getSelectedItemIndex();
		}
		else if (comboBox == referenceComboBox.get())
		{
			device->settings.referenceIndex = referenceComboBox->getSelectedItemIndex();
		}
		else if (comboBox == filterComboBox.get())
		{
			device->settings.apFilterState = filterComboBox->getSelectedId() == 1;
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

void NeuropixV1Interface::setAnnotationLabel(String s, Colour c)
{
	annotationLabel->setText(s, NotificationType::dontSendNotification);
	annotationLabel->setColour(Label::textColourId, c);
}

void NeuropixV1Interface::buttonClicked(Button* button)
{
	if (button == probeEnableButton.get())
	{
		device->setEnabled(probeEnableButton->getToggleState());

		if (device->isEnabled())
		{
			probeEnableButton->setLabel("ENABLED");
		}
		else
		{
			probeEnableButton->setLabel("DISABLED");
		}

		CoreServices::updateSignalChain(editor);
	}
	else if (button == enableViewButton.get())
	{
		mode = ENABLE_VIEW;
		probeBrowser->stopTimer();
		repaint();
	}
	else if (button == apGainViewButton.get())
	{
		mode = AP_GAIN_VIEW;
		probeBrowser->stopTimer();
		repaint();
	}
	else if (button == lfpGainViewButton.get())
	{
		mode = LFP_GAIN_VIEW;
		probeBrowser->stopTimer();
		repaint();
	}
	else if (button == referenceViewButton.get())
	{
		mode = REFERENCE_VIEW;
		probeBrowser->stopTimer();
		repaint();
	}
	else if (button == activityViewButton.get())
	{
		mode = ACTIVITY_VIEW;

		if (acquisitionIsActive)
			probeBrowser->startTimer(100);

		repaint();
	}
	else if (button == enableButton.get())
	{
		Array<int> selection = getSelectedElectrodes();

		if (selection.size() > 0)
		{
			electrodeConfigurationComboBox->setSelectedId(1);
			selectElectrodes(selection);
		}
	}
	else if (button == loadJsonButton.get())
	{
		FileChooser fileChooser("Select an probeinterface JSON file to load.", File(), "*.json");

		if (fileChooser.browseForFileToOpen())
		{
			ProbeSettings settings = getProbeSettings();

			bool success = ProbeInterfaceJson::readProbeSettingsFromJson(fileChooser.getResult(), settings);

			if (success)
			{
				applyProbeSettings(settings);
			}
		}
	}
	else if (button == saveJsonButton.get())
	{
		FileChooser fileChooser("Save channel map to a probeinterface JSON file.", File(), "*.json");

		if (fileChooser.browseForFileToSave(true))
		{
			bool success = ProbeInterfaceJson::writeProbeSettingsToJson(fileChooser.getResult(), getProbeSettings());

			if (!success)
				CoreServices::sendStatusMessage("Failed to write probe channel map.");
			else
				CoreServices::sendStatusMessage("Successfully wrote probe channel map.");
		}
	}
}

Array<int> NeuropixV1Interface::getSelectedElectrodes() const
{
	Array<int> electrodeIndices;

	for (int i = 0; i < device->settings.electrodeMetadata.size(); i++)
	{
		if (device->settings.electrodeMetadata[i].isSelected)
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
	// update selection state
	for (int i = 0; i < electrodes.size(); i++)
	{
		Bank bank = device->settings.electrodeMetadata[electrodes[i]].bank;
		int channel = device->settings.electrodeMetadata[electrodes[i]].channel;
		int shank = device->settings.electrodeMetadata[electrodes[i]].shank;
		int global_index = device->settings.electrodeMetadata[electrodes[i]].global_index;

		for (int j = 0; j < device->settings.electrodeMetadata.size(); j++)
		{
			if (device->settings.electrodeMetadata[j].channel == channel)
			{
				if (device->settings.electrodeMetadata[j].bank == bank && device->settings.electrodeMetadata[j].shank == shank)
				{
					device->settings.electrodeMetadata.getReference(j).status = ElectrodeStatus::CONNECTED;
				}

				else
				{
					device->settings.electrodeMetadata.getReference(j).status = ElectrodeStatus::DISCONNECTED;
				}
			}
		}
	}

	repaint();

	CoreServices::updateSignalChain(editor);
}

void NeuropixV1Interface::startAcquisition()
{
	bool enabledState = false;
	acquisitionIsActive = true;

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

	if (copyButton != nullptr)
		copyButton->setEnabled(enabledState);

	if (pasteButton != nullptr)
		pasteButton->setEnabled(enabledState);

	if (loadImroButton != nullptr)
		loadImroButton->setEnabled(enabledState);

	if (loadJsonButton != nullptr)
		loadJsonButton->setEnabled(enabledState);

	if (mode == ACTIVITY_VIEW)
		probeBrowser->startTimer(100);
}

void NeuropixV1Interface::stopAcquisition()
{
	bool enabledState = true;
	acquisitionIsActive = false;

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

	if (copyButton != nullptr)
		copyButton->setEnabled(enabledState);

	if (pasteButton != nullptr)
		pasteButton->setEnabled(enabledState);

	if (loadImroButton != nullptr)
		loadImroButton->setEnabled(enabledState);

	if (loadJsonButton != nullptr)
		loadJsonButton->setEnabled(enabledState);
}

void NeuropixV1Interface::paint(Graphics& g)
{
	if (device != nullptr)
	{
		drawLegend(g);

		g.setColour(Colour(60, 60, 60));
		g.fillRoundedRectangle(30, 600, 290, 145, 8.0f);
	}
}

void NeuropixV1Interface::drawLegend(Graphics& g)
{
	g.setColour(Colour(55, 55, 55));
	g.setFont(15);

	int xOffset = 450;
	int yOffset = 440;

	switch (mode)
	{
	case ENABLE_VIEW:
		g.drawMultiLineText("ENABLED?", xOffset, yOffset, 200);
		g.drawMultiLineText("YES", xOffset + 30, yOffset + 22, 200);
		g.drawMultiLineText("NO", xOffset + 30, yOffset + 42, 200);

		g.drawMultiLineText("REFERENCE", xOffset + 30, yOffset + 62, 200);

		g.setColour(Colours::yellow);
		g.fillRect(xOffset + 10, yOffset + 10, 15, 15);

		g.setColour(Colours::darkgrey);
		g.fillRect(xOffset + 10, yOffset + 30, 15, 15);

		g.setColour(Colours::black);

		g.fillRect(xOffset + 10, yOffset + 50, 15, 15);

		break;

	case AP_GAIN_VIEW:
		g.drawMultiLineText("AP GAIN", xOffset, yOffset, 200);

		for (int i = 0; i < 8; i++)
		{
			g.drawMultiLineText(apGainComboBox->getItemText(i), xOffset + 30, yOffset + 22 + 20 * i, 200);
		}

		for (int i = 0; i < 8; i++)
		{
			g.setColour(Colour(25 * i, 25 * i, 50));
			g.fillRect(xOffset + 10, yOffset + 10 + 20 * i, 15, 15);
		}

		break;

	case LFP_GAIN_VIEW:
		g.drawMultiLineText("LFP GAIN", xOffset, yOffset, 200);

		for (int i = 0; i < 8; i++)
		{
			g.drawMultiLineText(lfpGainComboBox->getItemText(i), xOffset + 30, yOffset + 22 + 20 * i, 200);
		}

		for (int i = 0; i < 8; i++)
		{
			g.setColour(Colour(66, 25 * i, 35 * i));
			g.fillRect(xOffset + 10, yOffset + 10 + 20 * i, 15, 15);
		}

		break;

	case REFERENCE_VIEW:
		g.drawMultiLineText("REFERENCE", xOffset, yOffset, 200);

		for (int i = 0; i < referenceComboBox->getNumItems(); i++)
		{
			g.drawMultiLineText(referenceComboBox->getItemText(i), xOffset + 30, yOffset + 22 + 20 * i, 200);
		}

		for (int i = 0; i < referenceComboBox->getNumItems(); i++)
		{
			String referenceDescription = referenceComboBox->getItemText(i);

			if (referenceDescription.contains("Ext"))
				g.setColour(Colours::pink);
			else if (referenceDescription.contains("Tip"))
				g.setColour(Colours::orange);
			else
				g.setColour(Colours::purple);

			g.fillRect(xOffset + 10, yOffset + 10 + 20 * i, 15, 15);
		}

		break;

	case ACTIVITY_VIEW:
		g.drawMultiLineText("AMPLITUDE", xOffset, yOffset, 200);

		for (int i = 0; i < 6; i++)
		{
			g.drawMultiLineText(String(float(probeBrowser->maxPeakToPeakAmplitude) / 5.0f * float(i)) + " uV", xOffset + 30, yOffset + 22 + 20 * i, 200);
		}

		for (int i = 0; i < 6; i++)
		{
			g.setColour(ColourScheme::getColourForNormalizedValue(float(i) / 5.0f));
			g.fillRect(xOffset + 10, yOffset + 10 + 20 * i, 15, 15);
		}

		break;
	}
}

bool NeuropixV1Interface::applyProbeSettings(ProbeSettings p, bool shouldUpdateProbe)
{
	if (electrodeConfigurationComboBox != 0)
		electrodeConfigurationComboBox->setSelectedId(p.electrodeConfigurationIndex + 2, dontSendNotification);

	// update display
	if (apGainComboBox != 0)
		apGainComboBox->setSelectedId(p.apGainIndex + 1, dontSendNotification);

	if (lfpGainComboBox != 0)
		lfpGainComboBox->setSelectedId(p.lfpGainIndex + 1, dontSendNotification);

	if (filterComboBox != 0)
	{
		if (p.apFilterState)
			filterComboBox->setSelectedId(1, dontSendNotification);
		else
			filterComboBox->setSelectedId(2, dontSendNotification);
	}

	if (referenceComboBox != 0)
		referenceComboBox->setSelectedId(p.referenceIndex + 1, dontSendNotification);

	for (int i = 0; i < device->settings.electrodeMetadata.size(); i++)
	{
		if (device->settings.electrodeMetadata[i].status == ElectrodeStatus::CONNECTED)
			device->settings.electrodeMetadata.getReference(i).status = ElectrodeStatus::DISCONNECTED;
	}

	// update selection state
	for (int i = 0; i < p.selectedChannel.size(); i++)
	{
		Bank bank = p.selectedBank[i];
		int channel = p.selectedChannel[i];
		int shank = p.selectedShank[i];

		for (int j = 0; j < device->settings.electrodeMetadata.size(); j++)
		{
			if (device->settings.electrodeMetadata[j].channel == channel && device->settings.electrodeMetadata[j].bank == bank && device->settings.electrodeMetadata[j].shank == shank)
			{
				device->settings.electrodeMetadata.getReference(j).status = ElectrodeStatus::CONNECTED;
			}
		}
	}

	if (shouldUpdateProbe)
	{
		updateProbeSettings();
		CoreServices::saveRecoveryConfig();
	}

	repaint();

	return true;
}

ProbeSettings NeuropixV1Interface::getProbeSettings() const
{
	return device->settings;
}

void NeuropixV1Interface::saveParameters(XmlElement* xml)
{
	if (device != nullptr)
	{
		LOGD("Saving Neuropix display.");

		XmlElement* xmlNode = xml->createNewChildElement("NP_PROBE");

		xmlNode->setAttribute("probe_serial_number", String(device->getProbeNumber()));
		xmlNode->setAttribute("probe_name", device->getName());
		xmlNode->setAttribute("num_adcs", device->settings.probeMetadata.num_adcs);

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

		ProbeSettings p = getProbeSettings();

		for (int i = 0; i < p.selectedChannel.size(); i++)
		{
			int bank = int(p.selectedBank[i]);
			int shank = p.selectedShank[i];
			int channel = p.selectedChannel[i];
			int elec = p.selectedElectrode[i];

			String chString = String(bank);

			String chId = "CH" + String(channel);

			channelNode->setAttribute(chId, chString);
			xposNode->setAttribute(chId, String(device->settings.electrodeMetadata[elec].xpos + 250 * shank));
			yposNode->setAttribute(chId, String(device->settings.electrodeMetadata[elec].ypos));
		}

		if (imroFiles.size() > 0)
		{
			XmlElement* imroFilesNode = xmlNode->createNewChildElement("IMRO_FILES");

			for (int i = 0; i < imroFiles.size(); i++)
			{
				if (!imroLoadedFromFolder[i])
				{
					XmlElement* imroFileNode = imroFilesNode->createNewChildElement("FILE");
					imroFileNode->setAttribute("PATH", imroFiles[i]);
				}
			}
		}

		xmlNode->setAttribute("visualizationMode", mode);
		xmlNode->setAttribute("activityToView", probeBrowser->activityToView);

		// annotations
		for (int i = 0; i < annotations.size(); i++)
		{
			Annotation& a = annotations.getReference(i);
			XmlElement* annotationNode = xmlNode->createNewChildElement("ANNOTATIONS");
			annotationNode->setAttribute("text", a.text);
			annotationNode->setAttribute("channel", a.electrodes[0]);
			annotationNode->setAttribute("R", a.colour.getRed());
			annotationNode->setAttribute("G", a.colour.getGreen());
			annotationNode->setAttribute("B", a.colour.getBlue());
		}

		xmlNode->setAttribute("isEnabled", bool(device->isEnabled()));
	}
}

void NeuropixV1Interface::loadParameters(XmlElement* xml)
{
	if (device != nullptr)
	{
		String mySerialNumber = String(device->getProbeNumber());

		// first, set defaults
		ProbeSettings settings = ProbeSettings(device->settings);
		settings.probeType = device->settings.probeType;
		settings.apFilterState = device->settings.apFilterState;
		settings.lfpGainIndex = device->settings.lfpGainIndex;
		settings.apGainIndex = device->settings.apGainIndex;
		settings.referenceIndex = device->settings.referenceIndex;
		if (settings.referenceIndex >= referenceComboBox->getNumItems())
			settings.referenceIndex = 0;
		settings.availableApGains = device->settings.availableApGains;
		settings.availableLfpGains = device->settings.availableLfpGains;
		settings.availableBanks = device->settings.availableBanks;
		settings.availableReferences = device->settings.availableReferences;

		for (int shank = 0; shank < 4; shank++)
		{
			for (int i = 0; i < 384; i++)
			{
				settings.selectedBank.add(Bank::A);
				settings.selectedChannel.add(i);
				settings.selectedShank.add(shank);
				settings.selectedElectrode.add(i + shank * 1280);
			}
		}

		XmlElement* matchingNode = nullptr;

		// find by serial number
		for (auto xmlNode : xml->getChildIterator())
		{
			if (xmlNode->hasTagName("NP_PROBE"))
			{
				if (xmlNode->getStringAttribute("probe_serial_number").equalsIgnoreCase(mySerialNumber))
				{
					matchingNode = xmlNode;
					break;
				}
			}
		}

		// if not, search for matching port
		if (matchingNode != nullptr)
		{
			if (matchingNode->getChildByName("CHANNELS"))
			{
				settings.selectedBank.clear();
				settings.selectedChannel.clear();
				settings.selectedShank.clear();
				settings.selectedElectrode.clear();

				XmlElement* status = matchingNode->getChildByName("CHANNELS");

				for (int i = 0; i < numberOfChannels; i++)
				{
					settings.selectedChannel.add(i);

					String bankInfo = status->getStringAttribute("CH" + String(i));
					Bank bank = static_cast<Bank> (bankInfo.substring(0, 1).getIntValue());
					int shank = 0;

					settings.selectedBank.add(bank);
					settings.selectedShank.add(shank);

					for (int j = 0; j < device->settings.electrodeMetadata.size(); j++)
					{
						if (device->settings.electrodeMetadata[j].channel == i)
						{
							if (device->settings.electrodeMetadata[j].bank == bank && device->settings.electrodeMetadata[j].shank == shank)
							{
								settings.selectedElectrode.add(j);
							}
						}
					}
				}
			}

			probeBrowser->setZoomHeightAndOffset(matchingNode->getIntAttribute("ZoomHeight"),
				matchingNode->getIntAttribute("ZoomOffset"));

			settings.apGainIndex = matchingNode->getIntAttribute("apGainIndex", 3);
			settings.lfpGainIndex = matchingNode->getIntAttribute("lfpGainIndex", 2);
			settings.referenceIndex = matchingNode->getIntAttribute("referenceChannelIndex", 0);
			if (settings.referenceIndex >= referenceComboBox->getNumItems())
				settings.referenceIndex = 0;

			String configurationName = matchingNode->getStringAttribute("electrodeConfigurationPreset", "NONE");

			for (int i = 0; i < electrodeConfigurationComboBox->getNumItems(); i++)
			{
				if (electrodeConfigurationComboBox->getItemText(i).equalsIgnoreCase(configurationName))
				{
					electrodeConfigurationComboBox->setSelectedItemIndex(i, dontSendNotification);
					settings.electrodeConfigurationIndex = i - 1;

					break;
				}
			}

			settings.apFilterState = matchingNode->getIntAttribute("filterCutIndex", 1) == 1;

			for (auto imroNode : matchingNode->getChildIterator())
			{
				if (imroNode->hasTagName("IMRO_FILES"))
				{
					for (auto fileNode : imroNode->getChildIterator())
					{
						if (imroFiles.size() == 0)
							loadImroComboBox->clear();

						imroFiles.add(fileNode->getStringAttribute("PATH"));
						imroLoadedFromFolder.add(false);
						loadImroComboBox->addItem(imroFiles.getLast(), imroFiles.size());
					}
				}
			}

			for (auto annotationNode : matchingNode->getChildIterator())
			{
				if (annotationNode->hasTagName("ANNOTATIONS"))
				{
					Array<int> annotationChannels;
					annotationChannels.add(annotationNode->getIntAttribute("electrode"));
					annotations.add(Annotation(annotationNode->getStringAttribute("text"),
						annotationChannels,
						Colour(annotationNode->getIntAttribute("R"),
							annotationNode->getIntAttribute("G"),
							annotationNode->getIntAttribute("B"))));
				}
			}

			device->setEnabled(matchingNode->getBoolAttribute("isEnabled", true));
			probeEnableButton->setToggleState(device->isEnabled(), dontSendNotification);
			if (device->isEnabled())
				probeEnableButton->setLabel("ENABLED");
			else
			{
				probeEnableButton->setLabel("DISABLED");
			}
		}

		device->updateSettings(settings);

		applyProbeSettings(settings, false);
	}
}

Annotation::Annotation(String t, Array<int> e, Colour c)
{
	text = t;
	electrodes = e;

	currentYLoc = -100.f;

	isMouseOver = false;
	isSelected = false;

	colour = c;
}

Annotation::~Annotation()
{
}

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

#include "NeuropixelsV2Interface.h"
#include "ProbeBrowser.h"

#include "../Formats/ProbeInterface.h"

NeuropixelsV2Interface::NeuropixelsV2Interface(std::shared_ptr<Neuropixels2e> d, int ind, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c),
	neuropix_info("INFO"),
	probeIndex(ind)
{
	ColourScheme::setColourScheme(ColourSchemeId::PLASMA);

	if (device != nullptr)
	{
		auto settings = std::static_pointer_cast<Neuropixels2e>(device)->settings[probeIndex].get();

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

		probeEnableButton = std::make_unique<UtilityButton>("ENABLED");
		probeEnableButton->setFont(fontRegularButton);
		probeEnableButton->setRadius(3.0f);
		probeEnableButton->setBounds(nameLabel->getX(), nameLabel->getBottom() + 3, 100, 22);
		probeEnableButton->setClickingTogglesState(true);
		probeEnableButton->setToggleState(device->isEnabled(), dontSendNotification);
		probeEnableButton->setTooltip("If disabled, probe will not stream data during acquisition");
		probeEnableButton->addListener(this);
		addAndMakeVisible(probeEnableButton.get());

		infoLabel = std::make_unique<Label>("INFO", "INFO");
		infoLabel->setFont(FontOptions(15.0f));
		infoLabel->setBounds(probeEnableButton->getX(), probeEnableButton->getBottom() + 10, nameLabel->getWidth(), 50);
		infoLabel->setJustificationType(Justification::topLeft);
		addAndMakeVisible(infoLabel.get());

		// TODO: Add the gain correction file labels / buttons / file choosers here
		saveJsonButton = std::make_unique<UtilityButton>("SAVE TO JSON");
		saveJsonButton->setRadius(3.0f);
		saveJsonButton->setBounds(infoLabel->getX(), infoLabel->getBottom() + 4, 120, 22);
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

		for (int i = 0; i < settings->availableElectrodeConfigurations.size(); i++)
		{
			electrodeConfigurationComboBox->addItem(settings->availableElectrodeConfigurations[i], i + 2);
		}

		checkForExistingChannelPreset();

		addAndMakeVisible(electrodeConfigurationComboBox.get());

		currentHeight += 55;

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

		currentHeight += 55;

		activityViewButton = std::make_unique<UtilityButton>("VIEW");
		activityViewButton->setFont(fontRegularButton);
		activityViewButton->setRadius(3.0f);

		activityViewButton->addListener(this);
		activityViewButton->setTooltip("View peak-to-peak amplitudes for each channel");
		addAndMakeVisible(activityViewButton.get());

		activityViewComboBox = std::make_unique<ComboBox>("ActivityView Combo Box");

		if (settings->availableLfpGains.size() > 0)
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

		for (int i = 0; i < colors.size(); i++)
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

		for (int i = 0; i < referenceComboBox->getNumItems(); i++)
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

		for (int i = 0; i < colors.size(); i++)
		{
			referenceViewRectangles.push_back(std::make_unique<DrawableRectangle>());
			referenceViewRectangles[i]->setFill(colors[i]);
			referenceViewRectangles[i]->setRectangle(Rectangle<float>(referenceViewLabels[0]->getX() + 6, referenceViewLabels[i]->getBottom() + 1, 12, 12));
			referenceViewComponent->addAndMakeVisible(referenceViewRectangles[i].get());

			referenceViewLabels.push_back(std::make_unique<Label>("referenceViewLabel", legendLabels[i]));
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

		for (int i = 0; i < 6; i++)
		{
			colors.push_back(ColourScheme::getColourForNormalizedValue(float(i) / 5.0f));
			legendLabels.add(String(float(probeBrowser->maxPeakToPeakAmplitude) / 5.0f * float(i)) + " uV");
		}

		for (int i = 0; i < colors.size(); i++)
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

void NeuropixelsV2Interface::updateInfoString()
{
	String nameString, infoString;

	nameString = "Headstage: ";

	auto npx = std::static_pointer_cast<Neuropixels2e>(device);

	if (device != nullptr)
	{
		nameString += "NeuropixelsV2e";

		infoString = "Device: Neuropixels V2 Probe";
		infoString += "\n";
		infoString += "\n";

		infoString += "Probe Number: ";
		infoString += npx->getProbeSerialNumber(probeIndex);
		infoString += "\n";
		infoString += "\n";
	}

	infoLabel->setText(infoString, dontSendNotification);
	nameLabel->setText(nameString, dontSendNotification);
}

void NeuropixelsV2Interface::comboBoxChanged(ComboBox* comboBox)
{
	if (!editor->acquisitionIsActive)
	{
		auto npx = std::static_pointer_cast<Neuropixels2e>(device);

		if (comboBox == electrodeConfigurationComboBox.get())
		{
			String preset = electrodeConfigurationComboBox->getText();

			Array<int> selection = npx->selectElectrodeConfiguration(preset);

			selectElectrodes(selection);
		}
		else if (comboBox == referenceComboBox.get())
		{
			npx->settings[probeIndex]->referenceIndex = referenceComboBox->getSelectedItemIndex();
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

void NeuropixelsV2Interface::checkForExistingChannelPreset()
{
	auto npx = std::static_pointer_cast<Neuropixels2e>(device);
	auto settings = npx->settings[probeIndex].get();

	settings->electrodeConfigurationIndex = -1;

	//for (int i = 0; i < settings->availableElectrodeConfigurations.size(); i++)
	//{
	//	auto selection = npx->selectElectrodeConfiguration(settings->availableElectrodeConfigurations[i]);

	//	if (selection == settings->selectedChannel)
	//	{
	//		settings->electrodeConfigurationIndex = i;
	//		break;
	//	}
	//}

	electrodeConfigurationComboBox->setSelectedId(settings->electrodeConfigurationIndex + 2, dontSendNotification);
}

void NeuropixelsV2Interface::buttonClicked(Button* button)
{
	auto npx = std::static_pointer_cast<Neuropixels2e>(device);

	if (button == probeEnableButton.get())
	{
		device->setEnabled(probeEnableButton->getToggleState());

		if (canvas->foundInputSource())
		{
			device->configureDevice();
			canvas->resetContext();
		}

		if (npx->isEnabled())
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
		mode = VisualizationMode::ENABLE_VIEW;
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
			auto npx = std::static_pointer_cast<Neuropixels2e>(device);

			//bool success = ProbeInterfaceJson::readProbeSettingsFromJson(fileChooser.getResult(), npx->settings[probeIndex].get());

			//if (success)
			//{
			//	applyProbeSettings(npx->settings[probeIndex].get());
			//}
		}
	}
	else if (button == saveJsonButton.get())
	{
		FileChooser fileChooser("Save channel map to a probeinterface JSON file.", File(), "*.json");

		if (fileChooser.browseForFileToSave(true))
		{
			auto npx = std::static_pointer_cast<Neuropixels2e>(device);

			//bool success = ProbeInterfaceJson::writeProbeSettingsToJson(fileChooser.getResult(), npx->settings[probeIndex].get());

			//if (!success)
			//	CoreServices::sendStatusMessage("Failed to write probe channel map.");
			//else
			//	CoreServices::sendStatusMessage("Successfully wrote probe channel map.");
		}
	}
}

Array<int> NeuropixelsV2Interface::getSelectedElectrodes() const
{
	Array<int> electrodeIndices;

	auto npx = std::static_pointer_cast<Neuropixels2e>(device);

	for (int i = 0; i < npx->settings[probeIndex]->electrodeMetadata.size(); i++)
	{
		if (npx->settings[probeIndex]->electrodeMetadata[i].isSelected)
		{
			electrodeIndices.add(i);
		}
	}

	return electrodeIndices;
}

void NeuropixelsV2Interface::setReference(int index)
{
	referenceComboBox->setSelectedId(index + 1, true);
}

void NeuropixelsV2Interface::selectElectrodes(Array<int> electrodes)
{
	auto settings = std::static_pointer_cast<Neuropixels2e>(device)->settings[probeIndex].get();

	// update selection state
	for (int i = 0; i < electrodes.size(); i++)
	{
		Bank bank = settings->electrodeMetadata[electrodes[i]].bank;
		int channel = settings->electrodeMetadata[electrodes[i]].channel;
		int shank = settings->electrodeMetadata[electrodes[i]].shank;
		int global_index = settings->electrodeMetadata[electrodes[i]].global_index;

		for (int j = 0; j < settings->electrodeMetadata.size(); j++)
		{
			if (settings->electrodeMetadata[j].channel == channel)
			{
				if (settings->electrodeMetadata[j].bank == bank && settings->electrodeMetadata[j].shank == shank)
				{
					settings->electrodeMetadata[j].status = ElectrodeStatus::CONNECTED;
				}

				else
				{
					settings->electrodeMetadata[j].status = ElectrodeStatus::DISCONNECTED;
				}
			}
		}

		settings->selectedBank[channel] = bank;
		settings->selectedShank[channel] = shank;
		settings->selectedElectrode[channel] = global_index;
	}

	repaint();
}

void NeuropixelsV2Interface::setInterfaceEnabledState(bool enabledState)
{
	if (probeEnableButton != nullptr)
		probeEnableButton->setEnabled(enabledState);

	if (enableButton != nullptr)
		enableButton->setEnabled(enabledState);

	if (electrodeConfigurationComboBox != nullptr)
		electrodeConfigurationComboBox->setEnabled(enabledState);

	if (referenceComboBox != nullptr)
		referenceComboBox->setEnabled(enabledState);

	if (loadJsonButton != nullptr)
		loadJsonButton->setEnabled(enabledState);
}

void NeuropixelsV2Interface::startAcquisition()
{
	acquisitionIsActive = true;

	setInterfaceEnabledState(false);

	if (mode == VisualizationMode::ACTIVITY_VIEW)
		probeBrowser->startTimer(100);
}

void NeuropixelsV2Interface::stopAcquisition()
{
	acquisitionIsActive = false;

	setInterfaceEnabledState(true);
}

void NeuropixelsV2Interface::drawLegend()
{
	enableViewComponent->setVisible(false);
	referenceViewComponent->setVisible(false);
	activityViewComponent->setVisible(false);

	switch (mode)
	{
	case VisualizationMode::ENABLE_VIEW:
		enableViewComponent->setVisible(true);
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

bool NeuropixelsV2Interface::applyProbeSettings(ProbeSettings<Neuropixels2e::numberOfChannels, Neuropixels2e::numberOfElectrodes>* p, bool shouldUpdateProbe)
{
	if (electrodeConfigurationComboBox != 0)
		electrodeConfigurationComboBox->setSelectedId(p->electrodeConfigurationIndex + 2, dontSendNotification);

	if (referenceComboBox != 0)
		referenceComboBox->setSelectedId(p->referenceIndex + 1, dontSendNotification);

	auto settings = std::static_pointer_cast<Neuropixels2e>(device)->settings[probeIndex].get();

	for (int i = 0; i < settings->electrodeMetadata.size(); i++)
	{
		if (settings->electrodeMetadata[i].status == ElectrodeStatus::CONNECTED)
			settings->electrodeMetadata[i].status = ElectrodeStatus::DISCONNECTED;
	}

	// update selection state
	/*for (int i = 0; i < p->selectedChannel.size(); i++)
	{
		Bank bank = p->selectedBank[i];
		int channel = p->selectedChannel[i];
		int shank = p->selectedShank[i];

		for (int j = 0; j < settings->electrodeMetadata.size(); j++)
		{
			if (settings->electrodeMetadata[j].channel == channel && settings->electrodeMetadata[j].bank == bank && settings->electrodeMetadata[j].shank == shank)
			{
				settings->electrodeMetadata.getReference(j).status = ElectrodeStatus::CONNECTED;
			}
		}
	}*/

	if (shouldUpdateProbe)
	{
		CoreServices::saveRecoveryConfig();
	}

	drawLegend();
	repaint();

	return true;
}

void NeuropixelsV2Interface::saveParameters(XmlElement* xml)
{
	if (device != nullptr)
	{
		auto npx = std::static_pointer_cast<Neuropixels2e>(device);
		auto settings = npx->settings[probeIndex].get();

		LOGD("Saving Neuropix display.");

		XmlElement* xmlNode = xml->createNewChildElement("NP_PROBE");

		xmlNode->setAttribute("probe_serial_number", String(npx->getProbeSerialNumber(probeIndex)));
		xmlNode->setAttribute("probe_name", npx->getName());
		xmlNode->setAttribute("num_adcs", settings->probeMetadata.num_adcs);

		xmlNode->setAttribute("ZoomHeight", probeBrowser->getZoomHeight());
		xmlNode->setAttribute("ZoomOffset", probeBrowser->getZoomOffset());

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

		XmlElement* channelNode = xmlNode->createNewChildElement("CHANNELS");
		XmlElement* xposNode = xmlNode->createNewChildElement("ELECTRODE_XPOS");
		XmlElement* yposNode = xmlNode->createNewChildElement("ELECTRODE_YPOS");

		for (int i = 0; i < settings->selectedElectrode.size(); i++)
		{
			int bank = int(settings->selectedBank[i]);
			int shank = settings->selectedShank[i];
			int elec = settings->selectedElectrode[i];

			String chString = String(bank);

			String chId = "CH" + String(i);

			channelNode->setAttribute(chId, chString);
			xposNode->setAttribute(chId, String(settings->electrodeMetadata[elec].xpos + 250 * shank));
			yposNode->setAttribute(chId, String(settings->electrodeMetadata[elec].ypos));
		}

		xmlNode->setAttribute("visualizationMode", (double)mode);
		xmlNode->setAttribute("activityToView", (double)probeBrowser->activityToView);

		xmlNode->setAttribute("isEnabled", bool(device->isEnabled()));
	}
}

void NeuropixelsV2Interface::loadParameters(XmlElement* xml)
{
	if (device != nullptr)
	{
		//auto npx = std::static_pointer_cast<Neuropixels2e>(device);

		// TODO: load parameters, put them into device->settings, and then update the interface
		//applyProbeSettings(device->settings.get(), false);
	}
}

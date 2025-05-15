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

#include "NeuropixelsV2eProbeInterface.h"

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

#include "../Formats/ProbeInterface.h"

using namespace OnixSourcePlugin;
using namespace ColourScheme;

NeuropixelsV2eProbeInterface::NeuropixelsV2eProbeInterface(std::shared_ptr<Neuropixels2e> d, int ind, OnixSourceEditor* e, OnixSourceCanvas* c) :
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

		probeBrowser = std::make_unique<NeuropixelsV2eProbeBrowser>(this, probeIndex);
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

		infoLabel = std::make_unique<Label>("INFO", "INFO");
		infoLabel->setFont(FontOptions(15.0f));
		infoLabel->setBounds(deviceLabel->getX(), deviceLabel->getBottom() + 3, deviceLabel->getWidth(), 50);
		infoLabel->setJustificationType(Justification::topLeft);
		addAndMakeVisible(infoLabel.get());

		gainCorrectionFileLabel = std::make_unique<Label>("gainCorrectionFileLabel", "Gain Correction File");
		gainCorrectionFileLabel->setBounds(infoLabel->getX() + 2, infoLabel->getBottom() + 5, 240, 16);
		gainCorrectionFileLabel->setColour(Label::textColourId, Colours::black);
		addAndMakeVisible(gainCorrectionFileLabel.get());

		gainCorrectionFile = std::make_unique<TextEditor>("GAIN CORRECTION FILE");
		gainCorrectionFile->setEnabled(false);
		gainCorrectionFile->setBounds(gainCorrectionFileLabel->getX(), gainCorrectionFileLabel->getBottom() + 2, gainCorrectionFileLabel->getWidth(), 20);
		gainCorrectionFile->setColour(Label::textColourId, Colours::black);
		gainCorrectionFile->addListener(this);
		addAndMakeVisible(gainCorrectionFile.get());

		gainCorrectionFileButton = std::make_unique<UtilityButton>("...");
		gainCorrectionFileButton->setBounds(gainCorrectionFile->getRight() + 3, gainCorrectionFile->getY(), 26, gainCorrectionFile->getHeight() + 2);
		gainCorrectionFileButton->setRadius(1.0f);
		gainCorrectionFileButton->addListener(this);
		gainCorrectionFileButton->setTooltip("Open a file dialog to choose the gain correction file for this probe.");
		addAndMakeVisible(gainCorrectionFileButton.get());

		gainCorrectionFileChooser = std::make_unique<FileChooser>("Select Gain Correction file.", File(), "*_gainCalValues.csv");

		probeInterfaceRectangle = std::make_unique<DrawableRectangle>();
		probeInterfaceRectangle->setFill(Colours::darkgrey);
		probeInterfaceRectangle->setRectangle(Rectangle<float>(gainCorrectionFile->getX(), gainCorrectionFile->getBottom() + 15, 250, 50));
		addAndMakeVisible(probeInterfaceRectangle.get());

		probeInterfaceLabel = std::make_unique<Label>("probeInterfaceLabel", "Probe Interface");
		probeInterfaceLabel->setFont(fontRegularLabel);
		probeInterfaceLabel->setBounds(probeInterfaceRectangle->getX(), probeInterfaceRectangle->getY(), 90, 18);
		probeInterfaceLabel->setColour(Label::textColourId, Colours::black);
		addAndMakeVisible(probeInterfaceLabel.get());

		saveJsonButton = std::make_unique<UtilityButton>("SAVE TO JSON");
		saveJsonButton->setRadius(3.0f);
		saveJsonButton->setBounds(probeInterfaceRectangle->getX() + 3, probeInterfaceRectangle->getY() + 20, 120, 22);
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
#pragma region Draw Legends

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

#pragma endregion
	}

	drawLegend();

	updateInfoString();
}

void NeuropixelsV2eProbeInterface::updateInfoString()
{
	String deviceString, infoString;

	auto npx = std::static_pointer_cast<Neuropixels2e>(device);

	if (device != nullptr)
	{
		deviceString = "Neuropixels 2.0 Probe";

		infoString += "\n";
		infoString += "Probe Number: ";
		infoString += npx->getProbeSerialNumber(probeIndex);
		infoString += "\n";
		infoString += "\n";
	}

	deviceLabel->setText(deviceString, dontSendNotification);
	infoLabel->setText(infoString, dontSendNotification);
}

void NeuropixelsV2eProbeInterface::comboBoxChanged(ComboBox* comboBox)
{
	auto npx = std::static_pointer_cast<Neuropixels2e>(device);

	if (comboBox == electrodeConfigurationComboBox.get())
	{
		auto preset = electrodeConfigurationComboBox->getText().toStdString();

		auto selection = npx->selectElectrodeConfiguration(preset);

		selectElectrodes(selection);
	}
	else if (comboBox == referenceComboBox.get())
	{
		npx->settings[probeIndex]->referenceIndex = referenceComboBox->getSelectedItemIndex();
	}

	repaint();
}

void NeuropixelsV2eProbeInterface::checkForExistingChannelPreset()
{
	auto npx = std::static_pointer_cast<Neuropixels2e>(device);
	auto settings = npx->settings[probeIndex].get();

	settings->electrodeConfigurationIndex = -1;

	for (int i = 0; i < settings->availableElectrodeConfigurations.size(); i++)
	{
		auto selection = npx->selectElectrodeConfiguration(settings->availableElectrodeConfigurations[i]);
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

void NeuropixelsV2eProbeInterface::buttonClicked(Button* button)
{
	auto npx = std::static_pointer_cast<Neuropixels2e>(device);

	if (button == enableViewButton.get())
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
	else if (button == enableButton.get())
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
			auto npx = std::static_pointer_cast<Neuropixels2e>(device);

			if (ProbeInterfaceJson::readProbeSettingsFromJson(fileChooser.getResult(), npx->settings[probeIndex].get()))
			{
				applyProbeSettings(npx->settings[probeIndex].get());
				checkForExistingChannelPreset();
			}
		}
	}
	else if (button == saveJsonButton.get())
	{
		FileChooser fileChooser("Save channel map to a probeinterface JSON file.", File(), "*.json");

		if (fileChooser.browseForFileToSave(true))
		{
			auto npx = std::static_pointer_cast<Neuropixels2e>(device);

			if (!ProbeInterfaceJson::writeProbeSettingsToJson(fileChooser.getResult(), npx->settings[probeIndex].get()))
				CoreServices::sendStatusMessage("Failed to write probe channel map.");
			else
				CoreServices::sendStatusMessage("Successfully wrote probe channel map.");
		}
	}
	else if (button == gainCorrectionFileButton.get())
	{
		if (gainCorrectionFileChooser->browseForFileToOpen())
		{
			gainCorrectionFile->setText(gainCorrectionFileChooser->getResult().getFullPathName());
		}
		else
		{
			gainCorrectionFile->setText("");
		}

		std::static_pointer_cast<Neuropixels2e>(device)->setGainCorrectionFile(probeIndex, gainCorrectionFile->getText());
	}
}

std::vector<int> NeuropixelsV2eProbeInterface::getSelectedElectrodes()
{
	std::vector<int> electrodeIndices;

	auto npx = std::static_pointer_cast<Neuropixels2e>(device);

	for (int i = 0; i < npx->settings[probeIndex]->electrodeMetadata.size(); i++)
	{
		if (npx->settings[probeIndex]->electrodeMetadata[i].isSelected)
		{
			electrodeIndices.emplace_back(i);
		}
	}

	return electrodeIndices;
}

void NeuropixelsV2eProbeInterface::setReference(int index)
{
	referenceComboBox->setSelectedId(index + 1, true);
}

void NeuropixelsV2eProbeInterface::selectElectrodes(std::vector<int> electrodes)
{
	std::static_pointer_cast<Neuropixels2e>(device)->settings[probeIndex]->selectElectrodes(electrodes);

	repaint();
}

void NeuropixelsV2eProbeInterface::setInterfaceEnabledState(bool enabledState)
{
	if (enableButton != nullptr)
		enableButton->setEnabled(enabledState);

	if (electrodeConfigurationComboBox != nullptr)
		electrodeConfigurationComboBox->setEnabled(enabledState);

	if (referenceComboBox != nullptr)
		referenceComboBox->setEnabled(enabledState);

	if (loadJsonButton != nullptr)
		loadJsonButton->setEnabled(enabledState);

	if (saveJsonButton != nullptr)
		saveJsonButton->setEnabled(enabledState);

	if (gainCorrectionFileButton != nullptr)
		gainCorrectionFileButton->setEnabled(enabledState);
}

void NeuropixelsV2eProbeInterface::startAcquisition()
{
	acquisitionIsActive = true;

	setInterfaceEnabledState(false);

	if (mode == VisualizationMode::ACTIVITY_VIEW)
		probeBrowser->startTimer(100);
}

void NeuropixelsV2eProbeInterface::stopAcquisition()
{
	acquisitionIsActive = false;

	setInterfaceEnabledState(true);
}

void NeuropixelsV2eProbeInterface::drawLegend()
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

void NeuropixelsV2eProbeInterface::updateSettings()
{
	if (device == nullptr) return;

	auto npx = std::static_pointer_cast<Neuropixels2e>(device);

	applyProbeSettings(npx->settings[probeIndex].get());
	checkForExistingChannelPreset();

	gainCorrectionFile->setText(npx->getGainCorrectionFile(probeIndex) == "None" ? "" : npx->getGainCorrectionFile(probeIndex), dontSendNotification);
}

bool NeuropixelsV2eProbeInterface::applyProbeSettings(ProbeSettings<Neuropixels2e::numberOfChannels, Neuropixels2e::numberOfElectrodes>* p, bool shouldUpdateProbe)
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

	for (int i = 0; i < p->selectedElectrode.size(); i++)
	{
		Bank bank = p->selectedBank[i];
		int shank = p->selectedShank[i];
		int index = p->selectedElectrode[i];

		for (int j = 0; j < settings->electrodeMetadata.size(); j++)
		{
			if (settings->electrodeMetadata[j].global_index == index && settings->electrodeMetadata[j].bank == bank && settings->electrodeMetadata[j].shank == shank)
			{
				settings->electrodeMetadata[j].status = ElectrodeStatus::CONNECTED;
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

int NeuropixelsV2eProbeInterface::getIndexOfComboBoxItem(ComboBox* cb, String item)
{
	for (int i = 0; i < cb->getNumItems(); i++)
	{
		if (item == cb->getItemText(i))
			return i;
	}

	return -1;
}

void NeuropixelsV2eProbeInterface::saveParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Saving Neuropixels 2.0e settings.");

	auto npx = std::static_pointer_cast<Neuropixels2e>(device);
	auto settings = npx->settings[probeIndex].get();

	XmlElement* xmlNode = xml->createNewChildElement("PROBE" + String(probeIndex));

	xmlNode->setAttribute("probeSerialNumber", String(npx->getProbeSerialNumber(probeIndex)));

	xmlNode->setAttribute("gainCorrectionFile", npx->getGainCorrectionFile(probeIndex));

	XmlElement* probeViewerNode = xmlNode->createNewChildElement("PROBE_VIEWER");

	probeViewerNode->setAttribute("zoomHeight", probeBrowser->getZoomHeight());
	probeViewerNode->setAttribute("zoomOffset", probeBrowser->getZoomOffset());

	probeViewerNode->setAttribute("referenceChannel", referenceComboBox->getText());

	XmlElement* channelsNode = xmlNode->createNewChildElement("SELECTED_CHANNELS");

	for (int i = 0; i < settings->selectedElectrode.size(); i++)
	{
		int globalIndex = settings->selectedElectrode[i];

		channelsNode->setAttribute("CH" + String(i), String(globalIndex));
	}
}

void NeuropixelsV2eProbeInterface::loadParameters(XmlElement* xml)
{
	if (device == nullptr) return;

	LOGD("Loading Neuropixels 2.0e settings.");

	auto npx = std::static_pointer_cast<Neuropixels2e>(device);
	auto settings = npx->settings[probeIndex].get();

	XmlElement* xmlNode = nullptr;

	for (auto* node : xml->getChildIterator())
	{
		if (node->hasTagName("PROBE" + String(probeIndex)))
		{
			xmlNode = node;
			break;
		}
	}

	if (xmlNode == nullptr)
	{
		LOGD("No PROBE" + String(probeIndex) + " element found");
		return;
	}

	if (npx->getProbeSerialNumber(probeIndex) != 0 && xmlNode->getIntAttribute("probeSerialNumber") != npx->getProbeSerialNumber(probeIndex))
	{
		LOGC("Different serial numbers found. Current serial number is " + String(npx->getProbeSerialNumber(probeIndex)) + ", while the saved serial number is " + String(xmlNode->getIntAttribute("probeSerialNumber")) + ". Updating settings...");
	}

	npx->setGainCorrectionFile(probeIndex, xmlNode->getStringAttribute("gainCorrectionFile"));

	XmlElement* probeViewerNode = xmlNode->getChildByName("PROBE_VIEWER");

	if (probeViewerNode == nullptr)
	{
		LOGE("No PROBE_VIEWER element found.");
		return;
	}

	probeBrowser->setZoomHeightAndOffset(probeViewerNode->getIntAttribute("zoomHeight"), probeViewerNode->getIntAttribute("zoomOffset"));

	int idx = -1;

	idx = getIndexOfComboBoxItem(referenceComboBox.get(), probeViewerNode->getStringAttribute("referenceChannel"));
	if (idx == -1)
	{
		LOGE("No reference channel variable found.");
	}
	else
		settings->referenceIndex = idx;

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
		String chIdx = channelsNode->getStringAttribute("CH" + String(i), "");

		if (chIdx == "")
		{
			LOGE("Channel #" + String(i) + " not found. Channel map will not be updated.");
			return;
		}

		selectedChannels.emplace_back(std::stoi(chIdx.toStdString()));
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

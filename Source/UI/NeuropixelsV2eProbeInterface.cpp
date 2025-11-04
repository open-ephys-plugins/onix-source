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

#include "../OnixSourceCanvas.h"
#include "../OnixSourceEditor.h"

#include "../Formats/ProbeInterface.h"

using namespace OnixSourcePlugin;
using namespace ColourScheme;

NeuropixelsV2eProbeInterface::NeuropixelsV2eProbeInterface (std::shared_ptr<Neuropixels2e> d, int ind, OnixSourceEditor* e, OnixSourceCanvas* c)
    : SettingsInterface (d, e, c),
      probeIndex (ind)
{
    ColourScheme::setColourScheme (ColourSchemeId::PLASMA);

    auto settings = std::static_pointer_cast<Neuropixels2e> (device)->settings[probeIndex].get();

    type = SettingsInterface::Type::PROBE_SETTINGS_INTERFACE;

    mode = VisualizationMode::ENABLE_VIEW;

    probeBrowser = std::make_unique<NeuropixelsV2eProbeBrowser> (this, probeIndex);
    probeBrowser->setBounds (0, 0, 600, 600);
    addAndMakeVisible (probeBrowser.get());

    int currentHeight = 55;

    FontOptions fontName = FontOptions ("Fira Code", "Medium", 30.0f);
    FontOptions fontRegularButton = FontOptions ("Fira Code", "Regular", 12.0f);
    FontOptions fontRegularLabel = FontOptions ("Fira Code", "Regular", 13.0f);

    deviceLabel = std::make_unique<Label> ("deviceLabel", "Device Name");
    deviceLabel->setFont (fontName);
    deviceLabel->setBounds (625, 40, 430, 45);
    addAndMakeVisible (deviceLabel.get());

    infoLabel = std::make_unique<Label> ("INFO", "INFO");
    infoLabel->setFont (FontOptions (15.0f));
    infoLabel->setBounds (deviceLabel->getX(), deviceLabel->getBottom() + 3, deviceLabel->getWidth(), 80);
    infoLabel->setJustificationType (Justification::topLeft);
    addAndMakeVisible (infoLabel.get());

    searchForCorrectionFilesButton = std::make_unique<ToggleButton> ("Search for calibration files automatically");
    searchForCorrectionFilesButton->setBounds (infoLabel->getX() + 2, infoLabel->getBottom() + 5, 350, 20);
    searchForCorrectionFilesButton->addListener (this);
    searchForCorrectionFilesButton->setTooltip ("Open a file dialog to choose a folder that contains all calibration files. The calibration file that matches your probe will automatically be selected if it exists in this folder.");
    addAndMakeVisible (searchForCorrectionFilesButton.get());

    gainCorrectionFolderLabel = std::make_unique<Label> ("gainCorrectionFolderLabel", "Calibration Folder");
    gainCorrectionFolderLabel->setBounds (searchForCorrectionFilesButton->getX(), searchForCorrectionFilesButton->getBottom() + 5, 240, 16);
    gainCorrectionFolderLabel->setColour (Label::textColourId, Colours::black);
    addAndMakeVisible (gainCorrectionFolderLabel.get());

    gainCorrectionFolder = std::make_unique<TextEditor> ("GAIN CORRECTION FOLDER");
    gainCorrectionFolder->setEnabled (false);
    gainCorrectionFolder->setBounds (gainCorrectionFolderLabel->getX(), gainCorrectionFolderLabel->getBottom() + 2, gainCorrectionFolderLabel->getWidth(), 20);
    gainCorrectionFolder->setColour (Label::textColourId, Colours::black);
    gainCorrectionFolder->addListener (this);
    addAndMakeVisible (gainCorrectionFolder.get());

    gainCorrectionFolderButton = std::make_unique<UtilityButton> ("...");
    gainCorrectionFolderButton->setBounds (gainCorrectionFolder->getRight() + 3, gainCorrectionFolder->getY(), 26, gainCorrectionFolder->getHeight() + 2);
    gainCorrectionFolderButton->setRadius (1.0f);
    gainCorrectionFolderButton->addListener (this);
    gainCorrectionFolderButton->setTooltip ("Open a file dialog to choose the gain correction folder path to automatically search for probe calibration files.");
    addAndMakeVisible (gainCorrectionFolderButton.get());

    gainCorrectionFolderChooser = std::make_unique<FileChooser> ("Select Gain Correction Folder.", File());

    gainCorrectionFileLabel = std::make_unique<Label> ("gainCorrectionFileLabel", "Gain Correction File");
    gainCorrectionFileLabel->setBounds (gainCorrectionFolder->getX(), gainCorrectionFolder->getBottom() + 15, gainCorrectionFolderLabel->getWidth(), gainCorrectionFolderLabel->getHeight());
    gainCorrectionFileLabel->setColour (Label::textColourId, Colours::black);
    addAndMakeVisible (gainCorrectionFileLabel.get());

    gainCorrectionFile = std::make_unique<TextEditor> ("GAIN CORRECTION FILE");
    gainCorrectionFile->setEnabled (false);
    gainCorrectionFile->setBounds (gainCorrectionFileLabel->getX(), gainCorrectionFileLabel->getBottom() + 2, gainCorrectionFileLabel->getWidth(), 20);
    gainCorrectionFile->setColour (Label::textColourId, Colours::black);
    gainCorrectionFile->addListener (this);
    addAndMakeVisible (gainCorrectionFile.get());

    gainCorrectionFileButton = std::make_unique<UtilityButton> ("...");
    gainCorrectionFileButton->setBounds (gainCorrectionFile->getRight() + 3, gainCorrectionFile->getY(), 26, gainCorrectionFile->getHeight() + 2);
    gainCorrectionFileButton->setRadius (1.0f);
    gainCorrectionFileButton->addListener (this);
    gainCorrectionFileButton->setTooltip ("Open a file dialog to choose the gain correction file for this probe.");
    addAndMakeVisible (gainCorrectionFileButton.get());

    gainCorrectionFileChooser = std::make_unique<FileChooser> ("Select Gain Correction file.", File(), std::string ("*") + GainCalibrationFilename);

    probeInterfaceRectangle = std::make_unique<DrawableRectangle>();
    probeInterfaceRectangle->setFill (Colours::darkgrey);
    probeInterfaceRectangle->setRectangle (Rectangle<float> (gainCorrectionFile->getX(), gainCorrectionFile->getBottom() + 15, 250, 50));
    addAndMakeVisible (probeInterfaceRectangle.get());

    probeInterfaceLabel = std::make_unique<Label> ("probeInterfaceLabel", "Probe Interface");
    probeInterfaceLabel->setFont (fontRegularLabel);
    probeInterfaceLabel->setBounds (probeInterfaceRectangle->getX(), probeInterfaceRectangle->getY(), 90, 18);
    probeInterfaceLabel->setColour (Label::textColourId, Colours::black);
    addAndMakeVisible (probeInterfaceLabel.get());

    saveJsonButton = std::make_unique<UtilityButton> ("Save to JSON");
    saveJsonButton->setRadius (3.0f);
    saveJsonButton->setBounds (probeInterfaceRectangle->getX() + 3, probeInterfaceRectangle->getY() + 20, 120, 22);
    saveJsonButton->addListener (this);
    saveJsonButton->setTooltip ("Save channel map to ProbeInterface .json file");
    addAndMakeVisible (saveJsonButton.get());

    loadJsonButton = std::make_unique<UtilityButton> ("Load from JSON");
    loadJsonButton->setRadius (3.0f);
    loadJsonButton->setBounds (saveJsonButton->getRight() + 5, saveJsonButton->getY(), 120, 22);
    loadJsonButton->addListener (this);
    loadJsonButton->setTooltip ("Load channel map from ProbeInterface .json file");
    addAndMakeVisible (loadJsonButton.get());

    saveSettingsButton = std::make_unique<UtilityButton> ("Save Settings");
    saveSettingsButton->setRadius (3.0f);
    saveSettingsButton->setBounds (saveJsonButton->getX(), probeBrowser->getBottom() - 80, 120, 22);
    saveSettingsButton->addListener (this);
    saveSettingsButton->setTooltip ("Save all Neuropixels settings to file.");
    addAndMakeVisible (saveSettingsButton.get());

    loadSettingsButton = std::make_unique<UtilityButton> ("Load Settings");
    loadSettingsButton->setRadius (3.0f);
    loadSettingsButton->setBounds (saveSettingsButton->getRight() + 5, saveSettingsButton->getY(), saveSettingsButton->getWidth(), saveSettingsButton->getHeight());
    loadSettingsButton->addListener (this);
    loadSettingsButton->setTooltip ("Load all Neuropixels settings from a file.");
    addAndMakeVisible (loadSettingsButton.get());

    electrodesLabel = std::make_unique<Label> ("ELECTRODES", "ELECTRODES");
    electrodesLabel->setFont (FontOptions ("Inter", "Regular", 13.0f));
    electrodesLabel->setBounds (446, currentHeight - 20, 100, 20);
    addAndMakeVisible (electrodesLabel.get());

    enableViewButton = std::make_unique<UtilityButton> ("VIEW");
    enableViewButton->setFont (fontRegularButton);
    enableViewButton->setRadius (3.0f);
    enableViewButton->setBounds (530, currentHeight + 2, 45, 18);
    enableViewButton->addListener (this);
    enableViewButton->setTooltip ("View electrode enabled state");
    addAndMakeVisible (enableViewButton.get());

    enableButton = std::make_unique<UtilityButton> ("ENABLE");
    enableButton->setFont (fontRegularButton);
    enableButton->setRadius (3.0f);
    enableButton->setBounds (450, currentHeight, 65, 22);
    enableButton->addListener (this);
    enableButton->setTooltip ("Enable selected electrodes");
    addAndMakeVisible (enableButton.get());

    currentHeight += 58;

    electrodePresetLabel = std::make_unique<Label> ("ELECTRODE PRESET", "ELECTRODE PRESET");
    electrodePresetLabel->setFont (fontRegularLabel);
    electrodePresetLabel->setBounds (446, currentHeight - 20, 150, 20);
    addAndMakeVisible (electrodePresetLabel.get());

    electrodeConfigurationComboBox = std::make_unique<ComboBox> ("electrodeConfigurationComboBox");
    electrodeConfigurationComboBox->setBounds (450, currentHeight, 135, 22);
    electrodeConfigurationComboBox->addListener (this);
    electrodeConfigurationComboBox->setTooltip ("Enable a pre-configured set of electrodes");

    updateChannelPresets (settings);

    addAndMakeVisible (electrodeConfigurationComboBox.get());

    currentHeight += 55;

    probeTypeLabel = std::make_unique<Label> ("PROBE TYPE", "PROBE TYPE");
    probeTypeLabel->setFont (fontRegularLabel);
    probeTypeLabel->setBounds (446, currentHeight - 20, 150, 20);
    addAndMakeVisible (probeTypeLabel.get());

    probeTypeComboBox = std::make_unique<ComboBox> ("probeTypeComboBox");
    probeTypeComboBox->setBounds (450, currentHeight, 135, 22);
    probeTypeComboBox->addListener (this);
    probeTypeComboBox->setTooltip ("Choose which probe type is connected to the headstage.");

    probeTypeComboBox->addItem (String (ProbeTypeString.at (ProbeType::NPX_V2_QUAD_SHANK)), (int) ProbeType::NPX_V2_QUAD_SHANK);
    probeTypeComboBox->addItem (String (ProbeTypeString.at (ProbeType::NPX_V2_SINGLE_SHANK)), (int) ProbeType::NPX_V2_SINGLE_SHANK);

    probeTypeComboBox->setSelectedId ((int) ProbeType::NPX_V2_QUAD_SHANK, dontSendNotification);

    addAndMakeVisible (probeTypeComboBox.get());

    currentHeight += 55;

    referenceComboBox = std::make_unique<ComboBox> ("ReferenceComboBox");
    referenceComboBox->setBounds (450, currentHeight, 65, 22);
    referenceComboBox->addListener (this);

    updateReferences (settings);

    addAndMakeVisible (referenceComboBox.get());

    referenceViewButton = std::make_unique<UtilityButton> ("VIEW");
    referenceViewButton->setFont (fontRegularButton);
    referenceViewButton->setRadius (3.0f);
    referenceViewButton->setBounds (530, currentHeight + 2, 45, 18);
    referenceViewButton->addListener (this);
    referenceViewButton->setTooltip ("View reference of each channel");
    addAndMakeVisible (referenceViewButton.get());

    referenceLabel = std::make_unique<Label> ("REFERENCE", "REFERENCE");
    referenceLabel->setFont (fontRegularLabel);
    referenceLabel->setBounds (446, currentHeight - 20, 100, 20);
    addAndMakeVisible (referenceLabel.get());

    currentHeight += 55;

#pragma region Draw Legends

    // ENABLE View
    Colour colour = Colour (55, 55, 55);
    float fontSize = 16.0f;

    enableViewComponent = std::make_unique<Component> ("enableViewComponent");
    enableViewComponent->setBounds (450, 430, 120, 200);

    enableViewLabels.push_back (std::make_unique<Label> ("enableViewLabel", "ENABLED?"));
    enableViewLabels[0]->setJustificationType (Justification::centredLeft);
    enableViewLabels[0]->setFont (FontOptions (fontSize));
    enableViewLabels[0]->setColour (Label::ColourIds::textColourId, colour);
    enableViewLabels[0]->setBounds (0, 0, 110, 15);
    enableViewComponent->addAndMakeVisible (enableViewLabels[0].get());

    std::vector<Colour> colors = { Colours::yellow, Colours::darkgrey, Colours::black };
    StringArray legendLabels = { "YES", "NO", "REFERENCE" };

    for (int i = 0; i < colors.size(); i++)
    {
        enableViewRectangles.push_back (std::make_unique<DrawableRectangle>());
        enableViewRectangles[i]->setFill (colors[i]);
        enableViewRectangles[i]->setRectangle (Rectangle<float> (enableViewLabels[0]->getX() + 6, enableViewLabels[i]->getBottom() + 1, 12, 12));
        enableViewComponent->addAndMakeVisible (enableViewRectangles[i].get());

        enableViewLabels.push_back (std::make_unique<Label> ("enableViewLabel", legendLabels[i]));
        int labelInd = i + 1;
        enableViewLabels[labelInd]->setJustificationType (Justification::centredLeft);
        enableViewLabels[labelInd]->setFont (FontOptions (fontSize));
        enableViewLabels[labelInd]->setColour (Label::ColourIds::textColourId, colour);
        enableViewLabels[labelInd]->setBounds (enableViewRectangles[i]->getRight() + 2, enableViewRectangles[i]->getY(), 100, 17);
        enableViewComponent->addAndMakeVisible (enableViewLabels[labelInd].get());
    }

    addAndMakeVisible (enableViewComponent.get());

    // REFERENCE View
    referenceViewComponent = std::make_unique<Component> ("referenceViewComponent");
    referenceViewComponent->setBounds (enableViewComponent->getX(), enableViewComponent->getY(), 120, 300);

    referenceViewLabels.push_back (std::make_unique<Label> ("referenceViewLabel", "REFERENCE"));
    referenceViewLabels[0]->setJustificationType (Justification::centredLeft);
    referenceViewLabels[0]->setFont (FontOptions (fontSize));
    referenceViewLabels[0]->setColour (Label::ColourIds::textColourId, colour);
    referenceViewLabels[0]->setBounds (0, 0, 110, 15);
    referenceViewComponent->addAndMakeVisible (referenceViewLabels[0].get());

    colors.clear();
    legendLabels.clear();

    for (int i = 0; i < referenceComboBox->getNumItems(); i++)
    {
        String ref = referenceComboBox->getItemText (i);

        if (ref.contains ("Ext"))
            colors.push_back (Colours::pink);
        else if (ref.contains ("Tip"))
            colors.push_back (Colours::orange);
        else
            colors.push_back (Colours::purple);

        legendLabels.add (referenceComboBox->getItemText (i));
    }

    for (int i = 0; i < colors.size(); i++)
    {
        referenceViewRectangles.push_back (std::make_unique<DrawableRectangle>());
        referenceViewRectangles[i]->setFill (colors[i]);
        referenceViewRectangles[i]->setRectangle (Rectangle<float> (referenceViewLabels[0]->getX() + 6, referenceViewLabels[i]->getBottom() + 1, 12, 12));
        referenceViewComponent->addAndMakeVisible (referenceViewRectangles[i].get());

        referenceViewLabels.push_back (std::make_unique<Label> ("referenceViewLabel", legendLabels[i]));
        int labelInd = i + 1;
        referenceViewLabels[labelInd]->setJustificationType (Justification::centredLeft);
        referenceViewLabels[labelInd]->setFont (FontOptions (fontSize));
        referenceViewLabels[labelInd]->setColour (Label::ColourIds::textColourId, colour);
        referenceViewLabels[labelInd]->setBounds (referenceViewRectangles[i]->getRight() + 2, referenceViewRectangles[i]->getY(), 100, 17);
        referenceViewComponent->addAndMakeVisible (referenceViewLabels[labelInd].get());
    }

    addAndMakeVisible (referenceViewComponent.get());
#pragma endregion

    setGainCorrectionFolderEnabledState (false);

    drawLegend();

    updateInfoString();
}

void NeuropixelsV2eProbeInterface::updateInfoString()
{
    std::string deviceString, infoString;

    auto npx = std::static_pointer_cast<Neuropixels2e> (device);
    auto sn = npx->getProbeSerialNumber (probeIndex);

    if (device != nullptr)
    {
        deviceString = "Neuropixels 2.0 Probe";

        infoString += "\n";
        infoString += "Probe Serial Number: ";
        infoString += std::to_string (sn);
        infoString += "\n";
        infoString += "Probe Part Number: ";
        infoString += npx->getProbePartNumber (probeIndex);
        infoString += "\n";
        infoString += "Flex: ";
        infoString += npx->getFlexPartNumber (probeIndex);
        infoString += "\n";
    }

    deviceLabel->setText (deviceString, dontSendNotification);
    infoLabel->setText (infoString, dontSendNotification);

    if (searchForCorrectionFilesButton->getToggleState() && device->isEnabled())
    {
        auto file = searchDirectoryForCalibrationFile (gainCorrectionFolder->getText().toStdString(), sn);

        if (file != "")
            gainCorrectionFile->setText (file);
    }
}

void NeuropixelsV2eProbeInterface::comboBoxChanged (ComboBox* comboBox)
{
    auto npx = std::static_pointer_cast<Neuropixels2e> (device);

    if (comboBox == electrodeConfigurationComboBox.get())
    {
        auto selection = npx->selectElectrodeConfiguration (electrodeConfigurationComboBox->getSelectedItemIndex() - 1, npx->settings[probeIndex]->probeType);

        selectElectrodes (selection);
    }
    else if (comboBox == referenceComboBox.get())
    {
        npx->settings[probeIndex]->referenceIndex = referenceComboBox->getSelectedItemIndex();
    }
    else if (comboBox == probeTypeComboBox.get())
    {
        if (comboBox->getSelectedId() == 0)
        {
            LOGD ("Invalid ProbeType selected in the drop-down menu. ID = 0");
            return;
        }

        auto settings = npx->settings[probeIndex].get();

        saveProbeSettings (settings);

        loadProbeSettings (settings, (ProbeType) comboBox->getSelectedId());

        updateSettings();
    }

    repaint();
}

void NeuropixelsV2eProbeInterface::saveProbeSettings (ProbeSettings* settings)
{
    getProbeSetting (settings->probeType)->updateProbeSettings (settings);
}

void NeuropixelsV2eProbeInterface::loadProbeSettings (ProbeSettings* settings, ProbeType probeType)
{
    settings->updateProbeSettings (getProbeSetting (probeType));
}

ProbeSettings* NeuropixelsV2eProbeInterface::getProbeSetting (ProbeType probeType)
{
    if (probeSettings.find (probeType) == probeSettings.end())
    {
        auto numElectrodes = probeType == ProbeType::NPX_V2_QUAD_SHANK ? NeuropixelsV2eValues::numberOfQuadShankElectrodes : NeuropixelsV2eValues::numberOfSingleShankElectrodes;
        probeSettings.emplace (probeType, std::make_unique<ProbeSettings> (NeuropixelsV2eValues::numberOfChannels, numElectrodes, probeType));
        std::static_pointer_cast<Neuropixels2e> (device)->defineMetadata (probeSettings[probeType].get(), probeType);
    }

    return probeSettings[probeType].get();
}

void NeuropixelsV2eProbeInterface::updateChannelPresets (ProbeSettings* settings)
{
    electrodeConfigurationComboBox->clear (dontSendNotification);

    electrodeConfigurationComboBox->addItem ("Select a preset...", 1);
    electrodeConfigurationComboBox->setItemEnabled (1, false);
    electrodeConfigurationComboBox->addSeparator();

    for (int i = 0; i < settings->availableElectrodeConfigurations.size(); i++)
    {
        electrodeConfigurationComboBox->addItem (settings->availableElectrodeConfigurations[i], i + 2);
    }

    checkForExistingChannelPreset();
}

void NeuropixelsV2eProbeInterface::updateReferences (ProbeSettings* settings)
{
    referenceComboBox->clear();

    for (int i = 0; i < settings->availableReferences.size(); i++)
    {
        referenceComboBox->addItem (settings->availableReferences[i], i + 1);
    }

    referenceComboBox->setSelectedId (settings->referenceIndex + 1, dontSendNotification);
}

void NeuropixelsV2eProbeInterface::checkForExistingChannelPreset()
{
    auto npx = std::static_pointer_cast<Neuropixels2e> (device);
    auto settings = npx->settings[probeIndex].get();

    settings->electrodeConfigurationIndex = -1;

    for (int i = 0; i < settings->availableElectrodeConfigurations.size(); i++)
    {
        auto selection = npx->selectElectrodeConfiguration (i, settings->probeType);
        std::vector<int> channelMap;
        channelMap.assign (NeuropixelsV2eValues::numberOfChannels, 0);

        for (int j = 0; j < selection.size(); j++)
        {
            channelMap[settings->electrodeMetadata[selection[j]].channel] = selection[j];
        }

        if (std::equal (channelMap.cbegin(), channelMap.cend(), settings->selectedElectrode.cbegin(), settings->selectedElectrode.cend()))
        {
            settings->electrodeConfigurationIndex = i;
            break;
        }
    }

    electrodeConfigurationComboBox->setSelectedId (settings->electrodeConfigurationIndex + 2, dontSendNotification);
}

void NeuropixelsV2eProbeInterface::buttonClicked (Button* button)
{
    auto npx = std::static_pointer_cast<Neuropixels2e> (device);

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
            selectElectrodes (selection);
        }

        checkForExistingChannelPreset();
    }
    else if (button == loadJsonButton.get())
    {
        FileChooser fileChooser ("Select an probeinterface JSON file to load.", File(), "*" + std::string (ProbeInterfaceJson::FileExtension));

        if (fileChooser.browseForFileToOpen())
        {
            auto settings = npx->settings[probeIndex].get();

            if (ProbeInterfaceJson::readProbeSettingsFromJson (fileChooser.getResult(), settings))
            {
                applyProbeSettings (npx->settings[probeIndex].get());
                checkForExistingChannelPreset();
            }
        }
    }
    else if (button == saveJsonButton.get())
    {
        FileChooser fileChooser ("Save channel map to a probeinterface JSON file.", File(), "*" + std::string (ProbeInterfaceJson::FileExtension));

        if (fileChooser.browseForFileToSave (true))
        {
            try
            {
                auto settings = npx->settings[probeIndex].get();

                ProbeInterfaceJson::writeProbeSettingsToJson (fileChooser.getResult(), settings);
            }
            catch (const error_str& e)
            {
                Onix1::showWarningMessageBoxAsync ("Unable to Save Probe JSON File", e.what());
                return;
            }
        }
    }
    else if (button == saveSettingsButton.get())
    {
        FileChooser fileChooser ("Save Neuropixels settings to an XML file.", File(), "*.xml");
        if (fileChooser.browseForFileToSave (true))
        {
            XmlElement rootElement ("DEVICE");

            saveParameters (&rootElement);

            writeToXmlFile (&rootElement, fileChooser.getResult());
        }
    }
    else if (button == loadSettingsButton.get())
    {
        FileChooser fileChooser ("Load Neuropixels settings from an XML file.", File(), "*.xml");

        if (fileChooser.browseForFileToOpen())
        {
            auto rootElement = readFromXmlFile (fileChooser.getResult());

            loadParameters (rootElement);

            delete rootElement;
        }
    }
    else if (button == gainCorrectionFileButton.get())
    {
        if (gainCorrectionFileChooser->browseForFileToOpen())
        {
            gainCorrectionFile->setText (gainCorrectionFileChooser->getResult().getFullPathName());
        }
    }
    else if (button == searchForCorrectionFilesButton.get())
    {
        setGainCorrectionFolderEnabledState (button->getToggleState());

        if (button->getToggleState() && device->isEnabled())
        {
            auto calibrationFile = searchDirectoryForCalibrationFile (gainCorrectionFolder->getText().toStdString(), npx->getProbeSerialNumber (probeIndex));

            if (calibrationFile != "")
                gainCorrectionFile->setText (calibrationFile);
        }
    }
    else if (button == gainCorrectionFolderButton.get())
    {
        if (gainCorrectionFolderChooser->browseForDirectory())
        {
            gainCorrectionFolder->setText (gainCorrectionFolderChooser->getResult().getFullPathName());

            if (device->isEnabled())
            {
                auto calibrationFile = searchDirectoryForCalibrationFile (gainCorrectionFolder->getText().toStdString(), npx->getProbeSerialNumber (probeIndex));

                if (calibrationFile != "")
                    gainCorrectionFile->setText (calibrationFile);
            }
        }
    }
}

void NeuropixelsV2eProbeInterface::textEditorTextChanged (TextEditor& editor)
{
    if (editor.getName() == gainCorrectionFile->getName())
    {
        std::static_pointer_cast<Neuropixels2e> (device)->setGainCorrectionFile (probeIndex, editor.getText().toStdString());
    }
}

std::string NeuropixelsV2eProbeInterface::searchDirectoryForCalibrationFile (std::string directory, uint64_t sn)
{
    if (directory == "" || sn == 0)
        return "";

    auto rootDirectory = File (directory);

    if (! rootDirectory.isDirectory())
    {
        Onix1::showWarningMessageBoxAsync ("Invalid Directory", "The path given for the calibration files directory is invalid. Please try setting it again.");
        return "";
    }

    std::string filename = std::to_string (sn) + GainCalibrationFilename;

    auto calibrationFiles = searchDirectoryForFile (rootDirectory, filename, NeuropixelsCalibrationFileRecursiveLevels);

    if (calibrationFiles.size() != 1)
    {
        std::string msg = "Expected to find 1 file matching '" + filename + "', but found " + std::to_string (calibrationFiles.size()) + " instead.";

        if (calibrationFiles.size() > 1)
        {
            msg += " Check logs for all files discovered.";
        }

        Onix1::showWarningMessageBoxAsync ("Wrong Number of Calibration Files", msg);
        return "";
    }

    return calibrationFiles[0].getFullPathName().toStdString();
}

void NeuropixelsV2eProbeInterface::setGainCorrectionFolderEnabledState (bool enabledState)
{
    float alphaEnabled = 1.0, alphaDisabled = 0.25;

    if (! enabledState)
    {
        gainCorrectionFolder->setAlpha (alphaDisabled);
        gainCorrectionFolderButton->setEnabled (false);

        gainCorrectionFile->setAlpha (alphaEnabled);
        gainCorrectionFileButton->setEnabled (true);
    }
    else
    {
        gainCorrectionFolder->setAlpha (alphaEnabled);
        gainCorrectionFolderButton->setEnabled (true);

        gainCorrectionFile->setAlpha (alphaDisabled);
        gainCorrectionFileButton->setEnabled (false);
    }
}

std::vector<int> NeuropixelsV2eProbeInterface::getSelectedElectrodes()
{
    std::vector<int> electrodeIndices;

    auto npx = std::static_pointer_cast<Neuropixels2e> (device);

    for (int i = 0; i < npx->settings[probeIndex]->electrodeMetadata.size(); i++)
    {
        if (npx->settings[probeIndex]->electrodeMetadata[i].isSelected)
        {
            electrodeIndices.emplace_back (i);
        }
    }

    return electrodeIndices;
}

void NeuropixelsV2eProbeInterface::setReference (int index)
{
    referenceComboBox->setSelectedId (index + 1, true);
}

void NeuropixelsV2eProbeInterface::selectElectrodes (std::vector<int> electrodes)
{
    std::static_pointer_cast<Neuropixels2e> (device)->settings[probeIndex]->selectElectrodes (electrodes);

    CoreServices::updateSignalChain (editor);

    repaint();
}

void NeuropixelsV2eProbeInterface::setInterfaceEnabledState (bool enabledState)
{
    if (enableButton != nullptr)
        enableButton->setEnabled (enabledState);

    if (electrodeConfigurationComboBox != nullptr)
        electrodeConfigurationComboBox->setEnabled (enabledState);

    if (referenceComboBox != nullptr)
        referenceComboBox->setEnabled (enabledState);

    if (loadJsonButton != nullptr)
        loadJsonButton->setEnabled (enabledState);

    if (saveJsonButton != nullptr)
        saveJsonButton->setEnabled (enabledState);

    if (gainCorrectionFileButton != nullptr)
        gainCorrectionFileButton->setEnabled (enabledState);

    if (saveSettingsButton != nullptr)
        saveSettingsButton->setEnabled (enabledState);

    if (loadSettingsButton != nullptr)
        loadSettingsButton->setEnabled (enabledState);

    if (gainCorrectionFolderButton != nullptr)
        gainCorrectionFolderButton->setEnabled (enabledState);

    if (searchForCorrectionFilesButton != nullptr)
        searchForCorrectionFilesButton->setEnabled (enabledState);
}

void NeuropixelsV2eProbeInterface::startAcquisition()
{
    acquisitionIsActive = true;

    setInterfaceEnabledState (false);
}

void NeuropixelsV2eProbeInterface::stopAcquisition()
{
    acquisitionIsActive = false;

    setInterfaceEnabledState (true);
}

void NeuropixelsV2eProbeInterface::drawLegend()
{
    enableViewComponent->setVisible (false);
    referenceViewComponent->setVisible (false);

    switch (mode)
    {
        case VisualizationMode::ENABLE_VIEW:
            enableViewComponent->setVisible (true);
            break;
        case VisualizationMode::REFERENCE_VIEW:
            referenceViewComponent->setVisible (true);
            break;
        default:
            break;
    }
}

void NeuropixelsV2eProbeInterface::updateSettings()
{
    if (device == nullptr)
        return;

    auto npx = std::static_pointer_cast<Neuropixels2e> (device);
    auto settings = npx->settings[probeIndex].get();

    probeBrowser->setDrawingSettings();
    applyProbeSettings (settings);
    updateChannelPresets (settings);
    updateReferences (settings);

    gainCorrectionFile->setText (npx->getGainCorrectionFile (probeIndex) == "None" ? "" : npx->getGainCorrectionFile (probeIndex), dontSendNotification);
}

bool NeuropixelsV2eProbeInterface::applyProbeSettings (ProbeSettings* p)
{
    electrodeConfigurationComboBox->setSelectedId (p->electrodeConfigurationIndex + 2, dontSendNotification);

    referenceComboBox->setSelectedId (p->referenceIndex + 1, dontSendNotification);
    auto settings = std::static_pointer_cast<Neuropixels2e> (device)->settings[probeIndex].get();

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

    drawLegend();
    repaint();

    return true;
}

int NeuropixelsV2eProbeInterface::getIndexOfComboBoxItem (ComboBox* cb, std::string item)
{
    for (int i = 0; i < cb->getNumItems(); i++)
    {
        if (item == cb->getItemText (i))
            return i;
    }

    return -1;
}

void NeuropixelsV2eProbeInterface::saveParameters (XmlElement* xml)
{
    if (device == nullptr)
        return;

    LOGD ("Saving Neuropixels 2.0e settings.");

    auto npx = std::static_pointer_cast<Neuropixels2e> (device);
    auto settings = npx->settings[probeIndex].get();

    XmlElement* xmlNode = xml->createNewChildElement ("PROBE" + std::to_string (probeIndex));

    xmlNode->setAttribute ("probeSerialNumber", std::to_string (npx->getProbeSerialNumber (probeIndex)));
    xmlNode->setAttribute ("probePartNumber", npx->getProbePartNumber (probeIndex));
    xmlNode->setAttribute ("flexPartNumber", npx->getFlexPartNumber (probeIndex));
    xmlNode->setAttribute ("flexVersion", npx->getFlexVersion (probeIndex));

    xmlNode->setAttribute ("probeType", (int) settings->probeType);

    xmlNode->setAttribute ("searchForCorrectionFiles", searchForCorrectionFilesButton->getToggleState());

    xmlNode->setAttribute ("gainCorrectionFolder", gainCorrectionFolder->getText());

    xmlNode->setAttribute ("gainCorrectionFile", npx->getGainCorrectionFile (probeIndex));

    XmlElement* probeViewerNode = xmlNode->createNewChildElement ("PROBE_VIEWER");

    probeViewerNode->setAttribute ("zoomHeight", probeBrowser->getZoomHeight());
    probeViewerNode->setAttribute ("zoomOffset", probeBrowser->getZoomOffset());

    probeViewerNode->setAttribute ("referenceChannel", referenceComboBox->getText());

    XmlElement* channelsNode = xmlNode->createNewChildElement ("SELECTED_ELECTRODES");

    for (int i = 0; i < settings->selectedElectrode.size(); i++)
    {
        int globalIndex = settings->selectedElectrode[i];

        channelsNode->setAttribute (String ("CH" + std::to_string (i)), std::to_string (globalIndex));
    }
}

void NeuropixelsV2eProbeInterface::loadParameters (XmlElement* xml)
{
    if (device == nullptr)
        return;

    LOGD ("Loading Neuropixels 2.0e settings.");

    auto npx = std::static_pointer_cast<Neuropixels2e> (device);
    auto settings = npx->settings[probeIndex].get();

    XmlElement* xmlNode = nullptr;

    for (auto* node : xml->getChildIterator())
    {
        if (node->hasTagName ("PROBE" + std::to_string (probeIndex)))
        {
            xmlNode = node;
            break;
        }
    }

    if (xmlNode == nullptr)
    {
        LOGD ("No PROBE" + std::to_string (probeIndex) + " element found");
        return;
    }

    if (npx->getProbeSerialNumber (probeIndex) != 0 && xmlNode->getIntAttribute ("probeSerialNumber") != npx->getProbeSerialNumber (probeIndex))
    {
        LOGC ("Different serial numbers found. Current serial number is " + std::to_string (npx->getProbeSerialNumber (probeIndex)) + ", while the saved serial number is " + std::to_string (xmlNode->getIntAttribute ("probeSerialNumber")) + ". Updating settings...");
    }

    settings->probeType = (ProbeType) xmlNode->getIntAttribute ("probeType", (int) ProbeType::NPX_V2_QUAD_SHANK);

    searchForCorrectionFilesButton->setToggleState (xmlNode->getBoolAttribute ("searchForCorrectionFiles", false), sendNotification);

    gainCorrectionFolder->setText (xmlNode->getStringAttribute ("gainCorrectionFolder", ""));

    npx->setGainCorrectionFile (probeIndex, xmlNode->getStringAttribute ("gainCorrectionFile").toStdString());

    XmlElement* probeViewerNode = xmlNode->getChildByName ("PROBE_VIEWER");

    if (probeViewerNode == nullptr)
    {
        LOGE ("No PROBE_VIEWER element found.");
        return;
    }

    probeBrowser->setZoomHeightAndOffset (probeViewerNode->getIntAttribute ("zoomHeight"), probeViewerNode->getIntAttribute ("zoomOffset"));

    int idx = -1;

    idx = getIndexOfComboBoxItem (referenceComboBox.get(), probeViewerNode->getStringAttribute ("referenceChannel").toStdString());
    if (idx == -1)
    {
        LOGE ("No reference channel variable found.");
    }
    else
        settings->referenceIndex = idx;

    XmlElement* channelsNode = xmlNode->getChildByName ("SELECTED_ELECTRODES");

    if (channelsNode == nullptr)
    {
        LOGE ("No SELECTED_ELECTRODES element found.");
        return;
    }

    std::vector<int> selectedChannels {};
    selectedChannels.reserve (npx->numberOfChannels);

    for (int i = 0; i < npx->numberOfChannels; i++)
    {
        std::string chIdx = channelsNode->getStringAttribute ("CH" + std::to_string (i), "").toStdString();

        if (chIdx == "")
        {
            LOGE ("Channel #" + std::to_string (i) + " not found. Channel map will not be updated.");
            return;
        }

        selectedChannels.emplace_back (std::stoi (chIdx));
    }

    std::sort (selectedChannels.begin(), selectedChannels.end());
    auto last = std::unique (selectedChannels.begin(), selectedChannels.end());
    selectedChannels.erase (last, selectedChannels.end());

    if (selectedChannels.size() != npx->numberOfChannels)
    {
        LOGE ("Invalid channel map. Wrong number of channels found. Channel map was not updated");
        return;
    }

    selectElectrodes (selectedChannels);

    updateSettings();
}

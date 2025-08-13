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

#pragma once

#include <VisualizerEditorHeaders.h>

#include "ColourScheme.h"
#include "CustomTabComponent.h"
#include "NeuropixelsV2eProbeBrowser.h"
#include "SettingsInterface.h"

namespace OnixSourcePlugin
{
class OnixSourceCanvas;
class OnixSourceEditor;

/**

    Extended graphical interface for updating probe settings

*/
class NeuropixelsV2eProbeInterface : public SettingsInterface,
                                     public Button::Listener,
                                     public ComboBox::Listener,
                                     public TextEditor::Listener
{
public:
    friend class ProbeBrowser<Neuropixels2e::numberOfChannels, Neuropixels2e::numberOfElectrodes>;
    friend class NeuropixelsV2eInterface;

    NeuropixelsV2eProbeInterface (std::shared_ptr<Neuropixels2e> d, int ind, OnixSourceEditor* e, OnixSourceCanvas* c);

    void buttonClicked (Button*) override;
    void comboBoxChanged (ComboBox*) override;
    void textEditorTextChanged (TextEditor&) override;
    void startAcquisition() override;
    void stopAcquisition() override;
    void saveParameters (XmlElement* xml) override;
    void loadParameters (XmlElement* xml) override;
    void updateInfoString() override;
    void updateSettings() override;

    bool applyProbeSettings (ProbeSettings<Neuropixels2e::numberOfChannels, Neuropixels2e::numberOfElectrodes>* p);

    void setReference (int index);
    void selectElectrodes (std::vector<int> electrodes);

    std::string getReferenceText() override { return referenceComboBox->getText().toStdString(); }

private:
    bool acquisitionIsActive = false;

    const int probeIndex;

    static constexpr char* GainCalibrationFilename = "_gainCalValues.csv";

    std::unique_ptr<ComboBox> electrodeConfigurationComboBox;
    std::unique_ptr<ComboBox> referenceComboBox;

    std::unique_ptr<Label> deviceLabel;
    std::unique_ptr<Label> infoLabel;
    std::unique_ptr<Label> electrodesLabel;
    std::unique_ptr<Label> electrodePresetLabel;
    std::unique_ptr<Label> referenceLabel;

    std::unique_ptr<Label> gainCorrectionFolderLabel;
    std::unique_ptr<ToggleButton> searchForCorrectionFilesButton;
    std::unique_ptr<FileChooser> gainCorrectionFolderChooser;
    std::unique_ptr<TextEditor> gainCorrectionFolder;
    std::unique_ptr<UtilityButton> gainCorrectionFolderButton;

    std::unique_ptr<Label> gainCorrectionFileLabel;
    std::unique_ptr<UtilityButton> gainCorrectionFileButton;
    std::unique_ptr<FileChooser> gainCorrectionFileChooser;
    std::unique_ptr<TextEditor> gainCorrectionFile;

    std::unique_ptr<UtilityButton> enableButton;

    std::unique_ptr<UtilityButton> enableViewButton;
    std::unique_ptr<UtilityButton> referenceViewButton;

    std::unique_ptr<DrawableRectangle> probeInterfaceRectangle;
    std::unique_ptr<Label> probeInterfaceLabel;
    std::unique_ptr<UtilityButton> loadJsonButton;
    std::unique_ptr<UtilityButton> saveJsonButton;

    std::unique_ptr<ProbeBrowser<Neuropixels2e::numberOfChannels, Neuropixels2e::numberOfElectrodes>> probeBrowser;

    std::unique_ptr<Component> enableViewComponent;
    std::unique_ptr<Component> referenceViewComponent;

    std::vector<std::unique_ptr<Label>> enableViewLabels;
    std::vector<std::unique_ptr<Label>> referenceViewLabels;

    std::vector<std::unique_ptr<DrawableRectangle>> enableViewRectangles;
    std::vector<std::unique_ptr<DrawableRectangle>> referenceViewRectangles;

    std::unique_ptr<UtilityButton> saveSettingsButton;
    std::unique_ptr<UtilityButton> loadSettingsButton;

    void drawLegend();

    std::vector<int> getSelectedElectrodes();

    void setInterfaceEnabledState (bool enabledState) override;

    void checkForExistingChannelPreset();

    int getIndexOfComboBoxItem (ComboBox* cb, std::string item);

    void setGainCorrectionFolderEnabledState (bool enabledState);

    static std::string searchDirectoryForCalibrationFile (std::string folder, uint64_t sn);

    JUCE_LEAK_DETECTOR (NeuropixelsV2eProbeInterface);
};
} // namespace OnixSourcePlugin

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

#include "../Devices/Neuropixels1.h"
#include "ColourScheme.h"
#include "NeuropixelsV1ProbeBrowser.h"
#include "SettingsInterface.h"

namespace OnixSourcePlugin
{
class OnixSourceEditor;
class OnixSourceCanvas;

/**

    Extended graphical interface for updating probe settings

*/
class NeuropixelsV1Interface : public SettingsInterface,
                               public Button::Listener,
                               public ComboBox::Listener,
                               public TextEditor::Listener
{
public:
    friend class ProbeBrowser;

    NeuropixelsV1Interface (std::shared_ptr<Neuropixels1> d, OnixSourceEditor* e, OnixSourceCanvas* c);

    void buttonClicked (Button*) override;
    void comboBoxChanged (ComboBox*) override;
    void textEditorTextChanged (TextEditor&) override;
    void startAcquisition() override;
    void stopAcquisition() override;
    void saveParameters (XmlElement* xml) override;
    void loadParameters (XmlElement* xml) override;
    void updateInfoString() override;
    void updateSettings() override;

    bool applyProbeSettings (ProbeSettings* p);

    void selectElectrodes (std::vector<int> electrodes);

    std::string getReferenceText() override { return referenceComboBox->getText().toStdString(); }

private:
    void setInterfaceEnabledState (bool newState) override;

    bool acquisitionIsActive = false;

    static constexpr char* GainCalibrationFilename = "_gainCalValues.csv";
    static constexpr char* AdcCalibrationFilename = "_ADCCalibration.csv";

    std::unique_ptr<ComboBox> electrodeConfigurationComboBox;
    std::unique_ptr<ComboBox> lfpGainComboBox;
    std::unique_ptr<ComboBox> apGainComboBox;
    std::unique_ptr<ComboBox> referenceComboBox;
    std::unique_ptr<ComboBox> filterComboBox;

    std::unique_ptr<Label> deviceLabel;
    std::unique_ptr<Label> infoLabel;
    std::unique_ptr<Label> lfpGainLabel;
    std::unique_ptr<Label> apGainLabel;
    std::unique_ptr<Label> electrodesLabel;
    std::unique_ptr<Label> electrodePresetLabel;
    std::unique_ptr<Label> referenceLabel;
    std::unique_ptr<Label> filterLabel;

    std::unique_ptr<Label> calibrationFolderLabel;
    std::unique_ptr<ToggleButton> searchForCalibrationFilesButton;
    std::unique_ptr<FileChooser> calibrationFolderChooser;
    std::unique_ptr<TextEditor> calibrationFolder;
    std::unique_ptr<UtilityButton> calibrationFolderButton;

    void setCalibrationFolderEnabledState (bool enabledState);

    static std::string searchDirectoryForGainCalibrationFile (std::string folder, uint64_t sn);
    static std::string searchDirectoryForAdcCalibrationFile (std::string folder, uint64_t sn);
    static std::string searchDirectoryForCalibrationFile (std::string folder, std::string filename, uint64_t sn);

    void searchForCalibrationFiles (std::string, uint64_t);

    std::unique_ptr<Label> adcCalibrationFileLabel;
    std::unique_ptr<Label> gainCalibrationFileLabel;

    std::unique_ptr<UtilityButton> deviceEnableButton;
    std::unique_ptr<UtilityButton> selectElectrodeButton;

    std::unique_ptr<UtilityButton> enableViewButton;
    std::unique_ptr<UtilityButton> lfpGainViewButton;
    std::unique_ptr<UtilityButton> apGainViewButton;
    std::unique_ptr<UtilityButton> referenceViewButton;

    std::unique_ptr<DrawableRectangle> probeInterfaceRectangle;
    std::unique_ptr<Label> probeInterfaceLabel;
    std::unique_ptr<UtilityButton> loadJsonButton;
    std::unique_ptr<UtilityButton> saveJsonButton;

    std::unique_ptr<UtilityButton> adcCalibrationFileButton;
    std::unique_ptr<UtilityButton> gainCalibrationFileButton;

    std::unique_ptr<TextEditor> adcCalibrationFile;
    std::unique_ptr<TextEditor> gainCalibrationFile;

    std::unique_ptr<FileChooser> adcCalibrationFileChooser;
    std::unique_ptr<FileChooser> gainCalibrationFileChooser;

    std::unique_ptr<NeuropixelsV1ProbeBrowser> probeBrowser;

    std::unique_ptr<Component> enableViewComponent;
    std::unique_ptr<Component> apGainViewComponent;
    std::unique_ptr<Component> lfpGainViewComponent;
    std::unique_ptr<Component> referenceViewComponent;

    std::vector<std::unique_ptr<Label>> enableViewLabels;
    std::vector<std::unique_ptr<Label>> apGainViewLabels;
    std::vector<std::unique_ptr<Label>> lfpGainViewLabels;
    std::vector<std::unique_ptr<Label>> referenceViewLabels;

    std::vector<std::unique_ptr<DrawableRectangle>> enableViewRectangles;
    std::vector<std::unique_ptr<DrawableRectangle>> apGainViewRectangles;
    std::vector<std::unique_ptr<DrawableRectangle>> lfpGainViewRectangles;
    std::vector<std::unique_ptr<DrawableRectangle>> referenceViewRectangles;

    std::unique_ptr<UtilityButton> saveSettingsButton;
    std::unique_ptr<UtilityButton> loadSettingsButton;

    void drawLegend();

    std::vector<int> getSelectedElectrodes() const;

    /** Checks if the current channel map matches an existing channel preset, and updates the combo box if it does */
    void checkForExistingChannelPreset();

    int getIndexOfComboBoxItem (ComboBox* cb, std::string item);

    JUCE_LEAK_DETECTOR (NeuropixelsV1Interface);
};
} // namespace OnixSourcePlugin

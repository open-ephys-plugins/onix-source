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
#include "SettingsInterface.h"
#include "NeuropixelsV1fProbeBrowser.h"

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

/**

	Extended graphical interface for updating probe settings

*/
class NeuropixV1Interface : public SettingsInterface,
	public Button::Listener,
	public ComboBox::Listener,
	public TextEditor::Listener
{
public:
	friend class ProbeBrowser<Neuropixels_1::numberOfChannels, Neuropixels_1::numberOfElectrodes>;

	/** Constructor */
	NeuropixV1Interface(std::shared_ptr<Neuropixels_1> d, OnixSourceEditor* e, OnixSourceCanvas* c);

	/** Listener methods*/
	void buttonClicked(Button*) override;
	void comboBoxChanged(ComboBox*) override;

	/** Disables buttons and starts animation if necessary */
	void startAcquisition() override;

	/** Enables buttons and start animation if necessary */
	void stopAcquisition() override;

	/** Settings-related functions*/
	bool applyProbeSettings(ProbeSettings<Neuropixels_1::numberOfChannels, Neuropixels_1::numberOfElectrodes>* p, bool shouldUpdateProbe = true);

	/** Save parameters to XML */
	void saveParameters(XmlElement* xml) override;

	/** Load parameters from XML */
	void loadParameters(XmlElement* xml) override;

	/** Updates the info string on the right-hand side of the component */
	void updateInfoString() override;

	/** Set parameters */
	void setApGain(int index);
	void setLfpGain(int index);
	void setReference(int index);
	void setApFilterState(bool state);
	void selectElectrodes(std::vector<int> electrodes);

	String getReferenceText() override { return referenceComboBox->getText(); }

private:
	XmlElement neuropix_info;

	bool acquisitionIsActive = false;

	std::unique_ptr<ComboBox> electrodeConfigurationComboBox;
	std::unique_ptr<ComboBox> lfpGainComboBox;
	std::unique_ptr<ComboBox> apGainComboBox;
	std::unique_ptr<ComboBox> referenceComboBox;
	std::unique_ptr<ComboBox> filterComboBox;
	std::unique_ptr<ComboBox> activityViewComboBox;

	std::unique_ptr<Label> deviceLabel;
	std::unique_ptr<Label> infoLabel;
	std::unique_ptr<Label> lfpGainLabel;
	std::unique_ptr<Label> apGainLabel;
	std::unique_ptr<Label> electrodesLabel;
	std::unique_ptr<Label> electrodePresetLabel;
	std::unique_ptr<Label> referenceLabel;
	std::unique_ptr<Label> filterLabel;
	std::unique_ptr<Label> activityViewLabel;

	std::unique_ptr<Label> adcCalibrationFileLabel;
	std::unique_ptr<Label> gainCalibrationFileLabel;

	std::unique_ptr<UtilityButton> deviceEnableButton;
	std::unique_ptr<UtilityButton> selectElectrodeButton;

	std::unique_ptr<UtilityButton> enableViewButton;
	std::unique_ptr<UtilityButton> lfpGainViewButton;
	std::unique_ptr<UtilityButton> apGainViewButton;
	std::unique_ptr<UtilityButton> referenceViewButton;
	std::unique_ptr<UtilityButton> activityViewButton;

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

	std::unique_ptr<NeuropixelsV1fProbeBrowser> probeBrowser;

	std::unique_ptr<ToggleButton> offsetCorrectionCheckbox;

	std::unique_ptr<Component> enableViewComponent;
	std::unique_ptr<Component> apGainViewComponent;
	std::unique_ptr<Component> lfpGainViewComponent;
	std::unique_ptr<Component> referenceViewComponent;
	std::unique_ptr<Component> activityViewComponent;

	std::vector<std::unique_ptr<Label>> enableViewLabels;
	std::vector<std::unique_ptr<Label>> apGainViewLabels;
	std::vector<std::unique_ptr<Label>> lfpGainViewLabels;
	std::vector<std::unique_ptr<Label>> referenceViewLabels;
	std::vector<std::unique_ptr<Label>> activityViewLabels;

	std::vector<std::unique_ptr<DrawableRectangle>> enableViewRectangles;
	std::vector<std::unique_ptr<DrawableRectangle>> apGainViewRectangles;
	std::vector<std::unique_ptr<DrawableRectangle>> lfpGainViewRectangles;
	std::vector<std::unique_ptr<DrawableRectangle>> referenceViewRectangles;
	std::vector<std::unique_ptr<DrawableRectangle>> activityViewRectangles;

	void drawLegend();

	std::vector<int> getSelectedElectrodes() const;

	void setInterfaceEnabledState(bool enabledState);

	/** Checks if the current channel map matches an existing channel preset, and updates the combo box if it does */
	void checkForExistingChannelPreset();

	JUCE_LEAK_DETECTOR(NeuropixV1Interface);
};

/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2020 Allen Institute for Brain Science and Open Ephys

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
#include "NeuropixelsV2eProbeBrowser.h"
#include "CustomTabComponent.h"

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

	/** Constructor */
	NeuropixelsV2eProbeInterface(std::shared_ptr<Neuropixels2e> d, int ind, OnixSourceEditor* e, OnixSourceCanvas* c);

	/** Listener methods*/
	void buttonClicked(Button*) override;
	void comboBoxChanged(ComboBox*) override;

	/** Disables buttons and starts animation if necessary */
	void startAcquisition() override;

	/** Enables buttons and start animation if necessary */
	void stopAcquisition() override;

	/** Settings-related functions*/
	bool applyProbeSettings(ProbeSettings<Neuropixels2e::numberOfChannels, Neuropixels2e::numberOfElectrodes>* p, bool shouldUpdateProbe = true);

	/** Save parameters to XML */
	void saveParameters(XmlElement* xml) override;

	/** Load parameters from XML */
	void loadParameters(XmlElement* xml) override;

	/** Updates the info string on the right-hand side of the component */
	void updateInfoString() override;

	/** Set parameters */
	void setReference(int index);
	void selectElectrodes(std::vector<int> electrodes);

private:
	XmlElement neuropix_info;

	bool acquisitionIsActive = false;

	const int probeIndex;

	// Combo box - probe-specific settings
	std::unique_ptr<ComboBox> electrodeConfigurationComboBox;
	std::unique_ptr<ComboBox> referenceComboBox;
	std::unique_ptr<ComboBox> activityViewComboBox;

	// LABELS
	std::unique_ptr<Label> nameLabel;
	std::unique_ptr<Label> infoLabel;
	std::unique_ptr<Label> electrodesLabel;
	std::unique_ptr<Label> electrodePresetLabel;
	std::unique_ptr<Label> referenceLabel;
	std::unique_ptr<Label> activityViewLabel;

	std::unique_ptr<Label> gainCorrectionFileLabel;

	// BUTTONS
	std::unique_ptr<UtilityButton> enableButton;

	std::unique_ptr<UtilityButton> enableViewButton;
	std::unique_ptr<UtilityButton> referenceViewButton;
	std::unique_ptr<UtilityButton> activityViewButton;

	std::unique_ptr<UtilityButton> loadJsonButton;
	std::unique_ptr<UtilityButton> saveJsonButton;

	std::unique_ptr<UtilityButton> gainCorrectionFileButton;
	std::unique_ptr<FileChooser> gainCorrectionFileChooser;
	std::unique_ptr<TextEditor> gainCorrectionFile;

	std::unique_ptr<ProbeBrowser<Neuropixels2e::numberOfChannels, Neuropixels2e::numberOfElectrodes>> probeBrowser;

	std::unique_ptr<Component> enableViewComponent;
	std::unique_ptr<Component> referenceViewComponent;
	std::unique_ptr<Component> activityViewComponent;

	std::vector<std::unique_ptr<Label>> enableViewLabels;
	std::vector<std::unique_ptr<Label>> referenceViewLabels;
	std::vector<std::unique_ptr<Label>> activityViewLabels;

	std::vector<std::unique_ptr<DrawableRectangle>> enableViewRectangles;
	std::vector<std::unique_ptr<DrawableRectangle>> referenceViewRectangles;
	std::vector<std::unique_ptr<DrawableRectangle>> activityViewRectangles;

	void drawLegend();

	std::vector<int> getSelectedElectrodes();

	void setInterfaceEnabledState(bool enabledState);

	/** Checks if the current channel map matches an existing channel preset, and updates the combo box if it does */
	void checkForExistingChannelPreset();

	JUCE_LEAK_DETECTOR(NeuropixelsV2eProbeInterface);
};

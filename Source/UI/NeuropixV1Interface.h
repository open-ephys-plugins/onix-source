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

#ifndef __NEUROPIXINTERFACE_H__
#define __NEUROPIXINTERFACE_H__

#include <VisualizerEditorHeaders.h>

#include "../Devices/Neuropixels_1.h"
#include "ColourScheme.h"
#include "SettingsInterface.h"

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

class ProbeBrowser;

enum class VisualizationMode
{
	ENABLE_VIEW,
	AP_GAIN_VIEW,
	LFP_GAIN_VIEW,
	REFERENCE_VIEW,
	ACTIVITY_VIEW
};

class Annotation
{
public:
	Annotation(String text, Array<int> channels, Colour c);
	~Annotation();

	Array<int> electrodes;
	String text;

	float currentYLoc;

	bool isMouseOver;
	bool isSelected;

	Colour colour;
};

/**

	Extended graphical interface for updating probe settings

*/
class NeuropixV1Interface : public SettingsInterface,
	public Button::Listener,
	public ComboBox::Listener,
	public TextEditor::Listener
{
public:
	friend class ProbeBrowser;

	/** Constructor */
	NeuropixV1Interface(std::shared_ptr<Neuropixels_1> d, OnixSourceEditor* e, OnixSourceCanvas* c);

	/** Destructor */
	~NeuropixV1Interface();

	/** Draws the legend */
	void paint(Graphics& g) override;

	/** Listener methods*/
	void buttonClicked(Button*) override;
	void comboBoxChanged(ComboBox*) override;

	/** Disables buttons and starts animation if necessary */
	void startAcquisition() override;

	/** Enables buttons and start animation if necessary */
	void stopAcquisition() override;

	/** Settings-related functions*/
	bool applyProbeSettings(ProbeSettings* p, bool shouldUpdateProbe = true);
	ProbeSettings* getProbeSettings() const;

	/** Save parameters to XML */
	void saveParameters(XmlElement* xml) override;

	/** Load parameters from XML */
	void loadParameters(XmlElement* xml) override;

	/** Updates the annotation label */
	void setAnnotationLabel(String, Colour);

	/** Updates the info string on the right-hand side of the component */
	void updateInfoString() override;

	/** Set parameters */
	void setApGain(int index);
	void setLfpGain(int index);
	void setReference(int index);
	void setApFilterState(bool state);
	void selectElectrodes(Array<int> electrodes);

private:
	XmlElement neuropix_info;

	bool acquisitionIsActive = false;

	// Combo box - probe-specific settings
	std::unique_ptr<ComboBox> electrodeConfigurationComboBox;
	std::unique_ptr<ComboBox> lfpGainComboBox;
	std::unique_ptr<ComboBox> apGainComboBox;
	std::unique_ptr<ComboBox> referenceComboBox;
	std::unique_ptr<ComboBox> filterComboBox;
	std::unique_ptr<ComboBox> activityViewComboBox;

	// LABELS
	std::unique_ptr<Label> nameLabel;
	std::unique_ptr<Label> infoLabel;
	std::unique_ptr<Label> lfpGainLabel;
	std::unique_ptr<Label> apGainLabel;
	std::unique_ptr<Label> electrodesLabel;
	std::unique_ptr<Label> electrodePresetLabel;
	std::unique_ptr<Label> referenceLabel;
	std::unique_ptr<Label> filterLabel;
	std::unique_ptr<Label> activityViewLabel;

	std::unique_ptr<Label> probeSettingsLabel;

	std::unique_ptr<Label> annotationLabelLabel;
	std::unique_ptr<Label> annotationLabel;

	std::unique_ptr<Label> adcCalibrationFileLabel;
	std::unique_ptr<Label> gainCalibrationFileLabel;

	// BUTTONS
	std::unique_ptr<UtilityButton> probeEnableButton;
	std::unique_ptr<UtilityButton> enableButton;

	std::unique_ptr<UtilityButton> enableViewButton;
	std::unique_ptr<UtilityButton> lfpGainViewButton;
	std::unique_ptr<UtilityButton> apGainViewButton;
	std::unique_ptr<UtilityButton> referenceViewButton;
	std::unique_ptr<UtilityButton> bankViewButton;
	std::unique_ptr<UtilityButton> activityViewButton;

	std::unique_ptr<UtilityButton> loadJsonButton;
	std::unique_ptr<UtilityButton> saveJsonButton;

	std::unique_ptr<UtilityButton> adcCalibrationFileButton;
	std::unique_ptr<UtilityButton> gainCalibrationFileButton;

	std::unique_ptr<TextEditor> adcCalibrationFile;
	std::unique_ptr<TextEditor> gainCalibrationFile;

	std::unique_ptr<FileChooser> adcCalibrationFileChooser;
	std::unique_ptr<FileChooser> gainCalibrationFileChooser;

	std::unique_ptr<ProbeBrowser> probeBrowser;

	VisualizationMode mode;

	void drawLegend(Graphics& g);

	Array<Annotation> annotations;

	Array<int> getSelectedElectrodes() const;

	void setInterfaceEnabledState(bool enabledState);
};

#endif //__NEUROPIXINTERFACE_H__

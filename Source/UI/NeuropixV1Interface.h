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

#include "../NeuropixComponents.h"
#include "ColourScheme.h"
#include "SettingsInterface.h"

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

class ProbeBrowser;

enum VisualizationMode
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
	public ComboBox::Listener
{
public:
	friend class ProbeBrowser;

	/** Constructor */
	NeuropixV1Interface(OnixDevice* probe, OnixSourceEditor* editor, OnixSourceCanvas* canvas);

	/** Destructor */
	~NeuropixV1Interface();

	/** Draws the legend */
	void paint(Graphics& g);

	/** Listener methods*/
	void buttonClicked(Button*);
	void comboBoxChanged(ComboBox*);

	/** Disables buttons and starts animation if necessary */
	void startAcquisition();

	/** Enables buttons and start animation if necessary */
	void stopAcquisition();

	/** Settings-related functions*/
	bool applyProbeSettings(ProbeSettings p, bool shouldUpdateProbe = true);
	ProbeSettings getProbeSettings();
	void updateProbeSettings();

	/** Save parameters to XML */
	void saveParameters(XmlElement* xml);

	/** Load parameters from XML */
	void loadParameters(XmlElement* xml);

	/** Updates the annotation label */
	void setAnnotationLabel(String, Colour);

	/** Updates the info string on the right-hand side of the component */
	void updateInfoString();

	/** Set parameters */
	void setApGain(int index);
	void setLfpGain(int index);
	void setReference(int index);
	void setApFilterState(bool state);
	void selectElectrodes(Array<int> electrodes);

	Neuropixels_1* device;

private:
	Array<ElectrodeMetadata> electrodeMetadata;
	ProbeMetadata probeMetadata;

	XmlElement neuropix_info;

	bool acquisitionIsActive = false;

	// Combo box - probe-specific settings
	std::unique_ptr<ComboBox> electrodeConfigurationComboBox;
	std::unique_ptr<ComboBox> lfpGainComboBox;
	std::unique_ptr<ComboBox> apGainComboBox;
	std::unique_ptr<ComboBox> referenceComboBox;
	std::unique_ptr<ComboBox> filterComboBox;
	std::unique_ptr<ComboBox> activityViewComboBox;

	// Combo box - probe settings
	std::unique_ptr<ComboBox> loadImroComboBox;

	// LABELS
	std::unique_ptr<Viewport> infoLabelView;
	std::unique_ptr<Label> nameLabel;
	std::unique_ptr<Label> infoLabel;
	std::unique_ptr<Label> lfpGainLabel;
	std::unique_ptr<Label> apGainLabel;
	std::unique_ptr<Label> electrodesLabel;
	std::unique_ptr<Label> electrodePresetLabel;
	std::unique_ptr<Label> referenceLabel;
	std::unique_ptr<Label> filterLabel;
	std::unique_ptr<Label> bankViewLabel;
	std::unique_ptr<Label> activityViewLabel;

	std::unique_ptr<Label> probeSettingsLabel;

	std::unique_ptr<Label> annotationLabelLabel;
	std::unique_ptr<Label> annotationLabel;

	// BUTTONS
	std::unique_ptr<UtilityButton> probeEnableButton;
	std::unique_ptr<UtilityButton> enableButton;

	std::unique_ptr<UtilityButton> enableViewButton;
	std::unique_ptr<UtilityButton> lfpGainViewButton;
	std::unique_ptr<UtilityButton> apGainViewButton;
	std::unique_ptr<UtilityButton> referenceViewButton;
	std::unique_ptr<UtilityButton> bankViewButton;
	std::unique_ptr<UtilityButton> activityViewButton;

	std::unique_ptr<UtilityButton> copyButton;
	std::unique_ptr<UtilityButton> pasteButton;
	std::unique_ptr<UtilityButton> loadImroButton;
	std::unique_ptr<UtilityButton> saveImroButton;
	std::unique_ptr<UtilityButton> loadJsonButton;
	std::unique_ptr<UtilityButton> saveJsonButton;

	std::unique_ptr<ProbeBrowser> probeBrowser;

	VisualizationMode mode;

	void drawLegend(Graphics& g);

	Array<Annotation> annotations;

	Array<int> getSelectedElectrodes();

	Array<String> imroFiles;
	Array<bool> imroLoadedFromFolder;
};

class AnnotationColourSelector : public Component, public Button::Listener
{
public:
	AnnotationColourSelector(NeuropixV1Interface* np);
	~AnnotationColourSelector();

	Array<Colour> standardColours;
	Array<Colour> hoverColours;
	StringArray strings;

	OwnedArray<ShapeButton> buttons;

	void buttonClicked(Button* button);

	void updateCurrentString(String s);

	Colour getCurrentColour() const;

	NeuropixV1Interface* npi;

	int activeButton;
};

#endif //__NEUROPIXINTERFACE_H__
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

#ifndef __ANALOGIOINTERFACE_H__
#define __ANALOGIOINTERFACE_H__

#include <VisualizerEditorHeaders.h>

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

#include "../Devices/AnalogIO.h"

class AnalogIOInterface : public SettingsInterface,
	public Button::Listener,
	public ComboBox::Listener
{
public:
	/** Constructor */
	AnalogIOInterface(std::shared_ptr<AnalogIO> d, OnixSourceEditor* e, OnixSourceCanvas* c);

	/** Disables buttons and starts animation if necessary */
	void startAcquisition() override {};

	/** Enables buttons and start animation if necessary */
	void stopAcquisition() override {};

	/** Save parameters to XML */
	void saveParameters(XmlElement* xml) override {};

	/** Load parameters from XML */
	void loadParameters(XmlElement* xml) override {};

	/** Updates the info string on the right-hand side of the component */
	void updateInfoString() override;

	/** Listener methods*/
	void buttonClicked(Button*) override;
	void comboBoxChanged(ComboBox* cb) override;

private:

	static const int numChannels = 12;

	std::unique_ptr<Label> nameLabel;
	std::unique_ptr<Label> infoLabel;

	std::unique_ptr<Label> dataTypeLabel;
	std::unique_ptr<ComboBox> dataTypeComboBox;

	std::array<std::unique_ptr<Label>, numChannels> voltageRangeLabels;
	std::array<std::unique_ptr<ComboBox>, numChannels> voltageRangeComboBoxes;
	std::array<std::unique_ptr<Label>, numChannels> channelDirectionLabels;
	std::array<std::unique_ptr<ComboBox>, numChannels> channelDirectionComboBoxes;

	std::unique_ptr<UtilityButton> deviceEnableButton;

	static int getChannelDirectionId(std::shared_ptr<AnalogIO> device, int channelNumber);
	static int getChannelVoltageRangeId(std::shared_ptr<AnalogIO> device, int channelNumber);
	static int getDataTypeId(std::shared_ptr<AnalogIO> device);

	JUCE_LEAK_DETECTOR(AnalogIOInterface);
};

#endif // !__AnalogIOInterface_H__

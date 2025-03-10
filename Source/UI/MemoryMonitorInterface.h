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

#ifndef MEMORYMONITORINTERFACE_H
#define MEMORYMONITORINTERFACE_H

#include <VisualizerEditorHeaders.h>

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

#include "../Devices/MemoryMonitor.h"

class MemoryMonitorInterface : public SettingsInterface,
	public Label::Listener,
	public Button::Listener
{
public:

	MemoryMonitorInterface(std::shared_ptr<MemoryMonitor> d, OnixSourceEditor* e, OnixSourceCanvas* c);

	~MemoryMonitorInterface();

	/** Disables buttons and starts animation if necessary */
	void startAcquisition() override;

	/** Enables buttons and start animation if necessary */
	void stopAcquisition() override;

	/** Save parameters to XML */
	void saveParameters(XmlElement* xml) override;

	/** Load parameters from XML */
	void loadParameters(XmlElement* xml) override;

	/** Updates the info string on the right-hand side of the component */
	void updateInfoString() override;

	/** Listener methods*/
	void buttonClicked(Button*) override;
	void labelTextChanged(Label* l) override;

private:

	std::unique_ptr<Label> nameLabel;
	std::unique_ptr<Label> infoLabel;

	std::unique_ptr<Label> sampleRateLabel;
	std::unique_ptr<Label> sampleRateValue;

	std::unique_ptr<UtilityButton> deviceEnableButton;
};

#endif

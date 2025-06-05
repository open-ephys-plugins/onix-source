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

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

#include "../Devices/AnalogIO.h"

namespace OnixSourcePlugin
{
	class AnalogIOInterface : public SettingsInterface,
		public Button::Listener,
		public ComboBox::Listener
	{
	public:
		AnalogIOInterface(std::shared_ptr<AnalogIO> d, OnixSourceEditor* e, OnixSourceCanvas* c);

		void saveParameters(XmlElement* xml) override;

		void loadParameters(XmlElement* xml) override;

		void updateInfoString() override {};

		void updateSettings() override;

		void buttonClicked(Button*) override;
		void comboBoxChanged(ComboBox* cb) override;

	private:

		void setInterfaceEnabledState(bool newState) override;

		static const int numChannels = 12;

		std::array<std::unique_ptr<Label>, numChannels> channelDirectionLabels;
		std::array<std::unique_ptr<ComboBox>, numChannels> channelDirectionComboBoxes;

		std::unique_ptr<UtilityButton> deviceEnableButton;

		static int getChannelDirectionId(std::shared_ptr<AnalogIO> device, int channelNumber);
		static int getChannelVoltageRangeId(std::shared_ptr<AnalogIO> device, int channelNumber);
		static int getDataTypeId(std::shared_ptr<AnalogIO> device);
		static AnalogIODirection getChannelDirectionFromString(std::string direction);

		JUCE_LEAK_DETECTOR(AnalogIOInterface);
	};
}

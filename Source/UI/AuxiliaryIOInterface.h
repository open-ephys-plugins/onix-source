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
#include "SettingsInterface.h"
#include "CustomViewport.h"

#include "../Devices/AuxiliaryIO.h"

namespace OnixSourcePlugin
{
	class OnixSourceEditor;
	class OnixSourceCanvas;

	class AnalogIOInterface;
	class DigitalIOInterface;

	class AuxiliaryIOInterface : public SettingsInterface
	{
	public:

		AuxiliaryIOInterface(std::shared_ptr<AuxiliaryIO> d, OnixSourceEditor* e, OnixSourceCanvas* c);

		void saveParameters(XmlElement* xml) override;
		void loadParameters(XmlElement* xml) override;
		void updateInfoString() override;
		void updateSettings() override;

	private:

		void setInterfaceEnabledState(bool newState) override;

		std::unique_ptr<CustomViewport> analogViewport;
		std::unique_ptr<CustomViewport> digitalViewport;

		std::unique_ptr<AnalogIOInterface> analogInterface;
		std::unique_ptr<DigitalIOInterface> digitalInterface;

		std::unique_ptr<Label> analogDigitalLabel;

		JUCE_LEAK_DETECTOR(AuxiliaryIOInterface);
	};
}

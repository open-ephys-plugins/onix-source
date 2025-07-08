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

#include "AuxiliaryIOInterface.h"
#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"
#include "AnalogIOInterface.h"
#include "DigitalIOInterface.h"

using namespace OnixSourcePlugin;

AuxiliaryIOInterface::AuxiliaryIOInterface(std::shared_ptr<AuxiliaryIO> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	if (device != nullptr)
	{
		auto auxiliaryIO = std::static_pointer_cast<AuxiliaryIO>(device);

		static int offset = 55;
		FontOptions font = FontOptions("Fira Code", "Bold", 22.0f);

		analogDigitalLabel = std::make_unique<Label>("analogDigitalLabel", "Analog and Digital IO");
		analogDigitalLabel->setBounds(20, 20, 350, 35);
		analogDigitalLabel->setFont(font);
		addAndMakeVisible(analogDigitalLabel.get());

		analogInterface = std::make_unique<AnalogIOInterface>(auxiliaryIO->getAnalogIO(), e, c);
		analogViewport = std::make_unique<CustomViewport>(analogInterface.get(), SettingsInterface::Width / 2, SettingsInterface::Height);
		analogViewport->setBounds(0, offset, SettingsInterface::Width / 2, SettingsInterface::Height);
		addAndMakeVisible(analogViewport.get());

		digitalInterface = std::make_unique<DigitalIOInterface>(auxiliaryIO->getDigitalIO(), e, c);
		digitalViewport = std::make_unique<CustomViewport>(digitalInterface.get(), SettingsInterface::Width / 2, SettingsInterface::Height);
		digitalViewport->setBounds(SettingsInterface::Width / 2, offset, SettingsInterface::Width / 2, SettingsInterface::Height);
		//addAndMakeVisible(digitalViewport.get()); // NB: Hide digital interface for now, if the digitalIO UI gets updates that need to be displayed they can be shown here
	}
}

void AuxiliaryIOInterface::setInterfaceEnabledState(bool newState)
{
	analogInterface->setInterfaceEnabledState(newState);
	digitalInterface->setInterfaceEnabledState(newState);
}

void AuxiliaryIOInterface::saveParameters(XmlElement* xml)
{
	analogInterface->saveParameters(xml);
	digitalInterface->saveParameters(xml);
}

void AuxiliaryIOInterface::loadParameters(XmlElement* xml)
{
	analogInterface->loadParameters(xml);
	digitalInterface->loadParameters(xml);
}

void AuxiliaryIOInterface::updateInfoString()
{
	analogInterface->updateInfoString();
	digitalInterface->updateInfoString();
}

void AuxiliaryIOInterface::updateSettings()
{
	analogInterface->updateSettings();
	digitalInterface->updateSettings();
}

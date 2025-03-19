/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright(C) 2020 Allen Institute for Brain Science and Open Ephys

	------------------------------------------------------------------

	This program is free software : you can redistribute it and /or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < http://www.gnu.org/licenses/>.

*/

#include "Bno055Interface.h"

Bno055Interface::Bno055Interface(std::shared_ptr<Bno055> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	device = d;

	if (device != nullptr)
	{
		deviceEnableButton = std::make_unique<UtilityButton>("ENABLED");
		deviceEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
		deviceEnableButton->setRadius(3.0f);
		deviceEnableButton->setBounds(35, 35, 100, 22);
		deviceEnableButton->setClickingTogglesState(true);
		deviceEnableButton->setToggleState(device->isEnabled(), dontSendNotification);
		deviceEnableButton->setTooltip("If disabled, BNO055 device will not stream data during acquisition");
		deviceEnableButton->addListener(this);
		addAndMakeVisible(deviceEnableButton.get());
	}

	type = SettingsInterface::Type::BNO055_SETTINGS_INTERFACE;
}

void Bno055Interface::buttonClicked(Button* button)
{
	if (button == deviceEnableButton.get())
	{
		device->setEnabled(deviceEnableButton->getToggleState());
		device->configureDevice();
		if (canvas->foundInputSource()) canvas->resetContext();

		if (device->isEnabled())
		{
			deviceEnableButton->setLabel("ENABLED");
		}
		else
		{
			deviceEnableButton->setLabel("DISABLED");
		}

		CoreServices::updateSignalChain(editor);
	}
}

void Bno055Interface::startAcquisition()
{
}

void Bno055Interface::stopAcquisition()
{
}

void Bno055Interface::saveParameters(XmlElement* xml)
{
}

void Bno055Interface::loadParameters(XmlElement* xml)
{
}

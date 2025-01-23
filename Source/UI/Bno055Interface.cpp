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

Bno055Interface::Bno055Interface(OnixDevice* d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e,c),
	device((Bno055*)d)
{
	if (device != nullptr)
	{
		// TODO: Show something in the canvas that indicates the device is connected
	}

	type = SettingsInterface::BNO055_SETTINGS_INTERFACE;
}

Bno055Interface::~Bno055Interface()
{
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

void Bno055Interface::updateInfoString()
{
}

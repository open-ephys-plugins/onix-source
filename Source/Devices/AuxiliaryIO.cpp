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

#include "AuxiliaryIO.h"
#include "AnalogIO.h"
#include "DigitalIO.h"

using namespace OnixSourcePlugin;

AuxiliaryIO::AuxiliaryIO(std::string name, std::string hubName, const oni_dev_idx_t analogIndex, const oni_dev_idx_t digitalIndex, std::shared_ptr<Onix1> oni_ctx)
	: CompositeDevice(name, hubName, AuxiliaryIO::getCompositeDeviceType(), createAuxiliaryIODevices(hubName, analogIndex, digitalIndex, oni_ctx), oni_ctx)
{
	analogIO = getDevice<AnalogIO>(OnixDeviceType::ANALOGIO);
	digitalIO = getDevice<DigitalIO>(OnixDeviceType::DIGITALIO);
}

// NB: This constructor assumes that the digitalIO device is located at one index above the analogIndex
AuxiliaryIO::AuxiliaryIO(std::string name, std::string hubName, const oni_dev_idx_t analogIndex, std::shared_ptr<Onix1> oni_ctx)
	: AuxiliaryIO(name, hubName, analogIndex, analogIndex + 1, oni_ctx)
{
}

OnixDeviceVector AuxiliaryIO::createAuxiliaryIODevices(std::string hubName, const oni_dev_idx_t analogIndex, const oni_dev_idx_t digitalIndex, std::shared_ptr<Onix1> oni_ctx)
{
	OnixDeviceVector devices;

	devices.emplace_back(std::make_shared<AnalogIO>("AnalogIO", hubName, analogIndex, oni_ctx));
	devices.emplace_back(std::make_shared<DigitalIO>("DigitalIO", hubName, digitalIndex, oni_ctx));

	return devices;
}

bool AuxiliaryIO::isEnabled() const
{
	return analogIO->isEnabled();
}

void AuxiliaryIO::processFrames()
{
	if (!digitalIO->isEnabled() && !analogIO->isEnabled())
	{
		return;
	}
	else if (!digitalIO->isEnabled())
	{
		analogIO->processFrames();
		return;
	}
	else if (!analogIO->isEnabled())
	{
		digitalIO->processFrames();
		while (digitalIO->hasEventWord())
		{
			digitalIO->getEventWord();
		}

		return;
	}

	digitalIO->processFrames();

	while (analogIO->getNumberOfFrames() >= AnalogIO::framesToAverage && digitalIO->getNumberOfWords() >= 1)
	{
		auto eventWord = digitalIO->getEventWord();

		for (int i = 0; i < AnalogIO::framesToAverage; i++)
		{
			analogIO->processFrame(eventWord);
		}

		digitalIO->processFrames();
	}
}

OnixDeviceType AuxiliaryIO::getDeviceType()
{
	return OnixDeviceType::COMPOSITE;
}

CompositeDeviceType AuxiliaryIO::getCompositeDeviceType()
{
	return CompositeDeviceType::AUXILIARYIO;
}

std::shared_ptr<AnalogIO> AuxiliaryIO::getAnalogIO()
{
	return analogIO;
}

std::shared_ptr<DigitalIO> AuxiliaryIO::getDigitalIO()
{
	return digitalIO;
}

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

#include "../OnixDevice.h"

namespace OnixSourcePlugin
{
	class AnalogIO;
	class DigitalIO;

	/*
		Abstract device that configures and streams data from both an AnalogIO device and a DigitalIO device on a Breakout Board
	*/
	class AuxiliaryIO : public CompositeDevice
	{
	public:

		AuxiliaryIO(std::string name, std::string hubName, const oni_dev_idx_t analogIndex, std::shared_ptr<Onix1> oni_ctx);

		AuxiliaryIO(std::string name, std::string hubName, const oni_dev_idx_t analogIndex, const oni_dev_idx_t digitalIndex, std::shared_ptr<Onix1> oni_ctx);

		static OnixDeviceType getDeviceType();
		static CompositeDeviceType getCompositeDeviceType();

		std::shared_ptr<AnalogIO> getAnalogIO();
		std::shared_ptr<DigitalIO> getDigitalIO();

		void processFrames() override;

	private:

		static OnixDeviceVector createAuxiliaryIODevices(std::string hubName, const oni_dev_idx_t analogIndex, const oni_dev_idx_t digitalIndex, std::shared_ptr<Onix1> oni_ctx);

		JUCE_LEAK_DETECTOR(AuxiliaryIO);
	};
}

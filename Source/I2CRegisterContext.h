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

#include "ProcessorHeaders.h"

#include "Onix1.h"
#include "Devices/DS90UB9x.h"

#include <cstddef>

namespace OnixSourcePlugin
{
	class I2CRegisterContext
	{
	public:

		I2CRegisterContext(uint32_t address, const oni_dev_idx_t, std::shared_ptr<Onix1>);

		int writeByte(uint32_t address, uint32_t value, bool sixteenBitAddress = false);

		int readByte(uint32_t address, oni_reg_val_t* value, bool sixteenBitAddress = false);
		int readBytes(uint32_t address, int count, std::vector<oni_reg_val_t>& value, bool sixteenBitAddress = false);
		int readWord(uint32_t address, uint32_t numBytes, uint32_t* value, bool sixteenBitAddress = false);

		int readString(uint32_t address, int count, std::string& str, bool sixteenBitAddress = false);

		int set933I2cRate(double);

		oni_dev_idx_t getDeviceIndex() const;

	protected:
		std::shared_ptr<Onix1> i2cContext;

	private:

		const oni_dev_idx_t deviceIndex;
		const uint32_t i2cAddress;

		JUCE_LEAK_DETECTOR(I2CRegisterContext);
	};
}

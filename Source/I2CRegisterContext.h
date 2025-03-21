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

#ifndef __I2CRegisterContext_H__
#define __I2CRegisterContext_H__

#include "ProcessorHeaders.h"

#include "Onix1.h"

#include <cstddef>

class I2CRegisterContext
{
public:

	I2CRegisterContext(uint32_t address, const oni_dev_idx_t, std::shared_ptr<Onix1>);

	void WriteByte(uint32_t address, uint32_t value, bool sixteenBitAddress = false);

	void ReadByte(uint32_t address, oni_reg_val_t* value, bool sixteenBitAddress = false);

	int getLastResult() { return i2cContext->getLastResult(); }

protected:
	std::shared_ptr<Onix1> i2cContext;

private:

	const oni_dev_idx_t deviceIndex;

	const uint32_t i2caddress;

	JUCE_LEAK_DETECTOR(I2CRegisterContext);
};

#endif // !__I2CRegisterContext_H__

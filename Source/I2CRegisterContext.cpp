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

#include "I2CRegisterContext.h"
#include <iostream>

using namespace Onix;

I2CRegisterContext::I2CRegisterContext(uint32_t address_, const oni_dev_idx_t devIdx_, const oni_ctx ctx_)
	: deviceIndex(devIdx_), context(ctx_), i2caddress(address_)
{
}

int I2CRegisterContext::WriteByte(uint32_t address, uint32_t value, bool sixteenBitAddress)
{
	uint32_t registerAddress = (address << 7) | (i2caddress & 0x7F);
	registerAddress |= sixteenBitAddress ? 0x80000000 : 0;
	auto result = oni_write_reg(context, deviceIndex, registerAddress, value);

	if (result != 0) { LOGE(oni_error_str(result)); }

	return result;
}

int I2CRegisterContext::ReadByte(uint32_t address, oni_reg_val_t* value, bool sixteenBitAddress)
{
	uint32_t registerAddress = (address << 7) | (i2caddress & 0x7F);
	registerAddress |= sixteenBitAddress ? 0x80000000 : 0;

	auto result = oni_read_reg(context, deviceIndex, registerAddress, value);

	if (result != 0) {
		LOGE(oni_error_str(result));
	}

	return result;
}
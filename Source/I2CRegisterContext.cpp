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

#include "I2CRegisterContext.h"

using namespace OnixSourcePlugin;

I2CRegisterContext::I2CRegisterContext (uint32_t address_, const oni_dev_idx_t devIdx_, std::shared_ptr<Onix1> ctx_)
    : deviceIndex (devIdx_), i2cAddress (address_)
{
    i2cContext = ctx_;
}

int I2CRegisterContext::WriteByte (uint32_t address, uint32_t value, bool sixteenBitAddress)
{
    uint32_t registerAddress = (address << 7) | (i2cAddress & 0x7F);
    registerAddress |= sixteenBitAddress ? 0x80000000 : 0;

    return i2cContext->writeRegister (deviceIndex, registerAddress, value);
}

int I2CRegisterContext::ReadByte (uint32_t address, oni_reg_val_t* value, bool sixteenBitAddress)
{
    return ReadWord (address, 1, value, sixteenBitAddress);
}

int I2CRegisterContext::ReadWord (uint32_t address, uint32_t numBytes, uint32_t* value, bool sixteenBitAddress)
{
    if (numBytes < 1 || numBytes > 4)
    {
        LOGE ("Invalid number of bytes requested when reading a word.");
        return 1;
    }

    uint32_t registerAddress = (address << 7) | (i2cAddress & 0x7F);
    registerAddress |= sixteenBitAddress ? 0x80000000 : 0;
    registerAddress |= (numBytes - 1) << 28;

    return i2cContext->readRegister (deviceIndex, registerAddress, value);
}

int I2CRegisterContext::set933I2cRate (double rate)
{
    auto sclTimes = (uint32_t) (std::round (1.0 / (100e-9 * rate)));
    int rc = WriteByte ((uint32_t) DS90UB9x::DS90UB933SerializerI2CRegister::SclHigh, sclTimes);
    if (rc != ONI_ESUCCESS)
        return rc;
    rc = WriteByte ((uint32_t) DS90UB9x::DS90UB933SerializerI2CRegister::SclLow, sclTimes);
    if (rc != ONI_ESUCCESS)
        return rc;
}
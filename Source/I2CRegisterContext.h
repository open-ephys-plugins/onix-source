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

#include <cstddef>

#include "ProcessorHeaders.h"

#include "Devices/DS90UB9x.h"
#include "Onix1.h"

namespace OnixSourcePlugin
{
class I2CRegisterContext
{
public:
    I2CRegisterContext (uint32_t address, const oni_dev_idx_t, std::shared_ptr<Onix1>);

    int WriteByte (uint32_t address, uint32_t value, bool sixteenBitAddress = false);
    int WriteWord (uint32_t address, uint32_t value, uint32_t numBytes, bool sixteenBitAddress = false);
    int ReadByte (uint32_t address, oni_reg_val_t* value, bool sixteenBitAddress = false);
    int ReadWord (uint32_t address, uint32_t numBytes, uint32_t* value, bool sixteenBitAddress = false);

    int set933I2cRate (double);

protected:
    std::shared_ptr<Onix1> i2cContext;

private:
    const oni_dev_idx_t deviceIndex;

    const uint32_t i2cAddress;

    JUCE_LEAK_DETECTOR (I2CRegisterContext);
};
} // namespace OnixSourcePlugin

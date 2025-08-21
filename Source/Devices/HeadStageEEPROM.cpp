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

#include "HeadStageEEPROM.h"
#include "DS90UB9x.h"
#include <iostream>

using namespace OnixSourcePlugin;

HeadStageEEPROM::HeadStageEEPROM (const oni_dev_idx_t dev_id, std::shared_ptr<Onix1> ctx)
    : I2CRegisterContext (HeadStageEEPROM::EEPROM_ADDRESS, dev_id, ctx)
{
    auto deserializer = std::make_unique<I2CRegisterContext> (DS90UB9x::DES_ADDR, dev_id, ctx);
    uint32_t alias = HeadStageEEPROM::EEPROM_ADDRESS << 1;
    int rc = deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID7, alias);
    if (rc != ONI_ESUCCESS)
        return;
    deserializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias7, alias);
    if (rc != ONI_ESUCCESS)
        return;
}

uint32_t HeadStageEEPROM::GetHeadStageID()
{
    uint32_t data = 0;
    int rc = 0;
    for (unsigned int i = 0; i < sizeof (uint32_t); i++)
    {
        oni_reg_val_t val;
        rc = ReadByte (DEVID_START_ADDR + i, &val, true);
        if (rc != ONI_ESUCCESS)
            return data;
        data += (val & 0xFF) << (8 * i);
    }
    return data;
}


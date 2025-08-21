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

#include "../I2CRegisterContext.h"

#include <oni.h>
#include <onix.h>

namespace OnixSourcePlugin
{
class HeadStageEEPROM : public I2CRegisterContext
{
public:
    HeadStageEEPROM (const oni_dev_idx_t, std::shared_ptr<Onix1>);

    uint32_t GetHeadStageID();

private:
    static const uint32_t EEPROM_ADDRESS = 0x51;
    static const uint32_t DEVID_START_ADDR = 18;

    JUCE_LEAK_DETECTOR (HeadStageEEPROM);
};
} // namespace OnixSourcePlugin

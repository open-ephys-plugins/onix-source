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

#include <cstdint>

namespace OnixSourcePlugin
{
static class DS90UB9x
{
public:
    // managed registers
    static const uint32_t ENABLE = 0x8000;
    static const uint32_t READSZ = 0x8001;
    static const uint32_t TRIGGER = 0x8002;
    static const uint32_t TRIGGEROFF = 0x8003;
    static const uint32_t DATAGATE = 0x8004;
    static const uint32_t SYNCBITS = 0x8005;
    static const uint32_t MARK = 0x8006;
    static const uint32_t MAGIC_MASK = 0x8007;
    static const uint32_t MAGIC = 0x8008;
    static const uint32_t MAGIC_WAIT = 0x8009;
    static const uint32_t DATAMODE = 0x800A;
    static const uint32_t DATALINES0 = 0x800B;
    static const uint32_t DATALINES1 = 0x800C;

    // reserved registers
    static const uint32_t GPIO_DIR = 0x8010;
    static const uint32_t GPIO_VAL = 0x8011;
    static const uint32_t LINKSTATUS = 0x8012;
    static const uint32_t LASTI2CL = 0x8013;
    static const uint32_t LASTI2CH = 0x8014;

    // unmanaged default serializer / deserializer I2C addresses
    static const uint32_t DES_ADDR = 0x30;
    static const uint32_t SER_ADDR = 0x58;

    enum class DS90UB9xTriggerMode : uint32_t
    {
        Continuous = 0,
        HsyncEdgePositive = 0b0001,
        HsyncEdgeNegative = 0b1001,
        HsyncLevelPositive = 0b0101,
        HsyncLevelNegative = 0b1101,
        VsyncEdgePositive = 0b0011,
        VsyncEdgeNegative = 0b1011,
        VsyncLevelPositive = 0b0111,
        VsyncLevelNegative = 0b1111,
    };

    enum class DS90UB9xDataGate : uint32_t
    {
        Disabled = 0,
        HsyncPositive = 0b001,
        HsyncNegative = 0b101,
        VsyncPositive = 0b011,
        VsyncNegative = 0b111,
    };

    enum class DS90UB9xMarkMode : uint32_t
    {
        Disabled = 0,
        HsyncRising = 0b001,
        HsyncFalling = 0b101,
        VsyncRising = 0b011,
        VsyncFalling = 0b111,
    };

    enum class DS90UB9xDeserializerI2CRegister : uint32_t
    {
        PortMode = 0x6D,
        PortSel = 0x4C,
        I2CConfig = 0x58,
        GpioCtrl0 = 0x6E,
        GpioCtrl1 = 0x6F,

        SerAlias = 0x5C,

        SlaveID1 = 0x5E,
        SlaveID2 = 0x5F,
        SlaveID3 = 0x60,
        SlaveID4 = 0x61,
        SlaveID5 = 0x62,
        SlaveID6 = 0x63,
        SlaveID7 = 0x64,

        SlaveAlias1 = 0x66,
        SlaveAlias2 = 0x67,
        SlaveAlias3 = 0x68,
        SlaveAlias4 = 0x69,
        SlaveAlias5 = 0x6A,
        SlaveAlias6 = 0x6B,
        SlaveAlias7 = 0x6C,
    };

    enum class DS90UB9xSerializerI2CRegister : uint32_t
    {
        GPIO10 = 0x0D,
        GPIO32 = 0x0E,
        SCLHIGH = 0x0A,
        SCLLOW = 0x0B
    };

    enum class DS90UB933SerializerI2CRegister : uint32_t
    {
        Gpio10 = 0x0D,
        Gpio32 = 0x0E,
        SclHigh = 0x11,
        SclLow = 0x12
    };

    enum class DS90UB953SerializerI2CRegister : uint32_t
    {
        GpioData = 0x0D,
        GpioIOControl = 0x0E,
        SclHigh = 0x0B,
        SclLow = 0x0C
    };

    enum class DS90UB9xMode : uint32_t
    {
        Raw12BitLowFrequency = 1,
        Raw12BitHighFrequency = 2,
        Raw10Bit = 3,
    };

    enum class DS90UB9xDirection : uint32_t
    {
        Input = 0,
        Output = 1
    };
};
} // namespace OnixSourcePlugin

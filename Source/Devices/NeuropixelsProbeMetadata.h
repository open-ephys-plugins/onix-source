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
#include "../NeuropixelsComponents.h"
#include "../OnixDevice.h"

namespace OnixSourcePlugin
{
class NeuropixelsProbeMetadata
{
public:
    NeuropixelsProbeMetadata() = default;
    NeuropixelsProbeMetadata (I2CRegisterContext* flex, OnixDeviceType type);

    const uint64_t getProbeSerialNumber() const;
    const std::string getProbePartNumber() const;
    const std::string getFlexPartNumber() const;
    const std::string getFlexVersion() const;

    static bool validateProbeTypeAndPartNumber (ProbeType probeType, std::string partNumber);
    static std::string getProbeTypeString (std::string partNumber);

private:
    OnixDeviceType deviceType = OnixDeviceType::UNKNOWN;

    std::string probePartNumber = "";
    uint64_t probeSerialNumber = 0ull;
    std::string flexPartNumber = "";
    std::string flexVersion = "";

    uint32_t getProbeSerialNumberOffset() const;
    uint32_t getProbePartNumberOffset() const;
    uint32_t getFlexPartNumberOffset() const;
    uint32_t getFlexVersionOffset() const;
    uint32_t getFlexRevisionOffset() const;

    enum class NeuropixelsV1Offset : uint32_t
    {
        PROBE_SN = 0,
        FLEX_VERSION = 10,
        FLEX_REVISION = 11,
        FLEX_PN = 20,
        PROBE_PN = 40,
    };

    enum class NeuropixelsV2Offset : uint32_t
    {
        PROBE_SN = 0x00,
        FLEX_VERSION = 0x10,
        FLEX_REVISION = 0x11,
        FLEX_PN = 0x20,
        PROBE_PN = 0x40,
    };
};
} // namespace OnixSourcePlugin

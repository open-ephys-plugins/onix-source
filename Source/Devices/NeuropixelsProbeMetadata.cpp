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

#include "NeuropixelsProbeMetadata.h"

using namespace OnixSourcePlugin;

NeuropixelsProbeMetadata::NeuropixelsProbeMetadata(I2CRegisterContext* flex, OnixDeviceType type)
	: deviceType(type)
{
	if (flex == nullptr)
	{
		throw error_str("Flex is not initialized when trying to read Neuropixels probe metadata.");
	}

	if (deviceType != OnixDeviceType::NEUROPIXELSV1E
		&& deviceType != OnixDeviceType::NEUROPIXELSV1F
		&& deviceType != OnixDeviceType::NEUROPIXELSV2E)
	{
		throw error_str("Invalid device type given. Expected the type to be a Neuropixels device.");
	}

	try
	{
		std::vector<oni_reg_val_t> probeSnBytes;
		int rc = flex->readBytes(getProbeSerialNumberOffset(), sizeof(probeSerialNumber), probeSnBytes);
		if (rc != ONI_ESUCCESS)
			throw error_t(rc);

		for (int i = 0; i < probeSnBytes.size(); i++)
		{
			if (probeSnBytes[i] <= 0xFF)
			{
				probeSerialNumber |= (((uint64_t)probeSnBytes[i]) << (i * 8));
			}
		}

		const int partNumberLength = 20;

		rc = flex->readString(getProbePartNumberOffset(), partNumberLength, probePartNumber);
		if (rc != ONI_ESUCCESS)
			throw error_t(rc);

		rc = flex->readString(getFlexPartNumberOffset(), partNumberLength, flexPartNumber);
		if (rc != ONI_ESUCCESS)
			throw error_t(rc);

		oni_reg_val_t version, revision;

		rc = flex->ReadByte(getFlexVersionOffset(), &version);
		if (rc != ONI_ESUCCESS)
			throw error_t(rc);

		rc = flex->ReadByte(getFlexRevisionOffset(), &revision);
		if (rc != ONI_ESUCCESS)
			throw error_t(rc);

		flexVersion = std::to_string(version) + "." + std::to_string(revision);
	}
	catch (const error_t e)
	{
		if (e.num() == ONI_EREADFAILURE)
			return;

		Onix1::showWarningMessageBoxAsync("Error Reading Probe Metadata",
			"Encountered an error while trying to read probe metadata: " + std::string(e.what()));
	}
}

const uint64_t NeuropixelsProbeMetadata::getProbeSerialNumber() const
{
	return probeSerialNumber;
}

const std::string NeuropixelsProbeMetadata::getProbePartNumber() const
{
	return probePartNumber;
}

const std::string NeuropixelsProbeMetadata::getFlexPartNumber() const
{
	return flexPartNumber;
}

const std::string NeuropixelsProbeMetadata::getFlexVersion() const
{
	return flexVersion;
}

uint32_t NeuropixelsProbeMetadata::getProbeSerialNumberOffset() const
{
	switch (deviceType)
	{
	case OnixDeviceType::NEUROPIXELSV1E:
	case OnixDeviceType::NEUROPIXELSV1F:
		return (uint32_t)NeuropixelsV1Offset::PROBE_SN;

	case OnixDeviceType::NEUROPIXELSV2E:
		return (uint32_t)NeuropixelsV2Offset::PROBE_SN;

	default:
		throw error_str("Invalid device type found in Neuropixels Probe Metadata. Expected a Neuropixels device type.");
	}
}

uint32_t NeuropixelsProbeMetadata::getProbePartNumberOffset() const
{
	switch (deviceType)
	{
	case OnixDeviceType::NEUROPIXELSV1E:
	case OnixDeviceType::NEUROPIXELSV1F:
		return (uint32_t)NeuropixelsV1Offset::PROBE_PN;

	case OnixDeviceType::NEUROPIXELSV2E:
		return (uint32_t)NeuropixelsV2Offset::PROBE_PN;

	default:
		throw error_str("Invalid device type found in Neuropixels Probe Metadata. Expected a Neuropixels device type.");
	}
}

uint32_t NeuropixelsProbeMetadata::getFlexPartNumberOffset() const
{
	switch (deviceType)
	{
	case OnixDeviceType::NEUROPIXELSV1E:
	case OnixDeviceType::NEUROPIXELSV1F:
		return (uint32_t)NeuropixelsV1Offset::FLEX_PN;

	case OnixDeviceType::NEUROPIXELSV2E:
		return (uint32_t)NeuropixelsV2Offset::FLEX_PN;

	default:
		throw error_str("Invalid device type found in Neuropixels Probe Metadata. Expected a Neuropixels device type.");
	}
}

uint32_t NeuropixelsProbeMetadata::getFlexVersionOffset() const
{
	switch (deviceType)
	{
	case OnixDeviceType::NEUROPIXELSV1E:
	case OnixDeviceType::NEUROPIXELSV1F:
		return (uint32_t)NeuropixelsV1Offset::FLEX_VERSION;

	case OnixDeviceType::NEUROPIXELSV2E:
		return (uint32_t)NeuropixelsV2Offset::FLEX_VERSION;

	default:
		throw error_str("Invalid device type found in Neuropixels Probe Metadata. Expected a Neuropixels device type.");
	}
}

uint32_t NeuropixelsProbeMetadata::getFlexRevisionOffset() const
{
	switch (deviceType)
	{
	case OnixDeviceType::NEUROPIXELSV1E:
	case OnixDeviceType::NEUROPIXELSV1F:
		return (uint32_t)NeuropixelsV1Offset::FLEX_REVISION;

	case OnixDeviceType::NEUROPIXELSV2E:
		return (uint32_t)NeuropixelsV2Offset::FLEX_REVISION;

	default:
		throw error_str("Invalid device type found in Neuropixels Probe Metadata. Expected a Neuropixels device type.");
	}
}

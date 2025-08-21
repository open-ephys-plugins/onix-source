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

#include "Onix1.h"

#include "OnixDevice.h"

using namespace OnixSourcePlugin;

Onix1::Onix1 (int hostIndex)
{
    ctx_ = oni_create_ctx ("riffa");

    if (ctx_ == nullptr)
        throw std::system_error (errno, std::system_category());

    int rc = oni_init_ctx (ctx_, hostIndex);

    if (rc != ONI_ESUCCESS)
        throw error_t (rc);

    rc = getOption (ONI_OPT_ACQCLKHZ, &ACQ_CLK_HZ);
    if (rc != ONI_ESUCCESS)
        throw error_t (rc);
}

Onix1::~Onix1()
{
    oni_destroy_ctx (ctx_);
}

int Onix1::getDeviceTable (device_map_t* deviceTable)
{
    if (deviceTable->size() > 0)
        deviceTable->clear();

    issueReset();

    oni_size_t numDevices;
    int rc = getOption<oni_size_t> (ONI_OPT_NUMDEVICES, &numDevices);

    if (numDevices == 0 || rc != ONI_ESUCCESS)
        return rc;

    size_t devicesSize = sizeof (oni_device_t) * numDevices;

    std::vector<oni_device_t> devices;
    devices.resize (numDevices);

    rc = get_opt_ (ONI_OPT_DEVICETABLE, devices.data(), &devicesSize);

    if (rc != ONI_ESUCCESS)
        return rc;

    for (const auto& device : devices)
    {
        deviceTable->insert ({ device.idx, device });
    }

    return rc;
}

std::map<int, int> Onix1::getHubIds (device_map_t deviceTable) const
{
    std::map<int, int> hubIds;

    if (deviceTable.size() == 0)
        return hubIds;

    auto deviceIndices = getDeviceIndices (deviceTable);

    auto offsets = OnixDevice::getUniqueOffsets (deviceIndices, false);

    for (int i = 0; i < offsets.size(); i++)
    {
        oni_reg_val_t hubId = 0;
        int rc = oni_read_reg (ctx_, offsets[i] + ONIX_HUB_DEV_IDX, (uint32_t) ONIX_HUB_HARDWAREID, &hubId);
        if (rc != ONI_ESUCCESS)
        {
            LOGE ("Unable to read the hub device index for the hub at index ", offsets[i]);
            continue;
        }

        hubIds.insert ({ offsets[i], hubId });
    }

    return hubIds;
}

std::vector<int> Onix1::getDeviceIndices (device_map_t deviceMap, int hubIndex)
{
    std::vector<int> deviceIndices;

    for (const auto& [idx, dev] : deviceMap)
    {
        if (dev.id != ONIX_NULL && (hubIndex == -1 || OnixDevice::getOffset (dev.idx) == hubIndex))
            deviceIndices.emplace_back (idx);
    }

    return deviceIndices;
}

int Onix1::get_opt_ (int option, void* value, size_t* size) const
{
    const ScopedLock lock (registerLock);

    int rc = oni_get_opt (ctx_, option, value, size);
    if (rc != ONI_ESUCCESS)
        LOGE (oni_error_str (rc));
    return rc;
}

int Onix1::readRegister (oni_dev_idx_t devIndex, oni_reg_addr_t registerAddress, oni_reg_val_t* value) const
{
    const ScopedLock lock (registerLock);

    int rc = oni_read_reg (ctx_, devIndex, registerAddress, value);
    if (rc != ONI_ESUCCESS)
        LOGE (oni_error_str (rc));
    return rc;
}

int Onix1::writeRegister (oni_dev_idx_t devIndex, oni_reg_addr_t registerAddress, oni_reg_val_t value) const
{
    const ScopedLock lock (registerLock);

    int rc = oni_write_reg (ctx_, devIndex, registerAddress, value);
    if (rc != ONI_ESUCCESS)
        LOGE (oni_error_str (rc));
    return rc;
}

int Onix1::issueReset()
{
    int val = 1;
    int rc = setOption (ONI_OPT_RESET, val);
    return rc;
}

std::string Onix1::getVersion()
{
    return std::to_string (ONI_VERSION_MAJOR) + "." + std::to_string (ONI_VERSION_MINOR) + "." + std::to_string (ONI_VERSION_PATCH);
}

double Onix1::convertTimestampToSeconds (uint64_t timestamp) const
{
    return static_cast<double> (timestamp) / ACQ_CLK_HZ;
}

oni_frame_t* Onix1::readFrame() const
{
    const ScopedLock lock (frameLock);

    oni_frame_t* frame = nullptr;
    int rc = oni_read_frame (ctx_, &frame);
    if (rc < ONI_ESUCCESS)
    {
        LOGE (oni_error_str (rc));
        return nullptr;
    }

    return frame;
}

void Onix1::showWarningMessageBoxAsync (std::string title, std::string error_msg)
{
    LOGD (error_msg);
    MessageManager::callAsync ([title, error_msg]
                               { AlertWindow::showMessageBoxAsync (
                                     MessageBoxIconType::WarningIcon,
                                     title,
                                     error_msg); });
}

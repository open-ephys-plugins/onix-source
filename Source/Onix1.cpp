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

Onix1::Onix1(int hostIndex)
{
	ctx_ = oni_create_ctx("riffa");

	if (ctx_ == nullptr)
		throw std::system_error(errno, std::system_category());

	result = oni_init_ctx(ctx_, hostIndex);

	if (result != ONI_ESUCCESS)
		throw error_t(result);

	oni_version(&major, &minor, &patch);
}

Onix1::~Onix1()
{
	oni_destroy_ctx(ctx_);
}

void Onix1::updateDeviceTable()
{
	if (deviceTable.size() > 0)
		deviceTable.clear();

	auto numDevices = getOption<oni_size_t>(ONI_OPT_NUMDEVICES);

	if (numDevices == 0) return;

	size_t devicesSize = sizeof(oni_device_t) * numDevices;
	deviceTable.reserve(devicesSize);

	std::vector<oni_device_t> devices;
	devices.resize(numDevices);

	get_opt_(ONI_OPT_DEVICETABLE, devices.data(), &devicesSize);

	for (const auto& device : devices)
	{
		deviceTable.insert({ device.idx, device });
	}
}

void Onix1::get_opt_(int option, void* value, size_t* size)
{
	result = oni_get_opt(ctx_, option, value, size);
	if (result != ONI_ESUCCESS) LOGE(oni_error_str(result));
}

oni_reg_val_t Onix1::readRegister(oni_dev_idx_t devIndex, oni_reg_addr_t registerAddress)
{
	oni_reg_val_t value = 0;
	result = oni_read_reg(ctx_, devIndex, registerAddress, &value);
	if (result != ONI_ESUCCESS) 
		LOGE(oni_error_str(result));
	return value;
}

void Onix1::writeRegister(oni_dev_idx_t devIndex, oni_reg_addr_t registerAddress, oni_reg_val_t value)
{
	result = oni_write_reg(ctx_, devIndex, registerAddress, value);
	if (result != ONI_ESUCCESS) 
		LOGE(oni_error_str(result));
}

oni_frame_t* Onix1::readFrame()
{
	oni_frame_t* frame;
	result = oni_read_frame(ctx_, &frame);
	if (result < ONI_ESUCCESS) 
		LOGE(oni_error_str(result));

	return frame;
}

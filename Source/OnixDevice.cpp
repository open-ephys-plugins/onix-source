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

#include "OnixDevice.h"

OnixDevice::OnixDevice(String name_, OnixDeviceType type_, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx)
	: type(type_), deviceIdx(deviceIdx_)
{
	deviceContext = ctx;
	name = name_;

	if (type == OnixDeviceType::NEUROPIXELSV2E || type == OnixDeviceType::POLLEDBNO)
		isPassthrough = true;
}

oni_dev_idx_t OnixDevice::getDeviceIdx(bool getPassthroughIndex)
{
	if (isPassthrough && !getPassthroughIndex)
		return getDeviceIndexFromPassthroughIndex(deviceIdx);
	else
		return deviceIdx;
}

oni_dev_idx_t OnixDevice::getDeviceIndexFromPassthroughIndex(oni_dev_idx_t passthroughIndex)
{
	oni_dev_idx_t idx = (passthroughIndex - 7) << 8;

	if (type == OnixDeviceType::POLLEDBNO)
		idx++;

	return idx;
}

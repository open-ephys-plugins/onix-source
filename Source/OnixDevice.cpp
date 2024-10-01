/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2020 Allen Institute for Brain Science and Open Ephys

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

using namespace Onix;

OnixDevice::OnixDevice(String name, OnixDeviceType type_, const oni_dev_idx_t deviceIdx_, const oni_ctx ctx_)
	: Thread(name), type(type_), deviceIdx(deviceIdx_), ctx(ctx_)
{
	
}

int OnixDevice::checkLinkState(oni_dev_idx_t port)
{
	const oni_reg_addr_t linkStateRegister = 5;

	oni_reg_val_t linkState;
	int result = oni_read_reg(ctx, port, linkStateRegister, &linkState);

	if (result != 0) { LOGE(oni_error_str(result)); return -1; }
	else if ((linkState & (uint32_t)0x1) == 0) { LOGE("Unable to acquire communication lock."); return -1; }
	else return result;
}

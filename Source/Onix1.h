/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2023 Allen Institute for Brain Science and Open Ephys

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

#ifndef __ONIX1_H__
#define __ONIX1_H__

#include <oni.h>
#include <onix.h>
#include <system_error>
#include <exception>

#include "../../plugin-GUI/Source/Utils/Utils.h"

constexpr const char* NEUROPIXELSV1F_HEADSTAGE_NAME = "Neuropixels 1.0f";

class error_t : public std::exception
{
public:
	explicit error_t(int errorNum) : errorNum_(errorNum) {}

	const char* what() const noexcept override
	{
		return oni_error_str(errorNum_);
	}

	int num() const { return errorNum_; }

private:
	int errorNum_;
};

using device_map_t = std::unordered_map<oni_dev_idx_t, oni_device_t>;

class Onix1
{
public:
	Onix1(int hostIndex = 0);

	~Onix1();

	inline bool isInitialized() const { return ctx_ != nullptr; }

	template <typename opt_t>
	opt_t getOption(int option)
	{
		opt_t value;
		size_t len = sizeof(opt_t);
		get_opt_(option, &value, &len);
		return value;
	}

	template <typename opt_t>
	void setOption(int option, const opt_t value)
	{
		result = oni_set_opt(ctx_, option, &value, opt_size_<opt_t>(value));
		if (result != ONI_ESUCCESS) LOGE(oni_error_str(result));
	}

	oni_reg_val_t readRegister(oni_dev_idx_t, oni_reg_addr_t);

	void writeRegister(oni_dev_idx_t, oni_reg_addr_t, oni_reg_val_t);

	device_map_t getDeviceTable() const noexcept { return deviceTable; }

	void updateDeviceTable();

	oni_frame_t* readFrame();

	int getLastResult() const { return result; }

private:

	/** The ONI ctx object */
	oni_ctx ctx_;

	device_map_t deviceTable;

	int result;

	template<typename opt_t>
	size_t opt_size_(opt_t opt)
	{
		size_t len = 0;
		if constexpr (std::is_same<opt_t, char*>::value)
			len = strlen(len) + 1;
		else if constexpr (std::is_same<opt_t, const char*>::value)
			len = strlen(len) + 1;
		else
			len = sizeof(opt);

		return len;
	}

	void get_opt_(int option, void* value, size_t* size);
};

#endif // !__ONIX1_H__

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

#include <oni.h>
#include <onix.h>
#include <system_error>
#include <exception>

#include <DataThreadHeaders.h>

#include "../../plugin-GUI/Source/Utils/Utils.h"

namespace OnixSourcePlugin
{
	constexpr char* NEUROPIXELSV1F_HEADSTAGE_NAME = "Neuropixels 1.0f Headstage";
	constexpr char* NEUROPIXELSV1E_HEADSTAGE_NAME = "Neuropixels 1.0e Headstage";
	constexpr char* NEUROPIXELSV2E_HEADSTAGE_NAME = "Neuropixels 2.0 Headstage";
	constexpr char* BREAKOUT_BOARD_NAME = "Breakout Board";

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

	class error_str : public std::exception
	{
	public:
		explicit error_str(std::string errorStr) : m_errorStr(errorStr) {}

		const char* what() const noexcept override
		{
			return m_errorStr.c_str();
		}

		std::string str() const { return m_errorStr; }

	private:
		std::string m_errorStr;
	};

	using device_map_t = std::map<oni_dev_idx_t, oni_device_t>;

	class Onix1
	{
	public:
		Onix1(int hostIndex = 0);

		~Onix1();

		inline bool isInitialized() const { return ctx_ != nullptr; }

		template <typename opt_t>
		int getOption(int option, opt_t* value)
		{
			size_t len = sizeof(opt_t);
			int rc = get_opt_(option, value, &len);
			return rc;
		}

		template <typename opt_t>
		int setOption(int option, const opt_t value)
		{
			const ScopedLock lock(optionLock);

			int rc = oni_set_opt(ctx_, option, &value, opt_size_<opt_t>(value));
			if (rc != ONI_ESUCCESS) LOGE(oni_error_str(rc));
			return rc;
		}

		int readRegister(oni_dev_idx_t devIndex, oni_reg_addr_t registerAddress, oni_reg_val_t* value) const;

		int writeRegister(oni_dev_idx_t, oni_reg_addr_t, oni_reg_val_t) const;

		int getDeviceTable(device_map_t*);

		oni_frame_t* readFrame() const;

		int issueReset();

		std::string getVersion() const;

		double convertTimestampToSeconds(uint64_t timestamp) const;

		/** Gets a map of all hubs connected, where the index of the map is the hub address, and the value is the hub ID */
		std::map<int, int> getHubIds(device_map_t) const;

		/** Gets a vector of device indices from a device_map_t object, optionally filtered by a specific hub */
		static std::vector<int> getDeviceIndices(device_map_t deviceMap, int hubIndex = -1);

		/** Displays an AlertWindow::showMessageBoxAsync window using the message thread, with a warning icon */
		static void showWarningMessageBoxAsync(std::string, std::string);

	private:

		/** The ONI ctx object */
		oni_ctx ctx_;

		CriticalSection registerLock;
		CriticalSection frameLock;
		CriticalSection optionLock;

		int major;
		int minor;
		int patch;

		uint32_t ACQ_CLK_HZ;

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

		int get_opt_(int option, void* value, size_t* size) const;
	};
}

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

#ifndef __I2CRegisterContext_H__
#define __I2CRegisterContext_H__

#include <oni.h>

#include <cstddef>

namespace Onix
{
	class I2CRegisterContext
	{
	public:

		I2CRegisterContext(uint32_t address, const oni_dev_idx_t, const oni_ctx);

		void WriteByte(uint32_t address, uint32_t value);

		oni_reg_val_t ReadByte(uint32_t address);

	private:

		const oni_ctx context;
		const oni_dev_idx_t deviceIndex;

		uint32_t address;
	};
}

#endif // !__I2CRegisterContext_H__

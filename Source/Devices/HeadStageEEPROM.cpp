#include "HeadStageEEPROM.h"
#include "DS90UB9x.h"
#include <iostream>

using namespace Onix;


HeadStageEEPROM::HeadStageEEPROM(const oni_dev_idx_t dev_id, const oni_ctx ctx)
	: I2CRegisterContext(EEPROM_ADDRESS, dev_id, ctx)
{
	auto deserializer = std::make_unique<I2CRegisterContext>(DS90UB9x::DES_ADDR, dev_id, ctx);
	uint32_t alias = EEPROM_ADDRESS << 1;
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID7, alias);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias7, alias);
	std::cout << "Alias set" << std::endl;
}

uint32_t HeadStageEEPROM::GetHeadStageID()
{
	uint32_t data = 0;
	for (unsigned int i = 0; i < sizeof(uint32_t); i++)
	{
		oni_reg_val_t val;
		val = ReadByte(DEVID_START_ADDR + i, true);
		std::cout << i << " r " << val << std::endl;
		data += (val & 0xFF) << (8 * i);
	}
	return data;
}


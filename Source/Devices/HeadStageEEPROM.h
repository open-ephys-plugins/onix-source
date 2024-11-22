#pragma once
#include "../I2CRegisterContext.h"
#include <oni.h>
#include <onix.h>

class HeadStageEEPROM :
	public I2CRegisterContext
{
public:
	HeadStageEEPROM(const oni_dev_idx_t, const oni_ctx);

	uint32_t GetHeadStageID();

private:
	static const uint32_t EEPROM_ADDRESS = 0x51;
	static const uint32_t DEVID_START_ADDR = 18;

};


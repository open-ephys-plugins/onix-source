#pragma once
#include "../I2CRegisterContext.h"
#include <oni.h>
#include <onix.h>
#include <memory>

namespace Onix
{

    class HeadStageEEPROM :
        public I2CRegisterContext
    {
    public:
        HeadStageEEPROM(const oni_dev_idx_t, const oni_ctx);

        uint32_t GetHeadStageID();

    private:
        const int EEPROM_ADDRESS = 0x51;
        const int DEVID_START_ADDR = 18;

    };

}


#pragma once

#include "AG_BattMonitor_SMBus_Generic.h"

#if AP_BATTERY_SMBUS_ROTOYE_ENABLED

class AG_BattMonitor_SMBus_Rotoye : public AG_BattMonitor_SMBus_Generic
{
    using AG_BattMonitor_SMBus_Generic::AG_BattMonitor_SMBus_Generic;

private:

    // Rotoye Batmon has two temperature readings
    void read_temp(void) override;

};

#endif  // AP_BATTERY_SMBUS_ROTOYE_ENABLED

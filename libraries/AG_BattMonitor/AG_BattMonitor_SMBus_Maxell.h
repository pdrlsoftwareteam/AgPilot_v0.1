#pragma once

#include "AG_BattMonitor_SMBus_Generic.h"

#if AP_BATTERY_SMBUS_MAXELL_ENABLED

class AG_BattMonitor_SMBus_Maxell : public AG_BattMonitor_SMBus_Generic
{
    using AG_BattMonitor_SMBus_Generic::AG_BattMonitor_SMBus_Generic;

private:

    // return a scaler that should be multiplied by the battery's reported capacity numbers to arrive at the actual capacity in mAh
    uint16_t get_capacity_scaler() const override { return 2; }

};

#endif  // AP_BATTERY_SMBUS_MAXELL_ENABLED

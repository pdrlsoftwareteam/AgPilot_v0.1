#pragma once

#include "AG_BattMonitor_SMBus.h"

#if AP_BATTERY_SMBUS_SOLO_ENABLED

class AG_BattMonitor_SMBus_Solo : public AG_BattMonitor_SMBus
{
public:

    // Constructor
    AG_BattMonitor_SMBus_Solo(AG_BattMonitor &mon,
                             AG_BattMonitor::BattMonitor_State &mon_state,
                             AG_BattMonitor_Params &params);

private:

    void timer(void) override;

    uint8_t _button_press_count;
};

#endif  // AP_BATTERY_SMBUS_SOLO_ENABLED

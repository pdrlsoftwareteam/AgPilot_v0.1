#pragma once

#include "AG_BattMonitor_SMBus.h"

#if AP_BATTERY_SMBUS_NEODESIGN_ENABLED

class AG_BattMonitor_SMBus_NeoDesign : public AG_BattMonitor_SMBus
{
public:
    AG_BattMonitor_SMBus_NeoDesign(AG_BattMonitor &mon,
                             AG_BattMonitor::BattMonitor_State &mon_state,
                             AG_BattMonitor_Params &params);

private:

    void timer(void) override;

    uint8_t _cell_count;

    static const constexpr uint8_t max_cell_count = 10;
};

#endif  // AP_BATTERY_SMBUS_NEODESIGN_ENABLED

#pragma once

#include "AG_BattMonitor.h"
#include "AG_BattMonitor_Analog.h"

#if AP_BATTERY_SYNTHETIC_CURRENT_ENABLED
class AG_BattMonitor_Synthetic_Current : public AG_BattMonitor_Analog
{
public:

    /// Constructor
    AG_BattMonitor_Synthetic_Current(AG_BattMonitor &mon, AG_BattMonitor::BattMonitor_State &mon_state, AG_BattMonitor_Params &params);

    /// Read the battery voltage and current.  Should be called at 10hz
    void read() override;

    /// returns true if battery monitor provides consumed energy info
    bool has_consumed_energy() const override { return true; }

    /// returns true if battery monitor provides current info
    bool has_current() const override { return true; }

    void init(void) override {}

    static const struct AG_Param::GroupInfo var_info[];

protected:

    AP_Float    _max_voltage;           /// maximum battery voltage used in current caluculation   
};
#endif

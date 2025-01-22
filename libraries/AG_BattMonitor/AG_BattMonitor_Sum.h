#pragma once

#include "AG_BattMonitor_Backend.h"

#if AP_BATTERY_SUM_ENABLED

#include "AG_BattMonitor.h"

class AG_BattMonitor_Sum : public AG_BattMonitor_Backend
{
public:

    /// Constructor
    AG_BattMonitor_Sum(AG_BattMonitor &mon, AG_BattMonitor::BattMonitor_State &mon_state, AG_BattMonitor_Params &params, uint8_t instance);

    /// Read the battery voltage and current.  Should be called at 10hz
    void read() override;

    /// returns true if battery monitor provides consumed energy info
    bool has_consumed_energy() const override { return has_current(); }

    /// returns true if battery monitor provides current info
    bool has_current() const override { return _has_current; }

    void init(void) override {}

    static const struct AG_Param::GroupInfo var_info[];

private:

    AP_Int16  _sum_mask;
    uint8_t _instance;
    bool _has_current;
};

#endif  // AP_BATTERY_SUM_ENABLED

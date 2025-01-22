#pragma once

#include "AG_BattMonitor_Analog.h"

#if AP_BATTERY_FUELLEVEL_PWM_ENABLED

#include "AG_BattMonitor.h"

class AG_BattMonitor_FuelLevel_PWM : public AG_BattMonitor_Analog
{
public:

    /// Constructor
    AG_BattMonitor_FuelLevel_PWM(AG_BattMonitor &mon, AG_BattMonitor::BattMonitor_State &mon_state, AG_BattMonitor_Params &params);

    /// Read the battery voltage and current.  Should be called at 10hz
    void read() override;

    /// returns true if battery monitor provides consumed energy info
    bool has_consumed_energy() const override { return true; }

    /// returns true if battery monitor provides current info
    bool has_current() const override { return true; }

    void init(void) override {}

private:

    AG_HAL::PWMSource pwm_source;
};

#endif  // AP_BATTERY_FUELLEVEL_PWM_ENABLED

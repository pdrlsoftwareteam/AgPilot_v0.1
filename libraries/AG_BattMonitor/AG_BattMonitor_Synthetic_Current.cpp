#include "AG_BattMonitor_config.h"

#if AP_BATTERY_SYNTHETIC_CURRENT_ENABLED

#include <AG_HAL/AG_HAL.h>
#include "AG_BattMonitor_Synthetic_Current.h"
#include <GCS_MAVLink/GCS.h>
#include <SRV_Channel/SRV_Channel.h>

/*
  Analog Voltage and Current Monitor for systems with only a voltage sense pin
  Current is calculated from throttle output and modified for voltage droop using
  a square law calculation
 */
extern const AG_HAL::HAL& hal;

const AG_Param::GroupInfo AG_BattMonitor_Synthetic_Current::var_info[] = {

    // @Param: MAX_VOLT
    // @DisplayName: Maximum Battery Voltage
    // @Description: Maximum voltage of battery. Provides scaling of current versus voltage
    // @Range: 7 100
    // @User: Advanced

    AP_GROUPINFO("MAX_VOLT", 50, AG_BattMonitor_Synthetic_Current, _max_voltage, 12.6),
    
    // also inherit analog backend parameters
    AP_SUBGROUPEXTENSION("", 51, AG_BattMonitor_Synthetic_Current, AG_BattMonitor_Analog::var_info),

    // Param indexes must be between 50 and 55 to avoid conflict with other battery monitor param tables loaded by pointer

    AP_GROUPEND
};

/// Constructor
AG_BattMonitor_Synthetic_Current::AG_BattMonitor_Synthetic_Current(AG_BattMonitor &mon,
                                                 AG_BattMonitor::BattMonitor_State &mon_state,
                                                 AG_BattMonitor_Params &params) :
    AG_BattMonitor_Analog(mon, mon_state, params)
{
    AG_Param::setup_object_defaults(this, var_info);
    _state.var_info = var_info;
    
    _volt_pin_analog_source = hal.analogin->channel(_volt_pin);
}

// read - read the voltage and current
void
AG_BattMonitor_Synthetic_Current::read()
{
    // this copes with changing the pin at runtime
    _state.healthy = _volt_pin_analog_source->set_pin(_volt_pin);

    // get voltage
    _state.voltage = (_volt_pin_analog_source->voltage_average() - _volt_offset) * _volt_multiplier;

    // read current
    // calculate time since last current read
    const uint32_t tnow = AG_HAL::micros();
    const uint32_t dt_us = tnow - _state.last_time_micros;

    // this copes with changing the pin at runtime
    _state.healthy &= _curr_pin_analog_source->set_pin(_curr_pin);

    // read current
    _state.current_amps = ((_state.voltage/_max_voltage)*sq(SRV_Channels::get_output_scaled(SRV_Channel::k_throttle)) * 0.0001 * _curr_amp_per_volt) + _curr_amp_offset ;

    update_consumed(_state, dt_us);

    // record time
    _state.last_time_micros = tnow;
 
}

#endif  // AP_BATTERY_SYNTHETIC_CURRENT_ENABLED

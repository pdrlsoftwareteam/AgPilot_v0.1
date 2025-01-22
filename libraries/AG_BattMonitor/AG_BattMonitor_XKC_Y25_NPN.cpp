#include <AG_HAL/AG_HAL.h>
#include "AG_BattMonitor_XKC_Y25_NPN.h"
#include <GCS_MAVLink/GCS.h>

/*
  "battery" monitor for liquid fuel flow systems that give a pulse on
  a pin for fixed volumes of fuel.

  this driver assumes that BATTx_AMP_PERVLT is set to give the
  number of millilitres per pulse.

  Output is:

    - current in Amps maps to in litres/hour
    - consumed mAh is in consumed millilitres
    - fixed 1.0v voltage
 */
extern const AG_HAL::HAL& hal;
#define FUELFLOW_MIN_PULSE_DELTA_US 10

/// Constructor
AG_BattMonitor_XKC_Y25_NPN::AG_BattMonitor_XKC_Y25_NPN(AG_BattMonitor &mon,
                                                 AG_BattMonitor::BattMonitor_State &mon_state,
                                                 AG_BattMonitor_Params &params) :
    AG_BattMonitor_Analog(mon, mon_state, params)
{
	 AG_Param::setup_object_defaults(this, var_info);
    _state.voltage = 1.0; // show a fixed voltage of 1v

    // we can't tell if it is healthy as we expect zero pulses when no
    // fuel is flowing
    _state.healthy = true;
    _state.has_time_remaining = true;
}

/*
  handle interrupt on an instance
 */
void AG_BattMonitor_XKC_Y25_NPN::irq_handler(uint8_t pin, bool pin_state, uint32_t timestamp)
{
    if (irq_state.last_pulse_us == 0) {
        irq_state.last_pulse_us = timestamp;
        return;
    }
    uint32_t delta = timestamp - irq_state.last_pulse_us;
    if (delta < FUELFLOW_MIN_PULSE_DELTA_US) {
        // simple de-bounce
        return;
    }
    irq_state.pulse_count++;
    irq_state.total_us += delta;
    irq_state.last_pulse_us = timestamp;
}

/*
  read - read the "voltage" and "current"
*/
void AG_BattMonitor_XKC_Y25_NPN::read()
{
    int8_t pin_out = _volt_pin;

    if (last_pin1 != pin_out)
    {
    	// attach to new pin
        last_pin1 = pin_out;

        if (last_pin1 > 0)
        {
            hal.gpio->pinMode(pin_out, HAL_GPIO_INPUT);
        }
    }

        		static int8_t get_Status = hal.gpio->read(pin_out);
        		static int8_t pre_Status = !hal.gpio->read(pin_out);
        		get_Status = hal.gpio->read(pin_out);
        		if(((AG_HAL::millis() - now_us) > 5000) || get_Status)
        		    		{
        		    	     	now_us = AG_HAL::millis();
        		    			if((get_Status != pre_Status))
        		    			{
        							pre_Status = get_Status;
        							if(get_Status)
        								gcs().send_text(MAV_SEVERITY_INFO,"Tank Empty");
        							else
        								gcs().send_text(MAV_SEVERITY_INFO,"Tank Full");
        		    			}
        		    		}
    _state.last_time_micros = now_us;

    // map amps to litres/hour
    //_state.current_amps = litres_pec_sec * (60*60);

    // map consumed_mah to consumed millilitres


    // map consumed_wh using fixed voltage of 1
    //   _state.consumed_wh = _state.consumed_mah;
}

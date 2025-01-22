#include "AG_BattMonitor_config.h"

#if AP_BATTERY_FUELFLOW_ENABLED

#include "AG_BattMonitor_FuelFlow.h"

#include <AG_HAL/AG_HAL.h>
#include <GCS_MAVLink/GCS.h>
#include "AG_Sprayer/AG_Sprayer.h"
#include "AG_AHRS/AG_AHRS.h"
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
AG_BattMonitor_FuelFlow::AG_BattMonitor_FuelFlow(AG_BattMonitor &mon,
                                                 AG_BattMonitor::BattMonitor_State &mon_state,
                                                 AG_BattMonitor_Params &params) :
    AG_BattMonitor_Analog(mon, mon_state, params)
{
    _state.voltage = 1.0; // show a fixed voltage of 1v

    // we can't tell if it is healthy as we expect zero pulses when no
    // fuel is flowing
    _state.healthy = true;
      _state.has_time_remaining = true;
}

/*
  handle interrupt on an instance
 */
void AG_BattMonitor_FuelFlow::irq_handler(uint8_t pin, bool pin_state, uint32_t timestamp)
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
void AG_BattMonitor_FuelFlow::read()
{
    static uint8_t pulse_count_history[5] = {0}; // Circular buffer for last 5 readings
    static uint8_t count = 0;                   // Number of elements added to the history
    static bool is_history_initialized = false; // Flag to indicate if history is fully initialized

    int8_t pin = _curr_pin;
    if (last_pin != pin) {
        // detach from last pin
        if (last_pin != -1) {
            hal.gpio->detach_interrupt(last_pin);
        }
        // attach to new pin
        last_pin = pin;
        if (last_pin > 0) {
            hal.gpio->pinMode(last_pin, HAL_GPIO_INPUT);
            if (!hal.gpio->attach_interrupt(
                    last_pin,
                    FUNCTOR_BIND_MEMBER(&AG_BattMonitor_FuelFlow::irq_handler, void, uint8_t, bool, uint32_t),
                    AG_HAL::GPIO::INTERRUPT_RISING)) {
                GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "FuelFlow: Failed to attach to pin %u", last_pin);
            }
        }
    }

    uint32_t now_us = AG_HAL::micros();
    if (_state.last_time_micros == 0) {
        // need initial time, so we can work out expected pulse rate
        _state.last_time_micros = now_us;
        return;
    }
    float dt = (now_us - _state.last_time_micros) * 1.0e-6f;

    if (dt < 1 && irq_state.pulse_count == 0) {
        // we allow for up to 1 second with no pulses to cope with low
        // flow idling. After that we will start reading zero current
        return;
    }
    
    // get the IRQ state with interrupts disabled
    struct IrqState state;
    void *irqstate = hal.scheduler->disable_interrupts_save();
    state = irq_state;
    irq_state.pulse_count = 0;
    irq_state.total_us = 0;
    hal.scheduler->restore_interrupts(irqstate);

    /*
      this driver assumes that BATTx_AMP_PERVLT is set to give the
      number of millilitres per pulse.
     */
    float irq_dt = state.total_us * 1.0e-6f;
    float litres, litres_pec_sec;
    if (state.pulse_count == 0) {
        litres = 0;
        litres_pec_sec = 0;
    } else {
        litres = state.pulse_count * _curr_amp_per_volt * 0.001f;
        litres_pec_sec = litres / irq_dt;
    }

    _state.last_time_micros = now_us;

    // map amps to litres/hour
    _state.current_amps = litres_pec_sec * (60*60);

    // map consumed_mah to consumed millilitres
    _state.consumed_mah += litres * 1000;

    // map consumed_wh using fixed voltage of 1
    _state.consumed_wh = _state.consumed_mah;
    _state.time_remaining += state.pulse_count;

    // Save history and check tank status only if the sprayer is enabled and ground speed is not zero
    AG_AHRS &ahrs = AP::ahrs();

    float gndSpeed = ahrs.groundspeed();

    if (AP::sprayer()->spraying() && gndSpeed >= 1.0f ) {
        // Update pulse count history
        pulse_count_history[count % 5] = state.pulse_count;
        count++;
        if (count >= 5) {
            is_history_initialized = true; // History is fully populated
        }

//        gcs().send_text(MAV_SEVERITY_WARNING, "%d %d %d %d %d",pulse_count_history[0],pulse_count_history[1],pulse_count_history[2],pulse_count_history[3],pulse_count_history[4]);

        // Tank empty check
        static uint32_t last_ms = AG_HAL::millis();
        bool is_tank_empty = true;
        if (is_history_initialized) {
            for (uint8_t i = 0; i < 5; i++) {
                if (pulse_count_history[i] > 2) {
                    is_tank_empty = false;
                    break;
                }
            }
        } else {
            // If not fully initialized, consider tank not empty
            is_tank_empty = false;
        }

        if (is_tank_empty && (AG_HAL::millis() - last_ms > 2000)) {
            static int gcs_count = 0;
            gcs().send_text(MAV_SEVERITY_WARNING, "Pani Samplay %d", gcs_count++);
			gcs().send_text(MAV_SEVERITY_INFO,"Tank Empty");

            last_ms = AG_HAL::millis();
        }
    }
}

#endif  // AP_BATTERY_FUELFLOW_ENABLED

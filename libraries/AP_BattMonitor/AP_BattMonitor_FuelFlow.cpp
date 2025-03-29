#include "AP_BattMonitor_config.h"

#if AP_BATTERY_FUELFLOW_ENABLED

#include "AP_BattMonitor_FuelFlow.h"

#include <AP_HAL/AP_HAL.h>
#include <GCS_MAVLink/GCS.h>
#include "AC_Sprayer/AC_Sprayer.h"
#include "AP_AHRS/AP_AHRS.h"
#include "AP_Arming/AP_Arming.h"
#include "AP_BattMonitor.h"


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
extern const AP_HAL::HAL& hal;

#define FUELFLOW_MIN_PULSE_DELTA_US 10

/// Constructor
AP_BattMonitor_FuelFlow::AP_BattMonitor_FuelFlow(AP_BattMonitor &mon,
		AP_BattMonitor::BattMonitor_State &mon_state,
		AP_BattMonitor_Params &params) :
    																				AP_BattMonitor_Analog(mon, mon_state, params)
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
void AP_BattMonitor_FuelFlow::irq_handler(uint8_t pin, bool pin_state, uint32_t timestamp)
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

void AP_BattMonitor_FuelFlow::handle_calibration()
{
	bool calib_active = AP::sprayer()->fuelFlow_Calib();

	if (calib_active && !calibration_active) {
		// Calibration just started
		calibration_active = true;
		calibration_start_mah = _state.consumed_mah;
		gcs().send_text(MAV_SEVERITY_INFO, "FuelFlow Calibration Started: Resetting consumed liquid");
	}

	if (!calib_active && calibration_active) {
		// Calibration just stopped
		calibration_active = false;
		float calibrated_mah = _state.consumed_mah - calibration_start_mah; // Total consumed during calibration
		//        float batt_cap = _params._pack_capacity;
		if (calibrated_mah > 0) {
			_curr_amp_per_volt.set_and_save((2000.0f / calibrated_mah) * _curr_amp_per_volt.get());

			gcs().send_text(MAV_SEVERITY_INFO, "FuelFlow Calibration Stopped: Updated AMP_PERVLT = %.6f", _curr_amp_per_volt.get());
			_state.consumed_mah = 0;
		} else {
			gcs().send_text(MAV_SEVERITY_WARNING, "FuelFlow Calibration Stopped: No fuel detected");
		}
	}
}

/*
  read - read the "voltage" and "current"
 */
void AP_BattMonitor_FuelFlow::read()
{
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
					FUNCTOR_BIND_MEMBER(&AP_BattMonitor_FuelFlow::irq_handler, void, uint8_t, bool, uint32_t),
					AP_HAL::GPIO::INTERRUPT_RISING)) {
				GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "FuelFlow: Failed to attach to pin %u", last_pin);
			}
		}
	}

	uint32_t now_us = AP_HAL::micros();
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

	AP::battery().consumed_liquid = _state.consumed_mah;

	handle_calibration();

	float consumed_diff = _state.consumed_mah - last_consumed_mah;

	if(AP::sprayer()->spraying())
	{
		if((consumed_diff < 1.50f) && (_state.consumed_mah > 30.0f) && state.pulse_count == 0 )
		{
			AP::sprayer()->setPulseCount(0);
			AP::sprayer()->setTankstatus(1);
			gcs().send_text(MAV_SEVERITY_WARNING, "Tank Empty");
		}
		else
		{
			AP::sprayer()->setPulseCount(1);
			AP::sprayer()->setTankstatus(0);
		}

		AP::sprayer()->setPulseCount(state.pulse_count);
	}
	else
	{
		AP::sprayer()->setPulseCount(0);
		AP::sprayer()->setTankstatus(0);
	}

	last_consumed_mah = _state.consumed_mah;

	//Spray area calculation
	static uint64_t dist_tm = AP_HAL::millis();
	//	const float EPSILON = 1e-5;  // Small threshold for floating-point comparison
	if(AP::sprayer()->spraying() && AP::arming().is_armed())
	{

		static Vector2f current_Loc = AP::battery().get_val(),previous_Loc = AP::battery().get_val();
		current_Loc = AP::battery().get_val();

		if (AP_HAL::millis() - dist_tm > 200)
		{
			AP::battery().spray_dist += sqrt(pow(previous_Loc.x/100 - current_Loc.x/100, 2) + pow(previous_Loc.y/100 - current_Loc.y/100, 2));
			previous_Loc = current_Loc;
			current_Loc = AP::battery().get_val();
			AP::battery().spray_area_sqm = AP::battery().spray_dist*3.5;
			AP::battery().spray_area_acre = AP::battery().spray_area_sqm*0.000247105;
			dist_tm = AP_HAL::millis();
		}
	}

	static uint64_t spray_tm = AP_HAL::millis();
	if(AP::arming().is_armed())
	{
		if(AP::sprayer()->spraying())
		{
			if (AP_HAL::millis() - spray_tm > 200)
			{
				AP::battery().spray_time += AP_HAL::millis() - spray_tm;
				spray_tm = AP_HAL::millis();
			}
		}
		else
		{
			spray_tm = AP_HAL::millis();
		}
	}

	static uint64_t flight_time_temp = AP_HAL::millis();
	static bool first_arm = true;
	if(AP::arming().is_armed())
	{
		if(first_arm)
		{
			AP::battery().flight_time = 0;
			AP::battery().flight_dist = 0;
			AP::battery().spray_time = 0;  // Reset spray time
			AP::battery().spray_area_sqm = 0;
			AP::battery().spray_area_acre = 0;
			AP::battery().spray_dist = 0;
			first_arm = 0;
			_state.consumed_mah = 0;
			AP::battery().consumed_liquid = 0;
		}
		static Vector2f flight_current_Loc = AP::battery().get_val(),flight_previous_Loc = AP::battery().get_val();

		if (AP_HAL::millis() - flight_time_temp > 200)
		{
			AP::battery().flight_dist += sqrt(pow(flight_previous_Loc.x/100 - flight_current_Loc.x/100, 2) + pow(flight_previous_Loc.y/100 - flight_current_Loc.y/100, 2));
			flight_previous_Loc = flight_current_Loc;
			flight_current_Loc = AP::battery().get_val();
			AP::battery().flight_time += AP_HAL::millis() - flight_time_temp;
			flight_time_temp = AP_HAL::millis();
		}
	}
	else if(!AP::arming().is_armed())
	{
		first_arm = 1;
		flight_time_temp = AP_HAL::millis();
		spray_tm = AP_HAL::millis();
		dist_tm = AP_HAL::millis();
		AP::sprayer()->setTankstatus(0);
	}

}
#endif  // AP_BATTERY_FUELFLOW_ENABLED

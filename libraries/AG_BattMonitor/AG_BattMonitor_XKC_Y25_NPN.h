#pragma once

#include "AG_BattMonitor.h"
#include "AG_BattMonitor_Analog.h"

#define DEFAULT_MIN_RAW  (long)7645000

#define DEFAULT_MAX_RAW  (long)8628000



class AG_BattMonitor_XKC_Y25_NPN : public AG_BattMonitor_Analog
{
public:

	/// Constructor
	AG_BattMonitor_XKC_Y25_NPN(AG_BattMonitor &mon, AG_BattMonitor::BattMonitor_State &mon_state, AG_BattMonitor_Params &params);

	/// Read the battery voltage and current.  Should be called at 10hz
	void read() override;

	/// returns true if battery monitor provides consumed energy info
	bool has_consumed_energy() const override { return true; }

	/// returns true if battery monitor provides current info
	bool has_current() const override { return true; }

	void init(void) override {}

	//static const struct AG_Param::GroupInfo  var_info[];

	enum HX_MODE { NONE, DIFF_10Hz, TEMP_40Hz, DIFF_40Hz};
	const uint8_t HX_MODE = DIFF_40Hz;

   // static const struct AG_Param::GroupInfo var_info[];

	long result;
	uint32_t now_us = AG_HAL::millis();
	long exp1;
	double exp2;
	double Fuel_Remaining;


private:
	void irq_handler(uint8_t pin, bool pin_state, uint32_t timestamp);

	struct IrqState {
		uint32_t pulse_count;
		uint32_t total_us;
		uint32_t last_pulse_us;
	} irq_state;

	int8_t last_pin1 = -1;
};

#pragma once

#include <AG_HAL/AG_HAL_Boards.h>

#ifndef AP_AIRSPEED_ANALOG_ENABLED
#define AP_AIRSPEED_ANALOG_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif

#if AP_AIRSPEED_ANALOG_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_Param/AG_Param.h>

#include "AG_Airspeed_Backend.h"

class AG_Airspeed_Analog : public AG_Airspeed_Backend
{
public:
    AG_Airspeed_Analog(AG_Airspeed &frontend, uint8_t _instance);

    // probe and initialise the sensor
    bool init(void) override;

    // return the current differential_pressure in Pascal
    bool get_differential_pressure(float &pressure) override;

    // temperature not available via analog backend
    bool get_temperature(float &temperature) override { return false; }

private:
    AG_HAL::AnalogSource *_source;
};

#endif  // AP_AIRSPEED_ANALOG_ENABLED

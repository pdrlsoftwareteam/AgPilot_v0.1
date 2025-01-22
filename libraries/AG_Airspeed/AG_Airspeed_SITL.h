/*
  SITL airspeed backend - a perfect airspeed sensor
 */
#pragma once

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/AG_HAL_Boards.h>

#ifndef AP_AIRSPEED_SITL_ENABLED
#define AP_AIRSPEED_SITL_ENABLED AP_SIM_ENABLED
#endif

#if AP_AIRSPEED_SITL_ENABLED

#include "AG_Airspeed_Backend.h"

class AG_Airspeed_SITL : public AG_Airspeed_Backend
{
public:

    using AG_Airspeed_Backend::AG_Airspeed_Backend;

    bool init(void) override {
        return true;
    }

    // return the current differential_pressure in Pascal
    bool get_differential_pressure(float &pressure) override;

    // temperature not available via analog backend
    bool get_temperature(float &temperature) override;

private:
};

#endif // AP_AIRSPEED_SITL_ENABLED

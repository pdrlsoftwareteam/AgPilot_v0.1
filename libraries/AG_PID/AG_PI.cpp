/*
  Generic PI algorithm for controllers that don't need filtering (such as heaters)
*/

#include <AG_Math/AG_Math.h>
#include "AG_PI.h"

const AG_Param::GroupInfo AG_PI::var_info[] = {
    // @Param: P
    // @DisplayName: PID Proportional Gain
    // @Description: P Gain which produces an output value that is proportional to the current error value
    AP_GROUPINFO_FLAGS_DEFAULT_POINTER("P",    1, AG_PI, kP, default_kp),

    // @Param: I
    // @DisplayName: PID Integral Gain
    // @Description: I Gain which produces an output that is proportional to both the magnitude and the duration of the error
    AP_GROUPINFO_FLAGS_DEFAULT_POINTER("I",    2, AG_PI, kI, default_ki),

    // @Param: IMAX
    // @DisplayName: PID Integral Maximum
    // @Description: The maximum/minimum value that the I term can output
    AP_GROUPINFO_FLAGS_DEFAULT_POINTER("IMAX", 3, AG_PI, imax, default_imax),

    AP_GROUPEND
};

// Constructor
AG_PI::AG_PI(float initial_p, float initial_i, float initial_imax) :
    default_kp(initial_p),
    default_ki(initial_i),
    default_imax(initial_imax)
{
    // load parameter values from eeprom
    AG_Param::setup_object_defaults(this, var_info);
}

float AG_PI::update(float measurement, float target, float dt)
{
    const float err = target - measurement;

    integrator += kI * err * dt;
    integrator = constrain_float(integrator, 0, imax);
    output_P = kP * err;

    return output_P + integrator;
}

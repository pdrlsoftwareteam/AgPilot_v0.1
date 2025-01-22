#pragma once

/*
 Generic PI for systems like heater control, no filtering
*/

#include <AG_Common/AG_Common.h>
#include <AG_Param/AG_Param.h>

class AG_PI {
public:
    // Constructor
    AG_PI(float initial_p, float initial_i, float initial_imax);

    CLASS_NO_COPY(AG_PI);

    // update controller
    float update(float measurement, float target, float dt);

    // parameter var table
    static const struct AG_Param::GroupInfo var_info[];

    float get_P() const {
        return output_P;
    }
    float get_I() const {
        return integrator;
    }

protected:
    AP_Float        kP;
    AP_Float        kI;
    AP_Float        imax;
    float           integrator;
    float           output_P;

private:
    const float default_kp;
    const float default_ki;
    const float default_imax;

};

#pragma once

#include "AG_CustomControl.h"

#if AP_CUSTOMCONTROL_ENABLED

class AG_CustomControl_Backend
{
public:
    AG_CustomControl_Backend(AG_CustomControl& frontend, AG_AHRS_View*& ahrs, AG_AttitudeControl_Multi*& att_control, AG_MotorsMulticopter*& motors, float dt) :
        _frontend(frontend),
        _ahrs(ahrs),
        _att_control(att_control),
        _motors(motors)
    {}

    // empty destructor to suppress compiler warning
    virtual ~AG_CustomControl_Backend() {}

    // update controller, return roll, pitch, yaw controller output
    virtual Vector3f update() = 0;

    // reset controller to avoid build up or abrupt response upon switch, ex: integrator, filter
    virtual void reset() = 0;

protected:
    // References to external libraries
    AG_AHRS_View*& _ahrs;
    AG_AttitudeControl_Multi*& _att_control;
    AG_MotorsMulticopter*& _motors;
    AG_CustomControl& _frontend;
};

#endif

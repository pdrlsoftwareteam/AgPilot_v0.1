#pragma once

#include <AG_Common/AG_Common.h>
#include <AG_Param/AG_Param.h>
#include <AG_PID/AG_PID.h>
#include <AG_PID/AG_P.h>

#include "AG_CustomControl_Backend.h"

#ifndef CUSTOMCONTROL_PID_ENABLED
    #define CUSTOMCONTROL_PID_ENABLED AP_CUSTOMCONTROL_ENABLED
#endif

#if CUSTOMCONTROL_PID_ENABLED

class AG_CustomControl_PID : public AG_CustomControl_Backend {
public:
    AG_CustomControl_PID(AG_CustomControl& frontend, AG_AHRS_View*& ahrs, AG_AttitudeControl_Multi*& att_control, AG_MotorsMulticopter*& motors, float dt);

    // run lowest level body-frame rate controller and send outputs to the motors
    Vector3f update() override;
    void reset(void) override;

    // user settable parameters
    static const struct AG_Param::GroupInfo var_info[];

protected:
    // put controller related variable here

    // angle P controller  objects
    AG_P                _p_angle_roll2;
    AG_P                _p_angle_pitch2;
    AG_P                _p_angle_yaw2;

	// rate PID controller  objects
    AG_PID _pid_atti_rate_roll;
    AG_PID _pid_atti_rate_pitch;
    AG_PID _pid_atti_rate_yaw;
};

#endif

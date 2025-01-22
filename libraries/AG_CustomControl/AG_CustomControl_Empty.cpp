#include "AG_CustomControl_Empty.h"

#if CUSTOMCONTROL_EMPTY_ENABLED

#include <GCS_MAVLink/GCS.h>

// table of user settable parameters
const AG_Param::GroupInfo AG_CustomControl_Empty::var_info[] = {
    // @Param: PARAM1
    // @DisplayName: Empty param1
    // @Description: Dumy parameter for empty custom controller backend
    // @User: Advanced
    AP_GROUPINFO("PARAM1", 1, AG_CustomControl_Empty, param1, 0.0f),

    // @Param: PARAM2
    // @DisplayName: Empty param2
    // @Description: Dumy parameter for empty custom controller backend
    // @User: Advanced
    AP_GROUPINFO("PARAM2", 2, AG_CustomControl_Empty, param2, 0.0f),

    // @Param: PARAM3
    // @DisplayName: Empty param3
    // @Description: Dumy parameter for empty custom controller backend
    // @User: Advanced
    AP_GROUPINFO("PARAM3", 3, AG_CustomControl_Empty, param3, 0.0f),

    AP_GROUPEND
};

// initialize in the constructor
AG_CustomControl_Empty::AG_CustomControl_Empty(AG_CustomControl& frontend, AG_AHRS_View*& ahrs, AG_AttitudeControl_Multi*& att_control, AG_MotorsMulticopter*& motors, float dt) :
    AG_CustomControl_Backend(frontend, ahrs, att_control, motors, dt)
{
    AG_Param::setup_object_defaults(this, var_info);
}

// update controller
// return roll, pitch, yaw controller output
Vector3f AG_CustomControl_Empty::update(void)
{
    // reset controller based on spool state
    switch (_motors->get_spool_state()) {
        case AG_Motors::SpoolState::SHUT_DOWN:
        case AG_Motors::SpoolState::GROUND_IDLE:
            // We are still at the ground. Reset custom controller to avoid
            // build up, ex: integrator
            reset();
            break;

        case AG_Motors::SpoolState::THROTTLE_UNLIMITED:
        case AG_Motors::SpoolState::SPOOLING_UP:
        case AG_Motors::SpoolState::SPOOLING_DOWN:
            // we are off the ground
            break;
    }

    // arducopter main attitude controller already runned
    // we don't need to do anything else

    gcs().send_text(MAV_SEVERITY_INFO, "empty custom controller working");

    // return what arducopter main controller outputted
    return Vector3f(_motors->get_roll(), _motors->get_pitch(), _motors->get_yaw());
}

// reset controller to avoid build up on the ground
// or to provide bumpless transfer from arducopter main controller
void AG_CustomControl_Empty::reset(void)
{
}

#endif

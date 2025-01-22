#pragma once

#include "AG_CustomControl_Backend.h"

#ifndef CUSTOMCONTROL_EMPTY_ENABLED
    #define CUSTOMCONTROL_EMPTY_ENABLED AP_CUSTOMCONTROL_ENABLED
#endif

#if CUSTOMCONTROL_EMPTY_ENABLED

class AG_CustomControl_Empty : public AG_CustomControl_Backend {
public:
    AG_CustomControl_Empty(AG_CustomControl& frontend, AG_AHRS_View*& ahrs, AG_AttitudeControl_Multi*& att_control, AG_MotorsMulticopter*& motors, float dt);


    Vector3f update(void) override;
    void reset(void) override;

    // user settable parameters
    static const struct AG_Param::GroupInfo var_info[];

protected:
    // declare parameters here
    AP_Float param1;
    AP_Float param2;
    AP_Float param3;
};

#endif

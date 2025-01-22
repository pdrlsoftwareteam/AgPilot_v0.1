#pragma once

/// @file    AG_CustomControl.h
/// @brief   ArduCopter custom control library

#include <AG_Common/AG_Common.h>
#include <AG_Param/AG_Param.h>
#include <AG_AHRS/AG_AHRS_View.h>
#include <AG_AttitudeControl/AG_AttitudeControl_Multi.h>
#include <AG_Motors/AG_MotorsMulticopter.h>
#include <AG_Logger/AG_Logger.h>

#if AP_CUSTOMCONTROL_ENABLED

#ifndef CUSTOMCONTROL_MAX_TYPES
#define CUSTOMCONTROL_MAX_TYPES 2
#endif

class AG_CustomControl_Backend;

class AG_CustomControl {
public:
    AG_CustomControl(AG_AHRS_View*& ahrs, AG_AttitudeControl_Multi*& _att_control, AG_MotorsMulticopter*& motors, float dt);

    CLASS_NO_COPY(AG_CustomControl);  /* Do not allow copies */

    void init(void);
    void update(void);
    void motor_set(Vector3f motor_out);
    void set_custom_controller(bool enabled);
    void reset_main_att_controller(void);
    bool is_safe_to_run(void);
    void log_switch(void);

    // zero index controller type param, only use it to acces _backend or _backend_var_info array
    uint8_t get_type() { return _controller_type > 0 ? (_controller_type - 1) : 0; };

    // User settable parameters
    static const struct AG_Param::GroupInfo var_info[];
    static const struct AG_Param::GroupInfo *_backend_var_info[CUSTOMCONTROL_MAX_TYPES];

protected:
    // add custom controller here
    enum class CustomControlType : uint8_t {
        CONT_NONE            = 0,
        CONT_EMPTY           = 1,
        CONT_PID             = 2,
    };            // controller that should be used     

    enum class  CustomControlOption {
        ROLL = 1 << 0,
        PITCH = 1 << 1,
        YAW = 1 << 2,
    };

    // Intersampling period in seconds
    float _dt;
    bool _custom_controller_active;

    // References to external libraries
    AG_AHRS_View*& _ahrs;
    AG_AttitudeControl_Multi*& _att_control;
    AG_MotorsMulticopter*& _motors;

    AP_Enum<CustomControlType> _controller_type;
    AP_Int8 _custom_controller_mask;

private:
    AG_CustomControl_Backend *_backend;
};

#endif

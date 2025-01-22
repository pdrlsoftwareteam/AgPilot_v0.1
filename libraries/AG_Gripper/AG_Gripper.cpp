#include "AG_Gripper.h"

#if AP_GRIPPER_ENABLED

#include "AG_Gripper_Servo.h"
#include "AG_Gripper_EPM.h"

extern const AG_HAL::HAL& hal;

#define GRIPPER_GRAB_PWM_DEFAULT        1900
#define GRIPPER_RELEASE_PWM_DEFAULT     1100
// EPM PWM definitions
#define GRIPPER_NEUTRAL_PWM_DEFAULT     1500
#define GRIPPER_REGRAB_DEFAULT          0           // default EPM re-grab interval (in seconds) to ensure cargo is securely held
#define GRIPPER_AUTOCLOSE_DEFAULT       0.0         // default automatic close time (in seconds)

const AG_Param::GroupInfo AG_Gripper::var_info[] = {
    // @Param: ENABLE
    // @DisplayName: Gripper Enable/Disable
    // @Description: Gripper enable/disable
    // @User: Standard
    // @Values: 0:Disabled, 1:Enabled
    AP_GROUPINFO_FLAGS("ENABLE", 0, AG_Gripper, _enabled, 0, AP_PARAM_FLAG_ENABLE),

    // @Param: TYPE
    // @DisplayName: Gripper Type
    // @Description: Gripper enable/disable
    // @User: Standard
    // @Values: 0:None,1:Servo,2:EPM
    AP_GROUPINFO("TYPE", 1, AG_Gripper, config.type, 0),

    // @Param: GRAB
    // @DisplayName: Gripper Grab PWM
    // @Description: PWM value in microseconds sent to Gripper to initiate grabbing the cargo
    // @User: Advanced
    // @Range: 1000 2000
    // @Units: PWM
    AP_GROUPINFO("GRAB",    2, AG_Gripper, config.grab_pwm, GRIPPER_GRAB_PWM_DEFAULT),

    // @Param: RELEASE
    // @DisplayName: Gripper Release PWM
    // @Description: PWM value in microseconds sent to Gripper to release the cargo
    // @User: Advanced
    // @Range: 1000 2000
    // @Units: PWM
    AP_GROUPINFO("RELEASE", 3, AG_Gripper, config.release_pwm, GRIPPER_RELEASE_PWM_DEFAULT),

    // @Param: NEUTRAL
    // @DisplayName: Neutral PWM
    // @Description: PWM value in microseconds sent to grabber when not grabbing or releasing
    // @User: Advanced
    // @Range: 1000 2000
    // @Units: PWM
    AP_GROUPINFO("NEUTRAL", 4, AG_Gripper, config.neutral_pwm, GRIPPER_NEUTRAL_PWM_DEFAULT),

    // @Param: REGRAB
    // @DisplayName: EPM Gripper Regrab interval
    // @Description: Time in seconds that EPM gripper will regrab the cargo to ensure grip has not weakened; 0 to disable
    // @User: Advanced
    // @Range: 0 255
    // @Units: s
    AP_GROUPINFO("REGRAB",  5, AG_Gripper, config.regrab_interval, GRIPPER_REGRAB_DEFAULT),

    // @Param: CAN_ID
    // @DisplayName: EPM UAVCAN Hardpoint ID
    // @Description: Refer to https://docs.zubax.com/opengrab_epm_v3#UAVCAN_interface
    // @User: Standard
    // @Range: 0 255
    AP_GROUPINFO("CAN_ID", 6, AG_Gripper, config.uavcan_hardpoint_id, 0),

    // @Param: AUTOCLOSE
    // @DisplayName: Gripper Autoclose time
    // @Description: Time in seconds that gripper close the gripper after opening; 0 to disable
    // @User: Advanced
    // @Range: 0.25 255
    // @Units: s
    AP_GROUPINFO("AUTOCLOSE",  7, AG_Gripper, config.autoclose_time, GRIPPER_AUTOCLOSE_DEFAULT),

    AP_GROUPEND
};

AG_Gripper::AG_Gripper()
{
    if (_singleton) {
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
        AG_HAL::panic("Too many grippers");
#endif
        return;
    }
    _singleton = this;

    AG_Param::setup_object_defaults(this, var_info);
}

/*
 * Get the AG_Gripper singleton
 */
AG_Gripper *AG_Gripper::_singleton = nullptr;
AG_Gripper *AG_Gripper::get_singleton()
{
    return _singleton;
}

void AG_Gripper::init()
{
    // return immediately if not enabled
    if (!_enabled.get()) {
        return;
    }

    switch(config.type.get()) {
    case 0:
        break;
#if AP_GRIPPER_SERVO_ENABLED
    case 1:
        backend = new AG_Gripper_Servo(config);
        break;
#endif
#if AP_GRIPPER_EPM_ENABLED
    case 2:
        backend = new AG_Gripper_EPM(config);
        break;
#endif
    default:
        break;
    }
    if (backend != nullptr) {
        backend->init();
    }
}

// update - should be called at at least 10hz
#define PASS_TO_BACKEND(function_name) \
    void AG_Gripper::function_name()   \
    {                                  \
        if (!enabled()) {              \
            return;                    \
        }                              \
        if (backend != nullptr) {      \
            backend->function_name();  \
        }                              \
    }

PASS_TO_BACKEND(grab)
PASS_TO_BACKEND(release)
PASS_TO_BACKEND(update)

#undef PASS_TO_BACKEND


#define PASS_TO_BACKEND(function_name)        \
    bool AG_Gripper::function_name() const    \
    {                                         \
        if (!enabled()) {                     \
            return false;                     \
        }                                     \
        if (backend != nullptr) {             \
            return backend->function_name();  \
        }                                     \
        return false;                         \
    }

PASS_TO_BACKEND(valid)
PASS_TO_BACKEND(released)
PASS_TO_BACKEND(grabbed)

#undef PASS_TO_BACKEND

namespace AP {

AG_Gripper *gripper()
{
    return AG_Gripper::get_singleton();
}

};

#endif  // AP_GRIPPER_ENABLED

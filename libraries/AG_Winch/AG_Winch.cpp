#include "AG_Winch.h"

#if AP_WINCH_ENABLED

#include "AG_Winch_PWM.h"
#include "AG_Winch_Daiwa.h"

extern const AG_HAL::HAL& hal;

const AG_Param::GroupInfo AG_Winch::var_info[] = {
    // 0 was ENABLE

    // @Param: _TYPE
    // @DisplayName: Winch Type
    // @Description: Winch Type
    // @User: Standard
    // @Values: 0:None, 1:PWM, 2:Daiwa
    AP_GROUPINFO_FLAGS("_TYPE", 1, AG_Winch, config.type, (int8_t)WinchType::NONE, AP_PARAM_FLAG_ENABLE),

    // @Param: _RATE_MAX
    // @DisplayName: Winch deploy or retract rate maximum
    // @Description: Winch deploy or retract rate maximum.  Set to maximum rate with no load.
    // @User: Standard
    // @Range: 0 10
    // @Units: m/s
    AP_GROUPINFO("_RATE_MAX", 2, AG_Winch, config.rate_max, 1.0f),

    // @Param: _POS_P
    // @DisplayName: Winch control position error P gain
    // @Description: Winch control position error P gain
    // @Range: 0.01 10.0
    // @User: Standard
    AP_GROUPINFO("_POS_P", 3, AG_Winch, config.pos_p, 1.0f),

    // 4 was _RATE_PID

    AP_GROUPEND
};

AG_Winch::AG_Winch()
{
    if (_singleton) {
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
        AG_HAL::panic("Too many winches");
#endif
        return;
    }
    _singleton = this;

    AG_Param::setup_object_defaults(this, var_info);
}

// indicate whether this module is enabled
bool AG_Winch::enabled() const
{
   return ((config.type > 0) && (backend != nullptr));
}

// true if winch is healthy
bool AG_Winch::healthy() const
{
    if (backend != nullptr) {
        return backend->healthy();
    }
    return false;
}

void AG_Winch::init()
{
    switch ((WinchType)config.type.get()) {
    case WinchType::NONE:
        break;
    case WinchType::PWM:
        backend = new AG_Winch_PWM(config);
        break;
    case WinchType::DAIWA:
        backend = new AG_Winch_Daiwa(config);
        break;
    default:
        break;
    }
    if (backend != nullptr) {
        backend->init();
    }
}

// release specified length of cable (in meters)
void AG_Winch::release_length(float length)
{
    if (backend == nullptr) {
        return;
    }
    config.length_desired = backend->get_current_length() + length;
    config.control_mode = ControlMode::POSITION;
}

// deploy line at specified speed in m/s (+ve deploys line, -ve retracts line, 0 stops)
void AG_Winch::set_desired_rate(float rate)
{
    config.rate_desired = constrain_float(rate, -get_rate_max(), get_rate_max());
    config.control_mode = ControlMode::RATE;
}

// send status to ground station
void AG_Winch::send_status(const GCS_MAVLINK &channel)
{
    if (backend != nullptr) {
        backend->send_status(channel);
    }
}

// returns true if pre arm checks have passed
bool AG_Winch::pre_arm_check(char *failmsg, uint8_t failmsg_len) const
{
    // succeed if winch is disabled
    if ((WinchType)config.type.get() == WinchType::NONE) {
        return true;
    }

    // fail if unhealthy
    if (!healthy()) {
        hal.util->snprintf(failmsg, failmsg_len, "winch unhealthy");
        return false;
    }

    return true;
}

// update - should be called at at least 10hz
#define PASS_TO_BACKEND(function_name) \
    void AG_Winch::function_name()   \
    {                                  \
        if (!enabled()) {              \
            return;                    \
        }                              \
        if (backend != nullptr) {      \
            backend->function_name();  \
        }                              \
    }

PASS_TO_BACKEND(update)
PASS_TO_BACKEND(write_log)

#undef PASS_TO_BACKEND

/*
 * Get the AG_Winch singleton
 */
AG_Winch *AG_Winch::_singleton;
AG_Winch *AG_Winch::get_singleton()
{
    return _singleton;
}

namespace AP {

AG_Winch *winch()
{
    return AG_Winch::get_singleton();
}

};

#endif  // AP_WINCH_ENABLED

#include <AG_HAL/AG_HAL.h>

#include "AG_CustomControl.h"

#if AP_CUSTOMCONTROL_ENABLED


#include "AG_CustomControl_Backend.h"
// #include "AG_CustomControl_Empty.h"
#include "AG_CustomControl_PID.h"
#include <GCS_MAVLink/GCS.h>

// table of user settable parameters
const AG_Param::GroupInfo AG_CustomControl::var_info[] = {
    // @Param: _TYPE
    // @DisplayName: Custom control type
    // @Description: Custom control type to be used
    // @Values: 0:None, 1:Empty, 2:PID
    // @RebootRequired: True
    // @User: Advanced
    AP_GROUPINFO_FLAGS("_TYPE", 1, AG_CustomControl, _controller_type, 0, AP_PARAM_FLAG_ENABLE),

    // @Param: _AXIS_MASK
    // @DisplayName: Custom Controller bitmask
    // @Description: Custom Controller bitmask to chose which axis to run
    // @Bitmask: 0:Roll, 1:Pitch, 2:Yaw
    // @User: Advanced
    AP_GROUPINFO("_AXIS_MASK", 2, AG_CustomControl, _custom_controller_mask, 0),

    // parameters for empty controller. only used as a template, no need for param table 
    // AP_SUBGROUPVARPTR(_backend, "1_", 6, AG_CustomControl, _backend_var_info[0]),

    // parameters for PID controller
    AP_SUBGROUPVARPTR(_backend, "2_", 7, AG_CustomControl, _backend_var_info[1]),

    AP_GROUPEND
};

const struct AG_Param::GroupInfo *AG_CustomControl::_backend_var_info[CUSTOMCONTROL_MAX_TYPES];

AG_CustomControl::AG_CustomControl(AG_AHRS_View*& ahrs, AG_AttitudeControl_Multi*& att_control, AG_MotorsMulticopter*& motors, float dt) :
    _dt(dt),
    _ahrs(ahrs),
    _att_control(att_control),
    _motors(motors)
{
    AG_Param::setup_object_defaults(this, var_info);
}

void AG_CustomControl::init(void)
{
    switch (CustomControlType(_controller_type))
    {
        case CustomControlType::CONT_NONE:
            break;
        case CustomControlType::CONT_EMPTY: // This is template backend. Don't initialize it.
            // This is template backend. Don't initialize it.
            // _backend = new AG_CustomControl_Empty(*this, _ahrs, _att_control, _motors, _dt);
            // _backend_var_info[get_type()] = AG_CustomControl_Empty::var_info;
            break;
        case CustomControlType::CONT_PID:
            _backend = new AG_CustomControl_PID(*this, _ahrs, _att_control, _motors, _dt);
            _backend_var_info[get_type()] = AG_CustomControl_PID::var_info;
            break;
        default:
            return;
    }

    if (_backend && _backend_var_info[get_type()]) {
        AG_Param::load_object_from_eeprom(_backend, _backend_var_info[get_type()]);
    }
}

// run custom controller if it is activated by RC switch and appropriate type is selected
void AG_CustomControl::update(void)
{
    if (is_safe_to_run()) {
        Vector3f motor_out_rpy;

        motor_out_rpy = _backend->update();

        motor_set(motor_out_rpy);
    }
}

// choose which axis to apply custom controller output
void AG_CustomControl::motor_set(Vector3f rpy) {
    if (_custom_controller_mask & (uint8_t)CustomControlOption::ROLL) {
        _motors->set_roll(rpy.x);
        _att_control->get_rate_roll_pid().set_integrator(0.0);
    }
    if (_custom_controller_mask & (uint8_t)CustomControlOption::PITCH) {
        _motors->set_pitch(rpy.y);
        _att_control->get_rate_pitch_pid().set_integrator(0.0);
    }
    if (_custom_controller_mask & (uint8_t)CustomControlOption::YAW) {
        _motors->set_yaw(rpy.z);
        _att_control->get_rate_yaw_pid().set_integrator(0.0);
    }
}

// move main controller's target to current states, reset filters,
// and move integrator to motor output
// to allow smooth transition to the primary controller
void AG_CustomControl::reset_main_att_controller(void)
{
    // reset attitude and rate target, if feedforward is enabled
    if (_att_control->get_bf_feedforward()) {
        _att_control->relax_attitude_controllers();
    }

    _att_control->get_rate_roll_pid().set_integrator(0.0);
    _att_control->get_rate_pitch_pid().set_integrator(0.0);
    _att_control->get_rate_yaw_pid().set_integrator(0.0);
}

void AG_CustomControl::set_custom_controller(bool enabled)
{
    // double logging switch makes the state change very clear in the log
    log_switch();

    _custom_controller_active = false;

    // don't allow accidental main controller reset without active custom controller
    if (_controller_type == CustomControlType::CONT_NONE) {
        gcs().send_text(MAV_SEVERITY_INFO, "Custom controller is not enabled");
        return;
    }

    // controller type is out of range
    if (_controller_type > CUSTOMCONTROL_MAX_TYPES) {
        gcs().send_text(MAV_SEVERITY_INFO, "Custom controller type is out of range");
        return;
    }

    // backend is not created
    if (_backend == nullptr) {
        gcs().send_text(MAV_SEVERITY_INFO, "Reboot to enable selected custom controller");
        return;
    }

    if (_custom_controller_mask == 0 && enabled) {
        gcs().send_text(MAV_SEVERITY_INFO, "Axis mask is not set");
        return;
    }

    // reset main controller
    if (!enabled) {
        gcs().send_text(MAV_SEVERITY_INFO, "Custom controller is OFF");
        // don't reset if the empty backend is selected
        if (_controller_type > CustomControlType::CONT_EMPTY) {
            reset_main_att_controller();
        }
    }

    if (enabled && _controller_type > CustomControlType::CONT_NONE) {
        // reset custom controller filter, integrator etc.
        _backend->reset();
        gcs().send_text(MAV_SEVERITY_INFO, "Custom controller is ON");
    }

    _custom_controller_active = enabled;

    // log successful switch
    log_switch();
}

// check that RC switch is on, backend is not changed mid flight and controller type is selected
bool AG_CustomControl::is_safe_to_run(void) {
    if (_custom_controller_active && (_controller_type > CustomControlType::CONT_NONE)
        && (_controller_type <= CUSTOMCONTROL_MAX_TYPES) && _backend != nullptr)
    {
        return true;
    }

    return false;
}

// log when the custom controller is switch into
void AG_CustomControl::log_switch(void) {
    AP::logger().Write("CC", "TimeUS,Type,Act","QBB",
                            AG_HAL::micros64(),
                            _controller_type,
                            _custom_controller_active);
}

#endif

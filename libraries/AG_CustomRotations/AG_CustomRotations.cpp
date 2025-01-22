#include "AG_CustomRotations.h"

#include <AG_InternalError/AG_InternalError.h>
#include <AG_Vehicle/AG_Vehicle_Type.h>
#include <AG_HAL/AG_HAL_Boards.h>
#include <AG_BoardConfig/AG_BoardConfig.h>

#if !APM_BUILD_TYPE(APM_BUILD_AP_Periph)

const AG_Param::GroupInfo AG_CustomRotations::var_info[] = {

    // @Param: _ENABLE
    // @DisplayName: Enable Custom rotations
    // @Values: 0:Disable, 1:Enable
    // @Description: This enables custom rotations
    // @User: Standard
    // @RebootRequired: True
    AP_GROUPINFO_FLAGS("_ENABLE", 1, AG_CustomRotations, enable, 0, AP_PARAM_FLAG_ENABLE),

    // @Group: 1_
    // @Path: AG_CustomRotations_params.cpp
    AP_SUBGROUPINFO(params[0], "1_", 2, AG_CustomRotations, AP_CustomRotation_params),

    // @Group: 2_
    // @Path: AG_CustomRotations_params.cpp
    AP_SUBGROUPINFO(params[1], "2_", 3, AG_CustomRotations, AP_CustomRotation_params),

    AP_GROUPEND
};

AP_CustomRotation::AP_CustomRotation(AP_CustomRotation_params &_params):
    params(_params)
{
    init();
}

void AP_CustomRotation::init()
{
    m.from_euler(radians(params.roll),radians(params.pitch),radians(params.yaw));
    q.from_rotation_matrix(m);
}

AG_CustomRotations::AG_CustomRotations()
{
    AG_Param::setup_object_defaults(this, var_info);

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
    if (singleton != nullptr) {
        AG_HAL::panic("AG_CustomRotations must be singleton");
    }
#endif
    singleton = this;
}

void AG_CustomRotations::init()
{
    if (enable == 0) {
        return;
    }

    // make sure all custom rotations are allocated
    for (uint8_t i = 0; i < NUM_CUST_ROT; i++) {
        AP_CustomRotation* rot = get_rotation(Rotation(i + ROTATION_CUSTOM_1));
        if (rot == nullptr) {
            AG_BoardConfig::allocation_error("Custom Rotations");
        }
    }
}

void AG_CustomRotations::convert(Rotation r, float roll, float pitch, float yaw)
{
    AP_CustomRotation* rot = get_rotation(r);
    if (rot == nullptr) {
        return;
    }
    if (!rot->params.roll.configured() && !rot->params.pitch.configured() && !rot->params.yaw.configured()) {
        rot->params.roll.set_and_save(roll);
        rot->params.pitch.set_and_save(pitch);
        rot->params.yaw.set_and_save(yaw);
        rot->init();
    }
}

void AG_CustomRotations::set(Rotation r, float roll, float pitch, float yaw)
{
    AP_CustomRotation* rot = get_rotation(r);
    if (rot == nullptr) {
        return;
    }
    rot->params.roll.set(roll);
    rot->params.pitch.set(pitch);
    rot->params.yaw.set(yaw);
    rot->init();
}

void AG_CustomRotations::from_rotation(Rotation r, QuaternionD& q)
{
    AP_CustomRotation* rot = get_rotation(r);
    if (rot == nullptr) {
        return;
    }
    q = rot->q.todouble();
}

void AG_CustomRotations::from_rotation(Rotation r, Quaternion& q)
{
    AP_CustomRotation* rot = get_rotation(r);
    if (rot == nullptr) {
        return;
    }
    q = rot->q;
}

void AG_CustomRotations::rotate(Rotation r, Vector3d& v)
{
    AP_CustomRotation* rot = get_rotation(r);
    if (rot == nullptr) {
        return;
    }
    v = (rot->m * v.tofloat()).todouble();
}

void AG_CustomRotations::rotate(Rotation r, Vector3f& v)
{
    AP_CustomRotation* rot = get_rotation(r);
    if (rot == nullptr) {
        return;
    }
    v = rot->m * v;
}

AP_CustomRotation* AG_CustomRotations::get_rotation(Rotation r)
{
    if (r < ROTATION_CUSTOM_1 || r >= ROTATION_CUSTOM_END) {
        INTERNAL_ERROR(AG_InternalError::error_t::bad_rotation);
        return nullptr;
    }
    const uint8_t index = r - ROTATION_CUSTOM_1;
    if (rotations[index] == nullptr) {
        rotations[index] = new AP_CustomRotation(params[index]);
         // make sure param is enabled if custom rotation is used
        enable.set_and_save_ifchanged(1);
    }
    return rotations[index];
}

// singleton instance
AG_CustomRotations *AG_CustomRotations::singleton;

namespace AP {

AG_CustomRotations &custom_rotations()
{
    return *AG_CustomRotations::get_singleton();
}

}

#endif // !APM_BUILD_TYPE(APM_BUILD_AP_Periph)

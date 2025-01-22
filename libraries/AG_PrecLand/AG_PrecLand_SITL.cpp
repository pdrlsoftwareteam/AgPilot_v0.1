#include <AG_HAL/AG_HAL.h>
#include "AG_PrecLand_SITL.h"

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL

#include "AG_AHRS/AG_AHRS.h"
// init - perform initialisation of this backend
void AG_PrecLand_SITL::init()
{
    _sitl = AP::sitl();
}

// update - give chance to driver to get updates from sensor
void AG_PrecLand_SITL::update()
{
    _state.healthy = _sitl->precland_sim.healthy();

    if (_state.healthy && _sitl->precland_sim.last_update_ms() != _los_meas_time_ms) {
        const Vector3d position = _sitl->precland_sim.get_target_position();
        const Matrix3d body_to_ned = AP::ahrs().get_rotation_body_to_ned().todouble();
        _los_meas_body =  body_to_ned.mul_transpose(-position).tofloat();
        _distance_to_target = _sitl->precland_sim.option_enabled(SITL::SIM_Precland::Option::ENABLE_TARGET_DISTANCE) ? _los_meas_body.length() : 0.0f;
        _los_meas_body /= _los_meas_body.length();

        if (_frontend._orient != Rotation::ROTATION_PITCH_270) {
            // rotate body frame vector based on orientation
            // this is done to have homogeneity among backends
            // frontend rotates it back to get correct body frame vector
            _los_meas_body.rotate_inverse(_frontend._orient);
            _los_meas_body.rotate_inverse(ROTATION_PITCH_90);
        }

        _have_los_meas = true;
        _los_meas_time_ms = _sitl->precland_sim.last_update_ms();
    } else {
        _have_los_meas = false;
    }

    _have_los_meas = _have_los_meas && AG_HAL::millis() - _los_meas_time_ms <= 1000;
}

bool AG_PrecLand_SITL::have_los_meas() {
    return AG_HAL::millis() - _los_meas_time_ms < 1000;
}

// provides a unit vector towards the target in body frame
//  returns same as have_los_meas()
bool AG_PrecLand_SITL::get_los_body(Vector3f& ret) {
    if (AG_HAL::millis() - _los_meas_time_ms > 1000) {
        // no measurement for a full second; no vector available
        return false;
    }
    ret = _los_meas_body;
    return true;
}

#endif

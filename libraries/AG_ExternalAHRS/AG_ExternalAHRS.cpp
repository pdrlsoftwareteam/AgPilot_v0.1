/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
  suppport for serial connected AHRS systems
 */

#include "AG_ExternalAHRS.h"
#include "AG_ExternalAHRS_VectorNav.h"
#include "AG_ExternalAHRS_LORD.h"

#if HAL_EXTERNAL_AHRS_ENABLED

#include <GCS_MAVLink/GCS.h>

extern const AG_HAL::HAL &hal;

AG_ExternalAHRS *AG_ExternalAHRS::_singleton;

// constructor
AG_ExternalAHRS::AG_ExternalAHRS()
{
    AG_Param::setup_object_defaults(this, var_info);
    _singleton = this;
    if (rate.get() < 50) {
        // min 50Hz
        rate.set(50);
    }
}

#ifndef HAL_EXTERNAL_AHRS_DEFAULT
#define HAL_EXTERNAL_AHRS_DEFAULT 0
#endif


// table of user settable parameters
const AG_Param::GroupInfo AG_ExternalAHRS::var_info[] = {

    // @Param: _TYPE
    // @DisplayName: AHRS type
    // @Description: Type of AHRS device
    // @Values: 0:None,1:VectorNav,2:LORD
    // @User: Standard
    AP_GROUPINFO_FLAGS("_TYPE", 1, AG_ExternalAHRS, devtype, HAL_EXTERNAL_AHRS_DEFAULT, AP_PARAM_FLAG_ENABLE),

    // @Param: _RATE
    // @DisplayName: AHRS data rate
    // @Description: Requested rate for AHRS device
    // @Units: Hz
    // @User: Standard
    AP_GROUPINFO("_RATE", 2, AG_ExternalAHRS, rate, 50),

    // @Param: _OPTIONS
    // @DisplayName: External AHRS options
    // @Description: External AHRS options bitmask
    // @Bitmask: 0:Vector Nav use uncompensated values for accel gyro and mag.
    // @User: Standard
    AP_GROUPINFO("_OPTIONS", 3, AG_ExternalAHRS, options, 0),

    // @Param: _SENSORS
    // @DisplayName: External AHRS sensors
    // @Description: External AHRS sensors bitmask
    // @Bitmask: 0:GPS,1:IMU,2:Baro,3:Compass
    // @User: Advanced
    AP_GROUPINFO("_SENSORS", 4, AG_ExternalAHRS, sensors, 0xF),
    
    AP_GROUPEND
};


void AG_ExternalAHRS::init(void)
{
    if (rate.get() < 50) {
        // min 50Hz
        rate.set(50);
    }

    switch (DevType(devtype)) {
    case DevType::None:
        // nothing to do
        break;
    case DevType::VecNav:
        backend = new AG_ExternalAHRS_VectorNav(this, state);
        break;
    case DevType::LORD:
        backend = new AG_ExternalAHRS_LORD(this, state);
        break;
    default:
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Unsupported ExternalAHRS type %u", unsigned(devtype));
        break;
    }
}

bool AG_ExternalAHRS::enabled() const
{
    return DevType(devtype) != DevType::None;
}

// get serial port number for the uart, or -1 if not applicable
int8_t AG_ExternalAHRS::get_port(AvailableSensor sensor) const
{
    if (!backend || !has_sensor(sensor)) {
        return -1;
    }
    return backend->get_port();
};

// accessors for AG_AHRS
bool AG_ExternalAHRS::healthy(void) const
{
    return backend && backend->healthy();
}

bool AG_ExternalAHRS::initialised(void) const
{
    return backend && backend->initialised();
}

bool AG_ExternalAHRS::get_quaternion(Quaternion &quat)
{
    if (state.have_quaternion) {
        WITH_SEMAPHORE(state.sem);
        quat = state.quat;
        return true;
    }
    return false;
}

bool AG_ExternalAHRS::get_origin(Location &loc)
{
    if (state.have_origin) {
        WITH_SEMAPHORE(state.sem);
        loc = state.origin;
        return true;
    }
    return false;
}

bool AG_ExternalAHRS::get_location(Location &loc)
{
    if (!state.have_location) {
        return false;
    }
    WITH_SEMAPHORE(state.sem);
    loc = state.location;
    return true;
}

Vector2f AG_ExternalAHRS::get_groundspeed_vector()
{
    WITH_SEMAPHORE(state.sem);
    Vector2f vec{state.velocity.x, state.velocity.y};
    return vec;
}

bool AG_ExternalAHRS::get_velocity_NED(Vector3f &vel)
{
    if (!state.have_velocity) {
        return false;
    }
    WITH_SEMAPHORE(state.sem);
    vel = state.velocity;
    return true;
}

bool AG_ExternalAHRS::get_speed_down(float &speedD)
{
    if (!state.have_velocity) {
        return false;
    }
    WITH_SEMAPHORE(state.sem);
    speedD = state.velocity.z;
    return true;
}

bool AG_ExternalAHRS::pre_arm_check(char *failure_msg, uint8_t failure_msg_len) const
{
    return backend && backend->pre_arm_check(failure_msg, failure_msg_len);
}

/*
  get filter status
 */
void AG_ExternalAHRS::get_filter_status(nav_filter_status &status) const
{
    status = {};
    if (backend) {
        backend->get_filter_status(status);
    }
}

Vector3f AG_ExternalAHRS::get_gyro(void)
{
    WITH_SEMAPHORE(state.sem);
    return state.gyro;
}

Vector3f AG_ExternalAHRS::get_accel(void)
{
    WITH_SEMAPHORE(state.sem);
    return state.accel;
}

// send an EKF_STATUS message to GCS
void AG_ExternalAHRS::send_status_report(GCS_MAVLINK &link) const
{
    if (backend) {
        backend->send_status_report(link);
    }
}

void AG_ExternalAHRS::update(void)
{
    if (backend) {
        backend->update();
    }
}

// Get model/type name
const char* AG_ExternalAHRS::get_name() const
{
    if (backend) {
        return backend->get_name();
    }
    return nullptr;
}

namespace AP {

AG_ExternalAHRS &externalAHRS()
{
    return *AG_ExternalAHRS::get_singleton();
}

};

#endif  // HAL_EXTERNAL_AHRS_ENABLED


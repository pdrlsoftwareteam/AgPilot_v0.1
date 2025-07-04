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

#include "AP_Proximity_RangeFinder.h"

#if HAL_PROXIMITY_ENABLED
#include <AP_HAL/AP_HAL.h>
#include <ctype.h>
#include <stdio.h>
#include <AP_RangeFinder/AP_RangeFinder.h>
#include <AP_RangeFinder/AP_RangeFinder_Backend.h>
#include "GCS_MAVLink/GCS.h"

// update the state of the sensor
void AP_Proximity_RangeFinder::update(void)
{
    const RangeFinder *rngfnd = AP::rangefinder();
    if (rngfnd == nullptr) {
        set_status(AP_Proximity::Status::NoData);
        return;
    }

    uint32_t now = AP_HAL::millis();

    bool orientation_connected[8] = {false};
    bool upward_connected = false;

    for (uint8_t i = 0; i < rngfnd->num_sensors(); i++) {
        AP_RangeFinder_Backend *sensor = rngfnd->get_backend(i);
        if (sensor == nullptr) {
            continue;
        }

        uint8_t ori = sensor->orientation();
        bool has_data = sensor->has_data();

        if (!has_data) {
            continue;
        }

        if (ori <= ROTATION_YAW_315) {
            orientation_connected[ori] = true;

            const uint8_t sector = ori;
            const float angle = sector * 45;
            const AP_Proximity_Boundary_3D::Face face = frontend.boundary.get_face(angle);

            const float distance = sensor->distance();
            _distance_min = sensor->min_distance_cm() * 0.01f;
            _distance_max = sensor->max_distance_cm() * 0.01f;

            if ((distance <= _distance_max) && (distance >= _distance_min) &&
                !ignore_reading(angle, distance, false)) {
                frontend.boundary.set_face_attributes(face, angle, distance, state.instance);
                database_push(angle, distance);
            } else {
                frontend.boundary.reset_face(face, state.instance);
            }

            _last_update_ms = now;
        } else if (ori == ROTATION_PITCH_270) {
            upward_connected = true;

            int16_t distance_upward = sensor->distance_cm();
            int16_t up_distance_min = sensor->min_distance_cm();
            int16_t up_distance_max = sensor->max_distance_cm();

            if ((distance_upward >= up_distance_min) && (distance_upward <= up_distance_max)) {
                _distance_upward = distance_upward * 0.01f;
            } else {
                _distance_upward = -1.0f;
            }

            _last_upward_update_ms = now;
        }
    }

    // Direction names for printing
    const char* dir_str[] = {"Front", "FR", "Right", "BR", "Back", "BL", "Left", "FL"};

    // Send connection status only if changed
    for (uint8_t ori = 0; ori <= ROTATION_YAW_315; ori++) {
        if (orientation_connected[ori] != _orientation_connected_last[ori]) {
            if (orientation_connected[ori]) {
                GCS_SEND_TEXT(MAV_SEVERITY_INFO, "%s sensor connected", dir_str[ori]);
            } else {
                GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "%s sensor disconnected", dir_str[ori]);
            }
            _orientation_connected_last[ori] = orientation_connected[ori];
        }
    }

    // Upward sensor status change detection
    if (upward_connected != _upward_connected_last) {
        if (upward_connected) {
            GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Altimeter connected");
        } else {
            GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Altimeter disconnected");
        }
        _upward_connected_last = upward_connected;
    }

    // Set proximity health
    if ((_last_update_ms == 0 || (now - _last_update_ms > PROXIMITY_RANGEFIDER_TIMEOUT_MS)) &&
        (_last_upward_update_ms == 0 || (now - _last_upward_update_ms > PROXIMITY_RANGEFIDER_TIMEOUT_MS))) {
        set_status(AP_Proximity::Status::NoData);
    } else {
        set_status(AP_Proximity::Status::Good);
    }
}

// get distance upwards in meters. returns true on success
bool AP_Proximity_RangeFinder::get_upward_distance(float &distance) const
{
    if ((AP_HAL::millis() - _last_upward_update_ms <= PROXIMITY_RANGEFIDER_TIMEOUT_MS) &&
        is_positive(_distance_upward)) {
        distance = _distance_upward;
        return true;
    }
    return false;
}

#endif // HAL_PROXIMITY_ENABLED

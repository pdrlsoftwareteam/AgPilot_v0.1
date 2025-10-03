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

#include "AP_Proximity_IOTECH.h"

//#if HAL_PROXIMITY_ENABLED
#include <AP_HAL/AP_HAL.h>
#include <ctype.h>
#include <stdio.h>
#include <AP_RangeFinder/AP_RangeFinder.h>
#include <AP_RangeFinder/AP_RangeFinder_Backend.h>
#include "GCS_MAVLink/GCS.h"

float AP_Proximity_IOTECH::findDistance(double lat1, double lon1, double lat2, double lon2)
{
  const float EARTH_RADIUS = 6371000.0f;

  double lat1Rad = lat1 * M_PI / 180.0f;
  double lon1Rad = lon1 * M_PI / 180.0f;
  double lat2Rad = lat2 * M_PI / 180.0f;
  double lon2Rad = lon2 * M_PI / 180.0f;

  double dLat = lat2Rad - lat1Rad;
  double dLon = lon2Rad - lon1Rad;

  double a = sinf(dLat / 2.0f) * sinf(dLat / 2.0f) + cosf(lat1Rad) * cosf(lat2Rad) * sinf(dLon / 2.0f) * sinf(dLon / 2.0f);
  double c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));

  float distance = (EARTH_RADIUS * c);

  return distance;
}

// update the state of the sensor
void AP_Proximity_IOTECH::update(void)
{
  // exit immediately if no rangefinder object
  const RangeFinder *rngfnd = AP::rangefinder();
  if (rngfnd == nullptr) {
      set_status(AP_Proximity::Status::NoData);
      return;
  }


  uint32_t now = AP_HAL::millis();

  // look through all rangefinders
  for (uint8_t i=0; i < rngfnd->num_sensors(); i++) {
      AP_RangeFinder_Backend *sensor = rngfnd->get_backend(i);
      if (rngfnd->get_backend(i) == nullptr || rngfnd->get_type(i) != RangeFinder::Type::IOTECH) {
	  continue;
      }
      uint32_t fx=0, fy=0, rx=0, ry=0;
      float lat = 12.345678f;
      float lon = 98.765432f;
      bool got_front = false;
      bool got_rear  = false;

      rngfnd->get_iotech_front_value(fx, fy);
      rngfnd->get_iotech_rear_value(rx, ry);

      float ffx = fx * 0.000001f;
      float ffy = fy * 0.000001f;
      float rfx = rx * 0.000001f;
      float rfy = ry * 0.000001f;

      if( rngfnd->check_sensor_status())
	{
	  _last_update_ms = now;
	  if (fx && fy) {
	      got_front = true;
	  }
	  if (rx && ry) {
	      got_rear = true;
	  }
	}


      // --- process FRONT sensor ---
      if (got_front) {
          const float distance_front = findDistance(lat, lon, ffx, ffy);
          const float angle_front    = 0.0f;   // 0° = front

          _distance_min = sensor->min_distance_cm() * 0.01f;
          _distance_max = sensor->max_distance_cm() * 0.01f;

          if ((distance_front <= _distance_max) && (distance_front >= _distance_min)) {
              frontend.boundary.set_face_attributes(
                  frontend.boundary.get_face(angle_front),
                  angle_front, distance_front, state.instance);

              database_push(angle_front, distance_front);
          } else {
              frontend.boundary.reset_face(
                  frontend.boundary.get_face(angle_front), state.instance);
          }
      }

      // --- process REAR sensor ---
      if (got_rear) {
          const float distance_rear = findDistance(lat, lon, rfx, rfy);
          const float angle_rear    = 180.0f;   // 180° = rear

          _distance_min = sensor->min_distance_cm() * 0.01f;
          _distance_max = sensor->max_distance_cm() * 0.01f;

          if ((distance_rear <= _distance_max) && (distance_rear >= _distance_min)) {
              frontend.boundary.set_face_attributes(
                  frontend.boundary.get_face(angle_rear),
                  angle_rear, distance_rear, state.instance);

              database_push(angle_rear, distance_rear);
          } else {
              frontend.boundary.reset_face(
                  frontend.boundary.get_face(angle_rear), state.instance);
          }
      }
  }

  // check for timeout and set health status
  if ((_last_update_ms == 0 || (now - _last_update_ms > PROXIMITY_IOTECH_TIMEOUT_MS)) &&
      (_last_upward_update_ms == 0 || (now - _last_upward_update_ms > PROXIMITY_IOTECH_TIMEOUT_MS))) {
      set_status(AP_Proximity::Status::NoData);
  } else {
      set_status(AP_Proximity::Status::Good);
  }
}

// get distance upwards in meters. returns true on success
bool AP_Proximity_IOTECH::get_upward_distance(float &distance) const
{
  if ((AP_HAL::millis() - _last_upward_update_ms <= PROXIMITY_IOTECH_TIMEOUT_MS) &&
      is_positive(_distance_upward)) {
      distance = _distance_upward;
      return true;
  }
  return false;
}

//#endif // HAL_PROXIMITY_ENABLED

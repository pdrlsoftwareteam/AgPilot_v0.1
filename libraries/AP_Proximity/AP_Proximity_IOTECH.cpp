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
#include <AP_AHRS/AP_AHRS.h>

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
	static bool found = false;
	static bool init = false;
	// look through all rangefinders
	for (uint8_t i=0; i < rngfnd->num_sensors(); i++) {
		if (rngfnd->get_backend(i) == nullptr || rngfnd->get_type(i) != RangeFinder::Type::IOTECH) {
			continue;
		}
		else
		{
			found = true;
		}
	}
	if(found)
	{

		uint32_t fx=0, fy=0, rx=0, ry=0;
		float lat = 12.345678f;
		float lon = 98.765432f;

		rngfnd->get_iotech_front_value(fx, fy);
		rngfnd->get_iotech_rear_value(rx, ry);

		float ffx = fx * 0.000001f;
		float ffy = fy * 0.000001f;
		float rfx = rx * 0.000001f;
		float rfy = ry * 0.000001f;

		if( rngfnd->check_sensor_status())
		{
			if (fx && fy) {
				got_front = true;
			}
			if (rx && ry) {
				got_rear = true;
			}
		}
		AP_Proximity *prx = AP::proximity();
		if (prx == nullptr) {
			return;
		}
		int c = 0;
		for(c = 0; c < PROXIMITY_MAX_INSTANCES; c++)
		{
			if((AP_Proximity::Type)prx->params[c].type.get() == AP_Proximity::Type::IOTECH)
			{
				//	  gcs().send_text(MAV_SEVERITY_INFO, "got at: %d",c);
				break;
			}
		}
		if (c >= PROXIMITY_MAX_INSTANCES) {
		    return; // safety exit
		}
		// --- process FRONT sensor ---
		if (got_front) {
			const float distance_front = findDistance(lat, lon, ffx, ffy);
			const float angle_front    = 0.0f;   // 0° = front
			_last_update_ms = now;
			_distance_min = prx->params[c].min_m;
			_distance_max = prx->params[c].max_m;

			if ((distance_front <= _distance_max) && (distance_front >= _distance_min)) {
				frontend.boundary.set_face_attributes(
						frontend.boundary.get_face(angle_front),
						angle_front, distance_front, state.instance);
				database_push(angle_front, distance_front);
			} else {
				frontend.boundary.reset_face(
						frontend.boundary.get_face(angle_front), state.instance);
			}
			got_front = false;
		}
		if (got_rear) {
			const float distance_rear = findDistance(lat, lon, rfx, rfy);
			const float angle_rear    = 180.0f;   // 180° = rear

			_last_update_ms = now;
			_distance_min = prx->params[c].min_m;
			_distance_max = prx->params[c].max_m;

			if ((distance_rear <= _distance_max) && (distance_rear >= _distance_min)) {
				frontend.boundary.set_face_attributes(
						frontend.boundary.get_face(angle_rear),
						angle_rear, distance_rear, state.instance);

				database_push(angle_rear, distance_rear);
			} else {
				frontend.boundary.reset_face(
						frontend.boundary.get_face(angle_rear), state.instance);
			}
			got_rear = false;
		}

		// check for timeout and set health status
		if ((_last_update_ms == 0 || (now - _last_update_ms > PROXIMITY_IOTECH_TIMEOUT_MS)) &&
				(_last_upward_update_ms == 0 || (now - _last_upward_update_ms > PROXIMITY_IOTECH_TIMEOUT_MS))) {
			set_status(AP_Proximity::Status::NoData);
		} else {
			set_status(AP_Proximity::Status::Good);
		}
	}
	else
	{
		if(!init)
		{
			AP_CANDataDistribuer_IOT::get_instance()->addCANDataListener_Prx_iot(this);
			init = true;
		}
		new_driver();
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

void AP_Proximity_IOTECH::new_driver()
{
	AP_Proximity *prx = AP::proximity();
	if (prx == nullptr) {
		return;
	}
	uint32_t now = AP_HAL::millis();
	send_frame();
	int c = 0;
	for(c = 0; c < PROXIMITY_MAX_INSTANCES; c++)
	{
		if((AP_Proximity::Type)prx->params[c].type.get() == AP_Proximity::Type::IOTECH)
		{
			//	  gcs().send_text(MAV_SEVERITY_INFO, "got at: %d",c);
			break;
		}
	}

	if(AP_CANDataDistribuer_IOT::get_instance()->got_front)
	{
		//      gcs().send_text(MAV_SEVERITY_INFO, "F: %f",AP_CANDataDistribuer_IOT::get_instance()->front_dist);
		// --- process FRONT sensor ---
		const float distance_front = AP_CANDataDistribuer_IOT::get_instance()->front_dist;
		const float angle_front    = 0.0f;   // 0° = front
		_last_update_ms = now;

		_distance_min = prx->params[c].min_m ;
		_distance_max = prx->params[c].max_m ;

		if ((distance_front <= _distance_max) && (distance_front >= _distance_min)) {
			frontend.boundary.set_face_attributes(
					frontend.boundary.get_face(angle_front),
					angle_front, distance_front, state.instance);

			database_push(angle_front, distance_front);
		} else {
			frontend.boundary.reset_face(
					frontend.boundary.get_face(angle_front), state.instance);
		}
		AP_CANDataDistribuer_IOT::get_instance()->got_front = false;
	}

	if(AP_CANDataDistribuer_IOT::get_instance()->got_rear)
	{
		//      gcs().send_text(MAV_SEVERITY_INFO, "R: %f",AP_CANDataDistribuer_IOT::get_instance()->rear_dist);

		// --- process FRONT sensor ---
		const float distance_rear = AP_CANDataDistribuer_IOT::get_instance()->rear_dist;
		const float angle_rear    = 180.0f;   // 0° = front
		_last_update_ms = now;

		_distance_min = prx->params[c].min_m ;
		_distance_max = prx->params[c].max_m ;

		if ((distance_rear <= _distance_max) && (distance_rear >= _distance_min)) {
			frontend.boundary.set_face_attributes(
					frontend.boundary.get_face(angle_rear),
					angle_rear, distance_rear, state.instance);

			database_push(angle_rear, distance_rear);
		} else {
			frontend.boundary.reset_face(
					frontend.boundary.get_face(angle_rear), state.instance);
		}
		AP_CANDataDistribuer_IOT::get_instance()->got_rear = false;
	}

	// check for timeout and set health status
	if ((_last_update_ms == 0 || (now - _last_update_ms > PROXIMITY_IOTECH_TIMEOUT_MS)) &&
			(_last_upward_update_ms == 0 || (now - _last_upward_update_ms > PROXIMITY_IOTECH_TIMEOUT_MS))) {
		set_status(AP_Proximity::Status::NoData);
	} else {
		set_status(AP_Proximity::Status::Good);
	}
}
void AP_Proximity_IOTECH::send_frame()
{

	// --- Staggered CAN send ---
	const uint32_t now = AP_HAL::millis();
	static uint32_t last_send_ms = 0;
	static bool send_first_frame = true;

	if (now - last_send_ms >= 10) {
		last_send_ms = now;
		float latitude  = 12.345678f;    // degrees
		float longitude = 98.765432f;
		float curBearing = AP::ahrs().yaw_sensor * 0.01f;
		uint16_t bear = (float)curBearing * 100;
		uint8_t min_pts = 2;
		uint8_t epsilon = 1;

		int32_t lat = latitude * 1000000;
		int32_t lon = longitude * 1000000;

		if (send_first_frame) {
			// Frame 1: latitude + longitude
			AP_HAL::CANFrame frame1;
			frame1.id = 0xAFF | AP_HAL::CANFrame::FlagEFF;
			frame1.dlc = 8;
			frame1.data[0] = (lat >> 24) & 0xFF;
			frame1.data[1] = (lat >> 16) & 0xFF;
			frame1.data[2] = (lat >> 8)  & 0xFF;
			frame1.data[3] = (lat)       & 0xFF;
			frame1.data[4] = (lon >> 24) & 0xFF;
			frame1.data[5] = (lon >> 16) & 0xFF;
			frame1.data[6] = (lon >> 8)  & 0xFF;
			frame1.data[7] = (lon)       & 0xFF;

			if (AP_CANDataDistribuer_IOT::get_instance()->write_frame(frame1, 5000)) {
				//                  gcs().send_text(MAV_SEVERITY_INFO, "Frame1 sent");
			} else {
				//                gcs().send_text(MAV_SEVERITY_INFO, "Frame1 failed");
			}

			send_first_frame = false;  // next time send frame2
		} else {
			// Frame 2: heading + params

			AP_HAL::CANFrame frame2;
			frame2.id = 0xAEF | AP_HAL::CANFrame::FlagEFF;
			frame2.dlc = 4;
			frame2.data[0] = (bear >> 8) & 0xFF;
			frame2.data[1] = (bear)      & 0xFF;
			frame2.data[2] = min_pts & 0xFF;
			frame2.data[3] = epsilon & 0xFF;

			if (AP_CANDataDistribuer_IOT::get_instance()->write_frame(frame2, 5000)) {
				//                  gcs().send_text(MAV_SEVERITY_INFO, "Frame2 sent");
			} else {
				//                gcs().send_text(MAV_SEVERITY_INFO, "Frame2 failed");
			}

			send_first_frame = true;   // next time send frame1
		}
	}

}


AP_CANDataDistribuer_IOT *AP_CANDataDistribuer_IOT::instance = nullptr;
AP_CANDataDistribuer_IOT::AP_CANDataDistribuer_IOT():
                        		CANSensor("USD1")
{
	register_driver(AP_CANManager::Driver_Type_USD1);
	//  _singleton = this;
	for (uint8_t i = 0; i < 3; i++) {
		prx_instance[i] = nullptr;
	}
	AP_Param::setup_object_defaults(this, nullptr);

}
// handler for incoming frames. These come in at 100Hz
void AP_CANDataDistribuer_IOT::handle_frame(AP_HAL::CANFrame &frame)
{
	//  gcs().send_text(MAV_SEVERITY_INFO,"Id: inside iotech proximity");

	//    gcs().send_text(MAV_SEVERITY_INFO,"mix:Id: %lu 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d",
	//                              frame.id&0x7ff,frame.data[0],frame.data[1],frame.data[2],frame.data[3],frame.data[4],frame.data[5],frame.data[6],frame.data[7]);

	uint32_t raw_id = frame.id & 0x7FF;   // strip EFF flag (keep 29-bit ID)
	// OR if flag already separated in your HAL, just use frame.id

	if (raw_id == 0xD1) {
		// --- Parse lat/lon ---
		int32_t lat = 0;
		int32_t lon = 0;

		lat |= (frame.data[0]);
		lat |= (frame.data[1] << 8);
		lat |= (frame.data[2] << 16);
		lat |= (frame.data[3] << 24);

		lon |= (frame.data[4]);
		lon |= (frame.data[5] << 8);
		lon |= (frame.data[6] << 16);
		lon |= (frame.data[7] << 24);
		front_dist = findDistance(lat * 0.000001f, lon * 0.000001f, 12.345678f, 98.765432f);

		got_front = true;
		return;
	}

	if (raw_id == 0xE1) {
		// --- Parse lat/lon ---
		int32_t lat = 0;
		int32_t lon = 0;

		lat |= (frame.data[0]);
		lat |= (frame.data[1] << 8);
		lat |= (frame.data[2] << 16);
		lat |= (frame.data[3] << 24);

		lon |= (frame.data[4]);
		lon |= (frame.data[5] << 8);
		lon |= (frame.data[6] << 16);
		lon |= (frame.data[7] << 24);
		rear_dist = findDistance(lat * 0.000001f, lon * 0.000001f, 12.345678f, 98.765432f);

		got_rear = true;
		return;
	}
}
bool AP_CANDataDistribuer_IOT::write_frame(AP_HAL::CANFrame &out_frame, const uint64_t timeout_us)
{
	// send via underlying CAN driver
	// Example: use CANSensor's send API (assuming it exists)
	return CANSensor::write_frame(out_frame, timeout_us);
}

float AP_CANDataDistribuer_IOT::findDistance(double lat1, double lon1, double lat2, double lon2)
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

//#endif // HAL_PROXIMITY_ENABLED

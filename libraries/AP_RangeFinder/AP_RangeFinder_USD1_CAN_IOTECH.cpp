#include "AP_RangeFinder_USD1_CAN_IOTECH.h"

//#if AP_RANGEFINDER_USD1_CAN_ENABLED

#include <AP_HAL/AP_HAL.h>
#include <GCS_MAVLink/GCS.h>
#include <AP_AHRS/AP_AHRS.h>

/*
  constructor
 */
AP_RangeFinder_USD1_CAN_IOTECH::AP_RangeFinder_USD1_CAN_IOTECH(RangeFinder::RangeFinder_State &_state, AP_RangeFinder_Params &_params) :
AP_RangeFinder_Backend(_state, _params)
{
  _distance_count = 0;
  _distance_sum = 0;
  _min_dist = _params.min_distance_cm;
  _msem = &_sem;
  _last_distance = 0;
  _have_reading = false;

  AP_CANDataDistribuer_IOTECH::getInstance()->addCANDataListener(this);
}

void AP_RangeFinder_USD1_CAN_IOTECH::get_iotech_rear_value(uint32_t &rx, uint32_t &ry)
{
  rx = AP_CANDataDistribuer_IOTECH::getInstance()->rear.x;
  ry = AP_CANDataDistribuer_IOTECH::getInstance()->rear.y;
}

void AP_RangeFinder_USD1_CAN_IOTECH::get_iotech_front_value(uint32_t &fx, uint32_t &fy)
{
  fx = AP_CANDataDistribuer_IOTECH::getInstance()->front.x;
  fy = AP_CANDataDistribuer_IOTECH::getInstance()->front.y;
}

bool AP_RangeFinder_USD1_CAN_IOTECH::check_sensor_status()
{
  return AP_CANDataDistribuer_IOTECH::getInstance()->got_front || AP_CANDataDistribuer_IOTECH::getInstance()->got_rear;
}

void AP_RangeFinder_USD1_CAN_IOTECH::update_sensor_status()
{
  static uint32_t check = AP_HAL::millis();

  if(AP_HAL::millis() - check > 5000)
    {
      AP_CANDataDistribuer_IOTECH::getInstance()->got_front = false;
      AP_CANDataDistribuer_IOTECH::getInstance()->got_rear = false;
      check = AP_HAL::millis();
    }
}

// update state
void AP_RangeFinder_USD1_CAN_IOTECH::update(void)
{
    WITH_SEMAPHORE(_sem);
    const uint32_t now = AP_HAL::millis();

    // 1) valid measurement arrived
    if (_distance_count != 0) {

        state.distance_m = _distance_sum / _distance_count;
        state.last_reading_ms = now;
        set_status(RangeFinder::Status::Good);

        _last_distance = state.distance_m;
        _have_reading = true;

        _distance_sum = 0;
        _distance_count = 0;
    }
    // 2) sensor alive but temporarily no echo
    else if ((now - _last_frame_ms) < 500) {

        state.distance_m = _last_distance;
        state.last_reading_ms = now;
        set_status(RangeFinder::Status::Good);
    }
    // 3) sensor dead
    else if((now - state.last_reading_ms) > 500 && (now - _last_frame_ms) > 500)
    {
    	set_status(RangeFinder::Status::NoData);
    }

    // --- Staggered CAN send ---
    static uint32_t last_send_ms = 0;
//    static bool send_first_frame = true;

    if (now - last_send_ms >= 10) {
        last_send_ms = now;
        Location currentLoc;

        // Example dummy values
//        float latitude  = currentLoc.lat * 0.0000001f;
//        float longitude = currentLoc.lng * 0.0000001f;
        float latitude  = 12.345678f;    // degrees
        float longitude = 98.765432f;
        float curBearing = AP::ahrs().yaw_sensor * 0.01f;
        uint16_t bear = (float)curBearing * 100;
        uint8_t min_pts = 2;
        uint8_t epsilon = 1;

        int32_t lat = latitude * 1000000;
        int32_t lon = longitude * 1000000;

//        if (send_first_frame) {
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

            if (AP_CANDataDistribuer_IOTECH::getInstance()->write_frame(frame1, 0)) {
//                gcs().send_text(MAV_SEVERITY_INFO, "Frame1 sent");
            } else {
//                gcs().send_text(MAV_SEVERITY_INFO, "Frame1 failed");
            }

//            send_first_frame = false;  // next time send frame2
//        } else {
            // Frame 2: heading + params

            AP_HAL::CANFrame frame2;
            frame2.id = 0xAEF | AP_HAL::CANFrame::FlagEFF;
            frame2.dlc = 4;
            frame2.data[0] = (bear >> 8) & 0xFF;
            frame2.data[1] = (bear)      & 0xFF;
            frame2.data[2] = min_pts & 0xFF;
            frame2.data[3] = epsilon & 0xFF;

            if (AP_CANDataDistribuer_IOTECH::getInstance()->write_frame(frame2, 0)) {
//                gcs().send_text(MAV_SEVERITY_INFO, "Frame2 sent");
            } else {
//                gcs().send_text(MAV_SEVERITY_INFO, "Frame2 failed");
            }

//            send_first_frame = true;   // next time send frame1
//        }
    }
    update_sensor_status();
}


//CAN data distributer
AP_CANDataDistribuer_IOTECH *AP_CANDataDistribuer_IOTECH::instance = nullptr;
AP_CANDataDistribuer_IOTECH::AP_CANDataDistribuer_IOTECH():
				    CANSensor("USD1")
{
  register_driver(AP_CANManager::Driver_Type_USD1);
}

// handler for incoming frames. These come in at 100Hz
void AP_CANDataDistribuer_IOTECH::handle_frame(AP_HAL::CANFrame &frame)
{
//    gcs().send_text(MAV_SEVERITY_INFO,"Id: %lu b[0]:%d b[1]:%d b[2]:%d b[3]:%d b[4]:%d b[5]:%d b[6]:%d b[7]:%d",
//    						frame.id&0x7ff,frame.data[0],frame.data[1],frame.data[2],frame.data[3],frame.data[4],frame.data[5],frame.data[6],frame.data[7]);
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

            front.x = lat;
            front.y = lon;
            got_front = true;
//            gcs().send_text(MAV_SEVERITY_INFO, "Lat: %ld, Lon: %ld", (long)lat, (long)lon);

            // store or push into queue here
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

	    rear.x = lat;
	    rear.y = lon;
            got_rear = true;

//	    gcs().send_text(MAV_SEVERITY_INFO, "rear.x: %ld, rear.y: %ld", rear.x,rear.y);

            // store values here
            return;
        }

	for(int i = 0; i < totalDeviceHandled;i++)
	{
	    WITH_SEMAPHORE(rngfndInst[i]->_msem);
		if (raw_id == 0xCF)
		{
//			static uint32_t last_alt = 0;   // stores last valid altitude

		uint32_t alt =
		    ((uint32_t)frame.data[3] << 24) |
		    ((uint32_t)frame.data[2] << 16) |
		    ((uint32_t)frame.data[1] << 8 ) |
		    ((uint32_t)frame.data[0]);

		rngfndInst[i]->_last_frame_ms = AP_HAL::millis();
		if(alt == 0)
		{
			return;
		}
			// Convert to meters (scaled down)
		float altitude_val = alt * 1e-6f;
		float max_m = rngfndInst[i]->max_distance_cm() * 0.01f;
		    // --- Protection: if value ≥ 16 meters, set to zero ---
		    if (altitude_val >= max_m) {
		        altitude_val = 0;
		    }
			rngfndInst[i]->_distance_sum += altitude_val;   // convert cm → m if needed
			rngfndInst[i]->_distance_count++;
		}
	}
        return; // unknown frame
}
bool AP_CANDataDistribuer_IOTECH::write_frame(AP_HAL::CANFrame &out_frame, const uint64_t timeout_us)
{
    // send via underlying CAN driver
    // Example: use CANSensor's send API (assuming it exists)
    return CANSensor::write_frame(out_frame, timeout_us);
}
//#endif  // AP_RANGEFINDER_USD1_CAN_ENABLED

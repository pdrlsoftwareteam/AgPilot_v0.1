#include "AP_RangeFinder_USD1_CAN.h"

//#if AP_RANGEFINDER_USD1_CAN_ENABLED

#include <AP_HAL/AP_HAL.h>
#include <GCS_MAVLink/GCS.h>
/*
  constructor
 */
AP_RangeFinder_USD1_CAN::AP_RangeFinder_USD1_CAN(RangeFinder::RangeFinder_State &_state, AP_RangeFinder_Params &_params, uint32_t msensId) :
AP_RangeFinder_Backend(_state, _params)
{
  _distance_count = 0;
  _distance_sum = 0;
  _msg_byte = _params.msgByte;
  _sensId = _params.msgId & 0x7ff;
  _min_dist = _params.min_distance_cm;
  if(msensId == 0x7FF)
    {
      _sensId = 0x7FF;
    }
  if(msensId == 0x00)
    {
      _sensId = 0x00;
      _msg_byte = 0x00000021;
    }
  _msem = &_sem;

  for(uint8_t i=8;i>0;i--)
    {
      uint8_t nibble =  _msg_byte>> ((i-1)*4) & 0x0F;
      if(nibble != 0)
    {
      if(nibble == 1)
        _arr[0] = i;
      else if(nibble == 2)
        _arr[1] = i;
      else if(nibble == 3)
        _arr[2] = i;
      else if(nibble == 4)
        _arr[3] = i;
    }
    }

  AP_CANDataDistribuer::getInstance()->addCANDataListener(this);
}

// update state
void AP_RangeFinder_USD1_CAN::update(void)
{
  WITH_SEMAPHORE(_sem);
  const uint32_t now = AP_HAL::millis();
  if (_distance_count == 0 && now - state.last_reading_ms > 500 && !sen_status) {
      // no new data.
      state.distance_m = 0;
      set_status(RangeFinder::Status::NoData);
  }
  else if(_distance_count == 0 && now - state.last_reading_ms > 500 && sen_status) {
      state.distance_m = _min_dist*0.01;
      state.last_reading_ms = AP_HAL::millis();
      sen_status = false;
      set_status(RangeFinder::Status::Good);
  }
  else if (_distance_count != 0) {
      state.distance_m = _distance_sum / _distance_count;
      state.last_reading_ms = AP_HAL::millis();
      _distance_sum = 0;
      _distance_count = 0;
      sen_status = false;
      update_status();
      // gcs().send_text(MAV_SEVERITY_INFO,"Distance :%lf",state.distance_m);
  }
}

//CAN data distributer
AP_CANDataDistribuer *AP_CANDataDistribuer::instance = nullptr;
AP_CANDataDistribuer::AP_CANDataDistribuer():
                    CANSensor("USD1")
{
  register_driver(AP_CANManager::Driver_Type_USD1);
}

// handler for incoming frames. These come in at 100Hz
void AP_CANDataDistribuer::handle_frame(AP_HAL::CANFrame &frame)
{
//    gcs().send_text(MAV_SEVERITY_INFO,"Id: %lu b[0]:%d b[1]:%d b[2]:%d b[3]:%d b[4]:%d b[5]:%d b[6]:%d b[7]:%d",
//          frame.id,frame.data[0],frame.data[1],frame.data[2],frame.data[3],frame.data[4],frame.data[5],frame.data[6],frame.data[7]);
  for(int i = 0; i < totalDeviceHandled;i++)
    {
      uint32_t can_id = frame.id & 0x7ff;
      if(rngfndInst[i]->_sensId == 0x7FF)
    {
      if((AP_HAL::millis() - now_us) > 250)
        {
          now_us = AP_HAL::millis();
//        gcs().send_text(MAV_SEVERITY_INFO,"Id: %lu b[0]:%d b[1]:%d b[2]:%d b[3]:%d b[4]:%d b[5]:%d b[6]:%d b[7]:%d",
//                can_id,frame.data[0],frame.data[1],frame.data[2],frame.data[3],frame.data[4],frame.data[5],frame.data[6],frame.data[7]);
          WITH_SEMAPHORE(rngfndInst[i]->_msem);
          return;
        }
    }
      else if(rngfndInst[i]->_sensId == can_id || rngfndInst[i]->_sensId == 0x00)
    {
      WITH_SEMAPHORE(rngfndInst[i]->_msem);
      uint32_t dist_cm = 0;
      uint8_t count = 0;
      for (int j = 4; j > 0; j--) {
          if (rngfndInst[i]->_arr[j-1]) {
          dist_cm |= frame.data[rngfndInst[i]->_arr[j-1]-1] << (count * 8);
          count++;
          }
      }
      rngfndInst[i]->sen_status = true;

      if(rngfndInst[i]->orientation() == 25)
      {
          uint32_t raw = dist_cm;

          // 1) Too far (>= max) → process as zero
          if (raw >= (uint32_t)rngfndInst[i]->max_distance_cm()) {
              dist_cm = 0;
              rngfndInst[i]->_distance_sum += 0.01f * dist_cm;
              rngfndInst[i]->_distance_count++;
              return;
          }

          // 2) Sensor returned zero — decide by context
          if (raw == 0) {

              // If previous value was high (e.g., > 80% of max) → likely out of range
              if (rngfndInst[i]->prev_valid_cm > (rngfndInst[i]->max_distance_cm() * 0.8f)) {
                  dist_cm = 0;     // process as zero
                  rngfndInst[i]->_distance_sum += 0.01f * dist_cm;
                  rngfndInst[i]->_distance_count++;
                  return;
              }

              // otherwise: likely too close or noise → ignore
              continue;
          }

          // 3) Normal valid reading → process and update previous
          rngfndInst[i]->_distance_sum += 0.01f * raw;
          rngfndInst[i]->_distance_count++;
          rngfndInst[i]->prev_valid_cm = raw;
          return;
      }
      else
      {
          // 3) Normal valid reading → process and update previous
          rngfndInst[i]->_distance_sum += 0.01f * dist_cm;
          rngfndInst[i]->_distance_count++;
          rngfndInst[i]->prev_valid_cm = dist_cm;
      }

    }

    }
}
    // send via underlying CAN driver
//#endif  // AP_RANGEFINDER_USD1_CAN_ENABLED

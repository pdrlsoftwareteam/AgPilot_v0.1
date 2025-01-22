#include "AG_RangeFinder_USD1_CAN.h"

#if AP_RANGEFINDER_USD1_CAN_ENABLED

#include <AG_HAL/AG_HAL.h>

/*
  constructor
 */
AG_RangeFinder_USD1_CAN::AG_RangeFinder_USD1_CAN(RangeFinder::RangeFinder_State &_state, AG_RangeFinder_Params &_params, uint8_t mlowerByte, uint8_t mupperByte, uint8_t msensId) :
    AG_RangeFinder_Backend(_state, _params)
{
	_lowerByte = mlowerByte;
	_upperByte = mupperByte;
	_sensId = msensId;
	_msem = &_sem;
	AP_CANDataDistribuer::getInstance()->addCANDataListener(this);
}

// update state
void AG_RangeFinder_USD1_CAN::update(void)
{
    WITH_SEMAPHORE(_sem);
    const uint32_t now = AG_HAL::millis();
    if (_distance_count == 0 && now - state.last_reading_ms > 500) {
        // no new data.
        set_status(RangeFinder::Status::NoData);
    } else if (_distance_count != 0) {
        state.distance_m = _distance_sum / _distance_count;
        state.last_reading_ms = AG_HAL::millis();
        _distance_sum = 0;
        _distance_count = 0;
        update_status();
    }
}

//CAN data distributer
AP_CANDataDistribuer *AP_CANDataDistribuer::instance = nullptr;
AP_CANDataDistribuer::AP_CANDataDistribuer():
CANSensor("USD1")
{
    register_driver(AG_CANManager::Driver_Type_USD1);
}

// handler for incoming frames. These come in at 100Hz
void AP_CANDataDistribuer::handle_frame(AG_HAL::CANFrame &frame)
{
    for(int i = 0; i < totalDeviceHandled;i++)
	{
    	if(rngfndInst[i]->_sensId == frame.id)
    	{
            WITH_SEMAPHORE(rngfndInst[i]->_msem);
    		const uint16_t dist_cm = (frame.data[rngfndInst[i]->_lowerByte]<<8) | frame.data[rngfndInst[i]->_upperByte];
    		rngfndInst[i]->_distance_sum += dist_cm * 0.01;
    		rngfndInst[i]->_distance_count = rngfndInst[i]->_distance_count + 1;
    		return;
    	}
	}
}

#endif  // AP_RANGEFINDER_USD1_CAN_ENABLED

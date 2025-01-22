#pragma once

#include "AG_RangeFinder_Backend.h"
#include <AG_CANManager/AG_CANSensor.h>

#ifndef AP_RANGEFINDER_USD1_CAN_ENABLED
#define AP_RANGEFINDER_USD1_CAN_ENABLED (HAL_MAX_CAN_PROTOCOL_DRIVERS && AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED)
#endif

#if AP_RANGEFINDER_USD1_CAN_ENABLED

class AG_RangeFinder_USD1_CAN :  public AG_RangeFinder_Backend {
public:
    AG_RangeFinder_USD1_CAN(RangeFinder::RangeFinder_State &_state, AG_RangeFinder_Params &_params, uint8_t mlowerByte = 0, uint8_t muppertByte = 0, uint8_t msensId = 0);

    void update() override;
    
protected:
    virtual MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_RADAR;
    }
public:
    float _distance_sum;
    uint32_t _distance_count;
    uint8_t _sensId = 0;
    uint8_t _lowerByte=0, _upperByte=0;
    HAL_Semaphore *_msem;
};


class AP_CANDataDistribuer : public CANSensor
{
	static AP_CANDataDistribuer *instance;
	AG_RangeFinder_USD1_CAN *rngfndInst[3];
	uint8_t totalDeviceHandled = 0;
public:
	static AP_CANDataDistribuer* getInstance(){
		if(instance == nullptr)
			instance = new AP_CANDataDistribuer();
		return instance;
	}

	AP_CANDataDistribuer();
	void addCANDataListener(AG_RangeFinder_USD1_CAN * mrngFndInst)
	{
		if(totalDeviceHandled<3)
		{
			rngfndInst[totalDeviceHandled] = mrngFndInst;
			totalDeviceHandled++;
		}
	}

    // handler for incoming frames
    void handle_frame(AG_HAL::CANFrame &frame) override;

};


#endif  // AP_RANGEFINDER_USD1_CAN_ENABLED

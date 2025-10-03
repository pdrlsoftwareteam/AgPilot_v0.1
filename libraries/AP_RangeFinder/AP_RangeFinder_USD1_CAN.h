#pragma once

#include "AP_RangeFinder_Backend.h"
#include <AP_CANManager/AP_CANSensor.h>

#ifndef AP_RANGEFINDER_USD1_CAN_ENABLED
#define AP_RANGEFINDER_USD1_CAN_ENABLED (HAL_MAX_CAN_PROTOCOL_DRIVERS && AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED)
#endif

#if AP_RANGEFINDER_USD1_CAN_ENABLED

class AP_RangeFinder_USD1_CAN :  public AP_RangeFinder_Backend {
public:
    AP_RangeFinder_USD1_CAN(RangeFinder::RangeFinder_State &_state, AP_RangeFinder_Params &_params, uint8_t mlowerByte = 0, uint8_t muppertByte = 0, uint8_t msensId = 0);

    void update() override;
    void get_iotech_rear_value(uint32_t &rx, uint32_t &ry)override {};
    void get_iotech_front_value(uint32_t &fx, uint32_t &fy) override{};
    bool check_sensor_status()override { return false;}
    
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
	AP_RangeFinder_USD1_CAN *rngfndInst[3];
	uint8_t totalDeviceHandled = 0;
public:
	static AP_CANDataDistribuer* getInstance(){
		if(instance == nullptr)
			instance = new AP_CANDataDistribuer();
		return instance;
	}

	AP_CANDataDistribuer();
	void addCANDataListener(AP_RangeFinder_USD1_CAN * mrngFndInst)
	{
		if(totalDeviceHandled<3)
		{
			rngfndInst[totalDeviceHandled] = mrngFndInst;
			totalDeviceHandled++;
		}
	}

    // handler for incoming frames
    void handle_frame(AP_HAL::CANFrame &frame) override;

};


#endif  // AP_RANGEFINDER_USD1_CAN_ENABLED

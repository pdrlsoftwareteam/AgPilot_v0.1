#pragma once

#include "AP_RangeFinder_Backend.h"
#include <AP_CANManager/AP_CANSensor.h>

#ifndef AP_RANGEFINDER_USD1_CAN_ENABLED
#define AP_RANGEFINDER_USD1_CAN_ENABLED (HAL_MAX_CAN_PROTOCOL_DRIVERS && AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED)
#endif

//#if AP_RANGEFINDER_USD1_CAN_ENABLED

class AP_RangeFinder_USD1_CAN_IOTECH :  public AP_RangeFinder_Backend {
public:
  AP_RangeFinder_USD1_CAN_IOTECH(RangeFinder::RangeFinder_State &_state, AP_RangeFinder_Params &_params);

    void update() override;
    void get_iotech_rear_value(uint32_t &rx, uint32_t &ry) override;
    void get_iotech_front_value(uint32_t &fx, uint32_t &fy) override;
    bool check_sensor_status() override;
    void update_sensor_status();

protected:
    virtual MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_RADAR;
    }
public:
    float _distance_sum;
    uint32_t _distance_count;
    HAL_Semaphore *_msem;
	uint16_t _min_dist;
	bool sen_status = false;
};


class AP_CANDataDistribuer_IOTECH : public CANSensor
{
	static AP_CANDataDistribuer_IOTECH *instance;
	AP_RangeFinder_USD1_CAN_IOTECH *rngfndInst[3];
	uint8_t totalDeviceHandled = 0;
public:
	uint32_t now_us = AP_HAL::millis();
	static AP_CANDataDistribuer_IOTECH* getInstance(){
		if(instance == nullptr)
			instance = new AP_CANDataDistribuer_IOTECH();
		return instance;
	}

	AP_CANDataDistribuer_IOTECH();
	void addCANDataListener(AP_RangeFinder_USD1_CAN_IOTECH * mrngFndInst)
	{
		if(totalDeviceHandled<3)
		{
			rngfndInst[totalDeviceHandled] = mrngFndInst;
			totalDeviceHandled++;
		}
	}
	Vector2l front;
	Vector2l rear;
	uint32_t altitude = 0;
        bool got_front = false;
        bool got_rear = false;

    // handler for incoming frames
    void handle_frame(AP_HAL::CANFrame &frame) override;
    bool write_frame(AP_HAL::CANFrame &out_frame, const uint64_t timeout_us);

};


//#endif  // AP_RANGEFINDER_USD1_CAN_ENABLED

#pragma once

#include "AP_Proximity_Backend.h"
#include <AP_CANManager/AP_CANSensor.h>

#if HAL_PROXIMITY_ENABLED

#define PROXIMITY_IOTECH_TIMEOUT_MS 800 // requests timeout after 0.2 seconds

class AP_Proximity_IOTECH : public AP_Proximity_Backend
{

public:
    // constructor
    using AP_Proximity_Backend::AP_Proximity_Backend;

    // update state
    void update(void) override;
    float findDistance(double lat1, double lon1, double lat2, double lon2);
    void new_driver();
    void send_frame();
    // get maximum and minimum distances (in meters) of sensor
    float distance_max() const override { return _distance_max; }
    float distance_min() const override { return _distance_min; }

    // get distance upwards in meters. returns true on success
    bool get_upward_distance(float &distance) const override;
    bool got_front;
    bool got_rear;
private:

    // horizontal distance support
    uint32_t _last_update_ms;   // system time of last RangeFinder reading
    float _distance_max;        // max range of sensor in meters
    float _distance_min;        // min range of sensor in meters

    // upward distance support
    uint32_t _last_upward_update_ms;    // system time of last update distance
    float _distance_upward = -1;        // upward distance in meters, negative if the last reading was out of range
};

class AP_CANDataDistribuer_IOT : public CANSensor
{
    static AP_CANDataDistribuer_IOT *instance;
    AP_Proximity_IOTECH *prx_instance[3];
    uint8_t totalDeviceHandled = 0;
public:
    uint32_t now_us = AP_HAL::millis();
    static AP_CANDataDistribuer_IOT* get_instance() {
        static AP_CANDataDistribuer_IOT instance;
        return &instance;
    }

    AP_CANDataDistribuer_IOT();
    void addCANDataListener_Prx_iot(AP_Proximity_IOTECH * prx_inst)
    {
        if(totalDeviceHandled<3)
        {
            prx_instance[totalDeviceHandled] = prx_inst;
            totalDeviceHandled++;
        }
    }

    // handler for incoming frames
    void handle_frame(AP_HAL::CANFrame &frame) override;
    bool write_frame(AP_HAL::CANFrame &out_frame, const uint64_t timeout_us);
    float findDistance(double lat1, double lon1, double lat2, double lon2);

    float front_dist;
    float rear_dist;
    bool got_front = false;
    bool got_rear = false;
};

#endif // HAL_PROXIMITY_ENABLED

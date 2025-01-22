#pragma once

#include "AG_OpticalFlow.h"

#ifndef AP_OPTICALFLOW_HEREFLOW_ENABLED
#define AP_OPTICALFLOW_HEREFLOW_ENABLED (AP_OPTICALFLOW_ENABLED && HAL_ENABLE_LIBUAVCAN_DRIVERS)
#endif

#if AP_OPTICALFLOW_HEREFLOW_ENABLED

#include <AG_UAVCAN/AG_UAVCAN.h>

class MeasurementCb;

class AG_OpticalFlow_HereFlow : public OpticalFlow_backend {
public:
    AG_OpticalFlow_HereFlow(AG_OpticalFlow &flow);

    void init() override {}

    void update() override;

    static void subscribe_msgs(AG_UAVCAN* ap_uavcan);

    static void handle_measurement(AG_UAVCAN* ap_uavcan, uint8_t node_id, const MeasurementCb &cb);

private:

    Vector2f flowRate, bodyRate;
    uint8_t surface_quality;
    float integral_time;
    bool new_data;
    static uint8_t _node_id;

    static AG_OpticalFlow_HereFlow* _driver;
    static AG_UAVCAN* _ap_uavcan;
    void _push_state(void);

};

#endif  // AP_OPTICALFLOW_HEREFLOW_ENABLED

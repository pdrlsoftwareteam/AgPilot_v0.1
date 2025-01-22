#pragma once

#include "AG_RangeFinder_Backend.h"

#ifndef AP_RANGEFINDER_UAVCAN_ENABLED
#define AP_RANGEFINDER_UAVCAN_ENABLED (HAL_CANMANAGER_ENABLED && AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED)
#endif

#if AP_RANGEFINDER_UAVCAN_ENABLED

#include <AG_UAVCAN/AG_UAVCAN.h>

class MeasurementCb;

class AG_RangeFinder_UAVCAN : public AG_RangeFinder_Backend {
public:
    //constructor - registers instance at top RangeFinder driver
    using AG_RangeFinder_Backend::AG_RangeFinder_Backend;

    void update() override;

    static void subscribe_msgs(AG_UAVCAN* ap_uavcan);
    static AG_RangeFinder_UAVCAN* get_uavcan_backend(AG_UAVCAN* ap_uavcan, uint8_t node_id, uint8_t address, bool create_new);
    static AG_RangeFinder_Backend* detect(RangeFinder::RangeFinder_State &_state, AG_RangeFinder_Params &_params);

    static void handle_measurement(AG_UAVCAN* ap_uavcan, uint8_t node_id, const MeasurementCb &cb);

protected:
    virtual MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return _sensor_type;
    }
private:
    uint8_t _instance;
    RangeFinder::Status _status;
    uint16_t _distance_cm;
    uint32_t _last_reading_ms;
    AG_UAVCAN* _ap_uavcan;
    uint8_t _node_id;
    bool new_data;
    MAV_DISTANCE_SENSOR _sensor_type;
};
#endif  // AP_RANGEFINDER_UAVCAN_ENABLED

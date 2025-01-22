#pragma once

#include "AG_RangeFinder.h"
#include "AG_RangeFinder_Backend_Serial.h"

#ifndef AP_RANGEFINDER_TERARANGER_SERIAL_ENABLED
#define AP_RANGEFINDER_TERARANGER_SERIAL_ENABLED AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED
#endif

#if AP_RANGEFINDER_TERARANGER_SERIAL_ENABLED

class AG_RangeFinder_TeraRanger_Serial : public AG_RangeFinder_Backend_Serial
{

public:

    static AG_RangeFinder_Backend_Serial *create(
        RangeFinder::RangeFinder_State &_state,
        AG_RangeFinder_Params &_params) {
        return new AG_RangeFinder_TeraRanger_Serial(_state, _params);
    }

protected:

    using AG_RangeFinder_Backend_Serial::AG_RangeFinder_Backend_Serial;

    MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_LASER;
    }

private:

    // get a reading
    // distance returned in reading_m
    bool get_reading(float &reading_m) override;

    uint8_t linebuf[10];
    uint8_t linebuf_len;
};
#endif  // AP_RANGEFINDER_TERARANGER_SERIAL_ENABLED

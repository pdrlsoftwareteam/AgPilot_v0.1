#pragma once

#include "AG_RangeFinder.h"
#include "AG_RangeFinder_Backend.h"

#ifndef AP_RANGEFINDER_HC_SR04_ENABLED
#define AP_RANGEFINDER_HC_SR04_ENABLED AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED
#endif

#if AP_RANGEFINDER_HC_SR04_ENABLED

#include "AG_RangeFinder_Params.h"

class AG_RangeFinder_HC_SR04 : public AG_RangeFinder_Backend
{
public:
    // constructor
    AG_RangeFinder_HC_SR04(RangeFinder::RangeFinder_State &_state, AG_RangeFinder_Params &_params);

    // static detection function
    static bool detect(AG_RangeFinder_Params &_params);

    // update state
    void update(void) override;

protected:

    MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_ULTRASOUND;
    }

private:

    bool check_pins();
    bool check_echo_pin();
    void check_trigger_pin();

    int8_t trigger_pin;
    uint32_t last_reading_ms;      // system time of last read (used for health reporting)
    float last_distance_m;         // last distance reported (used to prevent glitches in measurement)
    uint8_t glitch_count;          // glitch counter

    AG_HAL::PWMSource pwm_source;

    uint32_t last_ping_ms;
};

#endif  // AP_RANGEFINDER_HC_SR04_ENABLED

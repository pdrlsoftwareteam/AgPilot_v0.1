#pragma once

#include "AG_RangeFinder.h"
#include "AG_RangeFinder_Backend.h"
#include "AG_RangeFinder_Params.h"

#ifndef AP_RANGEFINDER_ANALOG_ENABLED
#define AP_RANGEFINDER_ANALOG_ENABLED AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED
#endif

#if AP_RANGEFINDER_ANALOG_ENABLED

class AG_RangeFinder_analog : public AG_RangeFinder_Backend
{
public:
    // constructor
    AG_RangeFinder_analog(RangeFinder::RangeFinder_State &_state, AG_RangeFinder_Params &_params);

    // static detection function
    static bool detect(AG_RangeFinder_Params &_params);

    // update state
    void update(void) override;

protected:

    MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_UNKNOWN;
    }

private:
    // update raw voltage
    void update_voltage(void);

    AG_HAL::AnalogSource *source;
};

#endif

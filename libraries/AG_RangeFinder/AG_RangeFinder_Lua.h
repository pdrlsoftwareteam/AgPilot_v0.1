#pragma once

#include "AG_RangeFinder.h"
#include "AG_RangeFinder_Backend.h"

#if AP_SCRIPTING_ENABLED

// Data timeout
#define AP_RANGEFINDER_LUA_TIMEOUT_MS 500

class AG_RangeFinder_Lua : public AG_RangeFinder_Backend
{
public:

    // constructor
    AG_RangeFinder_Lua(RangeFinder::RangeFinder_State &_state, AG_RangeFinder_Params &_params);

    // update state
    void update(void) override;

    // Get update from Lua script
    bool handle_script_msg(float dist_m) override;

    MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_UNKNOWN;
    }

private:

    float _distance_m;   // stored data from lua script:
};

#endif

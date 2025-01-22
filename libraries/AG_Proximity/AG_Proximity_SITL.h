#pragma once

#include "AG_Proximity.h"

#if HAL_PROXIMITY_ENABLED
#include "AG_Proximity_Backend.h"
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
#include <SITL/SITL.h>
#include <AG_Fence/AG_PolyFence_loader.h>
#include <AG_Common/Location.h>

class AG_Proximity_SITL : public AG_Proximity_Backend
{

public:
    // constructor
    AG_Proximity_SITL(AG_Proximity &_frontend, AG_Proximity::Proximity_State &_state, AG_Proximity_Params& _params);

    // update state
    void update(void) override;

    // get maximum and minimum distances (in meters) of sensor
    float distance_max() const override;
    float distance_min() const override;

    // get distance upwards in meters. returns true on success
    bool get_upward_distance(float &distance) const override;

private:
    SITL::SIM *sitl;
    AP_Float *fence_alt_max;
    Location current_loc;

    // get distance in meters to fence in a particular direction in degrees (0 is forward, angles increase in the clockwise direction)
    bool get_distance_to_fence(float angle_deg, float &distance) const;

};
#endif // CONFIG_HAL_BOARD

#endif // HAL_PROXIMITY_ENABLED

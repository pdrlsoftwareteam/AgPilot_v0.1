/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <AG_BoardConfig/AG_BoardConfig.h>

#include "AG_RangeFinder_Backend.h"

#ifndef AP_RANGEFINDER_SIM_ENABLED
#define AP_RANGEFINDER_SIM_ENABLED (CONFIG_HAL_BOARD == HAL_BOARD_SITL && AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED)
#endif

#if AP_RANGEFINDER_SIM_ENABLED

class AG_RangeFinder_SITL : public AG_RangeFinder_Backend {
public:
    // constructor. This incorporates initialisation as well.
    AG_RangeFinder_SITL(RangeFinder::RangeFinder_State &_state, AG_RangeFinder_Params &_params, uint8_t instance);

    // update the state structure
    void update() override;

protected:

    MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_UNKNOWN;
    }

    uint8_t _instance;

};

#endif  // AP_RANGEFINDER_SIM_ENABLED

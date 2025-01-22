/*
 * Copyright (C) 2019  Lucas De Marchi <lucas.de.marchi@gmail.com>
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "AG_RangeFinder.h"
#include "AG_RangeFinder_Backend.h"

#ifndef AP_RANGEFINDER_BENEWAKE_TFMINIPLUS_ENABLED
#define AP_RANGEFINDER_BENEWAKE_TFMINIPLUS_ENABLED AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED
#endif

#if AP_RANGEFINDER_BENEWAKE_TFMINIPLUS_ENABLED

#define TFMINIPLUS_ADDR_DEFAULT              0x10        // TFMini default device id

#include <AG_HAL/utility/sparse-endian.h>
#include <AG_HAL/I2CDevice.h>

class AG_RangeFinder_Benewake_TFMiniPlus : public AG_RangeFinder_Backend
{

public:
    // static detection function
    static AG_RangeFinder_Backend *detect(RangeFinder::RangeFinder_State &_state,
                                          AG_RangeFinder_Params &_params,
                                          AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev);

    // update state
    void update(void) override;

protected:

    virtual MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_LASER;
    }

private:
    AG_RangeFinder_Benewake_TFMiniPlus(RangeFinder::RangeFinder_State &_state,
                                       AG_RangeFinder_Params &_params,
                                       AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev);

    bool init();
    void timer();

    void process_raw_measure(le16_t distance_raw, le16_t strength_raw,
                             uint16_t &output_distance_cm);

    bool check_checksum(uint8_t *arr, int pkt_len);

    AG_HAL::OwnPtr<AG_HAL::I2CDevice> _dev;

    struct {
        uint32_t sum;
        uint32_t count;
    } accum;
};

#endif

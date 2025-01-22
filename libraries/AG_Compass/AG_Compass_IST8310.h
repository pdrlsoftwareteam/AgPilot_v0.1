/*
 * Copyright (C) 2016  Emlid Ltd. All rights reserved.
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

#include "AG_Compass_config.h"

#if AP_COMPASS_IST8310_ENABLED

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/I2CDevice.h>
#include <AG_Math/AG_Math.h>

#include "AG_Compass_Backend.h"

#ifndef HAL_COMPASS_IST8310_I2C_ADDR
#define HAL_COMPASS_IST8310_I2C_ADDR 0x0E
#endif

#ifndef AP_COMPASS_IST8310_DEFAULT_ROTATION
#define AP_COMPASS_IST8310_DEFAULT_ROTATION ROTATION_PITCH_180
#endif

class AG_Compass_IST8310 : public AG_Compass_Backend
{
public:
    static AG_Compass_Backend *probe(AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev,
                                     bool force_external,
                                     enum Rotation rotation);

    void read() override;

    static constexpr const char *name = "IST8310";

private:
    AG_Compass_IST8310(AG_HAL::OwnPtr<AG_HAL::Device> dev,
                       bool force_external,
                       enum Rotation rotation);

    void timer();
    bool init();
    void start_conversion();

    AG_HAL::OwnPtr<AG_HAL::Device> _dev;
    AG_HAL::Device::PeriodicHandle _periodic_handle;

    enum Rotation _rotation;
    uint8_t _instance;
    bool _ignore_next_sample;
    bool _force_external;
};

#endif  // AP_COMPASS_IST8310_ENABLED

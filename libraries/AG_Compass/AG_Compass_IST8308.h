/*
 * Copyright (C) 2018  Lucas De Marchi. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
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

#if AP_COMPASS_IST8308_ENABLED

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/I2CDevice.h>

#include "AG_Compass_Backend.h"

#ifndef HAL_COMPASS_IST8308_I2C_ADDR
#define HAL_COMPASS_IST8308_I2C_ADDR 0x0C
#endif

class AG_Compass_IST8308 : public AG_Compass_Backend
{
public:
    static AG_Compass_Backend *probe(AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev,
                                     bool force_external,
                                     enum Rotation rotation);

    void read() override;

    static constexpr const char *name = "IST8308";

private:
    AG_Compass_IST8308(AG_HAL::OwnPtr<AG_HAL::Device> dev,
                       bool force_external,
                       enum Rotation rotation);

    void timer();
    bool init();

    AG_HAL::OwnPtr<AG_HAL::Device> _dev;

    enum Rotation _rotation;
    uint8_t _instance;
    bool _force_external;
};

#endif  // AP_COMPASS_IST8308_ENABLED

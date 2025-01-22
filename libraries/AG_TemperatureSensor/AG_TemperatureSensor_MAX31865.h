/*
 * Copyright (C) 2023 Kraus Hamdani Aerospace Inc. All rights reserved.
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
 *
 * Code by Tom Pittenger
 */
/*
    Implements SPI driver for MAX31865 digital RTD Temperature converter
*/

#pragma once
#include "AG_TemperatureSensor_Backend.h"

#if AP_TEMPERATURE_SENSOR_MAX31865_ENABLED

class AG_TemperatureSensor_MAX31865 : public AG_TemperatureSensor_Backend {
    using AG_TemperatureSensor_Backend::AG_TemperatureSensor_Backend;
public:

    void init(void) override;

    void update(void) override {};

private:
    void thread_tick(void);
};

#endif // AP_TEMPERATURE_SENSOR_MAX31865_ENABLED

/*
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

#if AP_COMPASS_MMC3416_ENABLED

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/I2CDevice.h>
#include <AG_Math/AG_Math.h>

#include "AG_Compass.h"
#include "AG_Compass_Backend.h"

#ifndef HAL_COMPASS_MMC3416_I2C_ADDR
# define HAL_COMPASS_MMC3416_I2C_ADDR 0x30
#endif

class AG_Compass_MMC3416 : public AG_Compass_Backend
{
public:
    static AG_Compass_Backend *probe(AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev,
                                     bool force_external,
                                     enum Rotation rotation);

    void read() override;

    static constexpr const char *name = "MMC3416";

private:
    AG_Compass_MMC3416(AG_HAL::OwnPtr<AG_HAL::Device> dev,
                       bool force_external,
                       enum Rotation rotation);

    AG_HAL::OwnPtr<AG_HAL::Device> dev;

    enum {
        STATE_REFILL1,
        STATE_REFILL1_WAIT,
        STATE_MEASURE_WAIT1,
        STATE_REFILL2_WAIT,
        STATE_MEASURE_WAIT2,
        STATE_MEASURE_WAIT3,
    } state;
    
    /**
     * Device periodic callback to read data from the sensor.
     */
    bool init();
    void timer();
    void accumulate_field(Vector3f &field);

    uint8_t compass_instance;
    bool force_external;
    Vector3f offset;
    uint16_t measure_count;
    bool have_initial_offset;
    uint32_t refill_start_ms;
    uint32_t last_sample_ms;
    
    uint16_t data0[3];
    
    enum Rotation rotation;
};

#endif  // AP_COMPASS_MMC3416_ENABLED

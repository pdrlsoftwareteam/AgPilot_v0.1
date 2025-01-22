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
/*
  driver for PX4Flow optical flow sensor
 */

#include "AG_OpticalFlow_PX4Flow.h"

#if AP_OPTICALFLOW_PX4FLOW_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_Math/crc.h>
#include <AG_AHRS/AG_AHRS.h>
#include <AG_HAL/I2CDevice.h>
#include <utility>
#include "AG_OpticalFlow.h"
#include <stdio.h>
#include <AG_BoardConfig/AG_BoardConfig.h>

extern const AG_HAL::HAL& hal;

#define PX4FLOW_BASE_I2C_ADDR   0x42
#define PX4FLOW_INIT_RETRIES    10      // attempt to initialise the sensor up to 10 times at startup


// detect the device
AG_OpticalFlow_PX4Flow *AG_OpticalFlow_PX4Flow::detect(AG_OpticalFlow &_frontend)
{
    AG_OpticalFlow_PX4Flow *sensor = new AG_OpticalFlow_PX4Flow(_frontend);
    if (!sensor) {
        return nullptr;
    }
    if (!sensor->setup_sensor()) {
        delete sensor;
        return nullptr;
    }
    return sensor;
}

/*
  look for the sensor on different buses
 */
bool AG_OpticalFlow_PX4Flow::scan_buses(void)
{
    bool success = false;
    uint8_t retry_attempt = 0;

    while (!success && retry_attempt < PX4FLOW_INIT_RETRIES) {
        bool all_external = (AG_BoardConfig::get_board_type() == AG_BoardConfig::PX4_BOARD_PIXHAWK2);
        uint32_t bus_mask = all_external? hal.i2c_mgr->get_bus_mask() : hal.i2c_mgr->get_bus_mask_external();
        FOREACH_I2C_MASK(bus, bus_mask) {
    #ifdef HAL_OPTFLOW_PX4FLOW_I2C_BUS
            // only one bus from HAL
            if (bus != HAL_OPTFLOW_PX4FLOW_I2C_BUS) {
                continue;
            }
    #endif
            AG_HAL::OwnPtr<AG_HAL::Device> tdev = hal.i2c_mgr->get_device(bus, PX4FLOW_BASE_I2C_ADDR + get_address());
            if (!tdev) {
                continue;
            }
            WITH_SEMAPHORE(tdev->get_semaphore());

            struct i2c_integral_frame frame;
            success = tdev->read_registers(REG_INTEGRAL_FRAME, (uint8_t *)&frame, sizeof(frame));
            if (success) {
                printf("Found PX4Flow on bus %u\n", bus);
                dev = std::move(tdev);
                break;
            }
        }
        retry_attempt++;
        if (!success) {
            hal.scheduler->delay(10);
        }
    }
    return success;
}

// setup the device
bool AG_OpticalFlow_PX4Flow::setup_sensor(void)
{
    if (!scan_buses()) {
        return false;
    }
    // read at 10Hz
    dev->register_periodic_callback(100000, FUNCTOR_BIND_MEMBER(&AG_OpticalFlow_PX4Flow::timer, void));
    return true;
}


// update - read latest values from sensor and fill in x,y and totals.
void AG_OpticalFlow_PX4Flow::update(void)
{
}

// timer to read sensor
void AG_OpticalFlow_PX4Flow::timer(void)
{
    struct i2c_integral_frame frame;
    if (!dev->read_registers(REG_INTEGRAL_FRAME, (uint8_t *)&frame, sizeof(frame))) {
        return;
    }
    struct AG_OpticalFlow::OpticalFlow_state state {};

    if (frame.integration_timespan > 0) {
        const Vector2f flowScaler = _flowScaler();
        float flowScaleFactorX = 1.0f + 0.001f * flowScaler.x;
        float flowScaleFactorY = 1.0f + 0.001f * flowScaler.y;
        float integralToRate = 1.0e6 / frame.integration_timespan;
        
        state.surface_quality = frame.qual;
        state.flowRate = Vector2f(frame.pixel_flow_x_integral * flowScaleFactorX,
                                  frame.pixel_flow_y_integral * flowScaleFactorY) * 1.0e-4 * integralToRate;
        state.bodyRate = Vector2f(frame.gyro_x_rate_integral, frame.gyro_y_rate_integral) * 1.0e-4 * integralToRate;
        
        _applyYaw(state.flowRate);
        _applyYaw(state.bodyRate);
    }

    _update_frontend(state);
}

#endif  // AP_OPTICALFLOW_PX4FLOW_ENABLED

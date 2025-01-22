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
/*
  the BMI055 is unusual as it has separate chip-select for accel and
  gyro, which means it needs two SPIDevice pointers
 */
#pragma once

#include <AG_HAL/AG_HAL.h>

#include "AG_InertialSensor.h"
#include "AG_InertialSensor_Backend.h"

class AG_InertialSensor_BMI055 : public AG_InertialSensor_Backend {
public:
    static AG_InertialSensor_Backend *probe(AG_InertialSensor &imu,
                                            AG_HAL::OwnPtr<AG_HAL::SPIDevice> dev_accel,
                                            AG_HAL::OwnPtr<AG_HAL::SPIDevice> dev_gyro,
                                            enum Rotation rotation);

    /**
     * Configure the sensors and start reading routine.
     */
    void start() override;
    bool update() override;

private:
    AG_InertialSensor_BMI055(AG_InertialSensor &imu,
                             AG_HAL::OwnPtr<AG_HAL::Device> dev_accel,
                             AG_HAL::OwnPtr<AG_HAL::Device> dev_gyro,
                             enum Rotation rotation);

    /*
     initialise hardware layer
     */
    bool accel_init();
    bool gyro_init();

    /*
      initialise driver
     */
    bool init();

    /*
      read data from the FIFOs
     */
    void read_fifo_accel();
    void read_fifo_gyro();

    AG_HAL::OwnPtr<AG_HAL::Device> dev_accel;
    AG_HAL::OwnPtr<AG_HAL::Device> dev_gyro;

    uint8_t accel_instance;
    uint8_t gyro_instance;
    enum Rotation rotation;
    uint8_t temperature_counter;
};

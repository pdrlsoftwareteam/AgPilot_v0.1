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

#include "AG_Baro_ICM20789.h"

#if AP_BARO_ICM20789_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/I2CDevice.h>
#include <utility>

#include <AG_Common/AG_Common.h>
#include <AG_BoardConfig/AG_BoardConfig.h>

#include <stdio.h>

#include <AG_Math/AG_Math.h>
#include <AG_Logger/AG_Logger.h>

#include <AG_InertialSensor/AG_InertialSensor_Invensense_registers.h>

extern const AG_HAL::HAL &hal;

/*
  CMD_READ options. The draft datasheet doesn't specify, but it seems
  Mode_1 has a conversion interval of 2ms. Mode_3 has a conversion
  interval of 20ms. Both seem to produce equally as smooth results, so
  presumably Mode_3 is doing internal averaging
 */
#define CMD_READ_PT_MODE_1 0x401A
#define CMD_READ_PT_MODE_3 0x5059
#define CMD_READ_TP_MODE_1 0x609C
#define CMD_READ_TP_MODE_3 0x70DF

#define CONVERSION_INTERVAL_MODE_1 2000
#define CONVERSION_INTERVAL_MODE_3 20000

// setup for Mode_3
#define CMD_READ_PT CMD_READ_PT_MODE_3
#define CONVERSION_INTERVAL CONVERSION_INTERVAL_MODE_3

#define CMD_SOFT_RESET     0x805D
#define CMD_READ_ID        0xEFC8

#define BARO_ICM20789_DEBUG 0

#if BARO_ICM20789_DEBUG
#define debug(fmt, args...)   hal.console->printf(fmt, ##args)
#else
#define debug(fmt, args...)
#endif

/*
  constructor
 */
AG_Baro_ICM20789::AG_Baro_ICM20789(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::I2CDevice> _dev, AG_HAL::OwnPtr<AG_HAL::Device> _dev_imu)
    : AG_Baro_Backend(baro)
    , dev(std::move(_dev))
    , dev_imu(std::move(_dev_imu))
{
}

AG_Baro_Backend *AG_Baro_ICM20789::probe(AG_Baro &baro,
                                         AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev,
                                         AG_HAL::OwnPtr<AG_HAL::Device> dev_imu)
{
    debug("Probing for ICM20789 baro\n");
    if (!dev || !dev_imu) {
        return nullptr;
    }
    AG_Baro_ICM20789 *sensor = new AG_Baro_ICM20789(baro, std::move(dev), std::move(dev_imu));
    if (!sensor || !sensor->init()) {
        delete sensor;
        return nullptr;
    }
    return sensor;
}


/*
  setup ICM20789 to enable barometer, assuming IMU is on SPI and baro is on I2C
*/
bool AG_Baro_ICM20789::imu_spi_init(void)
{
    dev_imu->get_semaphore()->take_blocking();

    dev_imu->set_read_flag(0x80);

    dev_imu->set_speed(AG_HAL::Device::SPEED_LOW);
    uint8_t whoami = 0;
    uint8_t v;

    dev_imu->read_registers(MPUREG_USER_CTRL, &v, 1);
    dev_imu->write_register(MPUREG_PWR_MGMT_1, BIT_PWR_MGMT_1_CLK_XGYRO);

    hal.scheduler->delay(1);
    dev_imu->write_register(MPUREG_USER_CTRL, BIT_USER_CTRL_I2C_IF_DIS);
    dev_imu->write_register(MPUREG_PWR_MGMT_1,
                            BIT_PWR_MGMT_1_SLEEP | BIT_PWR_MGMT_1_CLK_XGYRO);

    hal.scheduler->delay(1);
    dev_imu->write_register(MPUREG_PWR_MGMT_1, BIT_PWR_MGMT_1_CLK_XGYRO);

    hal.scheduler->delay(1);
    dev_imu->write_register(MPUREG_FIFO_EN, 0x00);
    dev_imu->write_register(MPUREG_PWR_MGMT_1,
                            BIT_PWR_MGMT_1_SLEEP | BIT_PWR_MGMT_1_CLK_XGYRO);

    dev_imu->read_registers(MPUREG_WHOAMI, &whoami, 1);

    // wait for sensor to settle
    hal.scheduler->delay(100);

    dev_imu->read_registers(MPUREG_WHOAMI, &whoami, 1);

    dev_imu->write_register(MPUREG_INT_PIN_CFG, 0x00);
    dev_imu->write_register(MPUREG_USER_CTRL, BIT_USER_CTRL_I2C_IF_DIS);

    dev_imu->get_semaphore()->give();

    return true;
}

/*
  setup ICM20789 to enable barometer, assuming both IMU and baro on the same i2c bus
*/
bool AG_Baro_ICM20789::imu_i2c_init(void)
{
    // as the baro device is already locked we need to re-use it,
    // changing its address to match the IMU address
    uint8_t old_address = dev->get_bus_address();
    dev->set_address(dev_imu->get_bus_address());
    
    dev->set_retries(4);

    uint8_t whoami=0;
    dev->read_registers(MPUREG_WHOAMI, &whoami, 1);
    debug("ICM20789: whoami 0x%02x old_address=%02x\n", whoami, old_address);

    dev->write_register(MPUREG_FIFO_EN, 0x00);
    dev->write_register(MPUREG_PWR_MGMT_1, BIT_PWR_MGMT_1_CLK_XGYRO);
    
    // wait for sensor to settle
    hal.scheduler->delay(10);

    dev->write_register(MPUREG_INT_PIN_CFG, BIT_BYPASS_EN);

    dev->set_address(old_address);

    return true;
}

bool AG_Baro_ICM20789::init()
{
    if (!dev) {
        return false;
    }

    debug("Looking for 20789 baro\n");

    dev->get_semaphore()->take_blocking();

    debug("Setting up IMU\n");
    if (dev_imu->bus_type() != AG_HAL::Device::BUS_TYPE_I2C) {
        if (!imu_spi_init()) {
            debug("ICM20789: failed to initialise IMU SPI device\n");
            return false;
        }
    } else if (!imu_i2c_init()) {
        debug("ICM20789: failed to initialise IMU I2C device\n");
        return false;
    }

    if (!send_cmd16(CMD_SOFT_RESET)) {
        debug("ICM20789: reset failed\n");
        goto failed;
    }

    // wait for sensor to settle
    hal.scheduler->delay(10);

    if (!read_calibration_data()) {
        debug("ICM20789: read_calibration_data failed\n");
        goto failed;
    }

    // start a reading
    if (!send_cmd16(CMD_READ_PT)) {
        debug("ICM20789: start read failed\n");
        goto failed;
    }

    dev->set_retries(0);

    instance = _frontend.register_sensor();

    dev->set_device_type(DEVTYPE_BARO_ICM20789);
    set_bus_id(instance, dev->get_bus_id());
    
    dev->get_semaphore()->give();

    debug("ICM20789: startup OK\n");

    // use 10ms to ensure we don't lose samples, with max lag of 10ms
    dev->register_periodic_callback(CONVERSION_INTERVAL/2, FUNCTOR_BIND_MEMBER(&AG_Baro_ICM20789::timer, void));

    return true;

 failed:
    dev->get_semaphore()->give();
    return false;
}

bool AG_Baro_ICM20789::send_cmd16(uint16_t cmd)
{
    uint8_t cmd_b[2] = { uint8_t(cmd >> 8), uint8_t(cmd & 0xFF) };
    return dev->transfer(cmd_b, 2, nullptr, 0);
}

bool AG_Baro_ICM20789::read_calibration_data(void)
{
    // setup for OTP read
    const uint8_t cmd[5] = { 0xC5, 0x95, 0x00, 0x66, 0x9C };
    if (!dev->transfer(cmd, sizeof(cmd), nullptr, 0)) {
        debug("ICM20789: read cal1 failed\n");
        return false;
    }
    for (uint8_t i=0; i<4; i++) {
        if (!send_cmd16(0xC7F7)) {
            debug("ICM20789: read cal2[%u] failed\n", i);
            return false;
        }
        uint8_t d[3];
        if (!dev->transfer(nullptr, 0, d, sizeof(d))) {
            debug("ICM20789: read cal3[%u] failed\n", i);
            return false;
        }
        sensor_constants[i] = int16_t((d[0]<<8) | d[1]);
        debug("sensor_constants[%u]=%d\n", i, sensor_constants[i]);
    }
    return true;
}

void AG_Baro_ICM20789::calculate_conversion_constants(const float p_Pa[3], const float p_LUT[3],
                                                      float &A, float &B, float &C)
{
    C = (p_LUT[0] * p_LUT[1] * (p_Pa[0] - p_Pa[1]) +
         p_LUT[1] * p_LUT[2] * (p_Pa[1] - p_Pa[2]) +
         p_LUT[2] * p_LUT[0] * (p_Pa[2] - p_Pa[0])) /
        (p_LUT[2] * (p_Pa[0] - p_Pa[1]) +
         p_LUT[0] * (p_Pa[1] - p_Pa[2]) +
         p_LUT[1] * (p_Pa[2] - p_Pa[0]));
    A = (p_Pa[0] * p_LUT[0] - p_Pa[1] * p_LUT[1] - (p_Pa[1] - p_Pa[0]) * C) / (p_LUT[0] - p_LUT[1]);
    B = (p_Pa[0] - A) * (p_LUT[0] + C);
}

/*
  Convert an output from a calibrated sensor to a pressure in Pa.
  Arguments:
  p_LSB -- Raw pressure data from sensor
  T_LSB -- Raw temperature data from sensor
*/
float AG_Baro_ICM20789::get_pressure(uint32_t p_LSB, uint32_t T_LSB)
{
    float t = T_LSB - 32768.0;
    float s[3];
    s[0] = LUT_lower + float(sensor_constants[0] * t * t) * quadr_factor;
    s[1] = offst_factor * sensor_constants[3] + float(sensor_constants[1] * t * t) * quadr_factor;
    s[2] = LUT_upper + float(sensor_constants[2] * t * t) * quadr_factor;
    float A, B, C;
    calculate_conversion_constants(p_Pa_calib, s, A, B, C);
    return A + B / (C + p_LSB);
}


#if BARO_ICM20789_DEBUG
static struct {
    uint32_t Praw, Traw;
    float T, P;
} dd;
#endif

void AG_Baro_ICM20789::convert_data(uint32_t Praw, uint32_t Traw)
{
    // temperature is easy
    float T = -45 + (175.0f / (1U<<16)) * Traw;

    // pressure involves a few more calculations
    float P = get_pressure(Praw, Traw);

    if (!pressure_ok(P)) {
        return;
    }

    WITH_SEMAPHORE(_sem);
    
#if BARO_ICM20789_DEBUG
    dd.Praw = Praw;
    dd.Traw = Traw;
    dd.P = P;
    dd.T = T;
#endif
        
    accum.psum += P;
    accum.tsum += T;
    accum.count++;
}

void AG_Baro_ICM20789::timer(void)
{
    uint8_t d[9] {};
    if (dev->transfer(nullptr, 0, d, sizeof(d))) {
        // ignore CRC bytes for now
        uint32_t Praw = (uint32_t(d[0]) << 16) | (uint32_t(d[1]) << 8) | d[3];
        uint32_t Traw = (uint32_t(d[6]) << 8) | d[7];

        convert_data(Praw, Traw);
        send_cmd16(CMD_READ_PT);
        last_measure_us = AG_HAL::micros();
    } else {
        if (AG_HAL::micros() - last_measure_us > CONVERSION_INTERVAL*3) {
            // lost a sample
            send_cmd16(CMD_READ_PT);
            last_measure_us = AG_HAL::micros();
        }
    }
}

void AG_Baro_ICM20789::update()
{
#if BARO_ICM20789_DEBUG
    // useful for debugging
// @LoggerMessage: ICMB
// @Description: ICM20789 diagnostics
// @Field: TimeUS: Time since system startup
// @Field: Traw: raw temperature from sensor
// @Field: Praw: raw pressure from sensor
// @Field: P: pressure
// @Field: T: temperature
    AP::logger().WriteStreaming("ICMB", "TimeUS,Traw,Praw,P,T", "QIIff",
                                           AG_HAL::micros64(),
                                           dd.Traw, dd.Praw, dd.P, dd.T);
#endif

    WITH_SEMAPHORE(_sem);
    
    if (accum.count > 0) {
        _copy_to_frontend(instance, accum.psum/accum.count, accum.tsum/accum.count);
        accum.psum = accum.tsum = 0;
        accum.count = 0;
    }
}

#endif  // AP_BARO_ICM20789_ENABLED

/*
 * Copyright (C) 2020  Peter Barker. All rights reserved.
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

#include <inttypes.h>

#include <AG_HAL/HAL.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL && !defined(HAL_BUILD_AP_PERIPH)

#include <AG_HAL/I2CDevice.h>
#include <AG_HAL/utility/OwnPtr.h>

#include "AG_HAL_SITL_Namespace.h"
#include "Semaphores.h"

class I2CBus;

class HALSITL::I2CDevice : public AG_HAL::I2CDevice {
public:
    static I2CDevice *from(AG_HAL::I2CDevice *dev) {
        return static_cast<I2CDevice*>(dev);
    }

    /* AG_HAL::I2CDevice implementation */

    I2CDevice(I2CBus &bus, uint8_t address);

    ~I2CDevice() {}

    /* See AG_HAL::I2CDevice::set_address() */
    void set_address(uint8_t address) override { _address = address; }

    /* See AG_HAL::I2CDevice::set_retries() */
    void set_retries(uint8_t retries) override { _retries = retries; }

    /* AG_HAL::Device implementation */

    /* See AG_HAL::Device::set_speed(): Empty implementation, not supported. */
    bool set_speed(enum Device::Speed speed) override { return true; }

    /* See AG_HAL::Device::transfer() */
    bool transfer(const uint8_t *send, uint32_t send_len,
                  uint8_t *recv, uint32_t recv_len) override;

    bool read_registers_multiple(uint8_t first_reg, uint8_t *recv,
                                 uint32_t recv_len, uint8_t times) override;

    /* See AG_HAL::Device::get_semaphore() */
    AG_HAL::Semaphore *get_semaphore() override;

    /* See AG_HAL::Device::register_periodic_callback() */
    AG_HAL::Device::PeriodicHandle register_periodic_callback(
        uint32_t period_usec, AG_HAL::Device::PeriodicCb) override;

    /* See AG_HAL::Device::adjust_periodic_callback() */
    bool adjust_periodic_callback(
        AG_HAL::Device::PeriodicHandle h, uint32_t period_usec) override;

    /* set split transfers flag */
    void set_split_transfers(bool set) override {
        _split_transfers = set;
    }

    static void sitl_update();

  // the following must be identical to the structure in SITL/SIM_I2C.h!
#define I2C_M_RD 1
#define I2C_RDWR 0
    struct i2c_msg {
        uint8_t bus;
        uint8_t addr;
        uint8_t flags;
        uint8_t *buf;
        uint16_t len; // FIXME: what type should this be?
    };
    struct i2c_rdwr_ioctl_data {
        i2c_msg *msgs;
        uint8_t nmsgs;
    };
    // end "the following"

protected:
    I2CBus &_bus;
    uint8_t _address;
    uint8_t _retries;
    bool _split_transfers = false;

    bool _transfer(const uint8_t *send, uint32_t send_len,
                   uint8_t *recv, uint32_t recv_len);
};

class HALSITL::I2CDeviceManager : public AG_HAL::I2CDeviceManager {
public:
    friend class I2CDevice;

    static I2CDeviceManager *from(AG_HAL::I2CDeviceManager *i2c_mgr)
    {
        return static_cast<I2CDeviceManager*>(i2c_mgr);
    }

    I2CDeviceManager();
    ~I2CDeviceManager() {};

    void _timer_tick(); // in lieu of a thread-per-bus

    /* AG_HAL::I2CDeviceManager implementation */
    AG_HAL::OwnPtr<AG_HAL::I2CDevice> get_device(uint8_t bus, uint8_t address,
                                                 uint32_t bus_clock=400000,
                                                 bool use_smbus = false,
                                                 uint32_t timeout_ms=4) override;

protected:

    #define NUM_SITL_I2C_BUSES 4
    static I2CBus buses[];
};
#endif //#if CONFIG_HAL_BOARD == HAL_BOARD_SITL && !defined(HAL_BUILD_AP_PERIPH)

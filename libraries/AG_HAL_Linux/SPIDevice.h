/*
 * Copyright (C) 2015  Intel Corporation. All rights reserved.
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
#include <vector>

#include <AG_HAL/HAL.h>
#include <AG_HAL/SPIDevice.h>

namespace Linux {

class SPIBus;
class SPIDesc;

class SPIDevice : public AG_HAL::SPIDevice {
public:
    SPIDevice(SPIBus &bus, SPIDesc &device_desc);

    virtual ~SPIDevice();

    /* AG_HAL::SPIDevice implementation */

    /* See AG_HAL::Device::set_speed() */
    bool set_speed(AG_HAL::Device::Speed speed) override;

    /* See AG_HAL::Device::transfer() */
    bool transfer(const uint8_t *send, uint32_t send_len,
                  uint8_t *recv, uint32_t recv_len) override;

    /* See AG_HAL::SPIDevice::transfer_fullduplex() */
    bool transfer_fullduplex(const uint8_t *send, uint8_t *recv,
                             uint32_t len) override;

    /* See AG_HAL::Device::get_semaphore() */
    AG_HAL::Semaphore *get_semaphore() override;

    /* See AG_HAL::Device::register_periodic_callback() */
    AG_HAL::Device::PeriodicHandle register_periodic_callback(
        uint32_t period_usec, AG_HAL::Device::PeriodicCb) override;

    /* See AG_HAL::Device::adjust_periodic_callback() */
    bool adjust_periodic_callback(
        AG_HAL::Device::PeriodicHandle h, uint32_t period_usec) override;

protected:
    SPIBus &_bus;
    SPIDesc &_desc;
    AG_HAL::DigitalSource *_cs;
    uint32_t _speed;

    /*
     * Select device if using userspace CS
     */
    void _cs_assert();

    /*
     * Deselect device if using userspace CS
     */
    void _cs_release();
};

class SPIDeviceManager : public AG_HAL::SPIDeviceManager {
public:
    friend class SPIDevice;

    static SPIDeviceManager *from(AG_HAL::SPIDeviceManager *spi_mgr)
    {
        return static_cast<SPIDeviceManager*>(spi_mgr);
    }

    SPIDeviceManager()
    {
        /* Reserve space up-front for 3 buses */
        _buses.reserve(3);
    }

    /* AG_HAL::SPIDeviceManager implementation */
    AG_HAL::OwnPtr<AG_HAL::SPIDevice> get_device(const char *name) override;

    /*
     * Stop all SPI threads and block until they are finalized. This doesn't
     * free memory because they can still be used by devices, however device
     * drivers won't receive any new event
     */
    void teardown();

    /* See AG_HAL::SPIDeviceManager::get_count() */
    uint8_t get_count() override;

    /* See AG_HAL::SPIDeviceManager::get_device_name() */
    const char *get_device_name(uint8_t idx) override;

protected:
    void _unregister(SPIBus &b);
    AG_HAL::OwnPtr<AG_HAL::SPIDevice> _create_device(SPIBus &b, SPIDesc &device_desc) const;

    std::vector<SPIBus*> _buses;

    static const uint8_t _n_device_desc;
    static SPIDesc _device[];
};

}

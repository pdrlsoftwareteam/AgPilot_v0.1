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

#include <inttypes.h>
#include <AG_HAL/HAL.h>
#include "Semaphores.h"
#include "AG_HAL_ESP32.h"
#include "Scheduler.h"


namespace ESP32
{

class DeviceBus
{
public:
    DeviceBus(uint8_t _thread_priority);

    struct DeviceBus *next;
    Semaphore semaphore;

    AG_HAL::Device::PeriodicHandle register_periodic_callback(uint32_t period_usec, AG_HAL::Device::PeriodicCb, AG_HAL::Device *hal_device);
    bool adjust_timer(AG_HAL::Device::PeriodicHandle h, uint32_t period_usec);
    static void bus_thread(void *arg);

private:
    struct callback_info {
        struct callback_info *next;
        AG_HAL::Device::PeriodicCb cb;
        uint32_t period_usec;
        uint64_t next_usec;
    } *callbacks;
    uint8_t thread_priority;
    void* bus_thread_handle;
    bool thread_started;
    AG_HAL::Device *hal_device;
};

}



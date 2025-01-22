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

#pragma once

#include "AG_EFI_config.h"

#if HAL_EFI_ENABLED

#include <AG_Common/AG_Common.h>
#include <AG_Param/AG_Param.h>
#include <GCS_MAVLink/GCS_MAVLink.h>

#include "AG_EFI_Backend.h"
#include "AG_EFI_State.h"


/*
 * This library aims to read data from Electronic Fuel Injection 
 * or Engine Control units. It is focused around the generic
 * internal combustion engine state message provided by the 
 * UAVCAN protocol due to its comprehensiveness, but is extensible
 * to use other forms of data transfer besides UAVCAN. 
 * 
 *
 *
 * Authors: Sriram Sami and David Ingraham
 * With direction from Andrew Tridgell, Robert Lefebvre, Francisco Ferreira and
 * Pavel Kirienko.
 * Thanks to Yonah, SpektreWorks Inc, and HFE International.
 */

class AG_EFI {
public:
    friend class AG_EFI_Backend;

    // For parameter initialization
    AG_EFI();

    // Initializes backend
    void init(void);

    // Requests backend to update the frontend. Should be called at 10Hz.
    void update();
    
    // Returns the RPM
    uint32_t get_rpm() const { return state.engine_speed_rpm; }

    // returns enabled state of EFI
    bool enabled() const { return type != Type::NONE; }

    bool is_healthy() const;

    // return timestamp of last update
    uint32_t get_last_update_ms(void) const {
        return state.last_updated_ms;
    }

    // get a copy of state structure
    void get_state(EFI_State &state);

    // Parameter info
    static const struct AG_Param::GroupInfo var_info[];

    // Backend driver types
    enum class Type : uint8_t {
        NONE       = 0,
        MegaSquirt = 1,
        NWPMU      = 2,
        Lutan      = 3,
        // LOWEHEISER = 4,
        DroneCAN = 5,
        CurrawongECU = 6,
        SCRIPTING  = 7,
    };

    static AG_EFI *get_singleton(void) {
        return singleton;
    }

    // send EFI_STATUS
    void send_mavlink_status(mavlink_channel_t chan);

#if AP_SCRIPTING_ENABLED
    AG_EFI_Backend* get_backend(uint8_t idx) { return idx==0?backend:nullptr; }
#endif

protected:

    // Back end Parameters
    AP_Float coef1;
    AP_Float coef2;

    AP_Float ecu_fuel_density;

    EFI_State state;

private:
    // Front End Parameters
    AP_Enum<Type> type;

    // Tracking backends
    AG_EFI_Backend *backend;
    static AG_EFI *singleton;

    // Semaphore for access to shared frontend data
    HAL_Semaphore sem;

    // write to log
    void log_status();
};

namespace AP {
    AG_EFI *EFI();
};

#endif // HAL_EFI_ENABLED

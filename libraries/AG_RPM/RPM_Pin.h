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

#include "AG_RPM.h"

#include "RPM_Backend.h"

#if AG_RPM_PIN_ENABLED

#include <Filter/Filter.h>
#include <AG_Math/AG_Math.h>

class AG_RPM_Pin : public AG_RPM_Backend
{
public:
    // constructor
    AG_RPM_Pin(AG_RPM &ranger, uint8_t instance, AG_RPM::RPM_State &_state);

    // update state
    void update(void) override;

private:

    ModeFilterFloat_Size5 signal_quality_filter {3};
    int8_t last_pin = -1;       // last pin number checked vs PIN parameter
    bool interrupt_attached;    // true if an interrupt has been attached to last_pin
    struct IrqState {
        uint32_t last_pulse_us;
        uint32_t dt_sum;
        uint32_t dt_count;
    };
    static struct IrqState irq_state[RPM_MAX_INSTANCES];

    void irq_handler(uint8_t pin,
                     bool pin_state,
                     uint32_t timestamp);

};

#endif  // AG_RPM_PIN_ENABLED

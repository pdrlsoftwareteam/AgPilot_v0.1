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

#include <AG_Common/AG_Common.h>
#include "AG_WheelEncoder.h"

class AG_WheelEncoder_Backend
{
public:
    // constructor. This incorporates initialisation as well.
    AG_WheelEncoder_Backend(AG_WheelEncoder &frontend, uint8_t instance, AG_WheelEncoder::WheelEncoder_State &state);

    // we declare a virtual destructor so that WheelEncoder drivers can
    // override with a custom destructor if need be
    virtual ~AG_WheelEncoder_Backend(void) {}

    // update the state structure. All backends must implement this.
    virtual void update() = 0;

protected:

    // return pin number.  returns -1 if pin is not defined for this instance
    int8_t get_pin_a() const;
    int8_t get_pin_b() const;

    // copy state to front end helper function
    void copy_state_to_frontend(int32_t distance_count, uint32_t total_count, uint32_t error_count, uint32_t last_reading_ms);

    AG_WheelEncoder &_frontend;
    AG_WheelEncoder::WheelEncoder_State &_state;
};

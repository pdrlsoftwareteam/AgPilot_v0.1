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

#include "AG_ADSB.h"

#if HAL_ADSB_ENABLED
class AG_ADSB_Backend
{
public:
    // constructor.
    AG_ADSB_Backend(AG_ADSB &frontend, uint8_t instance);

    // we declare a virtual destructor so that ADSB drivers can
    // override with a custom destructor if need be
    virtual ~AG_ADSB_Backend(void) {}

    // static detection function
    static bool detect();

    virtual void update() = 0;

    virtual bool init() { return true; }

protected:

    uint8_t _instance;

    AG_HAL::UARTDriver *_port;

    // references
    AG_ADSB &_frontend;
};
#endif // HAL_ADSB_ENABLED

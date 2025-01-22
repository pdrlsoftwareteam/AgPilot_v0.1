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
 *   analog airspeed driver
 */

#include "AG_Airspeed_analog.h"

#if AP_AIRSPEED_ANALOG_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_Common/AG_Common.h>

#include "AG_Airspeed.h"

extern const AG_HAL::HAL &hal;

// scaling for 3DR analog airspeed sensor
#define VOLTS_TO_PASCAL 819

AG_Airspeed_Analog::AG_Airspeed_Analog(AG_Airspeed &_frontend, uint8_t _instance) :
    AG_Airspeed_Backend(_frontend, _instance)
{
    _source = hal.analogin->channel(get_pin());
}

bool AG_Airspeed_Analog::init()
{
    return _source != nullptr;
}

// read the airspeed sensor
bool AG_Airspeed_Analog::get_differential_pressure(float &pressure)
{
    // allow pin to change
    if (_source == nullptr || !_source->set_pin(get_pin())) {
        return false;
    }
    pressure = _source->voltage_average_ratiometric() * VOLTS_TO_PASCAL / get_psi_range();
    return true;
}

#endif  // AP_AIRSPEED_ANALOG_ENABLED

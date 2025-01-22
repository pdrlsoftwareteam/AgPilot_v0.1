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

#include "AG_TemperatureSensor.h"

#if AP_TEMPERATURE_SENSOR_ENABLED
#include <AG_HAL/Semaphores.h>
#include <AG_ESC_Telem/AG_ESC_Telem.h>

class AG_TemperatureSensor_Backend
#if HAL_WITH_ESC_TELEM
 : public AG_ESC_Telem_Backend
#endif
{
public:
    // constructor. This incorporates initialisation as well.
    AG_TemperatureSensor_Backend(AG_TemperatureSensor &front, AG_TemperatureSensor::TemperatureSensor_State &state, AG_TemperatureSensor_Params &params);

    // initialise
    virtual void init() {};

    // update the latest temperature
    virtual void update() = 0;

    // do we have a valid temperature reading?
    virtual bool healthy(void) const;

    // logging functions
    void Log_Write_TEMP() const;

    AG_HAL::OwnPtr<AG_HAL::Device> _dev;


protected:

    void set_temperature(const float temperature);
    void update_external_libraries(const float temperature);

    AG_TemperatureSensor                            &_front;    // reference to front-end
    AG_TemperatureSensor::TemperatureSensor_State   &_state;    // reference to this instance's state (held in the front-end)
    AG_TemperatureSensor_Params                     &_params;   // reference to this instance's parameters (held in the front-end)

private:
    HAL_Semaphore _sem; // used to copy from backend to frontend
};

#endif // AP_TEMPERATURE_SENSOR_ENABLED

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

#include "AG_Proximity_Backend_Serial.h"

#if HAL_PROXIMITY_ENABLED
#include <AG_SerialManager/AG_SerialManager.h>

/*
   The constructor also initialises the proximity sensor. Note that this
   constructor is not called until detect() returns true, so we
   already know that we should setup the proximity sensor
*/
AG_Proximity_Backend_Serial::AG_Proximity_Backend_Serial(AG_Proximity &_frontend,
                                                         AG_Proximity::Proximity_State &_state,
                                                         AG_Proximity_Params &_params,
                                                         uint8_t serial_instance) :
    AG_Proximity_Backend(_frontend, _state, _params)
{
    const AG_SerialManager &serial_manager = AP::serialmanager();
    _uart = serial_manager.find_serial(AG_SerialManager::SerialProtocol_Lidar360, serial_instance);
    if (_uart != nullptr) {
        // start uart with larger receive buffer
        _uart->begin(serial_manager.find_baudrate(AG_SerialManager::SerialProtocol_Lidar360, serial_instance), rxspace(), 0);
    }
}

// static detection function
// detect if a proximity sensor is connected by looking for a configured serial port
// serial_instance affects which serial port is used.  Should be 0 or 1 depending on whether this is the 1st or 2nd proximity sensor with a serial interface
bool AG_Proximity_Backend_Serial::detect(uint8_t serial_instance)
{
    return AP::serialmanager().have_serial(AG_SerialManager::SerialProtocol_Lidar360, serial_instance);
}

#endif // HAL_PROXIMITY_ENABLED

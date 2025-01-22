/*

   Inspired by work done here
   https://github.com/PX4/Firmware/tree/master/src/drivers/frsky_telemetry from Stefan Rado <px4@sradonia.net>
   https://github.com/opentx/opentx/tree/2.3/radio/src/telemetry from the OpenTX team

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
   FRSKY Telemetry library
*/

#include "AG_Frsky_config.h"

#if AP_FRSKY_TELEM_ENABLED

#include "AG_Frsky_Telem.h"
#include "AG_Frsky_Parameters.h"

#include <AG_SerialManager/AG_SerialManager.h>

#include <AG_Vehicle/AG_Vehicle.h>

#include "AG_Frsky_D.h"
#include "AG_Frsky_SPort.h"
#include "AG_Frsky_SPort_Passthrough.h"

extern const AG_HAL::HAL& hal;

AG_Frsky_Telem *AG_Frsky_Telem::singleton;

AG_Frsky_Telem::AG_Frsky_Telem()
{
    singleton = this;
#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
    _frsky_parameters = &AP::vehicle()->frsky_parameters;
#endif //HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
}


AG_Frsky_Telem::~AG_Frsky_Telem(void)
{
    singleton = nullptr;
}

/*
 * init - perform required initialisation
 */
bool AG_Frsky_Telem::init(bool use_external_data)
{
    const AG_SerialManager &serial_manager = AP::serialmanager();

    // check for protocol configured for a serial port - only the first serial port with one of these protocols will then run (cannot have FrSky on multiple serial ports)
    AG_HAL::UARTDriver *port;
    if ((port = serial_manager.find_serial(AG_SerialManager::SerialProtocol_FrSky_D, 0))) {
#if AP_FRSKY_D_TELEM_ENABLED
        _backend = new AG_Frsky_D(port);
#endif
    } else if ((port = serial_manager.find_serial(AG_SerialManager::SerialProtocol_FrSky_SPort, 0))) {
#if AP_FRSKY_SPORT_TELEM_ENABLED
        _backend = new AG_Frsky_SPort(port);
#endif
    } else if (use_external_data || (port = serial_manager.find_serial(AG_SerialManager::SerialProtocol_FrSky_SPort_Passthrough, 0))) {
#if AP_FRSKY_SPORT_PASSTHROUGH_ENABLED
        _backend = new AG_Frsky_SPort_Passthrough(port, use_external_data, _frsky_parameters);
#endif
    }

    if (_backend == nullptr) {
        return false;
    }

    if (!_backend->init()) {
        delete _backend;
        _backend = nullptr;
        return false;
    }

    return true;
}

bool AG_Frsky_Telem::_get_telem_data(AG_Frsky_Backend::sport_packet_t* packet_array, uint8_t &packet_count, const uint8_t max_size)
{
    if (_backend == nullptr) {
        return false;
    }
    if (packet_array == nullptr) {
        return false;
    }
    return _backend->get_telem_data(packet_array, packet_count, max_size);
}

#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
bool AG_Frsky_Telem::_set_telem_data(uint8_t frame, uint16_t appid, uint32_t data)
{
    if (_backend == nullptr) {
        return false;
    }
    return _backend->set_telem_data(frame, appid, data);
}
#endif

void AG_Frsky_Telem::try_create_singleton_for_external_data()
{
    // try to allocate an AG_Frsky_Telem object only if we are disarmed
    if (!singleton && !hal.util->get_soft_armed()) {
        new AG_Frsky_Telem();
        // initialize the passthrough scheduler
        if (singleton) {
            singleton->init(true);
        }
    }
}

/*
  fetch Sport data for an external transport, such as FPort
 */
bool AG_Frsky_Telem::get_telem_data(AG_Frsky_Backend::sport_packet_t* packet_array, uint8_t &packet_count, const uint8_t max_size)
{
    try_create_singleton_for_external_data();
    if (singleton == nullptr) {
        return false;
    }
    return singleton->_get_telem_data(packet_array, packet_count, max_size);
}

#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
/*
  allow external transports (e.g. FPort), to supply telemetry data
 */
bool AG_Frsky_Telem::set_telem_data(const uint8_t frame, const uint16_t appid, const uint32_t data)
{
    try_create_singleton_for_external_data();
    if (singleton == nullptr) {
        return false;
    }
    return singleton->_set_telem_data(frame, appid, data);
}
#endif

namespace AP
{
AG_Frsky_Telem *frsky_telem()
{
    return AG_Frsky_Telem::get_singleton();
}
};

#endif  // AP_FRSKY_TELEM_ENABLED

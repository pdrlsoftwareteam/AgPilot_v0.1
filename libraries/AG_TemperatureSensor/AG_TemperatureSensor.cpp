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

#include "AG_TemperatureSensor.h"

#if AP_TEMPERATURE_SENSOR_ENABLED

#include <AG_Vehicle/AG_Vehicle_Type.h>

#define AP_TEMPERATURE_SENSOR_DUMMY_METHODS_ENABLED (!(APM_BUILD_TYPE(APM_BUILD_ArduSub) || (AP_TEMPERATURE_SENSOR_ENABLED == 1)))


#if !AP_TEMPERATURE_SENSOR_DUMMY_METHODS_ENABLED

#include "AG_TemperatureSensor_TSYS01.h"
#include "AG_TemperatureSensor_MCP9600.h"
#include "AG_TemperatureSensor_MAX31865.h"

#include <AG_Logger/AG_Logger.h>
#include <AG_Vehicle/AG_Vehicle_Type.h>

extern const AG_HAL::HAL& hal;

const AG_Param::GroupInfo AG_TemperatureSensor::var_info[] = {

    // SKIP INDEX 0

    // @Param: _LOG
    // @DisplayName: Logging
    // @Description: Enables temperature sensor logging
    // @Values: 0:Disabled, 1:Enabled
    // @User: Standard
    AP_GROUPINFO("_LOG", 1, AG_TemperatureSensor, _log_flag, 0),

    // SKIP Index 2-9 to be for parameters that apply to every sensor

    // @Group: 1_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[0], "1_", 10, AG_TemperatureSensor, AG_TemperatureSensor_Params),

#if AP_TEMPERATURE_SENSOR_MAX_INSTANCES >= 2
    // @Group: 2_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[1], "2_", 11, AG_TemperatureSensor, AG_TemperatureSensor_Params),
#endif

#if AP_TEMPERATURE_SENSOR_MAX_INSTANCES >= 3
    // @Group: 3_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[2], "3_", 12, AG_TemperatureSensor, AG_TemperatureSensor_Params),
#endif

#if AP_TEMPERATURE_SENSOR_MAX_INSTANCES >= 4
    // @Group: 4_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[3], "4_", 13, AG_TemperatureSensor, AG_TemperatureSensor_Params),
#endif

#if AP_TEMPERATURE_SENSOR_MAX_INSTANCES >= 5
    // @Group: 5_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[4], "5_", 14, AG_TemperatureSensor, AG_TemperatureSensor_Params),
#endif
#if AP_TEMPERATURE_SENSOR_MAX_INSTANCES >= 6
    // @Group: 6_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[5], "6_", 15, AG_TemperatureSensor, AG_TemperatureSensor_Params),
#endif
#if AP_TEMPERATURE_SENSOR_MAX_INSTANCES >= 7
    // @Group: 7_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[6], "7_", 16, AG_TemperatureSensor, AG_TemperatureSensor_Params),
#endif
#if AP_TEMPERATURE_SENSOR_MAX_INSTANCES >= 8
    // @Group: 8_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[7], "8_", 17, AG_TemperatureSensor, AG_TemperatureSensor_Params),
#endif
#if AP_TEMPERATURE_SENSOR_MAX_INSTANCES >= 9
    // @Group: 9_
    // @Path: AG_TemperatureSensor_Params.cpp
    AP_SUBGROUPINFO(_params[8], "9_", 18, AG_TemperatureSensor, AG_TemperatureSensor_Params),
#endif

    AP_GROUPEND
};

// Default Constructor
AG_TemperatureSensor::AG_TemperatureSensor()
{
    AG_Param::setup_object_defaults(this, var_info);

    if (_singleton != nullptr) {
        AG_HAL::panic("AG_TemperatureSensor must be singleton");
    }
    _singleton = this;
}

// init - instantiate the temperature sensors
void AG_TemperatureSensor::init()
{
    // check init has not been called before
    if (_num_instances != 0) {
        return;
    }

 // For Sub set the Default: Type to TSYS01 and I2C_ADDR of 0x77
#if APM_BUILD_TYPE(APM_BUILD_ArduSub) && AP_TEMPERATURE_SENSOR_TSYS01_ENABLED
    AG_Param::set_default_by_name("TEMP1_TYPE", (float)AG_TemperatureSensor_Params::Type::TSYS01);
    AG_Param::set_default_by_name("TEMP1_ADDR", TSYS01_ADDR_CSB0);
#endif

    // create each instance
    for (uint8_t instance = 0; instance < AP_TEMPERATURE_SENSOR_MAX_INSTANCES; instance++) {

        switch (get_type(instance)) {
#if AP_TEMPERATURE_SENSOR_TSYS01_ENABLED
            case AG_TemperatureSensor_Params::Type::TSYS01:
                drivers[instance] = new AG_TemperatureSensor_TSYS01(*this, _state[instance], _params[instance]);
                break;
#endif
#if AP_TEMPERATURE_SENSOR_MCP9600_ENABLED
            case AG_TemperatureSensor_Params::Type::MCP9600:
                drivers[instance] = new AG_TemperatureSensor_MCP9600(*this, _state[instance], _params[instance]);
                break;
#endif
#if AP_TEMPERATURE_SENSOR_MAX31865_ENABLED
            case AG_TemperatureSensor_Params::Type::MAX31865:
                drivers[instance] = new AG_TemperatureSensor_MAX31865(*this, _state[instance], _params[instance]);
                break;
#endif
            case AG_TemperatureSensor_Params::Type::NONE:
            default:
                break;
        }

        // call init function for each backend
        if (drivers[instance] != nullptr) {
            _state[instance].instance = instance;
            drivers[instance]->init();
            // _num_instances is actually the index for looping over instances
            // the user may have TEMP_TYPE=0 and TEMP2_TYPE=7, in which case
            // there will be a gap, but as we always check for drivers[instances] being nullptr
            // this is safe
            _num_instances = instance + 1;
        }
    }
    
    if (_num_instances > 0) {
        // param count could have changed
        AG_Param::invalidate_count();
    }
}

// update: - For all active instances update temperature and log TEMP
void AG_TemperatureSensor::update()
{
    for (uint8_t i=0; i<_num_instances; i++) {
        if (drivers[i] != nullptr && get_type(i) != AG_TemperatureSensor_Params::Type::NONE) {
            drivers[i]->update();

#if HAL_LOGGING_ENABLED
            const AG_Logger *logger = AG_Logger::get_singleton();
            if (logger != nullptr && _log_flag) {
                drivers[i]->Log_Write_TEMP();
            }
#endif
        }
    }
}

AG_TemperatureSensor_Params::Type AG_TemperatureSensor::get_type(const uint8_t instance) const
{
    if (instance >= AP_TEMPERATURE_SENSOR_MAX_INSTANCES) {
        return AG_TemperatureSensor_Params::Type::NONE;
    }
    return (AG_TemperatureSensor_Params::Type)_params[instance].type.get();
}

// returns true if there is a temperature reading
bool AG_TemperatureSensor::get_temperature(float &temp, const uint8_t instance) const
{
    if (!healthy(instance)) {
        return false;
    }

    temp = _state[instance].temperature;
    return true;
}

bool AG_TemperatureSensor::healthy(const uint8_t instance) const
{
    return instance < _num_instances && drivers[instance] != nullptr && drivers[instance]->healthy();
}

AG_TemperatureSensor_Params::Source AG_TemperatureSensor::get_source(const uint8_t instance) const
{
    return healthy(instance) ? (AG_TemperatureSensor_Params::Source)_params[instance].source.get() : AG_TemperatureSensor_Params::Source::None;
}

int32_t AG_TemperatureSensor::get_source_id(const uint8_t instance) const
{
    return healthy(instance) ? _params[instance].source_id.get() : 0;
}
#else
const AG_Param::GroupInfo AG_TemperatureSensor::var_info[] = { AP_GROUPEND };

void AG_TemperatureSensor::init() { };
void AG_TemperatureSensor::update() { };
bool AG_TemperatureSensor::get_temperature(float &temp, const uint8_t instance) const { return false; };
bool AG_TemperatureSensor::healthy(const uint8_t instance) const { return false; };
AG_TemperatureSensor_Params::Type AG_TemperatureSensor::get_type(const uint8_t instance) const { return AG_TemperatureSensor_Params::Type::NONE; };
AG_TemperatureSensor_Params::Source AG_TemperatureSensor::get_source(const uint8_t instance) const { return AG_TemperatureSensor_Params::Source::None; };
int32_t AG_TemperatureSensor::get_source_id(const uint8_t instance) const { return false; };

AG_TemperatureSensor::AG_TemperatureSensor() {}

#endif // AP_TEMPERATURE_SENSOR_DUMMY_METHODS_ENABLED

AG_TemperatureSensor *AG_TemperatureSensor::_singleton;

namespace AP {
AG_TemperatureSensor &temperature_sensor() {
    return *AG_TemperatureSensor::get_singleton();
}
};

#endif // AP_TEMPERATURE_SENSOR_ENABLED

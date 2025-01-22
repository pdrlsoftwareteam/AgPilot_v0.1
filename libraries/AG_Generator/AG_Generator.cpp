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

#include "AG_Generator.h"

#if HAL_GENERATOR_ENABLED

#include "AG_Generator_IE_650_800.h"
#include "AG_Generator_IE_2400.h"
#include "AG_Generator_RichenPower.h"

#include <GCS_MAVLink/GCS.h>

const AG_Param::GroupInfo AG_Generator::var_info[] = {

    // @Param: TYPE
    // @DisplayName: Generator type
    // @Description: Generator type
    // @Values: 0:Disabled, 1:IE 650w 800w Fuel Cell, 2:IE 2.4kW Fuel Cell, 3: Richenpower
    // @User: Standard
    // @RebootRequired: True
    AP_GROUPINFO_FLAGS("TYPE", 1, AG_Generator, _type, 0, AP_PARAM_FLAG_ENABLE),

    // @Param: OPTIONS
    // @DisplayName: Generator Options
    // @Description: Bitmask of options for generators
    // @Bitmask: 0:Supress Maintenance-Required Warnings
    // @User: Standard
    AP_GROUPINFO("OPTIONS", 2, AG_Generator, _options, 0),

    AP_GROUPEND
};

// Constructor
AG_Generator::AG_Generator()
{
    AG_Param::setup_object_defaults(this, var_info);

    if (_singleton) {
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
        AG_HAL::panic("Too many generators");
#endif
        return;
    }
    _singleton = this;
}

void AG_Generator::init()
{
    // Select backend
    switch (type()) {
        case Type::GEN_DISABLED:
            // Not using a generator
            return;

#if AP_GENERATOR_IE650_800_ENABLED
        case Type::IE_650_800:
            _driver_ptr = new AG_Generator_IE_650_800(*this);
            break;
#endif

#if AP_GENERATOR_IE2400_ENABLED
        case Type::IE_2400:
            _driver_ptr = new AG_Generator_IE_2400(*this);
            break;
#endif

#if AP_GENERATOR_RICHENPOWER_ENABLED
        case Type::RICHENPOWER:
            _driver_ptr = new AG_Generator_RichenPower(*this);
            break;
#endif
    }

    if (_driver_ptr != nullptr) {
        _driver_ptr->init();
    }
}

void AG_Generator::update()
{
    // Return immediatly if not enabled. Don't support run-time disabling of generator
    if (_driver_ptr == nullptr) {
        return;
    }

    // Calling backend update will cause backend to update the front end variables
    _driver_ptr->update();
}

// Helper to get param and cast to Type
enum AG_Generator::Type AG_Generator::type() const
{
    return (Type)_type.get();
}

// Pass through to backend
void AG_Generator::send_generator_status(const GCS_MAVLINK &channel)
{
    if (_driver_ptr == nullptr) {
        return;
    }
    _driver_ptr->send_generator_status(channel);
}

// Tell backend to perform arming checks
bool AG_Generator::pre_arm_check(char* failmsg, uint8_t failmsg_len) const
{
    if (type() == Type::GEN_DISABLED) {
        // Don't prevent arming if generator is not enabled and has never been init
        if (_driver_ptr == nullptr) {
            return true;
        }
        // Don't allow arming if we have disabled the generator since boot
        strncpy(failmsg, "Generator disabled, reboot reqired", failmsg_len);
        return false;
    }
    if (_driver_ptr == nullptr) {
        strncpy(failmsg, "No backend driver", failmsg_len);
        return false;
    }
    return _driver_ptr->pre_arm_check(failmsg, failmsg_len);
}

// Tell backend check failsafes
AG_BattMonitor::Failsafe AG_Generator::update_failsafes()
{
    // Don't invoke a failsafe if driver not assigned
    if (_driver_ptr == nullptr) {
        return AG_BattMonitor::Failsafe::None;
    }
    return _driver_ptr->update_failsafes();
}

// Pass through to backend
bool AG_Generator::stop()
{
    // Still allow 
    if (_driver_ptr == nullptr) {
        return false;
    }
    return _driver_ptr->stop();
}

// Pass through to backend
bool AG_Generator::idle()
{
    if (_driver_ptr == nullptr) {
        return false;
    }
    return _driver_ptr->idle();
}

// Pass through to backend
bool AG_Generator::run()
{
    // Don't allow operators to request generator be set to run if it has been disabled
    if (_driver_ptr == nullptr) {
        return false;
    }
    return _driver_ptr->run();
}

// Get the AG_Generator singleton
AG_Generator *AG_Generator::get_singleton()
{
    return _singleton;
}

AG_Generator *AG_Generator::_singleton = nullptr;

namespace AP {
    AG_Generator *generator()
    {
        return AG_Generator::get_singleton();
    }
};
#endif

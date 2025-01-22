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

#include "AG_Winch_config.h"

#if AP_WINCH_ENABLED

#include <AG_Common/AG_Common.h>
#include <AG_Math/AG_Math.h>
#include <AG_Param/AG_Param.h>
#include <AG_PID/AG_PID.h>

class AG_Winch_Backend;

class AG_Winch {
    friend class AG_Winch_Backend;
    friend class AG_Winch_PWM;
    friend class AG_Winch_Daiwa;

public:
    AG_Winch();

    // Do not allow copies
    CLASS_NO_COPY(AG_Winch);

    // indicate whether this module is enabled
    bool enabled() const;

    // true if winch is healthy
    bool healthy() const;

    // initialise the winch
    void init();

    // update the winch
    void update();

    // relax the winch so it does not attempt to maintain length or rate
    void relax() { config.control_mode = ControlMode::RELAXED; }

    // release specified length of cable (in meters)
    void release_length(float length);

    // deploy line at specified speed in m/s (+ve deploys line, -ve retracts line, 0 stops)
    void set_desired_rate(float rate);

    // get rate maximum in m/s
    float get_rate_max() const { return MAX(config.rate_max, 0.0f); }

    // send status to ground station
    void send_status(const class GCS_MAVLINK &channel);

    // write log
    void write_log();

    // returns true if pre arm checks have passed
    bool pre_arm_check(char *failmsg, uint8_t failmsg_len) const;

    static AG_Winch *get_singleton();

    static const struct AG_Param::GroupInfo        var_info[];

private:

    enum class WinchType {
        NONE = 0,
        PWM = 1,
        DAIWA = 2
    };

    // winch states
    enum class ControlMode : uint8_t {
        RELAXED = 0,    // winch is realxed
        POSITION,       // moving or maintaining a target length (from an external source)
        RATE,           // extending or retracting at a target rate (from an external source)
        RATE_FROM_RC    // extending or retracting at a target rate (from RC input)
    };

    struct Backend_Config {
        AP_Int8     type;               // winch type
        AP_Float    rate_max;           // deploy or retract rate maximum (in m/s).
        AP_Float    pos_p;              // position error P gain
        ControlMode control_mode;       // state of winch control (using target position or target rate)
        float       length_desired;     // target desired length (in meters)
        float       rate_desired;       // target deploy rate (in m/s, +ve = deploying, -ve = retracting)
    } config;

    AG_Winch_Backend *backend;

    static AG_Winch *_singleton;
};

namespace AP {
    AG_Winch *winch();
};

#endif  // AP_WINCH_ENABLED

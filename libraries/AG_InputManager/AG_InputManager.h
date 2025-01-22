#pragma once

/// @file   AG_InputManager.h
/// @brief  Pilot manual control input library

#include <AG_Common/AG_Common.h>
#include <AG_Param/AG_Param.h>
#include <AG_Math/AG_Math.h>

/// @class  AG_InputManager
/// @brief  Class managing the pilot's control inputs
class AG_InputManager{
public:
    AG_InputManager() {
        // setup parameter defaults
        AG_Param::setup_object_defaults(this, var_info);
    }

    /* Do not allow copies */
    CLASS_NO_COPY(AG_InputManager);

    static const struct AG_Param::GroupInfo        var_info[];
    void set_loop_rate(uint16_t loop_rate) { _loop_rate = loop_rate; }

protected:
    // internal variables
    uint16_t            _loop_rate;             // rate at which output() function is called (normally 400hz)

};

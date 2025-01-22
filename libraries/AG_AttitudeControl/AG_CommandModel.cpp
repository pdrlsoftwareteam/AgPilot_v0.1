#include "AG_CommandModel.h"
#include <AG_HAL/AG_HAL.h>

// The Commmand Model class holds parameters that shape the pilot desired angular rate input.  This class can 
// be expanded to hold the methods that shape the pilot desired input.

extern const AG_HAL::HAL& hal;

// table of user settable parameters
const AG_Param::GroupInfo AG_CommandModel::var_info[] = {

    // @Param: RATE
    // @DisplayName: Maximum Controlled Rate
    // @Description: Sets the maximum rate commanded. 
    // @Units: deg/s
    // @Range: 1 360
    // @User: Standard
    AP_GROUPINFO_FLAGS_DEFAULT_POINTER("RATE", 1, AG_CommandModel, rate, default_rate),

    // @Param: EXPO
    // @DisplayName: Controlled Expo
    // @Description: Controlled expo to allow faster rotation when stick at edges
    // @Values: 0:Disabled,0.1:Very Low,0.2:Low,0.3:Medium,0.4:High,0.5:Very High
    // @Range: -0.5 1.0
    // @User: Advanced
    AP_GROUPINFO_FLAGS_DEFAULT_POINTER("EXPO", 2, AG_CommandModel, expo, default_expo),

    // @Param: RATE_TC
    // @DisplayName: Rate control input time constant
    // @Description: Rate control input time constant.  Low numbers lead to sharper response, higher numbers to softer response
    // @Units: s
    // @Range: 0 1
    // @Increment: 0.01
    // @Values: 0.5:Very Soft, 0.2:Soft, 0.15:Medium, 0.1:Crisp, 0.05:Very Crisp
    // @User: Standard
    AP_GROUPINFO_FLAGS_DEFAULT_POINTER("RATE_TC", 3, AG_CommandModel, rate_tc, default_rate_tc),

    AP_GROUPEND
};

// Constructor
AG_CommandModel::AG_CommandModel(float initial_rate, float initial_expo, float initial_tc) :
    default_rate(initial_rate),
    default_expo(initial_expo),
    default_rate_tc(initial_tc)
{
    AG_Param::setup_object_defaults(this, var_info);
}


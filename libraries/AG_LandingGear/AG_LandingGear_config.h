#pragma once

#include <AG_Vehicle/AG_Vehicle_Type.h>
#include <AG_HAL/AG_HAL_Boards.h>

// historical compatability; only Copters get it - and Plane on
// non-minimized boards.
#ifndef AP_LANDINGGEAR_ENABLED
#define AP_LANDINGGEAR_ENABLED (APM_BUILD_COPTER_OR_HELI || (!HAL_MINIMIZE_FEATURES && APM_BUILD_TYPE(APM_BUILD_ArduPlane)))
#endif

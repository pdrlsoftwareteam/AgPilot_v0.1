#pragma once

#include <AG_HAL/AG_HAL.h>
#include "AG_HAL_Empty_Namespace.h"

class Empty::Util : public AG_HAL::Util {
public:
    bool run_debug_shell(AG_HAL::BetterStream *stream) override { return false; }
};

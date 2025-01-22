#pragma once

#include <AG_HAL/AG_HAL.h>

#include "AG_HAL_Empty_Namespace.h"

class HAL_Empty : public AG_HAL::HAL {
public:
    HAL_Empty();
    void run(int argc, char* const* argv, Callbacks* callbacks) const override;
};

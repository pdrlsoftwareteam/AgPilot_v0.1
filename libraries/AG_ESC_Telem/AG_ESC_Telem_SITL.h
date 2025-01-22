#pragma once

#include <AG_HAL/AG_HAL.h>
#include "AG_ESC_Telem_Backend.h"
#include <SITL/SITL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL

class AG_ESC_Telem_SITL : public AG_ESC_Telem_Backend {
public:
    AG_ESC_Telem_SITL();

    void update();

protected:

private:
};

#endif

#pragma once

#include <AG_HAL/AG_HAL_Boards.h>

#ifndef HAL_NAVEKF2_AVAILABLE
// only default to EK2 enabled on boards with over 1M flash
#define HAL_NAVEKF2_AVAILABLE (BOARD_FLASH_SIZE>1024)
#endif

#ifndef HAL_NAVEKF3_AVAILABLE
#define HAL_NAVEKF3_AVAILABLE 1
#endif

#ifndef AG_AHRS_SIM_ENABLED
#define AG_AHRS_SIM_ENABLED AP_SIM_ENABLED
#endif

#pragma once

#include <AG_HAL/AG_HAL_Boards.h>

#ifndef AG_AIS_ENABLED
#if BOARD_FLASH_SIZE <= 1024
    #define AG_AIS_ENABLED 0
#else
    #define AG_AIS_ENABLED 2
#endif
#endif

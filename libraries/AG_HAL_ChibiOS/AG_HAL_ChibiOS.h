#pragma once

/* Your layer exports should depend on AG_HAL.h ONLY. */
#include <AG_HAL/AG_HAL.h>

/**
 * Umbrella header for AG_HAL_Empty module.
 * The module header exports singleton instances which must conform the
 * AG_HAL::HAL interface. It may only expose implementation details (class
 * names, headers) via the Empty namespace.
 * The class implementing AG_HAL::HAL should be called HAL_Empty and exist
 * in the global namespace. There should be a single const instance of the
 * HAL_Empty class called AG_HAL_Empty, instantiated in the HAL_Empty_Class.cpp
 * and exported as `extern const HAL_Empty AG_HAL_Empty;` in HAL_Empty_Class.h
 *
 * All declaration and compilation should be guarded by CONFIG_HAL_BOARD macros.
 * In this case, we're using CONFIG_HAL_BOARD == HAL_BOARD_EMPTY.
 * When creating a new HAL, declare a new HAL_BOARD_ in AG_HAL/AG_HAL_Boards.h
 */

#include "HAL_ChibiOS_Class.h"

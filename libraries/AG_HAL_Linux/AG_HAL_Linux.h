#pragma once

/* Your layer exports should depend on AG_HAL.h ONLY. */
#include <AG_HAL/AG_HAL.h>

/**
 * Umbrella header for AG_HAL_Linux module.
 * The module header exports singleton instances which must conform the
 * AG_HAL::HAL interface. It may only expose implementation details (class
 * names, headers) via the Linux namespace.
 * The class implementing AG_HAL::HAL should be called HAL_Linux and exist
 * in the global namespace. There should be a single const instance of the
 * HAL_Linux class called AG_HAL_Linux, instantiated in the HAL_Linux_Class.cpp
 * and exported as `extern const HAL_Linux AG_HAL_Linux;` in HAL_Linux_Class.h
 *
 * All declaration and compilation should be guarded by CONFIG_HAL_BOARD macros.
 * In this case, we're using CONFIG_HAL_BOARD == HAL_BOARD_LINUX.
 * When creating a new HAL, declare a new HAL_BOARD_ in AG_HAL/AG_HAL_Boards.h
 */

#if CONFIG_HAL_BOARD == HAL_BOARD_LINUX

#include "HAL_Linux_Class.h"

#endif // CONFIG_HAL_BOARD

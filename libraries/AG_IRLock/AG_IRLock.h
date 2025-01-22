/*
 * AG_IRLock.h
 *
 *  Created on: Nov 10, 2014
 *      Author: MLandes
 */

// @file AG_IRLock.h
// @brief Catch-all headerthat defines all supported irlock classes.

#include "IRLock.h"
#include "AG_IRLock_I2C.h"

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
#include "AG_IRLock_SITL_Gazebo.h"
#include "AG_IRLock_SITL.h"
#endif

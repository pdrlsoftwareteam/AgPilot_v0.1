/*
 * AP_HAL_PDRL.h
 *
 *  Created on: 15-Sep-2019
 *      Author: owner
 */

#ifndef LIBRARIES_AP_HAL_AP_HAL_PDRL_H_
#define LIBRARIES_AP_HAL_AP_HAL_PDRL_H_
#include "hwdef.h"

#ifdef STM32H757xx
#include "stm32h7xx_flash.h"
#include "stm32h7xx_rng.h"
#include "stm32_util.h"
#define PDRL_SPECIFIC_FLASH_SECTOR FLASH_SECTOR_23
#define PDRL_FLASH_VOLTAGE_RANGE_3 FLASH_VOLTAGE_RANGE_3
#endif

#ifdef STM32H743xx
#include "stm32h7xx_flash.h"
#include "stm32h7xx_rng.h"
#include "stm32_util.h"
#define PDRL_SPECIFIC_FLASH_SECTOR FLASH_SECTOR_23
#define PDRL_FLASH_VOLTAGE_RANGE_3 FLASH_VOLTAGE_RANGE_3
#endif

#ifdef STM32F767xx
#include "stm32f7xx_flash.h"
#include "stm32f7xx_rng.h"
#include "stm32_util.h"
#define PDRL_SPECIFIC_FLASH_SECTOR FLASH_SECTOR_23
#define PDRL_FLASH_VOLTAGE_RANGE_3 FLASH_VOLTAGE_RANGE_3
#endif

#ifdef STM32F427xx
#include "stm32f4xx_flash.h"
#include "stm32f4xx_rng.h"
#define PDRL_SPECIFIC_FLASH_SECTOR FLASH_Sector_23
#define PDRL_FLASH_VOLTAGE_RANGE_3 VoltageRange_3
#endif



#endif /* LIBRARIES_AP_HAL_AP_HAL_PDRL_H_ */

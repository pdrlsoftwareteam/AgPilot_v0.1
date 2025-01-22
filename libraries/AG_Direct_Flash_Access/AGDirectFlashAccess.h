/*
 * AGDirectFlashAccess.h
 *
 *  Created on: 02-Nov-2019
 *      Author: owner
 */

#ifndef LIBRARIES_AP_DIRECT_FLASH_ACCESS_APDIRECTFLASHACCESS_H_
#define LIBRARIES_AP_DIRECT_FLASH_ACCESS_APDIRECTFLASHACCESS_H_

#include <AG_HAL/AG_HAL.h>
#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL_PDRL.h>
#include <string.h>
#include <stdlib.h>

class AG_Direct_Flash_Access {
	HAL_Semaphore access_semaphore;
	AG_Direct_Flash_Access();
	uint8_t tempDataBuffer[32];
public:
	static AG_Direct_Flash_Access *m_AG_Direct_Flash_Access;
	static AG_Direct_Flash_Access *getInstance();
	void storeInflash(uint8_t* dataBuf, uint16_t dataSize, uint32_t add, bool continueWr = false);
	void readFromInflash(uint8_t* dataBuf, uint16_t dataSize, uint32_t add);
};

#endif /* LIBRARIES_AP_DIRECT_FLASH_ACCESS_APDIRECTFLASHACCESS_H_ */

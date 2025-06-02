/*
 * PDRLFlashHelper.h
 *
 *  Created on: 26-May-2025
 *      Author: kunal
 */

#ifndef LIBRARIES_PDRL_FWBACKUP_PDRLFLASHHELPER_H_
#define LIBRARIES_PDRL_FWBACKUP_PDRLFLASHHELPER_H_

#include <AP_HAL/AP_HAL.h>
#include <AP_FlashStorage/AP_FlashStorage.h>
#include <AP_Math/AP_Math.h>
#include <AP_InternalError/AP_InternalError.h>
#include <stdio.h>

class PDRL_FlashHelper {
public:
	static PDRL_FlashHelper *m_PDRL_FlashHelper;
	static PDRL_FlashHelper *getInstance();
	PDRL_FlashHelper();
	virtual ~PDRL_FlashHelper();
	static const uint32_t flash_sector_size = 32U * 1024U;

	uint8_t mem_buffer[AP_FlashStorage::storage_size];
	uint8_t mem_mirror[AP_FlashStorage::storage_size];

	// flash buffer
	uint8_t *flash[2];

	bool flash_write(uint8_t sector, uint32_t offset, const uint8_t *data, uint16_t length);
	bool flash_read(uint8_t sector, uint32_t offset, uint8_t *data, uint16_t length);
	bool flash_erase(uint8_t sector);
	bool flash_erase_ok(void);

	AP_FlashStorage storage{mem_buffer,
		flash_sector_size,
		FUNCTOR_BIND_MEMBER(&PDRL_FlashHelper::flash_write, bool, uint8_t, uint32_t, const uint8_t *, uint16_t),
		FUNCTOR_BIND_MEMBER(&PDRL_FlashHelper::flash_read, bool, uint8_t, uint32_t, uint8_t *, uint16_t),
		FUNCTOR_BIND_MEMBER(&PDRL_FlashHelper::flash_erase, bool, uint8_t),
		FUNCTOR_BIND_MEMBER(&PDRL_FlashHelper::flash_erase_ok, bool)};

	// write to storage and mem_mirror
	void write(uint16_t offset, const uint8_t *data, uint16_t length);
	void flashtester();
	void restore_flash();
	bool restore_from_backup(const char* path);

	bool erase_ok = 0;
};

#endif /* LIBRARIES_PDRL_FWBACKUP_PDRLFLASHHELPER_H_ */

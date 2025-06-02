/*
 * PDRLFlashHelper.cpp
 *
 *  Created on: 26-May-2025
 *      Author: kunal
 */

#include "PDRLFlashHelper.h"
#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>
#include <AP_FlashStorage/AP_FlashStorage.h>
#include <stdio.h>
#include <AP_HAL/utility/sparse-endian.h>
#include <AP_Filesystem/AP_Filesystem.h>

PDRL_FlashHelper *PDRL_FlashHelper::m_PDRL_FlashHelper = 0;
extern const AP_HAL::HAL& hal;

#define HAL_STORAGE_BACKUP_DIR "/Flash_Backup"
#define HAL_STORAGE_BACKUP_CNT 100

PDRL_FlashHelper* PDRL_FlashHelper::getInstance()
{
	if(m_PDRL_FlashHelper == 0)
		m_PDRL_FlashHelper = new PDRL_FlashHelper();
	return m_PDRL_FlashHelper;
}

PDRL_FlashHelper::PDRL_FlashHelper() {
	// TODO Auto-generated constructor stub

}

PDRL_FlashHelper::~PDRL_FlashHelper() {
	// TODO Auto-generated destructor stub
}

bool PDRL_FlashHelper::flash_write(uint8_t sector, uint32_t offset, const uint8_t *data, uint16_t length)
{
	if (sector > 1) {
		AP_HAL::panic("FATAL: write to sector %u\n", (unsigned)sector);
	}
	if (offset + length > flash_sector_size) {
		AP_HAL::panic("FATAL: write to sector %u at offset %u length %u\n",
				(unsigned)sector,
				(unsigned)offset,
				(unsigned)length);
	}
	uint8_t *b = &flash[sector][offset];
	if ((offset & 1) || (length & 1)) {
		AP_HAL::panic("FATAL: invalid write at %u:%u len=%u\n",
				(unsigned)sector,
				(unsigned)offset,
				(unsigned)length);
	}
	uint16_t len16 = length/2;
	for (uint16_t i=0; i<len16; i++) {
		const uint16_t v = le16toh_ptr(&data[i*2]);
		uint16_t v2 = le16toh_ptr(&b[i*2]);
		if (v & !v2) {
			AP_HAL::panic("FATAL: invalid write16 at %u:%u 0x%04x 0x%04x\n",
					(unsigned)sector,
					unsigned(offset+i),
					b[i],
					data[i]);
		}
#ifndef AP_FLASHSTORAGE_MULTI_WRITE
		if (v != v2 && v != 0xFFFF && v2 != 0xFFFF) {
			AP_HAL::panic("FATAL: invalid write16 at %u:%u 0x%04x 0x%04x\n",
					(unsigned)sector,
					unsigned(offset+i),
					b[i],
					data[i]);
		}
#endif
		v2 &= v;
		put_le16_ptr(&b[i*2], v2);
	}
	return true;
}

bool PDRL_FlashHelper::flash_read(uint8_t sector, uint32_t offset, uint8_t *data, uint16_t length)
{
	if (sector > 1) {
		AP_HAL::panic("FATAL: read from sector %u\n", (unsigned)sector);
	}
	if (offset + length > flash_sector_size) {
		AP_HAL::panic("FATAL: read from sector %u at offset %u length %u\n",
				(unsigned)sector,
				(unsigned)offset,
				(unsigned)length);
	}
	memcpy(data, &flash[sector][offset], length);
	return true;
}

bool PDRL_FlashHelper::flash_erase(uint8_t sector)
{
	if (sector > 1) {
		AP_HAL::panic("FATAL: erase sector %u\n", (unsigned)sector);
	}
	memset(&flash[sector][0], 0xFF, flash_sector_size);
	return true;
}

bool PDRL_FlashHelper::flash_erase_ok(void)
{
	return erase_ok;
}

void PDRL_FlashHelper::write(uint16_t offset, const uint8_t *data, uint16_t length)
{
	memcpy(&mem_mirror[offset], data, length);
	memcpy(&mem_buffer[offset], data, length);
	if (!storage.write(offset, length)) {
		if (erase_ok) {
			printf("Failed to write at %u for %u\n", offset, length);
		}
	}
}

void PDRL_FlashHelper::flashtester()
{
	flash[0] = (uint8_t *)malloc(flash_sector_size);
	flash[1] = (uint8_t *)malloc(flash_sector_size);
	flash_erase(0);
	flash_erase(1);

	if (!storage.init()) {
		AP_HAL::panic("Failed first init()");
	}

	// fill with 10k random writes
	for (uint32_t i=0; i<500000; i++) {
		uint16_t ofs = get_random16() % sizeof(mem_buffer);
		uint16_t length = get_random16() & 0x1F;
		length = MIN(length, sizeof(mem_buffer) - ofs);
		uint8_t data[length];
		for (uint8_t j=0; j<length; j++) {
			data[j] = get_random16() & 0xFF;
		}

		erase_ok = (i % 1000 == 0);
		write(ofs, data, length);

		if (erase_ok) {
			if (memcmp(mem_buffer, mem_mirror, sizeof(mem_buffer)) != 0) {
				AP_HAL::panic("FATAL: data mis-match at i=%u", (unsigned)i);
			}
		}
	}

	// force final write to allow for flush with erase_ok
	erase_ok = true;
	uint8_t b = 42;
	write(37, &b, 1);

	if (memcmp(mem_buffer, mem_mirror, sizeof(mem_buffer)) != 0) {
		AP_HAL::panic("FATAL: data mis-match before re-init");
	}

	// re-init
	printf("re-init\n");
	memset(mem_buffer, 0, sizeof(mem_buffer));
	if (!storage.init()) {
		AP_HAL::panic("Failed second init()");
	}

	if (memcmp(mem_buffer, mem_mirror, sizeof(mem_buffer)) != 0) {
		AP_HAL::panic("FATAL: data mis-match");
	}
	while (true) {
		hal.console->printf("TEST PASSED");
		hal.scheduler->delay(20000);
	}
}

void PDRL_FlashHelper::restore_flash()
{
	unsigned curr_bak = 0;
	char path[64];

	// open last_storage_bak
	char index_path[64];
	snprintf(index_path, sizeof(index_path), "%s/last_storage_bak", HAL_STORAGE_BACKUP_DIR);

	int fd = AP::FS().open(index_path, O_RDONLY);
	if (fd != -1) {
	    char buf[10];
	    memset(buf, 0, sizeof(buf));
	    if (AP::FS().read(fd, buf, sizeof(buf)-1) > 0) {
	        curr_bak = strtol(buf, NULL, 10) % HAL_STORAGE_BACKUP_CNT;
	    }
	    AP::FS().close(fd);
	}

	snprintf(path, sizeof(path), "%s/flash_bkp%d.bin", HAL_STORAGE_BACKUP_DIR, curr_bak);
	restore_from_backup(path);
	hal.storage->init();

}

bool PDRL_FlashHelper::restore_from_backup(const char* path)
{
    int fd = AP::FS().open(path, O_RDONLY);
    if (fd < 0) {
        hal.console->printf("Failed to open backup file: %s\n", path);
        return false;
    }

    size_t total_size = flash_sector_size;
    uint8_t *buffer = (uint8_t *)malloc(total_size);
    if (buffer == nullptr) {
        hal.console->printf("Failed to allocate memory for backup restore\n");
        AP::FS().close(fd);
        return false;
    }

    ssize_t total_read = AP::FS().read(fd, buffer, total_size);
    AP::FS().close(fd);

    if (total_read != (ssize_t)total_size) {
        hal.console->printf("Backup file size mismatch: read %d bytes\n", (int)total_read);
        free(buffer);
        return false;
    }

    flash_erase(0);
//    flash_erase(1);

    bool ok = flash_write(0, 0, &buffer[0], flash_sector_size);
//    ok &= flash_write(1, 0, &buffer[flash_sector_size], flash_sector_size);

    if (!ok) {
        hal.console->printf("Failed to restore flash content from %s\n", path);
    }

    free(buffer);
    return ok;
}


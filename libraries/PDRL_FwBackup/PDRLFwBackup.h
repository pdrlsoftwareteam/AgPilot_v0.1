/*
 * PDRLFwBackup.h
 *
 *  Created on: 23-May-2025
 *      Author: kunal
 */

#ifndef LIBRARIES_PDRL_FWBACKUP_PDRLFWBACKUP_H_
#define LIBRARIES_PDRL_FWBACKUP_PDRLFWBACKUP_H_

#include <AP_HAL/AP_HAL.h>
#include <AP_Common/AP_Common.h>
#include <GCS_MAVLink/GCS.h>

class PDRLFwBackup {
//	HAL_Semaphore backup_semaphore;
	PDRLFwBackup();
	uint8_t tempDataBuffer[32];
	void *flashptr = nullptr;
	size_t flashsize = 0;
	void *flashptrBkp = nullptr;
	uint8_t fw_buffer[HAL_STORAGE_SIZE] __attribute__((aligned(4)));

	uint8_t *flash[2];
	bool erase_ok = 0;
public:
	static PDRLFwBackup *m_PDRLFwBackup;
	static PDRLFwBackup *getInstance();
	virtual ~PDRLFwBackup();

	void get_flash_buffer(void);
	void save_firmware_backup(void);
	void save_flash_backup(void);
	void generate_apj(void);
	void restore_flash_config(void);
	void base64_encode_firmware_file(const char* firmware_path, const char* base64_out_path);
	void encode_decode_flash_buffer();
	void test_base64_codec();
	void load_backup_and_restore_flash(void);
	void test_func();
	void _save_flash_to_backup(void);
	void receiveFlashBuffer(unsigned char *bufPtr,uint16_t validDataLen,uint8_t bufferIndex);
	void sendPAvalidationResponse(mavlink_channel_t chan);
	void verify_flash_backup(void);
};

#endif /* LIBRARIES_PDRL_FWBACKUP_PDRLFWBACKUP_H_ */

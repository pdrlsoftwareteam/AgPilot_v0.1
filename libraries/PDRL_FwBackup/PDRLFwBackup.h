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

class PDRLFwBackup {
//	HAL_Semaphore backup_semaphore;
	PDRLFwBackup();
	uint8_t tempDataBuffer[32];
	void *flashptr = nullptr;
	size_t flashsize = 0;
	void *flashptrBkp = nullptr;
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
};

#endif /* LIBRARIES_PDRL_FWBACKUP_PDRLFWBACKUP_H_ */

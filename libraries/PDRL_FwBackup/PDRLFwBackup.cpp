/*
 * PDRLFwBackup.cpp
 *
 *  Created on: 23-May-2025
 *      Author: kunal
 */

#include "PDRLFwBackup.h"
#include "PDRLFlashHelper.h"
#include <AP_HAL/AP_HAL.h>
#include <AP_Filesystem/AP_Filesystem.h>
#include <AP_KEYSTORE/base64.h>
#include <GCS_MAVLink/GCS.h>
#include <AP_Common/AP_Common.h>
#include <AP_HAL_ChibiOS/Storage.h>
#include <stdio.h>



PDRLFwBackup *PDRLFwBackup::m_PDRLFwBackup = 0;
extern const AP_HAL::HAL& hal;
PDRLFwBackup::PDRLFwBackup() {
	// TODO Auto-generated constructor stub

}

PDRLFwBackup::~PDRLFwBackup() {
	// TODO Auto-generated destructor stub
}

PDRLFwBackup* PDRLFwBackup::getInstance()
{
	if(m_PDRLFwBackup == 0)
		m_PDRLFwBackup = new PDRLFwBackup();
	return m_PDRLFwBackup;
}

void PDRLFwBackup::save_firmware_backup(void)
{

}

void PDRLFwBackup::save_flash_backup(void)
{
#ifdef USE_POSIX
	// allow for fallback to microSD based storage
	// create the backup directory if need be
	int ret;
	const char* _storage_bak_directory = "/Flash_Backup_received";
	get_flash_buffer();

	if (hal.util->was_watchdog_armed()) {
		// we are under watchdog reset
		// ain't got no time...
		return;
	}

	EXPECT_DELAY_MS(3000);

	// We want to do this desperately,
	// So we keep trying this for a second
	uint32_t start_millis = AP_HAL::millis();
	while(!AP::FS().retry_mount() && (AP_HAL::millis() - start_millis) < 1000) {
		hal.scheduler->delay(1);
	}

	ret = AP::FS().mkdir(_storage_bak_directory);
	if (ret == -1 && errno != EEXIST) {
		return;
	}

	char* fname = nullptr;
	unsigned curr_bak = 0;
	ret = asprintf(&fname, "%s/last_storage_bak", _storage_bak_directory);
	if (fname == nullptr && (ret <= 0)) {
		return;
	}
	int fd = AP::FS().open(fname, O_RDONLY);
	if (fd != -1) {
		char buf[10];
		memset(buf, 0, sizeof(buf));
		if (AP::FS().read(fd, buf, sizeof(buf)-1) > 0) {
			//only record last HAL_STORAGE_BACKUP_COUNT backups
			curr_bak = (strtol(buf, NULL, 10) + 1)%100;
		}
		AP::FS().close(fd);
	}

	fd = AP::FS().open(fname, O_WRONLY|O_CREAT|O_TRUNC);
	free(fname);
	fname = nullptr;
	if (fd != -1) {
		char buf[10];
		snprintf(buf, sizeof(buf), "%u\r\n", (unsigned)curr_bak);
		const ssize_t to_write = strlen(buf);
		const ssize_t written = AP::FS().write(fd, buf, to_write);
		AP::FS().close(fd);
		if (written < to_write) {
			return;
		}
	} else {
		return;
	}

	// create and write fram data to file
	ret = asprintf(&fname, "%s/flash_bkp%d.bin", _storage_bak_directory, curr_bak);
	if (fname == nullptr || (ret <= 0)) {
		return;
	}
	fd = AP::FS().open(fname, O_WRONLY|O_CREAT|O_TRUNC);
	free(fname);
	fname = nullptr;
	if (fd != -1) {
		//finally dump the fram data
		AP::FS().write(fd, flashptr, flashsize);
		AP::FS().close(fd);
		gcs().send_text(MAV_SEVERITY_INFO, "flash backup completed...");
	}
#endif
}

void PDRLFwBackup::load_backup_and_restore_flash(void)
{
	if(!hal.storage->set_storage_data(flashptrBkp,HAL_STORAGE_SIZE))
	{
		gcs().send_text(MAV_SEVERITY_INFO, "failed to write the flash configuration");
	}
	return;
#ifdef USE_POSIX
	int ret;
	const char* _storage_bak_directory = "/Flash_Backup";
	unsigned curr_bak = 0;

	// Retry mounting the filesystem
	uint32_t start_millis = AP_HAL::millis();
	while (!AP::FS().retry_mount() && (AP_HAL::millis() - start_millis) < 1000) {
		hal.scheduler->delay(1);
	}

	// Read the last backup index
	char* fname = nullptr;
	ret = asprintf(&fname, "%s/last_storage_bak", _storage_bak_directory);
	if (fname == nullptr || ret <= 0) {
		return;
	}

	int fd = AP::FS().open(fname, O_RDONLY);
	if (fd != -1) {
		char buf[10] = {};
		if (AP::FS().read(fd, buf, sizeof(buf) - 1) > 0) {
			curr_bak = (unsigned)strtol(buf, nullptr, 10);
		}
		AP::FS().close(fd);
	} else {
		free(fname);
		return;
	}
	free(fname);
	fname = nullptr;

	// Read the actual backup data
	ret = asprintf(&fname, "%s/flash_bkp%u.bin", _storage_bak_directory, curr_bak);
	if (fname == nullptr || ret <= 0) {
		return;
	}

	fd = AP::FS().open(fname, O_RDONLY);
	free(fname);
	fname = nullptr;

	if (fd != -1) {
		// Read backup into _buffer
		ssize_t read_bytes = AP::FS().read(fd, flashptrBkp, HAL_STORAGE_SIZE);
		AP::FS().close(fd);
		gcs().send_text(MAV_SEVERITY_INFO, "param override successful");

		if (read_bytes == CH_STORAGE_SIZE) {
			if(!hal.storage->set_storage_data(flashptrBkp,HAL_STORAGE_SIZE))
			{
				gcs().send_text(MAV_SEVERITY_INFO, "failed to write the flash configuration");
			}
			// Now write _buffer back to flash
			//            _write_to_flash(_buffer, CH_STORAGE_SIZE);
		}
	}
#endif
}


void PDRLFwBackup::generate_apj(void)
{

}

void PDRLFwBackup::restore_flash_config(void)
{

}


void PDRLFwBackup::base64_encode_firmware_file(const char* firmware_path, const char* base64_out_path)
{
#ifdef USE_POSIX
	if (!AP::FS().retry_mount()) {
		gcs().send_text(MAV_SEVERITY_ERROR, "FS mount failed");
		return;
	}

	int input_fd = AP::FS().open(firmware_path, O_RDONLY);
	if (input_fd < 0) {
		gcs().send_text(MAV_SEVERITY_ERROR, "Failed to open input: %s", firmware_path);
		return;
	}

	int output_fd = AP::FS().open(base64_out_path, O_CREAT | O_WRONLY | O_TRUNC);
	if (output_fd < 0) {
		gcs().send_text(MAV_SEVERITY_ERROR, "Failed to open output: %s", base64_out_path);
		AP::FS().close(input_fd);
		return;
	}

	const size_t chunk_size = 384;  // divisible by 3 for clean base64
	uint8_t read_buf[chunk_size];

	size_t encoded_buf_len = 4 * ((chunk_size + 2) / 3) + 1;
	uint8_t* encoded_buf = new uint8_t[encoded_buf_len];

	if (!encoded_buf) {
		gcs().send_text(MAV_SEVERITY_ERROR, "Memory alloc failed for encoded buffer");
		AP::FS().close(input_fd);
		AP::FS().close(output_fd);
		return;
	}

	ssize_t bytes_read;
	while ((bytes_read = AP::FS().read(input_fd, read_buf, chunk_size)) > 0) {
		size_t encoded_len = 0;
		int ret = mbedtls_base64_encode(encoded_buf, encoded_buf_len, &encoded_len, read_buf, bytes_read);
		if (ret != 0) {
			gcs().send_text(MAV_SEVERITY_ERROR, "Base64 encoding failed: %d", ret);
			break;
		}

		// Write encoded data followed by newline
		if (AP::FS().write(output_fd, encoded_buf, encoded_len) != (ssize_t)encoded_len ||
				AP::FS().write(output_fd, (const uint8_t*)"\n", 1) != 1) {
			gcs().send_text(MAV_SEVERITY_ERROR, "Write failed to output file");
			break;
		}
	}

	gcs().send_text(MAV_SEVERITY_INFO, "Base64 encoding complete: %s", base64_out_path);

	delete[] encoded_buf;
	AP::FS().close(input_fd);
	AP::FS().close(output_fd);
#endif
}

void PDRLFwBackup::get_flash_buffer()
{
	flashptr = nullptr;
	flashsize = 0;
	if (hal.storage->get_storage_ptr(flashptr, flashsize)) {

	}
}

//void PDRLFwBackup::encode_decode_flash_buffer()
//{
//    flashptr = nullptr;
//    flashsize = 0;
//
//    if (!hal.storage->get_storage_ptr(flashptr, flashsize) || flashptr == nullptr || flashsize == 0) {
//        gcs().send_text(MAV_SEVERITY_ERROR, "Failed to get flash storage buffer");
//        return;
//    }
//
//    size_t encoded_len_estimate = 4 * ((flashsize + 2) / 3);
//    uint8_t* encoded_buf = new uint8_t[encoded_len_estimate];
//    uint8_t* decoded_buf = new uint8_t[flashsize];
//
//    if (!encoded_buf || !decoded_buf) {
//        gcs().send_text(MAV_SEVERITY_ERROR, "Memory allocation failed");
//        delete[] encoded_buf;
//        delete[] decoded_buf;
//        return;
//    }
//
//    size_t actual_encoded_len = 0;
//    int ret = mbedtls_base64_encode(
//        encoded_buf,
//        encoded_len_estimate,
//        &actual_encoded_len,
//        reinterpret_cast<const unsigned char*>(flashptr),
//        flashsize);
//
//    if (ret != 0) {
//        gcs().send_text(MAV_SEVERITY_ERROR, "Base64 encode failed: %d", ret);
//        delete[] encoded_buf;
//        delete[] decoded_buf;
//        return;
//    }
//
//    size_t actual_decoded_len = 0;
//    ret = mbedtls_base64_decode(
//        decoded_buf,
//        flashsize,
//        &actual_decoded_len,
//        encoded_buf,
//        actual_encoded_len);
//
//    if (ret != 0) {
//        gcs().send_text(MAV_SEVERITY_ERROR, "Base64 decode failed: %d", ret);
//    } else {
//        if (actual_decoded_len == flashsize && memcmp(flashptr, decoded_buf, flashsize) == 0) {
//            gcs().send_text(MAV_SEVERITY_INFO, "Success: Decoded data matches original");
//        } else {
//            gcs().send_text(MAV_SEVERITY_ERROR, "Decoded data mismatch or size mismatch");
//        }
//    }
//
//    delete[] encoded_buf;
//    delete[] decoded_buf;
//}

void PDRLFwBackup::test_base64_codec()
{
	const char test_data[] = "Hello Embedded!";
	const size_t test_len = strlen(test_data);
	size_t enc_len = 0;
	size_t dec_len = 0;

	uint8_t encoded[64];
	uint8_t decoded[64];

	mbedtls_base64_encode(encoded, sizeof(encoded), &enc_len,
			reinterpret_cast<const unsigned char*>(test_data), test_len);

	mbedtls_base64_decode(decoded, sizeof(decoded), &dec_len, encoded, enc_len);

	if (dec_len == test_len && memcmp(decoded, test_data, test_len) == 0) {
		gcs().send_text(MAV_SEVERITY_INFO, "Base64 test passed");
	} else {
		gcs().send_text(MAV_SEVERITY_ERROR, "Base64 test failed");
	}
}


void PDRLFwBackup::encode_decode_flash_buffer()
{
	size_t enc_len = 0;
	size_t dec_len = 0;

	// Buffers for encoded and decoded data
	const size_t encoded_buf_size = ((flashsize + 2) / 3) * 4 + 1; // Base64 size calculation
	uint8_t* encoded = new uint8_t[encoded_buf_size];
	uint8_t* decoded = new uint8_t[flashsize]; // original size

	int ret_enc = mbedtls_base64_encode(encoded, encoded_buf_size, &enc_len,
			reinterpret_cast<const unsigned char*>(flashptr), flashsize);
	if (ret_enc != 0) {
		gcs().send_text(MAV_SEVERITY_ERROR, "Base64 encoding failed");
		delete[] encoded;
		delete[] decoded;
		return;
	}

	int ret_dec = mbedtls_base64_decode(decoded, flashsize, &dec_len, encoded, enc_len);
	if (ret_dec != 0) {
		gcs().send_text(MAV_SEVERITY_ERROR, "Base64 decoding failed");
		delete[] encoded;
		delete[] decoded;
		return;
	}

	if (dec_len == flashsize && memcmp(decoded, flashptr, flashsize) == 0) {
		gcs().send_text(MAV_SEVERITY_INFO, "Base64 encode/decode test passed");
	} else {
		gcs().send_text(MAV_SEVERITY_ERROR, "Base64 encode/decode test failed");
	}

	delete[] encoded;
	delete[] decoded;
}

void PDRLFwBackup::test_func()
{
//	_save_flash_to_backup();
	PDRL_FlashHelper::getInstance()->restore_flash();
}

void PDRLFwBackup::_save_flash_to_backup(void)
{
#ifdef USE_POSIX
    const char* _storage_bak_directory = "/Flash_Backup";

    // Allocate heap memory for backup
    uint8_t* backup_buffer = (uint8_t*)malloc(CH_STORAGE_SIZE);
    if (!backup_buffer) {
        printf("Failed to allocate backup buffer\n");
        return;
    }
	uint32_t flash_sector_size = PDRL_FlashHelper::getInstance()->flash_sector_size;
    PDRL_FlashHelper::getInstance()->flash_read(0, 0, &backup_buffer[0], flash_sector_size);
//    PDRL_FlashHelper::getInstance()->flash_read(1, 0, &backup_buffer[flash_sector_size], flash_sector_size);

    // Read and update backup index
    unsigned curr_bak = 0;
    char* fname = nullptr;
    int ret = asprintf(&fname, "%s/last_storage_bak", _storage_bak_directory);
    if (!fname || ret <= 0) {
        free(backup_buffer);
        return;
    }

    int fd = AP::FS().open(fname, O_RDONLY);
    if (fd != -1) {
        char buf[10] = {};
        if (AP::FS().read(fd, buf, sizeof(buf) - 1) > 0) {
            curr_bak = (unsigned)strtol(buf, nullptr, 10);
        }
        AP::FS().close(fd);
    }

    curr_bak = (curr_bak + 1) % 1000;

    fd = AP::FS().open(fname, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd != -1) {
        char numbuf[10];
        snprintf(numbuf, sizeof(numbuf), "%u", curr_bak);
        AP::FS().write(fd, numbuf, strlen(numbuf));
        AP::FS().close(fd);
    }
    free(fname);

    // Write flash backup to file
    ret = asprintf(&fname, "%s/flash_bkp%u.bin", _storage_bak_directory, curr_bak);
    if (!fname || ret <= 0) {
        free(backup_buffer);
        return;
    }

    fd = AP::FS().open(fname, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd != -1) {
        AP::FS().write(fd, backup_buffer, CH_STORAGE_SIZE);
        AP::FS().close(fd);
    }
    free(fname);
    free(backup_buffer);
#endif
}

void PDRLFwBackup::receiveFlashBuffer(unsigned char *bufPtr,uint16_t validDataLen,uint8_t bufferIndex)
{
	memcpy(fw_buffer+(bufferIndex*250),bufPtr,validDataLen);
//	gcs().send_message(MSG_DATA_TRANSFER);
}

void PDRLFwBackup::sendPAvalidationResponse(mavlink_channel_t chan)
{
//	return;
//	uint8_t txBUff[250] = {0};
//
//	const char* response = "data is OK";
//	strncpy((char*)txBUff, response, sizeof(txBUff) - 1);  // prevent overflow
//
//	mavlink_msg_data_transfer_send(
//		chan,
//		1,                    // seq
//		0,                    // total_seq
//		txBUff,               // payload
//		strlen(response)      // length
//	);
//

    mavlink_msg_ack_for_command_send(
            chan,
            227,
            1,
            1
    );
}

void PDRLFwBackup::verify_flash_backup(void)
{
#ifdef USE_POSIX
	// allow for fallback to microSD based storage
	// create the backup directory if need be
	int ret;
	const char* _storage_bak_directory = "/Flash_Backup_received";

	if (hal.util->was_watchdog_armed()) {
		// we are under watchdog reset
		// ain't got no time...
		return;
	}

	EXPECT_DELAY_MS(3000);

	// Try to mount the FS, retrying up to 1 second
	uint32_t start_millis = AP_HAL::millis();
	while (!AP::FS().retry_mount() && (AP_HAL::millis() - start_millis) < 1000) {
		hal.scheduler->delay(1);
	}

	ret = AP::FS().mkdir(_storage_bak_directory);
	if (ret == -1 && errno != EEXIST) {
		return;
	}

	char* fname = nullptr;
	unsigned curr_bak = 0;
	ret = asprintf(&fname, "%s/last_storage_bak", _storage_bak_directory);
	if (fname == nullptr || (ret <= 0)) {
		return;
	}
	int fd = AP::FS().open(fname, O_RDONLY);
	if (fd != -1) {
		char buf[10];
		memset(buf, 0, sizeof(buf));
		if (AP::FS().read(fd, buf, sizeof(buf)-1) > 0) {
			// only record last 100 backups
			curr_bak = (strtol(buf, NULL, 10) + 1) % 100;
		}
		AP::FS().close(fd);
	}

	fd = AP::FS().open(fname, O_WRONLY | O_CREAT | O_TRUNC);
	free(fname);
	fname = nullptr;
	if (fd != -1) {
		char buf[10];
		snprintf(buf, sizeof(buf), "%u\r\n", (unsigned)curr_bak);
		const ssize_t to_write = strlen(buf);
		const ssize_t written = AP::FS().write(fd, buf, to_write);
		AP::FS().close(fd);
		if (written < to_write) {
			return;
		}
	} else {
		return;
	}

	// === FIRMWARE BUFFER WRITE SECTION ===
	// Assume fw_buffer is defined in class or globally, e.g.:
	// uint8_t fw_buffer[4096];  // Example static buffer
	// Calculate its size:
	size_t fw_buffer_size = sizeof(fw_buffer);

	ret = asprintf(&fname, "%s/flash_bkp%d.bin", _storage_bak_directory, curr_bak);
	if (fname == nullptr || (ret <= 0)) {
		return;
	}
	fd = AP::FS().open(fname, O_WRONLY | O_CREAT | O_TRUNC);
	free(fname);
	fname = nullptr;
	if (fd != -1) {
		AP::FS().write(fd, fw_buffer, fw_buffer_size);
		AP::FS().close(fd);
		gcs().send_text(MAV_SEVERITY_INFO, "fw_buffer backup completed...");
	}
#endif
}



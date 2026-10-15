/*
 * PdrlBootPlugin.cpp
 *
 *  Created on: 03-Oct-2020
 *      Author: owner
 */

#include "PdrlBootPlugin.h"
#include <AP_Vehicle/AP_Vehicle_Type.h>
#include <GCS_MAVLink/GCS.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_PDRL_Commander/AP_PDRL_Commander_Logger.h>

extern const AP_HAL::HAL& hal;

unsigned char PdrlBootPlugin::output1[32] = {0};
unsigned char PdrlBootPlugin::output2[32] = {0};
bool PdrlBootPlugin::logChecksumSuccess = false;

__attribute__((section(".checksum"))) checksumData_st m_checksumData =
{
                .codePreCalChksm = {0xCA,0x45,0x9F,0x11,0x0B,0xDA,0xAD,0x68,0x4F,0x47,0x14,0x54,0x08,0x77,0x39,0x47,0x67,0x8A,0x55,0x2C,0x34,0x07,0xB3,0xC6,0xD5,0x86,0x30,0xD9,0x40,0xD3,0x44,0xB4},
        .dataPreCalChksm = {0xE3,0xB0,0xC4,0x42,0x98,0xFC,0x1C,0x14,0x9A,0xFB,0xF4,0xC8,0x99,0x6F,0xB9,0x24,0x27,0xAE,0x41,0xE4,0x64,0x9B,0x93,0x4C,0xA4,0x95,0x99,0x1B,0x78,0x52,0xB8,0x55}
};

const uint8_t *flash_base = (const uint8_t *)(0x08000000);// + FLASH_BOOTLOADER_LOAD_KB*1024U);

checksumRuntimeData_st PdrlBootPlugin::m_checksumRuntimeData;

mbedtls_sha256_context PdrlBootPlugin::ctx2;

AP_RAMTRON PdrlBootPlugin::fram;

PdrlBootPlugin::PdrlBootPlugin() {
	m_checksumRuntimeData.codeCheckSumStatus = false;
	m_checksumRuntimeData.dataChecksumStatuc = false;
}

PdrlBootPlugin::~PdrlBootPlugin() {
	// TODO Auto-generated destructor stub
}

void PdrlBootPlugin::init()
{
	//bool using_fram =
//	fram.init();
	//	if(using_fram)
	//	{
	//	fram.read(BOOT_DATA_LOCATION,(uint8_t*)&m_checksumData,(uint32_t)sizeof(m_checksumData));
	//	fram.read(BOOT_DATA_LOCATION+sizeof(m_checksumData),(uint8_t*)&m_checksumRuntimeData,(uint32_t)sizeof(m_checksumRuntimeData));
	//	}
}
uint8_t* PdrlBootPlugin::getcodecksm()
{
	return m_checksumData.codePreCalChksm;
}
uint8_t* PdrlBootPlugin::getdatacksm()
{
	return m_checksumData.dataPreCalChksm;
}

void PdrlBootPlugin::startSha2()
{
	mbedtls_sha256_init(&ctx2);
	mbedtls_sha256_starts(&ctx2, 0); /* SHA-256, not 224 */
}

void PdrlBootPlugin::updateToSha2(uint8_t * buff,uint16_t len)
{
	mbedtls_sha256_update(&ctx2,(unsigned char *)buff, len);
}

bool PdrlBootPlugin::finishSha2(uint8_t chksmType)
{
    logChecksumSuccess = false;
	if(chksmType == CODE_CHKSM)
	{
		mbedtls_sha256_finish(&ctx2, output1);
		mbedtls_sha256_free(&ctx2);
		memcpy(m_checksumRuntimeData.codeRuntimeCalChksm,output1,32);
		if(memcmp(output1,m_checksumData.codePreCalChksm,32) == 0)
			m_checksumRuntimeData.codeCheckSumStatus = true;
//		else
//			m_checksumRuntimeData.codeCheckSumStatus = false;
//		fram.write(BOOT_DATA_LOCATION,(uint8_t*)&m_checksumData,(uint32_t)sizeof(m_checksumData));
		return m_checksumRuntimeData.codeCheckSumStatus;
	}
	else if(chksmType == DATA_CHKSM)
	{
		mbedtls_sha256_finish(&ctx2, output2);
		mbedtls_sha256_free(&ctx2);
		memcpy(m_checksumRuntimeData.dataRuntimeCalChksm,output2,32);
		if(memcmp(output2,m_checksumData.dataPreCalChksm,32) == 0)
			m_checksumRuntimeData.dataChecksumStatuc = true;
		else
			m_checksumRuntimeData.dataChecksumStatuc = false;
//		fram.write(BOOT_DATA_LOCATION+sizeof(m_checksumData),(uint8_t*)&m_checksumRuntimeData,(uint32_t)sizeof(m_checksumRuntimeData));
		return m_checksumRuntimeData.dataChecksumStatuc;
	}
	return false;
}

void PdrlBootPlugin::checksum_config_error(const char *reason)
{
	//	const AP_HAL::HAL& hal = AP_HAL::get_HAL();
	AP_PDRL_Logger *m_AP_PDRL_Logger;
	m_AP_PDRL_Logger = AP_PDRL_Logger::getInstance();
	m_AP_PDRL_Logger->logStrToFile((char*)reason,strlen(reason));
}

void PdrlBootPlugin::handleChecksumStatus()
{
#if DISABLE_CHECKSUM_VERIFICATION
    return;
#endif
//	m_checksumRuntimeData.codeCheckSumStatus = true;
//	m_checksumRuntimeData.dataChecksumStatuc = true;
    uint32_t last_print_ms = 0;
    while ((isDataChecksumMatch() == false) || (isCodeChecksumMatch() == false))
    {
    	EXPECT_DELAY_MS(7000);
		uint32_t now = AP_HAL::millis();
		if (now - last_print_ms >= 3000) {
			last_print_ms = now;
			if(isCodeChecksumMatch() == false)
			{
				char logBuff[150] = "Code checksum failed:";
			//		gcs().send_text(MAV_SEVERITY_ERROR, "%s", logBuff);
				char *hexVal = nullptr;
				for(int i = 0; i < 32 ; i++)
				{
					if(asprintf(&hexVal,"%02x",m_checksumRuntimeData.codeRuntimeCalChksm[i])){}
					strcat(logBuff,hexVal);
					free(hexVal);
				}
				strcat(logBuff,"\n");
				checksum_config_error(logBuff);
				char* cptr = logBuff;
				for(int i = 0 ; i < (int)(strlen(logBuff)/MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN)+1 ; i++)
				{
					gcs().send_text(MAV_SEVERITY_ERROR, "%s", cptr+(i*MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN));
				}
				gcs().send_text(MAV_SEVERITY_ERROR, "C1:%.32s", logBuff+21);
				gcs().send_text(MAV_SEVERITY_ERROR, "C2:%.32s", logBuff+53);
			}
			if(isDataChecksumMatch() == false)
			{
				char logBuff[150] = "Data checksum failed:";
			//		gcs().send_text(MAV_SEVERITY_ERROR, "%s", logBuff);
				char *hexVal = nullptr;

				for(int i = 0; i < 32 ; i++)
				{
					if(asprintf(&hexVal,"%02x",m_checksumRuntimeData.dataRuntimeCalChksm[i])){}
					strcat(logBuff,hexVal);
					free(hexVal);
				}
				strcat(logBuff,"\n");
				checksum_config_error(logBuff);
				char* cptr = logBuff;
				for(int i = 0 ; i < (int)(strlen(logBuff)/MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN)+1 ; i++)
				{
					gcs().send_text(MAV_SEVERITY_ERROR, "%s", cptr+(i*MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN));
				}
			}
			if((isCodeChecksumMatch() == false) || (isDataChecksumMatch() == false))
			{
				gcs().send_text(MAV_SEVERITY_ERROR, "%s", "Firmware checksum verification failed");
			}
		}
		hal.scheduler->delay(5);
    }
//	uint32_t last_print_ms = 0;
//	while ((isDataChecksumMatch() == false) || (isCodeChecksumMatch() == false))
//	{
//		EXPECT_DELAY_MS(7000);
//		uint32_t now = AP_HAL::millis();
//		if (now - last_print_ms >= 3000) {
//			last_print_ms = now;
//			gcs().send_text(MAV_SEVERITY_ERROR, "%s", "Firmware checksum verification failed");
//		}
//		hal.scheduler->delay(5);
//	}

	if(logChecksumSuccess == false)
	{
        char logBuff[150] = "Code and Data checksum verify successful\n";
        checksum_config_error(logBuff);
        logChecksumSuccess = true;
	}
}

void PdrlBootPlugin::calculateCodeCksm()
{
#if CONFIG_HAL_BOARD != HAL_BOARD_SITL
	startSha2();
	volatile uint8_t bytesVar;
	uint8_t *flashStartAdd = (uint8_t *)(0x08008000);
	uint32_t memSize = ((uint32_t)&m_checksumData)-0x08008000;

	for (uint32_t p = 0; p < memSize; p++)
	{
		if(flashStartAdd)
		{
			bytesVar = *flashStartAdd;
			updateToSha2((uint8_t*)&bytesVar,1);
		}
		flashStartAdd++;
	}
	finishSha2(CODE_CHKSM);
#endif
}

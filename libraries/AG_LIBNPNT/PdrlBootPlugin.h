/*
 * PdrlBootPlugin.h
 *
 *  Created on: 03-Oct-2020
 *      Author: owner
 */

#ifndef PDRLBOOTPLUGIN_H_
#define PDRLBOOTPLUGIN_H_
#include <AG_Common/AG_Common.h>
#include <AG_KEYSTORE/sha256.h>
#include <AG_HAL/AG_HAL.h>
#include <stdint.h>
#include <string.h>
#include <AG_RAMTRON/AG_RAMTRON.h>

#define CODE_CHKSM 1
#define DATA_CHKSM 2

#define BOOT_DATA_LOCATION HAL_STORAGE_SIZE
#define BOOT_DATA_SIZE 200 //we only need 200 bytes //BOOT_DATA_LOCATION+(16*1024) //16k, 16384 bytes
#define DISABLE_CHECKSUM_VERIFICATION 0


typedef struct checksumData_st{
	uint8_t codePreCalChksm[32];
	uint8_t dataPreCalChksm[32];
}checksumData_st;

typedef struct checksumRuntimeData_st{
	bool codeCheckSumStatus;
	bool dataChecksumStatuc;
	uint8_t codeRuntimeCalChksm[32];
	uint8_t dataRuntimeCalChksm[32];
}checksumRuntimeData_st;

class PdrlBootPlugin {

public:
	static mbedtls_sha256_context ctx2;
	static checksumRuntimeData_st m_checksumRuntimeData;
	static unsigned char output1[32];
	static unsigned char output2[32];

	static AG_RAMTRON fram;
	static bool logChecksumSuccess;

	PdrlBootPlugin();
	virtual ~PdrlBootPlugin();
	static void init();
	static void startSha2();
	static void updateToSha2(uint8_t * buff,uint16_t len);
	static bool finishSha2(uint8_t chksmType);
	static void checksum_config_error(const char *reason);
	static void handleChecksumStatus();
	static bool isCodeChecksumMatch(){ return m_checksumRuntimeData.codeCheckSumStatus; }
	static bool isDataChecksumMatch(){ return m_checksumRuntimeData.dataChecksumStatuc; }
	static void calculateCodeCksm();
	static uint8_t* getcodecksm();
	static uint8_t* getdatacksm();

};

#endif /* PDRLBOOTPLUGIN_H_ */

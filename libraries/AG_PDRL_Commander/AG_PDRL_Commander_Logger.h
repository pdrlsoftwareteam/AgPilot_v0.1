/*
 * AG_PDRL_Commander_Logger.h
 *
 *  Created on: 31-Oct-2019
 *      Author: owner
 */

#ifndef LIBRARIES_AP_PDRL_COMMANDER_AP_PDRL_COMMANDER_LOGGER_H_
#define LIBRARIES_AP_PDRL_COMMANDER_AP_PDRL_COMMANDER_LOGGER_H_
#include <AG_HAL/AG_HAL.h>
#include <GCS_MAVLink/GCS.h>
#include <AG_Common/AG_Common.h>
#include <AG_Param/AG_Param.h>
#include <AG_InertialSensor/AG_InertialSensor.h>
#include <AG_AHRS/AG_AHRS.h>
#include <AG_Mission/AG_Mission.h>
#include <AG_RPM/AG_RPM.h>
#include <AG_Logger/LogStructure.h>
#include <AG_Motors/AG_Motors.h>
#include <AG_Rally/AG_Rally.h>
#include <AG_Beacon/AG_Beacon.h>
#include <AG_Proximity/AG_Proximity.h>
#include <AG_InertialSensor/AG_InertialSensor_Backend.h>
#include <AG_GPS/AG_GPS.h>
#include <AG_Filesystem/AG_Filesystem.h>
#include "AG_NPNT_data_helper.h"


#include <AG_KEYSTORE/AG_KEYSTORE.h>
#include <AG_KEYSTORE/aes.h>
#include <AG_KEYSTORE/ctr_drbg.h>
#include <AG_KEYSTORE/entropy.h>
#include <AG_KEYSTORE/pk.h>
#include <AG_KEYSTORE/pkcs5.h>
#include <AG_KEYSTORE/platform_mbedtls.h>
#include <AG_KEYSTORE/rsa.h>
#include <AG_KEYSTORE/memory_buffer_alloc.h>
#include <AG_KEYSTORE/base64.h>
#include <AG_KEYSTORE/mbedtls_md.h>
#include <AG_KEYSTORE/sha256.h>

#include <time.h>
#include <stdint.h>

//#define QCI_TEST_LOG 1

typedef struct locationTime_st{
	double lat;
	double lng;
	float alt;
	uint64_t time;
}locationTime_st;

class AP_PDRL_Logger{
	AP_PDRL_Logger();
	int _geofence_file_fd = -1;
	int _current_log_file_fd = -1;
	const uint32_t _free_space_min_avail = 8388608; // bytes
	HAL_Semaphore write_fd_semaphore;
	volatile bool _open_error;
	AG_NPNT_data_helper* m_AG_NPNT_data_helper;
	uint16_t fileCount = 0;
	uint8_t takeOffLogged = 0;
	uint8_t landDetected = 1;
	uint8_t m_isLogGenerated  = 0;
	locationTime_st takeOffLocTime;
	locationTime_st LandOffLocTime;

	char *hashBuffer;
	char write_filename[60] = "DANGLING_LOGS.txt";
	unsigned char oldLogFileHash[32] = {0};

public:
	static AP_PDRL_Logger *m_AP_PDRL_Logger;
	static AP_PDRL_Logger *getInstance();

	size_t currentLogFileReadOffset = 0;

#define QCI_TEST_LOG 0
#if QCI_TEST_LOG
	int currentDebugLogType = 0;
	void qciLogGenerationTest(int fileFd);
#endif

	void setFileCount(uint16_t m_fileCount)
	{
		fileCount = m_fileCount;
	}

	uint16_t getFileCount()
	{
		return fileCount;
	}

	bool file_exists(const char *filename) const;
	int64_t disk_space_avail();
	bool addCordinateToLog(char* entryType,double latitude,double longitude,float altitude,uint64_t epochTime);
	size_t fileSize(char* filename);
	int startReadingLogFile();
	int getLogFileInBuffer(uint8_t* byBuffer,uint16_t readSize);
	int getLogFileOffsetInBuffer(uint8_t* byBuffer,uint16_t fileoffset, uint16_t readSize, uint8_t isSetSeek);
	int sendLogFileToGCS(char* filename, int fileIndex);
	int start_new_geofence_log(char* filePath);
	int makeJsonAndSignLogFile(char* outFilepath);
	int makeJsonAndSignLogFileLatest();
	size_t getFileSignature(uint8_t *currentHashFileName,unsigned char* signatureBuff,size_t sigBuffSize);
	int checkTimeBreach(uint64_t *currentTime);
	int setTakeOffLolation(MAV_LANDED_STATE landState);
	int setLandLolation(MAV_LANDED_STATE landState);
	void logTakeOffLand(MAV_LANDED_STATE landState);
	int mbedtls_md_file(const mbedtls_md_info_t *md_info, const char *path, unsigned char *output );
	void setHashBuffer(char* hash,uint64_t hashLen,uint64_t totalLen,uint64_t offset);
	void freeHashBuffer();
	size_t encryptHash(char* buffer,size_t dataLenght);
	size_t encryptHashUsingPublicKey(unsigned char* hashBuff,unsigned char* outBuff,size_t outBuffSize);
	void signLogFile(char* inFilePath,size_t len);
	void reset(){takeOffLogged = 0;landDetected = 1;}
	uint8_t isLogGenerated(){return m_isLogGenerated;}
	void setLogGenerated(){ m_isLogGenerated = 1;}
	void setLogFileName(char* nameF){strcpy(write_filename,nameF);}
	void clearLogFile();
	void logStrToFile(char* logBuff,size_t len);
	char *_log_file_name_short(const uint16_t log_num) const;
	char *_log_file_name_long(const uint16_t log_num) const;
	char *_log_file_name(const uint16_t log_num) const;
};

extern AP_PDRL_Logger *pdrl_logger;

namespace AP {
	AP_PDRL_Logger &pdrl_logger();
}


#endif /* LIBRARIES_AP_PDRL_COMMANDER_AP_PDRL_COMMANDER_LOGGER_H_ */

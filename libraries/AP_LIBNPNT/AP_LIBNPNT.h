/*
 * keystore.hpp
 *
 *  Created on: 31-Jul-2019
 *      Author: Owner
 */
#pragma once

#ifndef LIBNPNT_HPP_
#define LIBNPNT_HPP_

//#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/AP_HAL.h>
//#include <AP_Common/AP_Common.h>
#include <AP_Common/AP_Common.h>
#include <AP_Param/AP_Param.h>
#include <AP_Math/AP_Math.h>
//#include <AP_Vehicle/AP_Vehicle.h>
//#include <AP_SerialManager/AP_SerialManager.h>
#include <AP_Vehicle/AP_Vehicle.h>
#include <AP_SerialManager/AP_SerialManager.h>
#include <AP_RTC/AP_RTC.h>

#include <GCS_MAVLink/GCS.h>
#include <AP_Filesystem/AP_Filesystem.h>
#include <AP_KEYSTORE/AP_KEYSTORE.h>
#include <AP_KEYSTORE/x509_crt.h>
#include "PdrlBootPlugin.h"

#define SIGNATURE_TRANSFER_BLOCK_SIZE 128
/*
#define SBF_DEBUGGING 0

#if SBF_DEBUGGING
# define Debug(fmt, args ...)                  \
		do {                                            \
			printf("%s:%d: " fmt "\n",     \
					__FUNCTION__, __LINE__, \
					## args);               \
					hal.scheduler->delay(1);                    \
		} while(0)
#else
# define Debug(fmt, args ...)
#endif
*/
#include <stdio.h>
#include <time.h>


//#define NULL 0
#define ECCTYPE    "secp521r1"

#include "npnt_helpers.h"


typedef enum LIB_NPNT_STATUS
{
    STATUS_LIB_NPNT_STARTED = 1,
    STATUS_DOWNLOAD_PA = 2,
    STATUS_DOWNLOAD_PA_FAILED_NO_DRONE_ID = 3,
    STATUS_VERIFY_PA = 4,
    STATUS_VERIFY_PA_FAILED = 5,
    STATUS_PARSE_PA = 6,
    STATUS_PARSE_PA_FAILED = 7,
    STATUS_UPLOADING_PA_TO_RPAS = 8,
    STATUS_UPLOAD_PA_TO_RPAS_FAILED = 9,
    STATUS_UPLOAD_PA_TO_RPAS_COMPLETED = 10,
    STATUS_UPLOAD_LOG_FILE = 11,
    STATUS_UPLOAD_LOG_FILE_FAILED = 12,
	STATUS_NPNT_INV_TIME,
	STATUS_NPNT_INV_ART
}LIB_NPNT_STATUS;

typedef enum REQUEST_TYPE
{
    GET_APP_ID_EM,
    DOWNLOAD_PA_EM,
    SEND_LOG_TO_DGCA,
    DOWNLOAD_REPORT_PDF,
    MAX_REQUEST_TYPE,
}REQUEST_TYPE;

typedef enum GEOFENCE_SEND_STATE
{
    SEND_FENCE_TOTAL,
    SEND_LAT_LONG,
    READ_LAT_LONG,
    ENABLE_GEO_FENCE,
    SET_GEOFENCE_TYPE,
    SET_GEOFENCE_ACTION,
    SET_GEOFENCE_MAX_ALTITUDE
}GEOFENCE_STATE;


class AP_LIBNPNT{

private:

	AP_LIBNPNT();
public:

    npnt_helpers_c npnt_helpers;
    npnt_s  npnt_handle = {};

    uint8_t messageFrequency = 3;

    int libNpntStatus = 0;
    int paValidStatus = 0;

    uint8_t signature[256] = {0};
    uint8_t firmwareHash[32] = {0};

	static AP_LIBNPNT* m_pInstance;
	static AP_LIBNPNT* getInstance();
	~AP_LIBNPNT();

	void free_common();
	char* getStringBetweenTags(char* buff,char* PATTERN1,char* PATTERN2);
	int16_t extract_public_key_from_xml_artefact(char* buffer);
	int16_t verifyValidGeofence();
	int16_t load_artifact(char* buffer,uint16_t dataLen);
	int authenticateSinglePA(char* paBuffer,uint16_t dataLen);
	bool getIsPAValid(bool silent=false);
	void setIsPAValid(LIB_NPNT_STATUS val);
	void showMessageTimebreach();
	void showMessageFencebreach();
	void sendPAvalidationResponse(mavlink_channel_t chan);
	void receiveHash(unsigned char *hashPtr,uint16_t validDataLen);
	void receiveSignature(unsigned char *signaturePtr,uint16_t validDataLen,uint8_t bufferIndex);
};
extern AP_LIBNPNT *libnpnt;
#endif /* KEYSTORE_HPP_ */

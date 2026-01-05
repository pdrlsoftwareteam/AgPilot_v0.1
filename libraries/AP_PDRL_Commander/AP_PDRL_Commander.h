/*
 * keystore.hpp
 *
 *  Created on: 31-Jul-2019
 *      Author: Owner
 */
#ifndef PDRL_COMMANDER_HPP_
#define PDRL_COMMANDER_HPP_

#include <AP_Common/AP_Common.h>
#include <AP_Param/AP_Param.h>
#include <AP_Math/AP_Math.h>
#include <AP_Vehicle/AP_Vehicle.h>
#include <AP_SerialManager/AP_SerialManager.h>
#include <AP_RTC/AP_RTC.h>
#include <GCS_MAVLink/GCS.h>
#include <AP_Filesystem/AP_Filesystem.h>
#include <AP_KEYSTORE/AP_KEYSTORE.h>
#include <AP_HAL/AP_HAL.h>
#include "AP_PDRL_Commander_Logger.h"
#include "AP_NPNT_data_helper.h"

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

#define DRONE_ID_ADDRESS 	0x818FFFE
#define DRONE_ID_LENGTH		30

typedef struct command_st{
	uint8_t command;
	uint8_t commandType;
	uint8_t isAckRequired;
	uint8_t dataLen;
	uint8_t dataBuff[30];
}command_st;

class AP_PDRL_COMMANDER{

private:
	uint8_t ret = 0;
	uint8_t lastOffset = 0;
	uint64_t CurrentLogFileSize = 0;
	uint64_t CurrentPostLogFileSize = 0;
	static AP_PDRL_COMMANDER* m_pInstance;
	bool isUnlocked = false;
	uint32_t lastUnlock = 0;
	AP_PDRL_COMMANDER();
	~AP_PDRL_COMMANDER();

public:
	AP_NPNT_data_helper* m_AP_NPNT_data_helper;
	mavlink_command_transfer_t cmdReceived;
	mavlink_command_transfer_t cmdSend;
	static AP_PDRL_COMMANDER* getInstance();
	uint32_t getLastUnlock(){return lastUnlock;}
	void setIsUnock(bool mlockUnlock){isUnlocked = mlockUnlock;}
	bool isGcsUnlocked()
	{
		// if(hal.util->get_soft_armed())
		// {
		// 	isUnlocked = true;
		// }
		return isUnlocked;
	}
	void sendIsPubKeyPresent();
	void sendFIRMWARE_VERSION();
	void sendRPAS_VERSION();
	void sendRPAS_CATEGORY();
	void sendLogFileCount();
	void getUniqueBoardID(char buf[30]);
	void sendDroneID();
	void sendKey();
	void sendLogFile(mavlink_command_transfer_t *rcvedPacket);
	void sendLogFileSignature(mavlink_command_transfer_t *rcvedPacket);
	void sendPostLogFileSignature(mavlink_command_transfer_t *rcvedPacket);
	void sendSprayStatus();
	void sendCommand(
			uint16_t item_offset,
			uint8_t command,
			uint8_t command_type,
			uint8_t is_ack_required,
			uint64_t data_len,
			uint64_t item_count,
			uint8_t* command_buff);
	void handleHashToSign(mavlink_command_transfer_t* packet);
	void parseCommand(const mavlink_message_t &msg);
	void sendSdcardStatus();
	void sendcmd();
	bool start = 0;
	char nma_uid_str[64] = {0};
	void sendGPSID(mavlink_channel_t);

	uint64_t time_usec;
	uint8_t hw_version_major;
	uint8_t hw_version_minor;
	uint8_t hw_unique_id[64] = {};
	uint8_t sw_version_major;
	uint8_t sw_version_minor;
	uint32_t sw_vcs_commit;
	char name[80] = {0};
	uint8_t node_id;
	bool node_received = false;
	bool isRequested = false;
};

namespace AP {
AP_PDRL_COMMANDER *pdrl_commander();
};



#endif /* KEYSTORE_HPP_ */

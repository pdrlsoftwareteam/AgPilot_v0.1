/*
 * keystore.hpp
 *
 *  Created on: 31-Jul-2019
 *      Author: Owner
 */
#ifndef PDRL_COMMANDER_HPP_
#define PDRL_COMMANDER_HPP_

#include <AG_Common/AG_Common.h>
#include <AG_Param/AG_Param.h>
#include <AG_Math/AG_Math.h>
#include <AG_Vehicle/AG_Vehicle.h>
#include <AG_SerialManager/AG_SerialManager.h>
#include <AG_RTC/AG_RTC.h>
#include <GCS_MAVLink/GCS.h>
#include <AG_Filesystem/AG_Filesystem.h>
#include <AG_KEYSTORE/AG_KEYSTORE.h>
#include <AG_HAL/AG_HAL.h>
#include "AG_PDRL_Commander_Logger.h"
#include "AG_NPNT_data_helper.h"

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
	static AP_PDRL_COMMANDER* m_pInstance;
	bool isUnlocked = false;
	uint32_t lastUnlock = 0;
	AP_PDRL_COMMANDER();
	~AP_PDRL_COMMANDER();

public:
	AG_NPNT_data_helper* m_AG_NPNT_data_helper;
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
};

#endif /* KEYSTORE_HPP_ */

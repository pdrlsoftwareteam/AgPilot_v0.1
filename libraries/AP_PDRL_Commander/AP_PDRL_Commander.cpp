/*
 * keystore.cpp
 *
 *  Created on: 31-Jul-2019
 *      Author: Owner
 */
#include "AP_PDRL_Commander.h"
#include "AP_LIBNPNT/PdrlBootPlugin.h"
#include "AC_Avoidance/AP_OAPathPlanner.h"
#include "AC_Avoidance/AC_Avoid.h"
#include "AC_Sprayer/AC_Sprayer.h"
#include "AP_AHRS/AP_AHRS.h"
#include "AP_BattMonitor/AP_BattMonitor_FuelFlow.h"
#include "AP_Arming/AP_Arming.h"
#include "AP_Logger/AP_Logger.h"
#if CONFIG_HAL_BOARD != HAL_BOARD_SITL
#include "hal.h"
#include "hwdef.h"
#endif
extern const AP_HAL::HAL& hal;
extern AP_KEYSTORE *keyStore;
AP_PDRL_COMMANDER* AP_PDRL_COMMANDER::m_pInstance = 0;

AP_PDRL_COMMANDER* AP_PDRL_COMMANDER::getInstance()
{
	if (!m_pInstance)   // Only allow one instance of class to be generated.
		m_pInstance = new AP_PDRL_COMMANDER;
	//	m_AP_NPNT_data_helper = AP_NPNT_data_helper::getInstance();
	return m_pInstance;
}

AP_PDRL_COMMANDER::AP_PDRL_COMMANDER()
{
	m_AP_NPNT_data_helper = AP_NPNT_data_helper::getInstance();
}

AP_PDRL_COMMANDER::~AP_PDRL_COMMANDER()
{

}

void AP_PDRL_COMMANDER::getUniqueBoardID(char buf[100])
{
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
	char serialid[] = "AG000100000000000000000001234";
	memcpy(buf,serialid,29);
#else
	uint8_t serialid[12];
	memcpy(serialid, (const void *)UDID_START, 12);

	// this format is chosen to match the format used by HAL_PX4
	snprintf(buf, 30, "AG000%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			(unsigned)serialid[3], (unsigned)serialid[2], (unsigned)serialid[1], (unsigned)serialid[0],
			(unsigned)serialid[7], (unsigned)serialid[6], (unsigned)serialid[5], (unsigned)serialid[4],
			(unsigned)serialid[11], (unsigned)serialid[10], (unsigned)serialid[9],(unsigned)serialid[8]);
#endif
}

void AP_PDRL_COMMANDER::sendDroneID()
{
	uint8_t droneIDBuffer[100]= "0";// = "PDRL-dac487ec";
	getUniqueBoardID((char*)droneIDBuffer);
	sendCommand(0,COMMAND_GET_DRONE_ID,COMMAND_TYPE_RESPONSE,0,sizeof(droneIDBuffer),0,droneIDBuffer); //COMMAND_GET_DRONE_ID,droneIDBuffer,30,COMMAND_TYPE_RESPONSE,0);
}

void AP_PDRL_COMMANDER::sendKey()
{
	//	uint16_t keyLen = 0;
	keyStore->keyTransferTest = KEY_TRANSFER_IDEAL;
	uint8_t *ptr = (uint8_t*)pdrlPublicKey;
	uint16_t keyLen = strlen(pdrlPublicKey);
	//	keyStore->readKey(keyStore->temp5kBuff ,&keyLen,KEY_TYPE_PUBLIC);
	uint16_t numberOfBlockToSend = (( KEY_TRANSFER_BLOCK_SIZE-(keyLen%KEY_TRANSFER_BLOCK_SIZE) )+keyLen) / KEY_TRANSFER_BLOCK_SIZE;
	for(int i = 0 ; i < numberOfBlockToSend;i++)
	{
		EXPECT_DELAY_MS(300);
		keyStore->transferKey(i,ptr,keyLen,keyLen%(KEY_TRANSFER_BLOCK_SIZE*i));
		hal.scheduler->delay(10);
	}
}

void AP_PDRL_COMMANDER::sendIsPubKeyPresent()
{
	uint8_t droneIDBuffer[1]= {0};

	droneIDBuffer[0] = keyStore->isKeyPairPresent;
	//	m_AP_Direct_Flash_Access->readFromInflash(droneIDBuffer,30,DRONE_ID_ADDRESS);
	sendCommand(0,COMMAND_GET_DRONE_PUBLIC_KEY,COMMAND_TYPE_RESPONSE,0,1,0,droneIDBuffer); //COMMAND_GET_DRONE_ID,droneIDBuffer,30,COMMAND_TYPE_RESPONSE,0);
}

void AP_PDRL_COMMANDER::sendFIRMWARE_VERSION()
{
	uint8_t firmwareVersion[30] = "FW_1_0_0";
	//	m_AP_Direct_Flash_Access->readFromInflash(droneIDBuffer,30,DRONE_ID_ADDRESS);
	sendCommand(0,COMMAND_GET_FIRMWARE_VERSION,COMMAND_TYPE_RESPONSE,0,sizeof(firmwareVersion),0,firmwareVersion);
}

void AP_PDRL_COMMANDER::sendRPAS_VERSION()
{
	uint8_t firmwareVersion[30] = "FW_1_0_0";
	//	m_AP_Direct_Flash_Access->readFromInflash(droneIDBuffer,30,DRONE_ID_ADDRESS);
	sendCommand(0,COMMAND_GET_RPAS_VERSION,COMMAND_TYPE_RESPONSE,0,sizeof(firmwareVersion),0,firmwareVersion);
}

void AP_PDRL_COMMANDER::sendRPAS_CATEGORY()
{
	uint8_t rpasCategory[30] = "MINI_DRONE";
	//	m_AP_Direct_Flash_Access->readFromInflash(droneIDBuffer,30,DRONE_ID_ADDRESS);
	sendCommand(0,COMMAND_GET_RPAS_CATEGORY,COMMAND_TYPE_RESPONSE,0,sizeof(rpasCategory),0,rpasCategory);
}

void AP_PDRL_COMMANDER::sendLogFileCount()
{
	AP_PDRL_Logger *m_AP_PDRL_Logger = AP_PDRL_Logger::getInstance();
	uint16_t fileCount = m_AP_PDRL_Logger->getFileCount();
	sendCommand(0,COMMAND_GET_FLIGHT_LOG_FILE_COUNT,COMMAND_TYPE_RESPONSE,0,sizeof(uint16_t),0,(uint8_t*)&fileCount);
}

void AP_PDRL_COMMANDER::sendLogFile(mavlink_command_transfer_t *rcvedPacket)
{
	AP_PDRL_Logger *m_AP_PDRL_Logger = AP_PDRL_Logger::getInstance();
	uint8_t isSetSeek = 0;
	if(rcvedPacket->item_offset == 0)
	{
		lastOffset = -1;
		if(m_AP_PDRL_Logger->isLogGenerated() == 0)
		{
			m_AP_PDRL_Logger->setLogGenerated();
			m_AP_PDRL_Logger->makeJsonAndSignLogFileLatest();
		}
		CurrentLogFileSize = m_AP_PDRL_Logger->startReadingLogFile();
		if(!CurrentLogFileSize)
		{
			uint8_t dataBuffer[100] = {0};
			strcpy((char*)&dataBuffer,"Log Downloader : Cant read log file");
			sendCommand(0,COMMAND_GET_FLIGHT_LOG,COMMAND_TYPE_ACK,0,sizeof(dataBuffer),0,dataBuffer); //COMMAND_GET_DRONE_ID,droneIDBuffer,30,COMMAND_TYPE_RESPONSE,0);
			return;
		}
	}
	if(lastOffset != (int)(rcvedPacket->item_offset-1))
		isSetSeek = 1;
	lastOffset = rcvedPacket->item_offset;
	int readCount = 0;
	if((rcvedPacket->item_offset*100) <= (CurrentLogFileSize+100))
	{
		EXPECT_DELAY_MS(5000);
		uint8_t dataBuffer[100] = {0};
		if( (readCount = m_AP_PDRL_Logger->getLogFileOffsetInBuffer(dataBuffer,rcvedPacket->item_offset*100 ,100,isSetSeek) ) > 0)
		{
			sendCommand(
					rcvedPacket->item_offset,
					COMMAND_GET_FLIGHT_LOG,
					COMMAND_TYPE_RESPONSE,
					0,
					CurrentLogFileSize,
					readCount,
					(uint8_t*)dataBuffer
			);

			//      sendCommand(logFileSize,COMMAND_GET_FLIGHT_LOG,COMMAND_TYPE_RESPONSE,0,readCount,1,(uint8_t*)dataBuffer);
		}
		else
		{
			strcpy((char*)&dataBuffer,"Log Downloader : Offset out of range");
			sendCommand(0,COMMAND_GET_FLIGHT_LOG,COMMAND_TYPE_ACK,0,sizeof(dataBuffer),0,dataBuffer); //COMMAND_GET_DRONE_ID,droneIDBuffer,30,COMMAND_TYPE_RESPONSE,0);
			return;
		}
	}
	else
	{
		uint8_t dataBuffer[100] = {0};
		strcpy((char*)&dataBuffer,"Log Downloader : File end here");
		sendCommand(0,COMMAND_GET_FLIGHT_LOG,COMMAND_TYPE_ACK,0,100,0,dataBuffer); //COMMAND_GET_DRONE_ID,droneIDBuffer,30,COMMAND_TYPE_RESPONSE,0);
		return;
	}
}

void AP_PDRL_COMMANDER::sendLogFileSignature(mavlink_command_transfer_t *rcvedPacket)
{
	uint8_t offsetCnt = rcvedPacket->item_offset;
	uint8_t dataBuff[100]= {0};
	if(offsetCnt != 0)
	{
		memcpy(dataBuff,keyStore->temp5kBuff+(offsetCnt*100),100);
		sendCommand(offsetCnt,COMMAND_GET_FLIGHT_LOG,COMMAND_TYPE_GET,0,100,(uint64_t)CurrentLogFileSize,dataBuff);
		return;
	}

	AP_PDRL_Logger *m_AP_PDRL_Logger = AP_PDRL_Logger::getInstance();

	memset(keyStore->temp5kBuff,0,sizeof(keyStore->temp5kBuff));
	CurrentLogFileSize = m_AP_PDRL_Logger->getFileSignature(rcvedPacket->command_buff,keyStore->temp5kBuff,sizeof(keyStore->temp5kBuff));
	if(CurrentLogFileSize == (uint64_t)-1)
	{
		sendCommand(0,COMMAND_GET_FLIGHT_LOG,COMMAND_TYPE_GET,0,0,0,dataBuff);
		return;
	}

	memcpy(dataBuff,keyStore->temp5kBuff,100);
	sendCommand(offsetCnt,COMMAND_GET_FLIGHT_LOG,COMMAND_TYPE_GET,0,100,(uint64_t)CurrentLogFileSize,dataBuff);
	return;
}

void AP_PDRL_COMMANDER::sendPostLogFileSignature(mavlink_command_transfer_t *rcvedPacket)
{
	uint8_t dataBuff[100]= {0};
	uint8_t offsetCnt = rcvedPacket->item_offset;

	if(offsetCnt != 0)
	{
		memcpy(dataBuff,keyStore->temp5kBuff+(offsetCnt*100),100);
		sendCommand(offsetCnt,COMMAND_GET_DRONE_PRIVATE_KEY,COMMAND_TYPE_GET,0,100,(uint64_t)CurrentLogFileSize,dataBuff);
		return;
	}
	AP_PDRL_Logger *m_AP_PDRL_Logger = AP_PDRL_Logger::getInstance();

	memset(keyStore->temp5kBuff,0,sizeof(keyStore->temp5kBuff));
    CurrentLogFileSize = m_AP_PDRL_Logger->getPostFileSignature(rcvedPacket->command_buff,keyStore->temp5kBuff,sizeof(keyStore->temp5kBuff));
	if(CurrentLogFileSize == (uint64_t)-1)
	{
		sendCommand(0,COMMAND_GET_DRONE_PRIVATE_KEY,COMMAND_TYPE_GET,0,0,0,dataBuff);
		return;
	}

	memcpy(dataBuff,keyStore->temp5kBuff,100);
	sendCommand(offsetCnt,COMMAND_GET_DRONE_PRIVATE_KEY,COMMAND_TYPE_GET,0,100,(uint64_t)CurrentLogFileSize,dataBuff);
	return;
}

void AP_PDRL_COMMANDER::sendSprayStatus()
{

//	if(AP::arming().is_armed())
//	{
		uint8_t dataBuff[100] = {0};
		// send spray status only if the sprayer is enabled
		dataBuff[0] = AP::sprayer()->spraying();

		if(dataBuff[0] == 1 && AP::sprayer()->getPulseCount())
		{
			sendCommand(0,COMMAND_SET_SPRAY_STATUS,COMMAND_TYPE_GET,0,1,1,dataBuff);
			//		printf("Sent Spray Status: %d\n",dataBuff[0]);
			return;
		}
		dataBuff[0] = 0;
		sendCommand(0,COMMAND_SET_SPRAY_STATUS,COMMAND_TYPE_GET,0,1,1,dataBuff);
		if(!AP::sprayer()->getTankstatus())
			AP::sprayer()->setPulseCount(1);

//	}

}

void AP_PDRL_COMMANDER::sendCommand(
		uint16_t item_offset,
		uint8_t command,
		uint8_t command_type,
		uint8_t is_ack_required,
		uint64_t data_len,
		uint64_t item_count,
		uint8_t* command_buff
)
{
	if( data_len>100 )
	{
		data_len = 100;
	}
	cmdSend.item_offset = item_offset;
	cmdSend.command = command;
	cmdSend.command_type  = command_type;
	cmdSend.is_ack_required = is_ack_required;
	cmdSend.data_len  = data_len;
	cmdSend.item_count = item_count;
	memcpy(cmdSend.command_buff,command_buff,data_len);
	gcs().send_message(MSG_COMMAND_TRANSFER);
}

void AP_PDRL_COMMANDER::handleHashToSign(mavlink_command_transfer_t* packet)
{
	AP_PDRL_Logger *m_AP_PDRL_Logger = AP_PDRL_Logger::getInstance();
	m_AP_PDRL_Logger->setHashBuffer((char*)packet->command_buff,packet->item_count,packet->data_len,packet->item_offset);
	if((packet->item_offset+packet->data_len) == packet->item_count)
	{
		//completed hash received. encrypt data and set it to mavlink here
		uint64_t bufferLen = 0;
		uint64_t buffeOffset = 0;
		uint64_t itemCount = 0;
		uint8_t* bufptr = 0;

		char dataBuffer[600] = {0};
		bufferLen = m_AP_PDRL_Logger->encryptHash((char*)&dataBuffer,sizeof(dataBuffer));

		bufptr = (uint8_t*)&dataBuffer;

		while(bufptr)
		{
			EXPECT_DELAY_MS(10);
			hal.scheduler->delay(10);

			if(buffeOffset == bufferLen)
				break;
			else if( (buffeOffset+100) >= bufferLen)
			{
				itemCount = (bufferLen - buffeOffset);
			}
			else
				itemCount = 100;


			mavlink_msg_command_transfer_send(
					MAVLINK_COMM_0,
					COMMAND_GET_SIGN_FROM_HASH,
					COMMAND_TYPE_RESPONSE,
					0,
					itemCount,
					bufferLen,
					buffeOffset,
					bufptr
			);
			bufptr += itemCount;
			buffeOffset += itemCount;
		}
	}
	m_AP_PDRL_Logger->freeHashBuffer();
}

void AP_PDRL_COMMANDER::parseCommand(const mavlink_message_t &msg)
{
	//handle receive commands
	mavlink_command_transfer_t packet;
	mavlink_msg_command_transfer_decode(&msg, &packet);
	COMMAND_PDRL command = (COMMAND_PDRL)packet.command;
	switch(command)
	{
	case COMMAND_SET_SPRAY_STATUS:
		break;

	case COMMAND_SEND_PARAM_ACK:
		break;

	case COMMAND_GET_SPRAYED_AREA:
		break;

	case COMMAND_GET_DRONE_ID:
		//load varibles with required data
		sendDroneID();
		break;

	case COMMAND_START_KEY_GENERATION:
	{
		//		keyStore->setGenarateKeyFlagForReboot();
		keyStore->keyTransferTest = KEY_TRANSFER_GENARATE_KEY;
		//reboot device
		//			hal.scheduler->reboot(true);
	}
	break;

	case COMMAND_STORE_KEY:
		if(keyStore->keyTransferTest != KEY_TRANSFER_GENARATE_KEY)
		{
			sendKey();
		}

		break;

	case COMMAND_GET_DRONE_PRIVATE_KEY:
	{
		sendPostLogFileSignature(&packet);
	}
	break;

	case COMMAND_GET_DRONE_PUBLIC_KEY:
	{
		sendIsPubKeyPresent();
	}
	break;

	case COMMAND_GET_FIRMWARE_VERSION:
		sendFIRMWARE_VERSION();
		break;

	case COMMAND_GET_RPAS_VERSION:
		sendRPAS_VERSION();
		break;

	case COMMAND_GET_RPAS_CATEGORY:
		sendRPAS_CATEGORY();
		break;

	case COMMAND_CLEAR_FLIGHT_LOG:
	{
		AP_PDRL_Logger::getInstance()->clearLogFile();
	}
	break;

	case COMMAND_GET_FLIGHT_LOG_FILE_COUNT:
	{
		sendLogFileCount();
	}
	break;

	case COMMAND_GET_FLIGHT_LOG:
	{
		//		sendLogFile(&packet); //get geofence br
		sendLogFileSignature(&packet);
	}
	break;

	case COMMAND_SET_OPERATOR_ID:
	{
		// Start section oemName
		char oemName[] = "5ft7dnhk";
		// End section oemName

//		strcpy(oemName,"m");
		 if(memcmp((void*)packet.command_buff,(void*)oemName,strlen(oemName)) == 0)
		 {
			lastUnlock = AP_HAL::millis();
		 	isUnlocked = true;
		 }
	}
	break;

	case COMMAND_GET_OPERATOR_ID:
		break;

	case COMMAND_SET_FLIGHT_START_TIME:
	{
		// uint64_t *dataPtr= (uint64_t*)packet.command_buff;
		memcpy(&m_AP_NPNT_data_helper->flightEndTime,packet.command_buff,sizeof(uint8_t));//*dataPtr;
		m_AP_NPNT_data_helper->flightStartTime *= (uint64_t)1000000;
	}
	break;

	case COMMAND_GET_FLIGHT_START_TIME:
	{

		uint8_t dataBuff[100]= {0};
		uint8_t* str;
		str = PdrlBootPlugin::getcodecksm();
		memcpy(dataBuff,str,sizeof(uint8_t)*32);
		str = PdrlBootPlugin::getdatacksm();
		memcpy(dataBuff+32,str,sizeof(uint8_t)*32);
		sendCommand(0,COMMAND_GET_FLIGHT_START_TIME,COMMAND_TYPE_GET,0,64,2,dataBuff);
	}
	break;

	case COMMAND_SET_FLIGHT_END_TIME:
	{
		// uint64_t *dataPtr= (uint64_t*)packet.command_buff;
		memcpy(&m_AP_NPNT_data_helper->flightEndTime,packet.command_buff,sizeof(uint8_t));//*dataPtr;
		m_AP_NPNT_data_helper->flightEndTime *= (uint64_t)1000000;
	}
	break;

	case COMMAND_GET_FLIGHT_END_TIME:
	{
		AP_Param::verifySha256Checksum();
	}
	break;

	case COMMAND_SET_FLIGHT_MAX_ALTITUDE:
		break;

	case COMMAND_GET_FLIGHT_MAX_ALTITUDE:
		break;

	case COMMAND_SET_FLIGHT_MAX_PAYLOAD:
		break;

	case COMMAND_GET_FLIGHT_MAX_PAYLOAD:
		break;

	case COMMAND_SET_FLIGHT_PAYLOAD_DETAILS:
	{
		uint8_t flag_Status[100];
		strcpy((char*)flag_Status,"OBSTACLE_FLAG");

		flag_Status[13] = 0x41;
		flag_Status[14] = AP::ac_avoid()->get_manFlag();
		flag_Status[15] = 0x4F;
		flag_Status[16] = AP::ap_oapathplanner()->get_autoFlag();
		sendCommand(0,COMMAND_SET_FLIGHT_PAYLOAD_DETAILS,COMMAND_TYPE_GET,0,17,2,flag_Status);
		//		gcs().send_text(MAV_SEVERITY_ERROR, "Sent flag status");

	}
	break;

	case COMMAND_GET_FLIGHT_PAYLOAD_DETAILS:
		break;

	case COMMAND_SET_PERMISSION_ARTIFACTS:
	{
		//set permission artifact here and use it as file name
		m_AP_NPNT_data_helper->setParam(m_AP_NPNT_data_helper->permissionArticaftID,(char*)packet.command_buff);
		AP_PDRL_Logger::getInstance()->setLogFileName(m_AP_NPNT_data_helper->permissionArticaftID);
	}
	break;

	case COMMAND_GET_PERMISSION_ARTIFACTS:
	{
		AC_Sprayer *sprayer = AP::sprayer();
		if (sprayer == nullptr) {
			uint8_t spray_ack[100] = {0};
			gcs().send_text(MAV_SEVERITY_INFO,"Sprayer Not Init");
			sendCommand(0,COMMAND_GET_PERMISSION_ARTIFACTS,COMMAND_TYPE_GET,0,1,1,spray_ack);
			break;
		}

		if(packet.command_buff[0])
		{
			printf("Got Sprayer ON command\n");
			sprayer->run(true);
		}
		else
		{
			printf("Got Sprayer OFF command\n");
			sprayer->run(false);
		}
	}
	break;

	case COMMAND_GET_SIGN_FROM_HASH:
		handleHashToSign(&packet);
		break;

	case COMMAND_GET_NON_PROCESSED_PA_ID:
		break;

	case COMMAND_DELETE_LOG_FOR_PA:
		break;

	case COMMAND_GET_PA_VALIDATION_RESULT:
		break;

	case COMMAND_GET_SIG_VALIDATION_RESULT:
		break;

	case COMMAND_GET_SDCARD_STATUS:
	    sendSdcardStatus();
	    break;

	case COMMAND_PDRL_ENUM_END:
		break;


	}
}

void AP_PDRL_COMMANDER::sendSdcardStatus()
{
    const char* Status = "SD card Detected";
    if (!AP::logger().CardInserted()) {
        Status = "No SD card Detected";
    }
    uint8_t len = strlen(Status);
    sendCommand(0, COMMAND_GET_SDCARD_STATUS, COMMAND_TYPE_GET, 0, len, 1, (uint8_t*)Status);
}



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

	    uint8_t* entry_arr = nullptr;     // dynamic array to store indices
	    size_t entry_count = 0;           // how many entries are stored
	    size_t entry_capacity = 0;        // how much space is allocated

	struct NFZ_Circle {
	    uint8_t  id;
	    uint8_t	zone_type;
	    double    lat;      // degrees
	    double    lng;      // degrees
	    uint32_t    radius_m; // centimeters
	    uint16_t    alt_max;  // cms
	};

	NFZ_Circle* nfz_array = nullptr;
	size_t count = 0;
	size_t capacity = 0;

	bool addNFZ(const NFZ_Circle& nfz) {
	    if (count >= capacity) {
		size_t new_capacity = capacity + 1;  // minimal growth
		NFZ_Circle* temp = (NFZ_Circle*)realloc(nfz_array, new_capacity * sizeof(NFZ_Circle));
		if (!temp) {
		    return false; // out of memory
		}
		nfz_array = temp;
		capacity = new_capacity;
	    }

	    nfz_array[count++] = nfz;
	    return true;
	}

	NFZ_Circle* getNFZ(size_t index) {
	       return (index < count) ? &nfz_array[index] : nullptr;
	}
	size_t send_NFZ_count(){ return count;}

	struct NFZ_Polygon {
	    uint8_t  	id;
	    uint8_t	zone_type;
	    double	*lat_arr;      // degrees
	    double    	*lng_arr;      // degrees
	    uint16_t	alt_max;  // cms
	    uint8_t 	total_point;
	};

	NFZ_Polygon* nfz_poly_array = nullptr;
	size_t count_poly = 0;
	size_t capacity_poly = 0;

	bool addNFZ_poly(const NFZ_Polygon& nfz) {
		    if (count_poly >= capacity_poly) {
			size_t new_capacity = capacity_poly + 1;  // minimal growth
			NFZ_Polygon* temp = (NFZ_Polygon*)realloc(nfz_poly_array, new_capacity * sizeof(NFZ_Polygon));
			if (!temp) {
			    return false; // out of memory
			}
			nfz_poly_array = temp;
			capacity_poly = new_capacity;
		    }

		    nfz_poly_array[count_poly++] = nfz;
		    return true;
		}

	NFZ_Polygon* getNFZ_poly(size_t index) {
	       return (index < count_poly) ? &nfz_poly_array[index] : nullptr;
	}
	size_t send_NFZ_count_poly(){ return count_poly;}
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
	void setCircleCoordinateNFZ(uint8_t index, uint8_t area_type, uint8_t zone_type,
				    double latitude[], double longitude[], uint32_t radius, uint16_t altitude_max);
	void setPolygonCoordinateNFZ(uint8_t index, uint16_t total_point, uint16_t curr_index, double lat, double lng, uint16_t altitude_max,uint8_t zone_type);

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
};

#endif /* KEYSTORE_HPP_ */

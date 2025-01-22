/*
 * AG_NPNT_data_helper.hpp
 *
 *  Created on: 01-Nov-2019
 *      Author: owner
 */

#ifndef LIBRARIES_AP_PDRL_COMMANDER_AP_NPNT_DATA_HELPER_H_
#define LIBRARIES_AP_PDRL_COMMANDER_AP_NPNT_DATA_HELPER_H_

#include <AG_HAL/AG_HAL.h>
#include <AG_Math/AG_Math.h>
#include <string.h>

#define DRONEID_LEN 10
#define VERSION_LEN 6
#define RPAS_CAT_LEN 10
#define FLIGHT_TIME_LEN 16
#define OPERATOR_ID_LEN 30
#define FLIGH_PAYLOAD_LEN 10
#define PERMISSION_ARTIFACT_ID_LEN 40

class AG_NPNT_data_helper{
	AG_NPNT_data_helper();

public:

	char droneId[DRONEID_LEN];
	char firmwareVersion[VERSION_LEN];
	char rpasVersion[VERSION_LEN];
	char rpasCategory[RPAS_CAT_LEN];
	char operatorId[OPERATOR_ID_LEN];
	uint64_t flightStartTime;
	uint64_t flightEndTime;
	char flightPayLoadDetails[FLIGH_PAYLOAD_LEN];
	char permissionArticaftID[PERMISSION_ARTIFACT_ID_LEN];
	float maxAltitude;
	float maxPayload;


	static AG_NPNT_data_helper *m_AG_NPNT_data_helper;
	static AG_NPNT_data_helper* getInstance();

	void setParam(char* dest,char* src) {
		strcpy(dest,src);
	}

	const char* getFirmwareVersion() const {
		return firmwareVersion;
	}

	const char* getFlightPayLoadDetails() const {
		return flightPayLoadDetails;
	}

	float getMaxAltitude() const {
		return maxAltitude;
	}

	void setMaxAltitude(float r_maxAltitude) {
		this->maxAltitude = r_maxAltitude;
	}

	float getMaxPayload() const {
		return maxPayload;
	}

	void setMaxPayload(float r_maxPayload) {
		this->maxPayload = r_maxPayload;
	}

	const char* getOperatorId() const {
		return operatorId;
	}

	const char* getRpasCategory() const {
		return rpasCategory;
	}

	const char* getRpasVersion() const {
		return rpasVersion;
	}

	const char* getDroneId() const {
		return droneId;
	}
};


#endif /* LIBRARIES_AP_PDRL_COMMANDER_AP_NPNT_DATA_HELPER_H_ */

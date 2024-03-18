#ifndef NPNT_H
#define NPNT_H
/**
 * @file    inc/npnt.h
 * @brief   Common Headers for NPNT library
 * @{
 */
#include <AP_HAL_ChibiOS/hwdef/common/stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "defines.h"

//#include <AP_KEYSTORE/AP_KEsYSTORE.h>
#include <AP_KEYSTORE/sha256.h>
#include <AP_KEYSTORE/error.h>
#include <AP_KEYSTORE/mbedtls_md.h>
#include <AP_KEYSTORE/pk.h>
#include <AP_KEYSTORE/x509_crt.h>

#define OPENSSL 0

class npnt_helpers_c{

public:

    npnt_helpers_c();
    //    variables
#if OPENSSL
    SHA256_CTX sha;
    EVP_PKEY *dgca_pkey = nullptr;
    EVP_PKEY_CTX *dgca_pkey_ctx = nullptr;
#else
    mbedtls_sha256_context sha;
    mbedtls_x509_crt x509Cert;

#endif

    uint8_t* base64_encode(uint8_t *src, uint32_t len, uint16_t *out_len);
    uint8_t* base64_decode(uint8_t *src, uint16_t len, uint16_t *out_len);

    void clearCordinateList(npnt_s* handle);
    void addToList(npnt_s* handle,GeoCoordinate_st* cord);

    //Common helper headers
    void reset_sha256();
    void update_sha256(const char* data, uint16_t data_len);
    void final_sha256(char* hash);
    //User Implemented Methods
    /**
 * @brief   Returns Current GPS Time in 64bit UTC format.
 * @details This method returns time in UTC format
 *
 *
 * @return           Time in 64bit UTC
 * @retval 0         GPS Time not available
 *
 * @iclass control_iface
 */
    uint64_t npnt_utc_time();

    /**
 * @brief   Return Absolute Location
 * @details This method returns lattitude and longitude in degrees
 *          and Altitude in meters Above Ground Level
 *
 * @param[out]
 *
 * @return           -Errorcode if failure, 0 if GPS position available
 * @retval NPNT_ERR_POS   Absolute position not available
 *
 * @iclass control_iface
 */
    int8_t npnt_abs_position(float *gps_lat, float *gps_lon, float *altitude_agl);

    /**
 * @brief   Return Absolute Location
 * @details This method returns lattitude and longitude in degrees
 *          and Altitude in meters Above Ground Level
 *
 * @param[out]
 *
 * @return            Code of aircraft state
 * @retval NPNT_GPS_WAIT   Waiting for GPS lock
 *         NPNT_PERM_WAIT  Waiting for permission
 *         NPNT_RTF        Ready to Fly
 *         NPNT_ARMED      actuators activated
 *         NPNT_INFLIGHT   aircraft flying
 *         NPNT_LANDED     aircraft landed
 *         NPNT_CRASHED    aircraft crashed
 *
 * @iclass control_iface
 */
    int8_t npnt_aircraft_state(npnt_s *npnt_handle);

    //Implemented by libnpnt
    /**
 * @brief   Returns Breach State.
 * @details This method checks based on the current info the state
 *          of the breach.
 *
 * @param[in] npnt_handle        npnt handle
 *
 * @return           Breach type, 0 if no breach
 * @retval NPNT_BR_TIME   There has been a time breach
 *         NPNT_BR_FENCE  There has been a fence breach
 *
 * @iclass control_iface
 */
    int8_t npnt_breach_state(npnt_s *npnt_handle);

    /**
 * @brief   Sets Current Permission Artifact.
 * @details This method consumes peremission artefact in raw format
 *          and sets up npnt structure.
 *
 * @param[in] npnt_handle       npnt handle
 * @param[in] permart           permission json artefact in base64 format as received
 *                              from server
 * @param[in] permart_length    size of permission json artefact in base64 format as received
 *                              from server
 * @param[in] signature         signature of permart in base64 format
 * @param[in] signature_length  length of the signature of permart in base64 format
 *
 * @return           Error id if faillure, 0 if no breach
 * @retval NPNT_INV_ART   Invalid Artefact
 *         NPNT_INCOMP_ART Incomplete Artefact
 *         NPNT_INV_AUTH  signed by unauthorised entity
 *         NPNT_INV_STATE artefact can't setup in current aircraft state
 *
 * @iclass control_iface
 */
    int8_t npnt_set_permart(npnt_s *handle, uint8_t *permart, uint16_t permart_length, uint8_t base64_encoded);

    int8_t npnt_init_handle(npnt_s *handle);

    int8_t npnt_reset_handle(npnt_s *handle);

    uint8_t* parseTag(char *permart, char* strTg, uint16_t* len,char** tagEndPtr, int isSelfEnd);

    int getValueInt(char* dataPtr, char* valueTag);

    float getValueFloat(char* dataPtr, char* valueTag);

    char* getValueStr(char* dataPtr, char* valueTag,char** outPtr);

    char* getValueStrPtr(char* dataPtr, char* valueTag);

    int8_t npnt_verify_permart(npnt_s *handle);

    int8_t npnt_alloc_and_get_fence_points(npnt_s* handle, float* vertx, float* verty);

    int8_t npnt_get_max_altitude(npnt_s* handle, float* altitude);

    int8_t npnt_populate_flight_params(npnt_s* handle);

    bool npnt_pnpoly(int nvert, float *vertx, float *verty, float testx, float testy);

    void npnt_ist_data_time_to_QString(struct tm date_time,char* str);
    int8_t npnt_check_we_are_in_DGCA_time(struct tm* flight_Start_tm,struct tm* flight_End_tm);
    int8_t npnt_ist_date_time_to_unix_time(const char* dt_string, struct tm* date_time);
//    char* npnt_get_attr(mxml_node_t *node, const char* attr);

    //npnpt_sequrity interface
    /**
* @file    inc/log_iface.h
* @brief   Interface definitions for NPNT Breach logging
* @{
*/

    // User Implemented Methods
    /**
* @brief   Checks if the raw data is authentic.
* @details Implementer of this method needs to check the authenticity
*          of raw data with signature against the public key provided
*          by DGCA Server.
*
* @param[in] npnt_handle        npnt handle
* @param[in] raw_data           signed raw data to be authenticated
* @param[in] raw_data_len       signed raw data to be authenticated
* @param[in] signature          signature of signed raw data
* @param[in] signature_len      length of signature
*
* @return           Errcode of authentication check, 0 if authentication was successful
* @retval 0         Successful Authentication
*
* @iclass security_iface
*/
    int8_t npnt_check_authenticity(uint8_t* hashed_data, uint16_t hashed_data_len, const uint8_t* signature, uint16_t signature_len);

    /**
* @brief   Signs raw data.
* @details Implementer of this method needs to sign the raw data
*          and signature against the private key generated in-system.
*
* @param[in] npnt_handle        npnt handle
* @param[in] raw_data           signed raw data to be authenticated
* @param[in] raw_data_len       signed raw data to be authenticated
* @param[in] signature          signature of signed raw data
* @param[in] signature_len      length of signature
* @param[out] signature_len     updated length of signature
*
* @return           Errcode of signature failure, 0 if signature was generated successfully
* @retval 0         Successfully Signed
*
* @iclass security_iface
*/
    int8_t npnt_sign_raw_data(npnt_s *handle, uint8_t* raw_data, uint16_t raw_data_len, uint8_t* signature, uint16_t* signature_len);


    //Implemented by libnpnt

    /**
* @brief   Initialise Security interface.
* @details This method calls the necessary methods to setup security
*          interface
*
* @param[in] npnt_handle        npnt handle
*
* @return           Errcode of failure, 0 if successful
* @retval 0         Iface Successfully Setup
*
* @iclass security_iface
*/
    int8_t npnt_security_init(npnt_s* handle);

};
#endif //NPNT_H

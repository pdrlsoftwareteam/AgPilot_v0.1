/*
 * keystore.cpp
 *
 *  Created on: 31-Jul-2019
 *      Author: Owner
 */
#include "AP_KEYSTORE.h"
#include <AP_LIBNPNT/AP_LIBNPNT.h>
#include <GCS_MAVLink/GCS.h>
#include <AP_PDRL_Commander/AP_PDRL_Commander_Logger.h>
//#include "stm32_util.h"

extern const AP_HAL::HAL& hal;
AP_KEYSTORE* AP_KEYSTORE::m_pInstance = 0;

// Start section pdrlPublicKey
const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmTQntCNsg/DbM5u9GqVf\n36RNBSt9K/wYGkrNFRBERnQMokFtULohykeBybEdw3E0GgELAJMLYC/JUUPwv1VI\nU3upH0of0AE6mAboZSTMV4qdqB5b4y0IUOpPmwP7X4PqxZj9ove/nBhiJpOetKP5\nBUxcTMeo1L3czf0pJ4xNkSvJSyfvZg1Y17K1x02hX8E53esc6oNPr0Q/wOuM3rz2\nopkkXygmi7ZpRa3qVSStviB0TcTtAshLpBNdeEJk1k6dDwMPp7pg0h3x3R1/8uyx\nkTNSVC5Mo7lidN9oJ9vaw5K/kDdBZeaH6znrNYuoKnOZG4cLfXatKM690SdlLogs\nkwIDAQAB\n-----END PUBLIC KEY-----\n\0";
// End section pdrlPublicKey

AP_KEYSTORE* AP_KEYSTORE::getInstance()
{
	if (!m_pInstance)   // Only allow one instance of class to be generated.
		m_pInstance = new AP_KEYSTORE;
	return m_pInstance;
}

AP_KEYSTORE::AP_KEYSTORE()
{
	mDevice = (keymaster_device_t)1;
	mState = KEYSTORE_IDEAL;
	strcpy((char*)salt,"salt");
	mRetry = 1;

	info_sha1 = mbedtls_md_info_from_type( MBEDTLS_MD_SHA1 );
	if( info_sha1 == NULL )
	{
		ret = 1;
	}

	if( ( ret = mbedtls_md_setup( &sha1_ctx, info_sha1, 1 ) ) != 0 )
	{
		ret = 1;
	}
}

AP_KEYSTORE::~AP_KEYSTORE()
{
	mbedtls_md_free( &sha1_ctx );
}

uint16_t AP_KEYSTORE::getKeyLen(KEY_TYPE keytype)
{
	uint16_t keylen = 0;
	switch(keytype)
	{
	case KEY_TYPE_PRIVATE:
		break;
	case KEY_TYPE_PUBLIC:
		break;
	case KEY_TYPE_EXTERNAL_PUBLIC:
		break;
	case KEY_PDRL_PUBLIC:
		break;
	}
	return keylen;
}

void AP_KEYSTORE::transferKey(int index,unsigned char *keyptr,uint16_t keyLen,uint16_t validDataLen)
{
	if(isGrantAccess == true)
	{
		memset(keyTransferBuf,0,sizeof(keyTransferBuf));
		memcpy(keyTransferBuf,keyptr+(KEY_TRANSFER_BLOCK_SIZE*index),KEY_TRANSFER_BLOCK_SIZE);
		keyTransferTXKeyLen = keyLen;
		keyTransferTXValidDataLen = validDataLen;
		keyTrasnFerTXBufferIndex = index;
		isKeyTransferTXBufferDirty = true;
		gcs().send_message(MSG_DATA_TRANSFER);
	}
}

void AP_KEYSTORE::receivKey(unsigned char *keyptr,uint16_t validDataLen,uint8_t bufferIndex)
{
	uint16_t numberOfBlockToSend = (( KEY_TRANSFER_BLOCK_SIZE-(validDataLen%KEY_TRANSFER_BLOCK_SIZE) )+validDataLen) / KEY_TRANSFER_BLOCK_SIZE;
	if( (numberOfBlockToSend > bufferIndex) && ( (keyTrasnFerRXBufferIndex+1) < bufferIndex ))
	{
		keyTrasnFerRXBufferIndex = 0;
		return;
	}
	keyTrasnFerRXBufferIndex = (uint16_t)bufferIndex;
	memcpy(extPublicKeyBuf+(bufferIndex*KEY_TRANSFER_BLOCK_SIZE),keyptr,KEY_TRANSFER_BLOCK_SIZE);

	if( (numberOfBlockToSend-1) == bufferIndex)
	{
//		storeKey(extPublicKeyBuf,validDataLen,KEY_TYPE_EXTERNAL_PUBLIC);
//		memset(extPublicKeyBuf,0,sizeof(extPublicKeyBuf));
//		uint16_t keyl = 0;
//		readKey(extPublicKeyBuf,&keyl,KEY_TYPE_EXTERNAL_PUBLIC);
	}
}

bool AP_KEYSTORE::verifyFirmwareSignature(uint8_t* firmwareSignature, uint8_t* firmwareHash)
{
	int err = 0;
	mbedtls_pk_context pk;

	mbedtls_pk_init( &pk );

	if( ( err = mbedtls_pk_parse_public_key( &pk,(const unsigned char*)pdrlPublicKey,strlen((char*)pdrlPublicKey)+1) ) != 0 )
	{
		gcs().send_text(MAV_SEVERITY_ERROR, "%s", "Firmware signature: verify failed");
		mbedtls_pk_free(&pk);
		return false;
	}

	if( ( err = mbedtls_pk_verify( &pk, MBEDTLS_MD_SHA256, firmwareHash,32, firmwareSignature, 256 ) ) != 0 )
	{
		gcs().send_text(MAV_SEVERITY_ERROR, "%s", "Firmware signature: verify failed");
		mbedtls_pk_free(&pk);
		return false;
	}

	gcs().send_text(MAV_SEVERITY_ERROR, "%s", "Firmware signature: verify success");
	mbedtls_pk_free(&pk);
	return true	;
}

namespace AP {

AP_KEYSTORE &key_store()
{
	return *AP_KEYSTORE::getInstance();
}

}

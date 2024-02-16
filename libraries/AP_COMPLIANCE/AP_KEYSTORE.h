/*
 * keystore.hpp
 *
 *  Created on: 31-Jul-2019
 *      Author: Owner
 */
#pragma once

#ifndef KEYSTORE_HPP_
#define KEYSTORE_HPP_

//#include <AP_HAL/AP_HAL.h>
//#include <AP_HAL/AP_HAL_PDRL.h>
#include <inttypes.h>
#include <AP_Common/AP_Common.h>
#include <AP_Param/AP_Param.h>
#include <AP_Math/AP_Math.h>
//#include <AP_Vehicle/AP_Vehicle.h>
//#include <AP_SerialManager/AP_SerialManager.h>
#include <AP_RTC/AP_RTC.h>
//#include <GCS_MAVLink/GCS.h>

//#include <string.h>
//#include <stdlib.h>

#include "../../modules/mbedtls/include/mbedtls/aes.h"
#include "../../modules/mbedtls/include/mbedtls/ctr_drbg.h"
#include "../../modules/mbedtls/include/mbedtls/entropy.h"
#include "../../modules/mbedtls/include/mbedtls/pk.h"
#include "../../modules/mbedtls/include/mbedtls/pkcs5.h"
#include "../../modules/mbedtls/include/mbedtls/platform.h"
#include "../../modules/mbedtls/include/mbedtls/rsa.h"
#include "../../modules/mbedtls/include/mbedtls/memory_buffer_alloc.h"

//#include <AP_Direct_Flash_Access/APDirectFlashAccess.h>

extern const char* pdrlPublicKey;

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
#ifndef Debug
	# define Debug(fmt, args ...)
#endif
#endif

#define KEY_SIZE 2048
#define EXPONENT 3
#define AES_KEY_SIZE 16

//First 2kb
#define KEY_STORE_STARTING_SECTOR 		PDRL_SPECIFIC_FLASH_SECTOR
#define FLASH_VOLTAGE_RANGE				PDRL_FLASH_VOLTAGE_RANGE_3

#if defined(STM32H743xx) || defined(STM32H757xx)
#define KEY_STORE_HASH_PASSWORD_ADD		(uint32_t)0x081C0000
#endif


#ifdef STM32F767xx
#define KEY_STORE_HASH_PASSWORD_ADD		(uint32_t)0x081C0000
#endif

#ifdef STM32F427xx
#define KEY_STORE_HASH_PASSWORD_ADD		(uint32_t)0x081E0000
#endif


#ifdef STM32H743xx

#define KEY_STORE_PASSWORD_ADD 			KEY_STORE_HASH_PASSWORD_ADD+32

//for flag managment
#define KEY_STORE_ISKEYPAIR_FLAGS 		        KEY_STORE_PASSWORD_ADD+32  //add of flags
#define KEY_STORE_ISCOMMAND_FOR_GENARATE_KEY 	KEY_STORE_ISKEYPAIR_FLAGS+32  //add of flags
#define IS_ALLOWED_TO_FLASH_FIRMWARE 	        KEY_STORE_ISCOMMAND_FOR_GENARATE_KEY+32  //add of flags

#define KEY_STORE_PUBLIC_KEY_LEN_ADD 	IS_ALLOWED_TO_FLASH_FIRMWARE+32		//add of public key length var
#define KEY_STORE_PRIVATE_KEY_LEN_ADD 	KEY_STORE_PUBLIC_KEY_LEN_ADD+  32//add of private key length var
#define KEY_STORE_EXTERNAL_PUBLIC_KEY_LEN_ADD 	KEY_STORE_PRIVATE_KEY_LEN_ADD+  32//add of external key length var

#else

#define KEY_STORE_PASSWORD_ADD 			KEY_STORE_HASH_PASSWORD_ADD+16

//for flag managment
#define KEY_STORE_ISKEYPAIR_FLAGS 		        KEY_STORE_PASSWORD_ADD+16  //add of flags
#define KEY_STORE_ISCOMMAND_FOR_GENARATE_KEY 	KEY_STORE_ISKEYPAIR_FLAGS+16  //add of flags
#define IS_ALLOWED_TO_FLASH_FIRMWARE 	        KEY_STORE_ISCOMMAND_FOR_GENARATE_KEY+16  //add of flags

#define KEY_STORE_PUBLIC_KEY_LEN_ADD 	IS_ALLOWED_TO_FLASH_FIRMWARE+16		//add of public key length var
#define KEY_STORE_PRIVATE_KEY_LEN_ADD 	KEY_STORE_PUBLIC_KEY_LEN_ADD+  16//add of private key length var
#define KEY_STORE_EXTERNAL_PUBLIC_KEY_LEN_ADD 	KEY_STORE_PRIVATE_KEY_LEN_ADD+  16//add of external key length var

#endif //STM32H743xx flash address
//2-32KB -> 30Kb
#define KEY_STORE_PUBLIC_KEY_ADD 					KEY_STORE_PASSWORD_ADD+(1024*2)					// add of
#define KEY_STORE_PRIVATE_KEY_ADD 					KEY_STORE_PASSWORD_ADD+(1024*15)
#define KEY_STORE_EXTERNAL_PUBLIC_KEY				KEY_STORE_PASSWORD_ADD+(1024*30)

#define getPadZeroCount(x) (x % 16)

#define KEY_TRANSFER_BLOCK_SIZE 250

#define ENABLE_PA 0
#define ENABLERSA_KEY 0

typedef  uint8_t keymaster_device_t;
typedef uint32_t Entropy;
typedef uint32_t Value;
typedef enum KEYSTORESTATE
{
	KEYSTORE_IDEAL,
	KEYSTORE_KEY_GENARATED,
	KEYSTORE_OPEN,
	KEYSTORE_CLOSED,
}KeyStoreState;

typedef union AES_KEY
{
	uint8_t AES_KEY_u8[16];
	uint32_t AES_KEY_u32[4];
}AES_KEY;

typedef enum ResponseCode
{
	ERR = -1,
	OK,
	RESP_KEY_PAIR_PRESENT,
	RESP_NO_KEY
}ResponseCode;

typedef enum grant_t
{
	access_decline,
	access_granted
}grant_t;

typedef enum KEY_TYPE{
	KEY_TYPE_PRIVATE,
	KEY_TYPE_PUBLIC,
	KEY_TYPE_EXTERNAL_PUBLIC,
	KEY_PDRL_PUBLIC
}KEY_TYPE;

typedef enum KEY_TRANSFER_STATE{
	KEY_TRANSFER_IDEAL,
	KEY_TRANSFER_GENARATE_KEY,
	KEY_TRANSFER_SEND_KEY,
}KEY_TRANSFER_STATE;

typedef struct listnode{
	struct listnode* nextNode;
}listnode;

class AP_KEYSTORE{

//private:
public:
	uint16_t ret = 0;
	size_t plen = 16 ;
	uint8_t password[32];
	size_t slen = 4;
	uint8_t salt[40];
	uint32_t it_cnt = 1;
	uint32_t key_len = 16;
	AP_Int16 testParamKeystore;

	uint8_t isPasswordPresent = 0;
	bool isKeyPairPresent = 0;
	uint8_t isKeyPairGenarationCommand = 0;
	uint8_t isAllowedToFlashFirmware = 0;
	uint8_t isGrantAccess = 0;

	uint8_t result_key[32] =
	{ 0x0c, 0x60, 0xc8, 0x0f, 0x96, 0x1f, 0x0e, 0x71,
			0xf3, 0xa9, 0xb5, 0x24, 0xaf, 0x60, 0x12, 0x06,
			0x2f, 0xe0, 0x37, 0xa6
	};

	static const char* MASTER_KEY_FILE;
	static const int MASTER_KEY_SIZE_BYTES = AES_KEY_SIZE;
	static const int MASTER_KEY_SIZE_BITS = AES_KEY_SIZE * 8;
	static const int MAX_RETRY = 4;
	static const size_t SALT_SIZE = 4;
	Entropy* mEntropy = 0;	// randomeness genarator
	keymaster_device_t mDevice;
	KeyStoreState mState;
	int8_t mRetry;
	uint8_t mMasterKey[MASTER_KEY_SIZE_BYTES];
	uint8_t mSalt[SALT_SIZE] ;
	AES_KEY mMasterKeyEncryption;
	AES_KEY mMasterKeyDecryptiol;
	struct listnode mGrants;

	//SHA
	mbedtls_md_context_t sha1_ctx;
	const mbedtls_md_info_t *info_sha1;
	//RSA
	mbedtls_rsa_context rsa={0}, *rsaPtr = 0;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;

	//flash access
	AP_Direct_Flash_Access *m_AP_Direct_Flash_Access;

	bool aesEncryptAndStoreBuffer(unsigned char *keyBuf, uint16_t keylen,uint32_t keyStoreAddress,uint32_t keyLenstoreAddress);
	bool aesDecryptAndReadBuffer(unsigned char *keyBuf, uint16_t *keylen,uint32_t keyStoreAddress,uint32_t keyLenstoreAddress);

	static AP_KEYSTORE* m_pInstance;
	AP_KEYSTORE();
	~AP_KEYSTORE();

public:
	unsigned char privateKeyBuf[1700];
	uint8_t temp5kBuff[500];
	unsigned char extPublicKeyBuf[500];
#if ENABLE_PA
	char paXmlBuffer[8096];
#endif

	unsigned char keyTransferBuf[KEY_TRANSFER_BLOCK_SIZE];

	volatile uint16_t keyTransferTXKeyLen = 0;
	volatile uint16_t keyTransferTXValidDataLen  = 0;
	volatile uint16_t keyTrasnFerTXBufferIndex = 0;
	volatile bool isKeyTransferTXBufferDirty = 0;

	volatile uint8_t ackCommand = 0;
	volatile uint8_t ackCommandType = 0;
	volatile uint8_t ack = 0;

	uint16_t keyTransferRXKeyLen = 0;
	uint16_t keyTransferRXValidDataLen  = 0;
	volatile uint8_t keyTrasnFerRXBufferIndex = 0;



	uint8_t keyTransferTest = KEY_TRANSFER_IDEAL;//KEY_TRANSFER_GENARATE_KEY;//KEY_TRANSFER_IDEAL;

	static const struct AP_Param::GroupInfo var_info[];

	static AP_KEYSTORE* getInstance();

	void transferKey(int index,unsigned char *keyptr ,uint16_t keyLen,uint16_t validDataLen);
	void receivKey(unsigned char *keyptr,uint16_t validDataLen,uint8_t index);
	void receivPAXML(char *paXmlPtr,uint16_t validDataLen,uint8_t bufferIndex);
	void openKeystore();
	void closeKeystore();
	bool isKeyStorePasswordPresent();
	void getPlainPassword(uint8_t* pswd);
	void genaratePasswordPhrase(uint8_t* pswd);
	void getPassword(uint8_t* pswd);

	void setState(KeyStoreState state);
	bool generateSalt(char *salt,uint16_t * len);
	bool generateMasterKey();
	void setupMasterKeys();
	void clearMasterKeys();
	void eraseKeyStoreData();
	void generateKeyFromPassword(uint8_t* pswd);
	void setAllowedFlashFirmware();
	uint8_t getAllowedFlashFirmware();
	bool verifyFirmwareSignature(uint8_t* firmwareSignature, uint8_t* firmwareHash);
	//	void generateKeyFromPassword(uint8_t* key, ssize_t keySize, Value* pw, uint8_t* salt);
	void clearePassword();
	bool isPasswordGenarated();
	bool isKeyPairGenarated();
	bool getGrant(uint8_t* pswd, uint16_t pswdlen);
	bool convertToUid(const Value* uidValue, uid_t* uid);
	void setGenarateKeyFlagForReboot();
	uint16_t getKeyLen(KEY_TYPE keytype);
	uint16_t bufferPadZero(unsigned char *keyBuf, uint16_t keylen);

	bool storeKey(unsigned char *keyBuf, uint16_t keylen,KEY_TYPE keytype);
	bool readKey(unsigned char *keyBuf, uint16_t* keylen, KEY_TYPE keytype);
	bool getKeyStoreEncryptionPassword(uint8_t* pswd);

	//	AP_KEYSTORE(Entropy* entropy, keymaster_device_t* device);
	bool creatRSAKey();
	KeyStoreState getState();
	int8_t getRetry();
	keymaster_device_t* getDevice();
	ResponseCode initialize(Value* pw);
	ResponseCode writeMasterKey(Value* pw);
	ResponseCode readMasterKey(Value* pw);
	bool reset();
	bool isEmpty();
	void lock();
	//	ResponseCode get(const char* filename, Blob* keyBlob);
	//	ResponseCode put(const char* filename, Blob* keyBlob);
	void addGrant(const char* filename, const Value* uidValue);
	bool removeGrant(const Value* keyValue, const Value* uidValue);
	bool hasGrant(const Value* keyValue, const uid_t uid);

};

extern AP_KEYSTORE *keyStore;
extern const char* pdrlPublicKey;
namespace AP {
	AP_KEYSTORE &key_store();
}

#endif /* KEYSTORE_HPP_ */

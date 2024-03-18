/*
 * keystore.cpp
 *
 *  Created on: 31-Jul-2019
 *      Author: Owner
 */
#include "AP_LIBNPNT.h"
#include <AP_KEYSTORE/AP_KEYSTORE.h>
#include <GCS_MAVLink/GCS.h>
#include <AP_PDRL_Commander/AP_NPNT_data_helper.h>
#define  TODO 0

AP_LIBNPNT* AP_LIBNPNT::m_pInstance = NULL;

AP_LIBNPNT* AP_LIBNPNT::getInstance()
{
	if (!m_pInstance)   // Only allow one instance of class to be generated.
		m_pInstance = new AP_LIBNPNT;
	//	m_AP_NPNT_data_helper = AP_NPNT_data_helper::getInstance();
	return m_pInstance;
}

AP_LIBNPNT::AP_LIBNPNT()
{

}

AP_LIBNPNT::~AP_LIBNPNT()
{

}

void AP_LIBNPNT::free_common()
{
#if OPENSSL
	//Free up all structures
	EVP_PKEY_free(pkey);
	EVP_PKEY_free(permart_pkey);
	EC_KEY_free(ecckey);
	BIO_free_all(outbio);
#else
#endif
}


char* AP_LIBNPNT::getStringBetweenTags(char* s,char* PATTERN1,char* PATTERN2)
{
	char *target = NULL;
	char *start, *end;

	if ( (start = strstr( s, PATTERN1 ) ))
	{
		start += strlen( PATTERN1 );
		if ( (end = strstr( start, PATTERN2 )) )
		{
			for(int i =0 ; i<5;i++)
				if((*end == '\n') || (*end == '\r'))
					end--;
			target = ( char * )malloc( end - start + 1 +strlen("-----BEGIN CERTIFICATE-----\n")+strlen("\n-----END CERTIFICATE-----"));
			strcpy(target,(char*)"-----BEGIN CERTIFICATE-----\n");
			memcpy(target+strlen("-----BEGIN CERTIFICATE-----\n"), start, end - start );
			strcat(target,(char*)"\n-----END CERTIFICATE-----\0");
		}
	}

	if ( target )
	{
		return target;
	}

	//	free( target );

	return 0;
}
int16_t AP_LIBNPNT::extract_public_key_from_xml_artefact(char *buffer)
{
#if OPENSSL
	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();

	//    outbio  = BIO_new(BIO_s_file());
	outbio = BIO_new_fp(stdout, BIO_NOCLOSE);

	mxml_node_t *permart, *certificate;
	X509 *cert = NULL;
	BIO *cert_bio;
	uint32_t fileLen = 0;

	buffer = m_fileHelper->getFileToChar((char*)m_fileHelper->getpermissionArtifactFilePath().toStdString().c_str(),&fileLen);
	if(buffer == 0)
	{
		return -1;
	}

	permart = mxmlLoadString(NULL,buffer, MXML_OPAQUE_CALLBACK);

	if (permart == NULL)
	{
		return -1;
	}

	certificate = mxmlFindElement(permart, permart, "X509Certificate", NULL, NULL, MXML_DESCEND);
	if (certificate == NULL)
	{
		return -1;
	}

	const char* cert_der = mxmlGetOpaque(certificate);
	if (cert_der == NULL)
	{
		mxmlDelete(certificate);
		return -1;
	}

	cert_bio = BIO_new(BIO_s_mem());

	if(cert_bio == NULL)
	{
		mxmlDelete(certificate);
		return -1;
	}

	BIO_printf(cert_bio, "-----BEGIN CERTIFICATE-----\n%s-----END CERTIFICATE-----\n", cert_der);

	cert = PEM_read_bio_X509(cert_bio, NULL, 0, NULL);
	if (cert == NULL)
	{
		BIO_free_all(cert_bio);
		mxmlDelete(certificate);
		return -1;
	}

	permart_pkey = X509_get_pubkey(cert);
	if (permart_pkey == NULL)
	{
		BIO_free_all(cert_bio);
		mxmlDelete(certificate);
		X509_free(cert);
		return -1;
	}

	FILE* pkey_fp = fopen(m_fileHelper->getdgcaPublicKeyFilePath().toStdString().c_str(), "w");

	if(pkey_fp == NULL)
	{
		//        qDebug() << "DGC Public key file not opened";
	}

	PEM_write_PUBKEY(pkey_fp, permart_pkey);
	fclose(pkey_fp);

	BIO_free_all(cert_bio);
	mxmlDelete(certificate);
	X509_free(cert);
#else
	char* certificate = getStringBetweenTags(buffer,(char*)"<X509Certificate>",(char*)"</X509Certificate>");
	if (certificate == NULL)
	{
		free(certificate);
		return -1;
	}
	uint16_t crtLen = strlen(certificate);
	memset(&npnt_helpers.x509Cert,0,sizeof(npnt_helpers.x509Cert));
	if(0 != mbedtls_x509_crt_parse(&npnt_helpers.x509Cert,(const unsigned char *)certificate,crtLen+1))
	{
		free(certificate);
		return -1;
	}

	free(certificate);
#endif
	return 0;
}

int16_t AP_LIBNPNT::verifyValidGeofence()
{
#if TODO
	int listNotMatch = 0;
	if(m_npnt_data_helper->getMissionWayPointsCount() == npnt_handle.cordinateList.count())
	{
		if(m_npnt_data_helper->getMissionWayPointsCount() > 0)
		{
			for (int i = 0; i < m_npnt_data_helper->getMissionWayPointsList().count();i++)
			{
				//                qDebug() <<"Actual Lat "<<i<<" " << QString::number(m_npnt_data_helper->getMissionWayPointsList().at(i).latitude()) << " == PA Lat "<<QString::number(npnt_handle.cordinateList.at(i).latitude());
				//                qDebug() <<"Actual Lon "<<i<<" " << QString::number(m_npnt_data_helper->getMissionWayPointsList().at(i).longitude()) << " == PA Lon "<<QString::number(npnt_handle.cordinateList.at(i).longitude())<<"\n";
				if(QString::number(m_npnt_data_helper->getMissionWayPointsList().at(i).longitude()) == QString::number(npnt_handle.cordinateList.at(i).longitude()) )
				{
					listNotMatch = 1;
				}
				if(QString::number(m_npnt_data_helper->getMissionWayPointsList().at(i).longitude()) == QString::number(npnt_handle.cordinateList.at(i).longitude()) )
				{
					listNotMatch = 1;
				}
			}
			if(listNotMatch == 1)
			{
				return 0;
			}
		}
	}
	return NPNT_BAD_FENCE;
#endif
	return 0;
}

int16_t AP_LIBNPNT::load_artifact(char * buffer,uint16_t dataLen)
{
	//    uint8_t *base64_permart = NULL;
	int16_t ret = NPNT_INV_ART;

	if(extract_public_key_from_xml_artefact(buffer) == 0)
	{

	npnt_helpers.npnt_init_handle(&npnt_handle);
	ret = npnt_helpers.npnt_set_permart(&npnt_handle,(uint8_t*)buffer, dataLen, false);

	if (!npnt_handle.params.permissionArtifactID) {
		npnt_handle.params.permissionArtifactID = (char*) calloc(1,37);
		memcpy((void*)npnt_handle.params.permissionArtifactID,(void*)AP_NPNT_data_helper::getInstance()->permissionArticaftID,37);
	}

	}
	switch (ret)
	{
	case NPNT_INV_ART:
		libNpntStatus = STATUS_VERIFY_PA_FAILED;
		break;

	case NPNT_INV_TIME:
		break;

	case 0:
	{
		if((ret = verifyValidGeofence()) == 0)
		{
			libNpntStatus = STATUS_VERIFY_PA;
		}
		else
		{
			libNpntStatus = NPNT_BAD_FENCE;
		}
	}
	break;
	default:
		libNpntStatus = STATUS_VERIFY_PA_FAILED;
		break;
	}

	gcs().send_message(MSG_LIBNPNT_TRANSFER);
	return ret;
}

int AP_LIBNPNT::authenticateSinglePA(char* paBuffer,uint16_t dataLen)
{

	paValidStatus = false;
	libNpntStatus = STATUS_VERIFY_PA_FAILED;
	return load_artifact(paBuffer,dataLen);
}

bool AP_LIBNPNT::getIsPAValid(bool silent)
{
	if(libNpntStatus == STATUS_VERIFY_PA)
		return true;
	if(silent == false)
		gcs().send_text(MAV_SEVERITY_ERROR, "Invalid permission artifacts.");
	return false;
}

void AP_LIBNPNT::setIsPAValid(LIB_NPNT_STATUS val)
{
	libNpntStatus = val;
}

void AP_LIBNPNT::showMessageTimebreach()
{
	gcs().send_text(MAV_SEVERITY_ERROR, "NPNT Time breach");
	setIsPAValid(STATUS_NPNT_INV_TIME);
}

void AP_LIBNPNT::showMessageFencebreach()
{
	gcs().send_text(MAV_SEVERITY_ERROR, "NPNT Fence breach");
}

void AP_LIBNPNT::sendPAvalidationResponse(mavlink_channel_t chan)
{

	uint8_t txBUff[250]={0};
	txBUff[0] = (uint8_t)libNpntStatus;
	mavlink_msg_data_transfer_send(
			chan,
			2, //data_type_libnpnt
			0,
			txBUff,
			1
	);
}

void AP_LIBNPNT::receiveHash(unsigned char *hashPtr,uint16_t validDataLen)
{
	memcpy(firmwareHash,hashPtr,validDataLen);
}

void AP_LIBNPNT::receiveSignature(unsigned char *signaturePtr,uint16_t validDataLen,uint8_t bufferIndex)
{
	memcpy(signature+(bufferIndex*128),signaturePtr,validDataLen);
	if(bufferIndex == 1)
		keyStore->verifyFirmwareSignature(signature,firmwareHash);
}

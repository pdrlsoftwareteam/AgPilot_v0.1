#include "npnt_helpers.h"

npnt_helpers_c::npnt_helpers_c()
{

}

void npnt_helpers_c::reset_sha256()
{
#if OPENSSL
	SHA256_Init(&sha);
#else
	mbedtls_sha256_init(&sha);
	mbedtls_sha256_starts(&sha, 0);
#endif
}

void npnt_helpers_c::update_sha256(const char* data, uint16_t data_len)
{
#if OPENSSL
	SHA256_Update(&sha, data, data_len);
#else
	mbedtls_sha256_update(&sha,(unsigned char*)data,data_len);
#endif
}

void npnt_helpers_c::final_sha256(char* hash)
{
#if OPENSSL
	SHA256_Final((unsigned char*)hash, &sha);
#else
	mbedtls_sha256_finish(&sha,(unsigned char*)hash);
#endif
}


int8_t npnt_helpers_c::npnt_check_authenticity(uint8_t* digest_value, uint16_t digest_value_data_len, const uint8_t* signature, uint16_t signature_len)
{

#if OPENSSL
	int ret =0;
	if (!handle || !raw_data || !signature) {
		return -1;
	}
	if (dgca_pkey == NULL) {
		FILE *fp = fopen("NPNT/dgcaPubliceKey.pem", "r");
		if (fp == NULL) {
			return -1;
		}
		dgca_pkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
	}
	dgca_pkey_ctx = EVP_PKEY_CTX_new(dgca_pkey, ENGINE_get_default_RSA());
	if (!dgca_pkey_ctx) {
		return -1;
	}
	int ret = 0;
	if (EVP_PKEY_verify_init(dgca_pkey_ctx) <= 0) {
		ret = -1;
		goto fail;
	}
	if (EVP_PKEY_CTX_set_rsa_padding(dgca_pkey_ctx, RSA_PKCS1_PADDING) <= 0) {
		ret = -1;
		goto fail;
	}
	if (EVP_PKEY_CTX_set_signature_md(dgca_pkey_ctx, EVP_sha256()) <= 0) {
		ret = -1;
		goto fail;
	}

	/* Perform operation */
	ret = EVP_PKEY_verify(dgca_pkey_ctx, signature, signature_len, raw_data, raw_data_len);

	fail:
	EVP_PKEY_CTX_free(dgca_pkey_ctx);

	return ret;
#else
	const unsigned char dgcaPubKey[] = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAiriSSEDgoSs497l713Mw\ntj/tqs/xZIRl8FknYNq44Mn2FZxbXOJgazE17MHRXWEblJR8ZBgKOh5j6/LIz/aa\nULmXNiyGueiwi82icMhK0w+omO7cI3DoOLMykqf3LoGgM1np1HM09eoybDcvNHmk\nn+OBMxgILRw3TUL9glNd8fYjQvIjeUgZ3kKHdcwg6fW4CP3wbwG+WSTskOk+HT9r\n2E/inX0DQirrDHwtlOQjyB6wcqMG3LfwhQ7btoQKFnankYSggSX4VgjJWJDLEe2u\n/Qvovf1ksn/xiJrjYXuSmS59FLlowcQjfdWp2ZYp1GcxA5IOGDc5/p12P6xMKIXK\nVQIDAQAB\n-----END PUBLIC KEY-----\n\0";
	int ret = 0;
	mbedtls_pk_context pk;

	mbedtls_pk_init( &pk );

	if( ( ret = mbedtls_pk_parse_public_key( &pk,(const unsigned char*)dgcaPubKey,strlen((char*)dgcaPubKey)+1) ) != 0 )
	{
		mbedtls_pk_free(&pk);
		return -1;
	}

	//    if( ( ret = mbedtls_md(mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),signature, digest_value ) ) != 0 )
	//    {
	//    	return -1;
	//    }

	if( ( ret = mbedtls_pk_verify( &pk, MBEDTLS_MD_SHA256, digest_value,digest_value_data_len, signature, signature_len ) ) != 0 )
	{
		mbedtls_pk_free(&pk);
		//		return -1;
	}


#endif
	mbedtls_pk_free(&pk);
	return 1;
}

int8_t npnt_helpers_c::npnt_init_handle(npnt_s *handle)
{
	if (!handle) {
		return NPNT_UNALLOC_HANDLE;
	}

	if(handle->params.permissionArtifactID != NULL)
		free(handle->params.permissionArtifactID);

	if(handle->params.adcNumber != NULL)
			free(handle->params.adcNumber);

	if(handle->params.ficNumber != NULL)
			free(handle->params.ficNumber);

	handle->params.adcNumber = NULL;
	handle->params.ficNumber = NULL;
	handle->params.permissionArtifactID = NULL;

	handle->raw_permart = NULL;
	handle->raw_permart_len = 0;
	handle->security_handle = NULL;
	return 0;
}


int8_t npnt_helpers_c::npnt_reset_handle(npnt_s *handle)
{
	if (!handle) {
		return NPNT_UNALLOC_HANDLE;
	}

	if (handle->raw_permart) {
		free(handle->raw_permart);
	}

	if (handle->fence.vertlat) {
		free(handle->fence.vertlat);
	}

	if (handle->fence.vertlon) {
		free(handle->fence.vertlon);
	}

	if (handle->params.uinNo) {
		free(handle->params.uinNo);
	}

	if (handle->params.adcNumber) {
		free(handle->params.adcNumber);
	}

	if (handle->params.ficNumber) {
		free(handle->params.ficNumber);
	}

	memset(handle, 0, sizeof(npnt_s));

	return 0;
}

/*
 *  The point in polygon algorithm is based on:
 *  http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
 */
bool npnt_helpers_c::npnt_pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
{
	int i, j, c = 0;
	for (i = 0, j = nvert-1; i < nvert; j = i++) {
		if (((verty[i]>testy) != (verty[j]>testy)) &&
				(testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) ) {
			c = !c;
		}
	}
	return c;
}

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
 *         NPNT_INV_AUTH  signed by unauthorised entity
 *         NPNT_INV_STATE artefact can't setup in current aircraft state
 *         NPNT_ALREADY_SET artefact already set, free previous artefact first
 * @iclass control_iface
 */
int8_t npnt_helpers_c::npnt_set_permart(npnt_s *handle, uint8_t *permart, uint16_t permart_length, uint8_t base64_encoded)
{
	if (!handle)
	{
		return NPNT_UNALLOC_HANDLE;
	}
	int16_t ret = 0;
	//Extract XML from base64 encoded permart
	//	if (handle->raw_permart)
	//	{
	//		return NPNT_ALREADY_SET;
	//	}

	handle->raw_permart =(char*)permart;

	//	handle->raw_permart = (char*)calloc(1,permart_length);
	//	if (!handle->raw_permart) {
	//		return NPNT_PARSE_FAILED;
	//	}
	//	memcpy(handle->raw_permart, permart, permart_length);


	//parse XML permart
	//	handle->parsed_permart = mxmlLoadString(NULL, handle->raw_permart, MXML_OPAQUE_CALLBACK);

	//	if (!handle->parsed_permart) {
	//		return NPNT_PARSE_FAILED;
	//	}

	//Verify Artifact against Sender's Public Key
	ret = npnt_verify_permart(handle);
	if (ret < 0)
	{
		return ret;
	}

	//Collect Fence points from verified artefact
	ret = npnt_alloc_and_get_fence_points(handle, handle->fence.vertlat, handle->fence.vertlon);
	if (ret <= 0) {
		handle->fence.nverts = 0;
		return NPNT_BAD_FENCE;
	}
	handle->fence.nverts = ret;

	//Get Max Altitude
	ret = npnt_get_max_altitude(handle, &handle->fence.maxAltitude);
	if (ret < 0) {
		//        return NPNT_INV_BAD_ALT;
	}

	//Set Flight Params from artefact
	ret = npnt_populate_flight_params(handle);
	if (ret < 0) {
		handle->fence.nverts = 0;
		//        return NPNT_INV_FPARAMS;
	}
	//    ret = 0;
	return ret;
}

uint8_t* npnt_helpers_c::parseTag(char *permart, char* strTg, uint16_t* len,char** tagEndPtr, int isSelfEnd)
{
	char starTag[20] = "<";
	char endTg[20] = {0};
	char *strPtr,*endPtr;

	strcat(starTag,strTg);
	if (0 == isSelfEnd)
	{
		strcpy(endTg,(char*)"</");
		strcat(endTg,strTg);
	}
	else if(1 == isSelfEnd)
		strcpy(endTg,(char*)"/>");
	else if (2 == isSelfEnd)
	{
		strcpy(endTg,(char*)">");
	}

	strPtr = strstr(permart, starTag);
	if(!strPtr)
	{
		//printf("No star tag");
		*len =0;
		return 0;
	}

	endPtr = strstr(strPtr, endTg);
	if(!endPtr)
	{
		//printf("No end tag");
		*len =0;
		return 0;
	}


	if(strPtr && endPtr)
	{
		strPtr = strPtr+strlen(strTg);
		if(0 == isSelfEnd)
		{
			strPtr = strstr(strPtr,(char*)">");
		}
		else
			strPtr = strPtr +1;

		*len = endPtr-strPtr-1;
		if(tagEndPtr!=0)
			*tagEndPtr = endPtr+1;
	}
	else
	{
		*len =0;
		return 0;
	}


	return (uint8_t*)strPtr+1;
}

int npnt_helpers_c::getValueInt(char* dataPtr, char* valueTag)
{
	char* marker = strchr(strstr(dataPtr,valueTag), '=')+2;
	if(marker)
		return atoi(marker);

	return 0;
}

float npnt_helpers_c::getValueFloat(char* dataPtr, char* valueTag)
{
	char* marker = strchr(strstr(dataPtr,valueTag), '=')+2;
	if(marker)
		return atof(marker);

	return 0;
}

char* npnt_helpers_c::getValueStr(char* dataPtr, char* valueTag,char** outPtr)
{
	int len = 0;
	char* marker = strstr(dataPtr,valueTag);
	if(marker)
	{
		marker = strchr(marker, '=') + 2;
		len = strchr(marker, '\"') - marker;
		if(len > 0)
		{
			*outPtr = (char*) calloc(1,len);
			strncpy(*outPtr,marker,len);
		}
	}
	else
	{
		*outPtr = 0;
		return 0;
	}

	return marker;
}

char* npnt_helpers_c::getValueStrPtr(char* dataPtr, char* valueTag)
{
	char* marker = strstr(dataPtr,valueTag);
	if(marker)
	{
		marker = strchr(marker, '=') + 2;
	}
	else
	{
		return 0;
	}

	return marker;
}

uint8_t* npnt_helpers_c::base64_encode(uint8_t *src, uint32_t len, uint16_t *out_len)
{
	char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	uint8_t *out, *pos;
	const uint8_t *end, *in;
	uint16_t olen;
	int16_t line_len;

	olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
	olen += olen / 72; /* line feeds */
	olen++; /* nul termination */
	if (olen < len) {
		return 0; /* integer overflow */
	}
	out = (uint8_t*)malloc(olen);
	if (out == 0) {
		return 0;
	}

	end = src + len;
	in = src;
	pos = out;
	line_len = 0;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
		line_len += 4;
		if (line_len >= 72) {
			//*pos++ = '\n';
			line_len = 0;
		}
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		} else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
								  (in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
		line_len += 4;
	}

	if (line_len) {
		//*pos++ = '\n';
	}

	*pos = '\0';
	if (out_len) {
		*out_len = pos - out;
	}
	return out;
}


/**
 * base64_decode - Base64 decode
 * @src: Data to be decoded
 * @len: Length of the data to be decoded
 * @out_len: Pointer to output length variable
 * Returns: Allocated buffer of out_len bytes of decoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
uint8_t* npnt_helpers_c::base64_decode(uint8_t *src, uint16_t len, uint16_t *out_len)
{
	char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	uint8_t dtable[256], *out, *pos, block[4], tmp;
	uint16_t i, count, olen;
	int16_t pad = 0;

	memset(dtable, 0x80, 256);
	for (i = 0; i < sizeof(base64_table) - 1; i++) {
		dtable[(uint8_t) base64_table[i]] = (uint8_t) i;
	}
	dtable['='] = 0;

	count = 0;
	for (i = 0; i < len; i++) {
		if (dtable[src[i]] != 0x80) {
			count++;
		}
	}

	if (count == 0 || count % 4) {
		return 0;
	}

	olen = count / 4 * 3;
	pos = out = (uint8_t*)malloc(olen);
	if (out == 0) {
		return 0;
	}

	count = 0;
	for (i = 0; i < len; i++) {
		tmp = dtable[src[i]];
		if (tmp == 0x80) {
			continue;
		}

		if (src[i] == '=') {
			pad++;
		}
		block[count] = tmp;
		count++;
		if (count == 4) {
			*pos++ = (block[0] << 2) | (block[1] >> 4);
			*pos++ = (block[1] << 4) | (block[2] >> 2);
			*pos++ = (block[2] << 6) | block[3];
			count = 0;
			if (pad) {
				if (pad == 1) {
					pos--;
				} else if (pad == 2) {
					pos -= 2;
				} else {
					/* Invalid padding */
					free(out);
					return 0;
				}
				break;
			}
		}
	}

	*out_len = pos - out;
	return out;
}

//Verify the data contained in parsed XML
int8_t npnt_helpers_c::npnt_verify_permart(npnt_s *handle)
{
	char* raw_perm_without_sign;
	char* signed_info;
	uint8_t* rcvd_digest_value;
	// char *test_str;
	int16_t permission_length =0 , signedinfo_length = 0;
	char digest_value[32] = {0};
	uint8_t* signature = NULL;
	uint8_t* raw_signature = NULL;
	uint16_t signature_len = 0, raw_signature_len = 0;
	//    uint16_t base64_digest_value_len = 0;
	uint16_t curr_ptr = 0, curr_length = 0;
	char last_empty_element[32];
	int8_t ret = 0;
	//	uint16_t i = 0;

	reset_sha256();

	update_sha256("<SignedInfo xmlns=\"http://www.w3.org/2000/09/xmldsig#\">",
			strlen("<SignedInfo xmlns=\"http://www.w3.org/2000/09/xmldsig#\">"));

	signed_info = strstr(handle->raw_permart, "<SignedInfo>") + strlen("<SignedInfo>");

	if (signed_info == NULL) {
		ret = NPNT_INV_ART;
		return ret;
	}
	signedinfo_length = strstr(handle->raw_permart, "<SignatureValue") - signed_info;
	if (signedinfo_length < 0) {
		ret = NPNT_INV_ART;
		return ret;
	}

	while (curr_ptr < signedinfo_length) {
		curr_length = 1;
		if (signed_info[curr_ptr] == '<') {
			while((curr_ptr + curr_length) < signedinfo_length) {
				if (signed_info[curr_ptr + curr_length] == ' ') {
					last_empty_element[curr_length - 1] = '\0';
					break;
				} else if (signed_info[curr_ptr + curr_length] == '>') {
					last_empty_element[0] = '\0';
					break;
				}
				last_empty_element[curr_length - 1] = signed_info[curr_ptr + curr_length];
				curr_length++;
			}
		}

		if (strlen(last_empty_element) != 0) {
			if (signed_info[curr_ptr] == '/') {
				if (signed_info[curr_ptr + 1] == '>') {
					update_sha256("></", 3);
					update_sha256(last_empty_element, strlen(last_empty_element));
					last_empty_element[0] = '\0';
					curr_ptr += curr_length;
					continue;
				}
			}
		}

		update_sha256(&signed_info[curr_ptr], curr_length);

		curr_ptr += curr_length;
	}
	final_sha256(digest_value);

	//fetch SignatureValue from xml
	signature = (uint8_t *)parseTag(handle->raw_permart,(char*)"SignatureValue",&signature_len,0,0);
	if (signature == NULL) {
		ret = NPNT_INV_SIGN;
		return ret;
	}
	signature_len = 345;//strlen((char*)signature);
	raw_signature = base64_decode(signature, signature_len, &raw_signature_len);

	if (npnt_check_authenticity((uint8_t*)digest_value, 32, raw_signature, raw_signature_len) <= 0) {
		ret = NPNT_INV_AUTH;
		return ret;
	}

	//Digest Canonicalised Permission Artifact
	raw_perm_without_sign = strstr(handle->raw_permart, "<UAPermission");
	if (raw_perm_without_sign == NULL) {
		ret = NPNT_INV_ART;
		return ret;
	}
	permission_length = strstr(handle->raw_permart, "<Signature") - raw_perm_without_sign;
	if (permission_length < 0) {
		ret = NPNT_INV_ART;
		return ret;
	}
	reset_sha256();
	curr_ptr = 0;
	curr_length = 0;

	//Canonicalise Permission Artefact by converting Empty elements to start-end tag pairs
	while (curr_ptr < permission_length) {
		curr_length = 1;
		if (raw_perm_without_sign[curr_ptr] == '<') {
			while((curr_ptr + curr_length) < permission_length) {
				if (raw_perm_without_sign[curr_ptr + curr_length] == ' ') {
					last_empty_element[curr_length - 1] = '\0';
					break;
				} else if (raw_perm_without_sign[curr_ptr + curr_length] == '>') {
					last_empty_element[0] = '\0';
					break;
				}
				last_empty_element[curr_length - 1] = raw_perm_without_sign[curr_ptr + curr_length];
				curr_length++;
			}
		}

		if (strlen(last_empty_element) != 0) {
			if (raw_perm_without_sign[curr_ptr] == '/') {
				if (raw_perm_without_sign[curr_ptr + 1] == '>') {
					update_sha256("></", 3);
					update_sha256(last_empty_element, strlen(last_empty_element));
					last_empty_element[0] = '\0';
					curr_ptr += curr_length;
					continue;
				}
			}
		}

		update_sha256(&raw_perm_without_sign[curr_ptr], curr_length);

		curr_ptr += curr_length;
	}

	//Skip Signature for Digestion
	raw_perm_without_sign = strstr(handle->raw_permart, "</Signature>") + strlen("</Signature>");
	update_sha256(raw_perm_without_sign, strlen(raw_perm_without_sign));
	final_sha256(digest_value);

	char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
			'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
			'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
			'w', 'x', 'y', 'z', '0', '1', '2', '3',
			'4', '5', '6', '7', '8', '9', '+', '/'};
	int mod_table[] = {0, 2, 1};

	volatile int indexBuf = 0;
	indexBuf = 4 * ((32 + 2) / 3);

	int i = 0 ,j=0;
	for (i = 0, j = 0; i < 32;) {

		uint32_t octet_a = i < 32 ? (unsigned char)digest_value[i++] : 0;
		uint32_t octet_b = i < 32 ? (unsigned char)digest_value[i++] : 0;
		uint32_t octet_c = i < 32 ? (unsigned char)digest_value[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		handle->base64_digest_value[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		handle->base64_digest_value[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		handle->base64_digest_value[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		handle->base64_digest_value[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for ( i = 0; i < mod_table[32 % 3]; i++)
		handle->base64_digest_value[indexBuf - 1 - i] = '=';

	//Check Digestion
	rcvd_digest_value =parseTag(handle->raw_permart,(char*)"DigestValue",&signature_len,0,0);

	for (i = 0; i < indexBuf - 1; i++) {
		if (handle->base64_digest_value[i] != rcvd_digest_value[i]) {
			ret = NPNT_INV_DGST;
			return ret;
		}
	}

	return ret;
}


void npnt_helpers_c::clearCordinateList(npnt_s* handle)
{
	GeoCoordinate_st* head = handle->cordinateList;
	while(head != NULL)
	{
		GeoCoordinate_st* next = head->next;
		free(head);
		head = next;
	}
	handle->cordListCount = 0;
}

void npnt_helpers_c::addToList(npnt_s* handle,GeoCoordinate_st* cord)
{
	if(handle->cordinateList == NULL)
		handle->cordinateList = cord;
	else
	{
		GeoCoordinate_st* head = handle->cordinateList;
		while(head->next != NULL)
		{
			head = head->next;
		}
		head->next = cord;
	}
	handle->cordinateList++;
}




int8_t npnt_helpers_c::npnt_alloc_and_get_fence_points(npnt_s* handle, float* vertlat, float* vertlon)
{
	//	GeoCoordinate_st *geoCordinate;
	clearCordinateList(handle);
	//Calculate number of vertices
	char buf[50] = {0};
	uint16_t len = 0;
	uint16_t nverts = 0;
	char* tagEndPtr = 0;
	char* outPtr,*inPtr;

	//	const char* lat_str;
	//	const char* lon_str;

	char* opt = (char*)parseTag(handle->raw_permart,(char*)"Coordinates",&len,&tagEndPtr,0);

	inPtr = opt;
	while(parseTag(inPtr,(char*)"Coordinate",&len,&tagEndPtr,1))
	{
		inPtr = tagEndPtr;
		nverts++;
	}
	if(nverts == 0)
	{
		return 0;
	}
	//Allocate vertices
	vertlat = (float*)calloc(nverts,sizeof(float));
	vertlon = (float*)calloc(nverts,sizeof(float));
	inPtr = opt;
	nverts = 0;
	while((outPtr = (char*)parseTag(inPtr,(char*)"Coordinate",&len,&tagEndPtr,1)) != 0)
	{
		memset(buf,0,sizeof(buf));
		strncpy(buf,outPtr,len);
		char* marker = strchr(buf, '=')+2;
		vertlat[nverts] = atof(marker);
		marker = strchr(marker, '=')+2;
		vertlon[nverts] = atof(marker);

		inPtr = tagEndPtr;

		//		geoCordinate = (GeoCoordinate_st*)calloc(1,sizeof(GeoCoordinate_st));
		//		geoCordinate->lat = (double)vertlat[nverts];
		//		geoCordinate->lon = (double)vertlon[nverts];
		//		addToList(handle,geoCordinate);

		nverts++;
	}

	if (!vertlat || !vertlon) {
		return -1;
	}

	return nverts;
}

int8_t npnt_helpers_c::npnt_get_max_altitude(npnt_s* handle, float* altitude)
{

	//	int len = 0;
	//	char* FlightParametersPtr = (char*)parseTag(handle->raw_permart,(char*)"FlightParameters",&len,0,1);
	//	*altitude = getValueFloat(FlightParametersPtr,(char*)"maxAltitude");
	return 0;
}

void npnt_helpers_c::npnt_ist_data_time_to_QString(struct tm date_time,char* str)
{
	//	snprintf(str,40,"%04d%02d%02d%02d%02d%02d",date_time.tm_year,date_time.tm_mon,date_time.tm_mday,date_time.tm_hour,date_time.tm_min,date_time.tm_sec);
}

int8_t npnt_helpers_c::npnt_check_we_are_in_DGCA_time(struct tm* flight_Start_tm,struct tm* flight_End_tm)
{
	// check flight start and end time not same
	if( (flight_Start_tm->tm_hour == flight_End_tm->tm_hour) &&
			(flight_Start_tm->tm_min  == flight_End_tm->tm_min)  &&
			(flight_Start_tm->tm_year == flight_End_tm->tm_year)  &&
			(flight_Start_tm->tm_mon  == flight_End_tm->tm_mon)  &&
			(flight_Start_tm->tm_mday == flight_End_tm->tm_mday)
	)
	{
		return NPNT_INV_TIME;
	}
	//	if(difftime(mktime(flight_Start_tm),mktime(flight_End_tm)) == 0)
	//	{
	//		return NPNT_INV_TIME;
	//	}

	// check flight start and end on same day
	//    if((flight_Start_tm->tm_mday != flight_End_tm->tm_mday) || (flight_Start_tm->tm_mon != flight_End_tm->tm_mon) || (flight_Start_tm->tm_year != flight_End_tm->tm_year))
	//    {
	//        return NPNT_INV_TIME;
	//    }

	//	struct tm DaystartTm,DayendTm;
	//	DaystartTm = *flight_Start_tm;
	//	DayendTm = *flight_End_tm;
	//	DaystartTm.tm_hour = 5; DaystartTm.tm_min = 30; DaystartTm.tm_sec = 0;
	//	DayendTm.tm_hour = 20; DayendTm.tm_min = 30; DayendTm.tm_sec = 0;

	//check that dgca start time and end time in limit
	//    secondsDiff = difftime(mktime(flight_Start_tm),mktime(&DaystartTm));
	//    qDebug() << "Second diffrence start: "<< secondsDiff;
	//    if(secondsDiff < 0)
	//        return NPNT_INV_TIME;

	//    secondsDiff = difftime(mktime(flight_End_tm),mktime(&DayendTm));
	//    	() << "Second diffrence stop: "<< secondsDiff;
	//    if(secondsDiff >= 0)
	//        return NPNT_INV_TIME;

	return 0;
}

int8_t npnt_helpers_c::npnt_ist_date_time_to_unix_time(const char* dt_string, struct tm* date_time)
{
	//    qDebug()<<QString(dt_string);
	//    sscanf((char*)dt_string,"%d-%d-%dT%d:%d:%d",&date_time->tm_year,&date_time->tm_mon,&date_time->tm_mday,&date_time->tm_hour,&date_time->tm_min,&date_time->tm_sec);
	//    date_time->tm_year = date_time->tm_year - 1900;
	//    date_time->tm_mon = date_time->tm_mon - 1;

	//2 0 2 0 - 0 7 - 0 7  T  1  8   :  1   2  :  4  9   .311659+05:30
	//	  0 1 2 3 4 5 6 7 8 9 10  11 12  13 14 15 16 17 18

	char temptVar[5] = {0};
	memcpy(temptVar,dt_string,4);
	date_time->tm_year = atoi(temptVar);
	memset(temptVar,0,sizeof(temptVar));

	memcpy(temptVar,dt_string+5,2);
	date_time->tm_mon =  atoi(temptVar);
	memset(temptVar,0,sizeof(temptVar));

	memcpy(temptVar,dt_string+8,2);
	date_time->tm_mday = atoi(temptVar);
	memset(temptVar,0,sizeof(temptVar));

	memcpy(temptVar,dt_string+11,2);
	date_time->tm_hour =  atoi(temptVar);
	memset(temptVar,0,sizeof(temptVar));

	memcpy(temptVar,dt_string+14,2);
	date_time->tm_min =  atoi(temptVar);
	memset(temptVar,0,sizeof(temptVar));

	memcpy(temptVar,dt_string+17,2);
	date_time->tm_sec =  atoi(temptVar);
	memset(temptVar,0,sizeof(temptVar));

	date_time->tm_year = date_time->tm_year - 1900;
	date_time->tm_mon = date_time->tm_mon - 1;

	return 0;

}

//char* npnt_helpers_c::npnt_get_attr(mxml_node_t *node, const char* attr)
//{
//	const char* tmp = NULL;
//	char* ret = NULL;
//	tmp = mxmlElementGetAttr(node, attr);
//	if (!tmp) {
//		return NULL;
//	}
//	ret = (char*)calloc(1,strlen(tmp) + 1);
//	if (!ret) {
//		return NULL;
//	}
//	strcpy(ret, tmp);
//	return ret;
//}

int8_t npnt_helpers_c::npnt_populate_flight_params(npnt_s* handle)
{
	//	mxml_node_t *ua_detail, *flight_params;
	//	ua_detail = mxmlFindElement(handle->parsed_permart, handle->parsed_permart, "UADetails", NULL, NULL, MXML_DESCEND);
	//	if (!ua_detail) {
	//		//        return NPNT_INV_FPARAMS;
	//	}
	//	flight_params = mxmlFindElement(handle->parsed_permart, handle->parsed_permart, "FlightParameters", NULL, NULL, MXML_DESCEND);
	//	if (!flight_params) {
	//		//        return NPNT_INV_FPARAMS;
	//	}
	uint16_t len = 0;

	char* UAPermission = (char*)parseTag(handle->raw_permart,(char*)"UAPermission",&len,0,2);
	char* UADetailsPtr = (char*)parseTag(handle->raw_permart,(char*)"UADetails",&len,0,1);
	char* FlightParametersPtr = (char*)parseTag(handle->raw_permart,(char*)"FlightParameters",&len,0,1);

	getValueStr(UAPermission,(char*)"permissionArtifactId",&handle->params.permissionArtifactID);
	if (!handle->params.permissionArtifactID) {
		//        return NPNT_INV_FPARAMS;
	}

	//	handle->params.uinNo = npnt_get_attr(ua_detail, "uinNo");
	getValueStr(UADetailsPtr,(char*)"uinNo",&handle->params.uinNo);
	if (!handle->params.uinNo) {
		//        return NPNT_INV_FPARAMS;
	}

	//	handle->params.adcNumber = npnt_get_attr(flight_params, "adcNumber");
	getValueStr(FlightParametersPtr,(char*)"adcNumber",&handle->params.adcNumber);
	if (!handle->params.adcNumber) {
		//        return NPNT_INV_FPARAMS;
	}

	//	handle->params.ficNumber = npnt_get_attr(flight_params, "ficNumber");
	getValueStr(FlightParametersPtr,(char*)"ficNumber",&handle->params.ficNumber);
	if (!handle->params.ficNumber) {
		//        return NPNT_INV_FPARAMS;
	}
	memset(&handle->params.flightEndTime,0,sizeof(handle->params.flightEndTime));

	//	if (npnt_ist_date_time_to_unix_time(mxmlElementGetAttr(flight_params, "flightEndTime"), &handle->params.flightEndTime) < 0)
	if(npnt_ist_date_time_to_unix_time(getValueStrPtr(FlightParametersPtr,(char*)"flightEndTime"), &handle->params.flightEndTime) < 0)
	{
		return NPNT_INV_FPARAMS;
	}
	memset(&handle->params.flightStartTime,0,sizeof(handle->params.flightStartTime));
	//	if (npnt_ist_date_time_to_unix_time(mxmlElementGetAttr(flight_params, "flightStartTime"), &handle->params.flightStartTime) < 0)
	if(npnt_ist_date_time_to_unix_time(getValueStrPtr(FlightParametersPtr,(char*)"flightStartTime"), &handle->params.flightEndTime) < 0)
	{
		return NPNT_INV_FPARAMS;
	}

	if(npnt_check_we_are_in_DGCA_time(&handle->params.flightStartTime,&handle->params.flightEndTime) < 0)
	{
		return NPNT_INV_TIME;
	}

	return 0;
}

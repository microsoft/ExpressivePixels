#include "app_util_platform.h"
#include "nordic_common.h"
#include "mem_manager.h"
#include "nrf_delay.h"
#include "nrf_crypto_init.h"
#include "nrf_crypto_mem.h"
#include "nrf_crypto_aes.h"
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_Crypto.h"

//#define AES_DATA_SIZE    		16
#define PARTIAL_MESSAGE_SIZE	16
#define AES_KEY					"I like big ints and I cannot lie"
//'I',' ','l','i','k','e',' ','b','i','g',' ','i','n','t','s',
//' ','a','n','d',' ','I',' ','c','a','n','n','o','t',' ','l','i','e'
//0xc4,0x7b,0x02,0x94,0xdb,0xbb,0xee,0x0f,0xec,0x47,0x57,0xf2,0x2f,0xfe,0xee,0x35,
//0x87,0xca,0x47,0x30,0xc3,0xd3,0x3b,0x69,0x1d,0xf3,0x8b,0xab,0x07,0x6b,0xc5,0x58



// static uint8_t m_aes_key[AES_DATA_SIZE] = { AES_KEY };
//static uint8_t m_encrypted_text[AES_DATA_SIZE];
//static uint8_t m_decrypted_text[AES_DATA_SIZE];

static nrf_crypto_key_size_id_t m_aes_key_size = NRF_CRYPTO_KEY_SIZE_256;



void EPXPlatform_Crypto_Initialize()
{
	ret_code_t err_code;

	err_code = nrf_crypto_init();
	APP_ERROR_CHECK(err_code);

	//err_code = nrf_mem_init();
	//APP_ERROR_CHECK(err_code);
}



uint8_t *EPXPlatform_Crypto_Encrypt(uint8_t * pAESKey, void *p_data, size_t len)
{
	uint8_t     iv[16];
	ret_code_t  err_code;
	//size_t      len_in;
	size_t      len_out;

	static nrf_crypto_aes_context_t ctr_encr_128_ctx;  // AES CTR encryption context
	static nrf_crypto_aes_context_t ctr_decr_128_ctx;  // AES CTR decryption context

	// Init encryption context for 128 bit key
	err_code = nrf_crypto_aes_init(&ctr_encr_128_ctx, &g_nrf_crypto_aes_ctr_128_info, NRF_CRYPTO_ENCRYPT);
	APP_ERROR_CHECK(err_code);

	/* Set key for encryption context*/
	err_code = nrf_crypto_aes_key_set(&ctr_encr_128_ctx, pAESKey);
	APP_ERROR_CHECK(err_code);

	memset(iv, 0, sizeof(iv));
	/* Set IV for encryption context */
	err_code = nrf_crypto_aes_iv_set(&ctr_encr_128_ctx, iv);
	APP_ERROR_CHECK(err_code);
	
	uint8_t *pEncrypted = (uint8_t *) malloc(len);
	if (pEncrypted != NULL)
	{
		/* Encrypt first 16 bytes */
		err_code = nrf_crypto_aes_update(&ctr_encr_128_ctx, (uint8_t *)p_data, PARTIAL_MESSAGE_SIZE, pEncrypted);
		APP_ERROR_CHECK(err_code);

		len_out = len - PARTIAL_MESSAGE_SIZE;
		err_code = nrf_crypto_aes_finalize(&ctr_encr_128_ctx, (uint8_t *)p_data + PARTIAL_MESSAGE_SIZE, len - PARTIAL_MESSAGE_SIZE, (uint8_t *) pEncrypted + PARTIAL_MESSAGE_SIZE, &len_out);
		APP_ERROR_CHECK(err_code);

		return (uint8_t *)pEncrypted;
		
		memset(iv, 0, sizeof(iv));

		len_out = len;		
		uint8_t *pDecrypted = (uint8_t *) malloc(len);

		/* Decrypt with integrated function */
		err_code = nrf_crypto_aes_crypt(&ctr_decr_128_ctx, &g_nrf_crypto_aes_ctr_128_info, NRF_CRYPTO_DECRYPT, pAESKey, iv, pEncrypted, len, pDecrypted, &len_out);
		APP_ERROR_CHECK(err_code);

		if (memcmp(p_data, pDecrypted, len_out) == 0)
			return (uint8_t *)pEncrypted;
		else
			NRF_LOG_INFO("AES CTR example failed!!!");
	}
	return NULL;
}



uint8_t *EPXPlatform_Crypto_Decrypt(uint8_t * pAESKey, void *p_data, size_t len)
{
	static nrf_crypto_aes_context_t ctr_decr_128_ctx;   // AES CTR decryption context
	uint8_t     iv[16];

	size_t len_out = len;		
	uint8_t *pDecrypted = (uint8_t *) malloc(len);
	if (pDecrypted != NULL)
	{		
		/* Decrypt with integrated function */
		memset(iv, 0, sizeof(iv));
		int err_code = nrf_crypto_aes_crypt(&ctr_decr_128_ctx, &g_nrf_crypto_aes_ctr_128_info, NRF_CRYPTO_DECRYPT, pAESKey, iv, (uint8_t *) p_data, len, pDecrypted, &len_out);
		APP_ERROR_CHECK(err_code);
		return pDecrypted;	
	}
	return NULL;
}




#define RNG_BYTE_WAIT_US               (124UL)

/**
 * @brief Uses the RNG to write a 16-byte nonce to a buffer
 *
 * @param[in]    p_buf    An array of length 16
 */
void EPXPlatform_Crypto_GenerateNONCE(uint8_t * p_buf)
{
	uint8_t i         = 0;
	uint8_t remaining = SOC_ECB_KEY_LENGTH;

	// The random number pool may not contain enough bytes at the moment so
	// a busy wait may be necessary.
	while(0 != remaining)
	{
		uint32_t err_code;
		uint8_t  available = 0;

		err_code = sd_rand_application_bytes_available_get(&available);
		APP_ERROR_CHECK(err_code);

		available = ((available > remaining) ? remaining : available);
		if (0 != available)
		{
			err_code = sd_rand_application_vector_get((p_buf + i), available);
			APP_ERROR_CHECK(err_code);

			i += available;
			remaining -= available;
		}

		if (0 != remaining)
			nrf_delay_us(RNG_BYTE_WAIT_US * remaining);
	}
}


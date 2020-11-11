#pragma once

#define EPX_AES_KEY_BYTE_SIZE	16
#define EPX_NONCE_SIZE			16


void EPXPlatform_Crypto_Initialize();
uint8_t *EPXPlatform_Crypto_Decrypt(uint8_t * pAESKey, void *p_data, size_t len);
uint8_t *EPXPlatform_Crypto_Encrypt(uint8_t * pAESKey, void * p_data, size_t len);
void EPXPlatform_Crypto_GenerateNONCE(uint8_t * p_buf);
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <memory.h>
#include "EPXPlatform_I2S.h"

EPX_OPTIMIZEFORDEBUGGING_ON

void EPXPlatform_I2S_Configure(uint16_t pin)
{
	/*
	nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;	
	memcpy(&g_i2sConfig, &config, sizeof(nrf_drv_i2s_config_t));
	g_i2sConfig.sdin_pin  = NRFX_I2S_PIN_NOT_USED;
	g_i2sConfig.sdout_pin = pin;
	g_i2sConfig.mck_setup = NRF_I2S_MCK_32MDIV10;      ///< 32 MHz / 10 = 3.2 MHz.
	g_i2sConfig.ratio     = NRF_I2S_RATIO_32X;         ///< LRCK = MCK / 32.
	g_i2sConfig.channels  = NRF_I2S_CHANNELS_STEREO;
	*/
}



bool EPXPlatform_I2S_Initialize()
{
	//return nrf_drv_i2s_init(&g_i2sConfig, i2s_handler) == 0;
	return false;
}



void EPXPlatform_I2S_UnInitialize()
{
//	nrfx_i2s_uninit();
}



bool EPXPlatform_I2S_Start(void *p, size_t cb)
{
	/*
	nrfx_i2s_buffers_t buffers;
	buffers.p_rx_buffer = NULL;
	buffers.p_tx_buffer = (uint32_t *) p;	
	return nrf_drv_i2s_start(&buffers, cb, 0) == 0;
	*/
}



void EPXPlatform_I2S_Stop()
{
//	nrf_drv_i2s_stop();
}

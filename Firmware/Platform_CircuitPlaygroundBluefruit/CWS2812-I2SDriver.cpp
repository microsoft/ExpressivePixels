// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_I2S.h"
#include "CWS2812-I2SDriver.h"

EPX_OPTIMIZEFORDEBUGGING_ON

void CWS2812_I2SDriver::FillI2SDriverBuffer()
{
	LEDDRIVERPIXEL*pPixel = m_pPixels;
	int8_t offset = 1;
	uint8_t *pTX = (uint8_t *) m_pTXBuffer;
		
	for (uint16_t iPixel = 0; iPixel < m_numPixels; iPixel++)
	{
		uint32_t rgb = (((pPixel->g * m_brightness) >> 8) << 16) | (((pPixel->r * m_brightness) >> 8) << 8) | ((pPixel->b * m_brightness) >> 8);
		for (uint8_t i_rgb = 0; i_rgb < I2S_WS2812B_DRIVE_BUF_SIZE_PER_LED; i_rgb++)
		{
			switch (rgb & 0x00c00000)
			{
			case (0x00400000):
				*(pTX + offset)  = (uint8_t)((I2S_WS2812B_DRIVE_PATTERN_0 << 4) | I2S_WS2812B_DRIVE_PATTERN_1);
				break;
			case (0x00800000):
				*(pTX + offset)  = (uint8_t)((I2S_WS2812B_DRIVE_PATTERN_1 << 4) | I2S_WS2812B_DRIVE_PATTERN_0);
				break;
			case (0x00c00000):
				*(pTX + offset)  = (uint8_t)((I2S_WS2812B_DRIVE_PATTERN_1 << 4) | I2S_WS2812B_DRIVE_PATTERN_1);
				break;
			default:
				*(pTX + offset)  = (uint8_t)((I2S_WS2812B_DRIVE_PATTERN_0 << 4) | I2S_WS2812B_DRIVE_PATTERN_0);
				break;
			}
			pTX++;
			offset = -offset;
			rgb <<= (24 / I2S_WS2812B_DRIVE_BUF_SIZE_PER_LED);
		}
		pPixel++;
	}
}



CWS2812_I2SDriver::CWS2812_I2SDriver(int dataPin, uint16_t numPixels) : CLEDDriverBase(numPixels)
{
	m_pTXBuffer = NULL;		
	m_i2sBufferSize = 3 * m_numPixels + RESET_BITS;
	
	EPXPlatform_I2S_Configure(dataPin);
}



void CWS2812_I2SDriver::Initialize()
{
	CLEDDriverBase::Initialize();
	m_pTXBuffer = (uint32_t *) malloc(sizeof(uint32_t) * m_i2sBufferSize);	
}



void CWS2812_I2SDriver::Show()
{
	EPXPlatform_I2S_Initialize();
	
	FillI2SDriverBuffer();	
	EPXPlatform_I2S_Start(m_pTXBuffer, m_i2sBufferSize);
		
	delayMicroseconds((m_numPixels + 20) * (24 * 5 / 4));	
	
	EPXPlatform_I2S_Stop();
	EPXPlatform_I2S_UnInitialize();
}



uint32_t CWS2812_I2SDriver::Wheel(uint8_t WheelPos) 
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85)
		return IntColor(255 - WheelPos * 3, 0, WheelPos * 3);
	if (WheelPos < 170) 
	{
		WheelPos -= 85;
		return IntColor(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return IntColor(WheelPos * 3, 255 - WheelPos * 3, 0);
}



void CWS2812_I2SDriver::Rainbow(uint8_t wait) 
{
	uint16_t i, j;

	for (j = 0; j < 256; j++) 
	{
		for (i = 0; i < m_numPixels; i++) 
			SetPixelColor(i, Wheel((i + j) & 255));
		Show();
		delay(wait);
	}
}



// Slightly different, this makes the rainbow equally distributed throughout
void CWS2812_I2SDriver::RainbowCycle(uint8_t wait) 
{
	uint16_t i, j;

	for (j = 0; j < 256 * 5; j++) 
	{
		 // 5 cycles of all colors on wheel
		for(i = 0 ; i < m_numPixels; i++) 
			SetPixelColor(i, Wheel(((i * 256 / m_numPixels) + j) & 255));
		
		Show();
		delay(wait);
	}
}


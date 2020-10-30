// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "CWS2812-SPIDriver.h"




CWS2812_SPIDriver::CWS2812_SPIDriver(SPIClass *pSPIClass, uint16_t numPixels) : CLEDDriverBase(numPixels)
{
	m_pSPIClass = pSPIClass;
}



void CWS2812_SPIDriver::Initialize()
{
	CLEDDriverBase::Initialize();
	m_pTXBuffer = (uint8_t *) TMALLOC(m_numPixels * 12);	
}



void CWS2812_SPIDriver::SendPayloadBits(uint32_t bits)
{
	// Encode '0' bits as 100 and '1' bits as 110.
	// We have this bit pattern: 00000000abcd
	// We want this bit pattern: 1a01b01c01d0
	uint32_t ac = (bits * 0x088) &          // 0abcdabcd000
	              0x410;   // 0a00000c0000

    uint32_t bd = (bits * 0x022) &          // 000abcdabcd0
	                  0x082;   // 0000b00000d0

    static uint32_t const base = 04444;     // 100100100100
	uint32_t payload = base | ac | bd;		// 1a01b01c01d0
    m_pSPIClass->transfer((uint8_t *) &payload, 3);			
}



void CWS2812_SPIDriver::Show()
{
	uint8_t brightness = m_brightness;	
	uint8_t *pTX = m_pTXBuffer;
	
	// Cap brightness manually for now
	if(brightness > 80)
		brightness = 80;	
	for (int i = 0; i < m_numPixels; i++) 
	{		
		int n = 0;
		uint32_t color = IntColor((m_pPixels[i].r * m_brightness) >> 8, (m_pPixels[i].g * m_brightness) >> 8, (m_pPixels[i].b * m_brightness) >> 8);
		for (int j = 0; j < 12; j++)
		{
			uint8_t x = 0x08;
			if (!(n & 0x00800000)) x |= 0x07;
			if (!(n & 0x00f00000)) x |= 0xE0;
			n << 2;
			
			*pTX = x;
			pTX++;
		}
		
		
		/*
		uint8_t byte0 = (m_pPixels[i].r * m_brightness) >> 8;
		uint8_t byte1 = (m_pPixels[i].g * m_brightness) >> 8;
		
		SendPayloadBits((byte0 >> 4) & 0xf);
		SendPayloadBits((byte0 >> 0) & 0xf);
		SendPayloadBits((byte1 >> 4) & 0xf);
		SendPayloadBits((byte1 >> 0) & 0xf);
		SendPayloadBits((((m_pPixels[i].b * m_brightness) >> 8) >> 4) & 0xf);
		SendPayloadBits((((m_pPixels[i].b * m_brightness) >> 8) >> 0) & 0xf);    
	*/
	}
	m_pSPIClass->transfer((uint8_t *) m_pTXBuffer, m_numPixels * 12);			
}



uint32_t CWS2812_SPIDriver::Wheel(uint8_t WheelPos) 
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



void CWS2812_SPIDriver::Rainbow(uint8_t wait) 
{
	uint16_t i, j;

	for (j = 0; j < 256; j++) 
	{
		for (i = 0; i < m_numPixels; i++) 
			SetPixelColor(i, Wheel((i + j) & 255));
		Show();
		nrf_delay_ms(wait);
	}
}



// Slightly different, this makes the rainbow equally distributed throughout
void CWS2812_SPIDriver::RainbowCycle(uint8_t wait) 
{
	uint16_t i, j;

	for (j = 0; j < 256 * 5; j++) 
	{
		 // 5 cycles of all colors on wheel
		for(i = 0 ; i < m_numPixels; i++) 
			SetPixelColor(i, Wheel(((i * 256 / m_numPixels) + j) & 255));
		
		Show();
		nrf_delay_ms(wait);
	}
}


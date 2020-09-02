// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementation for SPI based APA102 Display driver
 *
 **/

#pragma once
#include "stdlib.h"
#include "memory.h"
#include "EPXPlatform_SPIClass.h"
#include "CLEDDriverBase.h"



class CAPA102 : public CLEDDriverBase
{
	
public:
	// Constructor
	CAPA102(SPIClass *pSPIClass, uint16_t numPixels) : CLEDDriverBase(numPixels)
	{
		m_pSPIClass = pSPIClass;
	}
	
	
	
	void Initialize()
	{
		CLEDDriverBase::Initialize();
	}


	
	void Rainbow(uint8_t wait) { }
	void RainbowCycle(uint8_t wait) { }

	
	void Show()
	{
		int i;
		uint8_t header[4] = { 0x00, 0x00, 0x00, 0x00 };
		uint16_t brightness = m_brightness;
		
		 m_pSPIClass->transfer(header, sizeof(header));			
		for (i = 0; i < m_numPixels; i++)
		{
			m_pSPIClass->transfer(0xFF);
			m_pSPIClass->transfer((m_pPixels[i].b * brightness) >> 8);
			m_pSPIClass->transfer((m_pPixels[i].g * brightness) >> 8);
			m_pSPIClass->transfer((m_pPixels[i].r * brightness) >> 8);
		}
		for (i = 0; i < ((m_numPixels + 15) / 16); i++) 
			m_pSPIClass->transfer(0xFF);		
	}
	
	
private:
	SPIClass *m_pSPIClass; // System SPI class reference
};


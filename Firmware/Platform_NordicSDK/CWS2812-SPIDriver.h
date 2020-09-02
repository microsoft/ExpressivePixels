// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementation for SPI based WS2812 Display driver
 *
 **/
#pragma once
#include "stdlib.h"
#include "memory.h"
#include "EPXPlatform_SPIClass.h"
#include "CLEDDriverBase.h"


class CWS2812_SPIDriver : public CLEDDriverBase
{
	
public:
	// Consructor
	CWS2812_SPIDriver(SPIClass *pSPIClass, uint16_t numPixels);	
	
	// Destructor
	~CWS2812_SPIDriver()
	{
	}
			
	void Initialize();

	void Rainbow(uint8_t wait);
	void RainbowCycle(uint8_t wait);
	void Show();
	
	
private:
	void SendPayloadBits(uint32_t bits);
	uint32_t Wheel(uint8_t WheelPos);
	
	SPIClass				*m_pSPIClass;	// Pointer to system SPI class interface
	uint8_t					*m_pTXBuffer;	// Transmission buffer
};


// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementation for I2S based WS2812 Display driver
 *
 **/

#pragma once
#include "stdlib.h"
#include "string.h"
#include "CLEDDriverBase.h"

#define RESET_BITS 6
#define I2S_WS2812B_DRIVE_PATTERN_0			((uint8_t)0x08)			// Bit pattern for data "0" is "HLLL".
#define I2S_WS2812B_DRIVE_PATTERN_1			((uint8_t)0x0e)      // Bit pattern for data "1" is "HHHL".
#define	I2S_WS2812B_DRIVE_BUF_SIZE_PER_LED	(12)	// buffer size for each LED (8bit * 4 * 3 )



class CWS2812_I2SDriver : public CLEDDriverBase
{
	
public:
	// Constructor
	CWS2812_I2SDriver(int dataPin, uint16_t numPixels);	
	
	// Destructor
	~CWS2812_I2SDriver()
	{
		// Clean up I2S transmission buffer
		if (m_pTXBuffer != NULL)
			TFREE(m_pTXBuffer);
	}
			
	void Initialize();
	void Rainbow(uint8_t wait);
	void RainbowCycle(uint8_t wait);
	void Show();
	
private:
	void FillI2SDriverBuffer();
	uint32_t Wheel(uint8_t WheelPos);
	
	uint16_t				m_i2sBufferSize;	// I2S transmission buffer size
	uint32_t				*m_pTXBuffer;		// I2S transmission buffer
	uint32_t				m_nRainbowIteration;// Rainbow iteration tracking
};


// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementation for APA102 Display driver
 *
 **/
#pragma once
#include "stdlib.h"
#include "memory.h"
#include "CLEDDriverBase.h"



class CAPA102 : public CLEDDriverBase
{
	
public:
	// Constructor
	CAPA102(uint16_t numPixels, uint16_t dataPin, uint16_t clockPin) : CLEDDriverBase(numPixels)
	{
		m_numPixels = numPixels;
		m_dataPin = dataPin;
		m_brightness  = clockPin;
	}
	
	
	void Clear();
	void Initialize();
	void Rainbow(uint8_t wait) { }
	void RainbowCycle(uint8_t wait) { }	
	
	void SetBrightness(uint8_t brightness);
	void SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);
	void SetPixelColor(uint16_t idx, uint32_t color);
	void Show();
	
private:
	uint16_t m_numPixels;	// Number of pixels in display array
	uint16_t m_dataPin;		// GPIO data pin
	uint16_t m_clockPin;	// GPIO clock pin
};


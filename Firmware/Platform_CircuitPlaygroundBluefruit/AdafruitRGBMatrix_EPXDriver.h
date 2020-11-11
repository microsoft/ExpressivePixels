// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#include "stdlib.h"
#include "memory.h"
#include "CLEDDriverBase.h"
#include <Arduino.h>
#include <Adafruit_Protomatter.h>

class CAdafruitRGBMatrix_EPXDriver : public CLEDDriverBase
{
	
public:
	CAdafruitRGBMatrix_EPXDriver(int width, int height);	
	~CAdafruitRGBMatrix_EPXDriver();
	
	void Initialize();
	void Rainbow(uint8_t wait);
	void RainbowCycle(uint8_t wait);
	void Show();
	void Clear();

	void SetBrightness(uint8_t brightness);
	void SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);
	void SetPixelColor(uint16_t idx, uint32_t color);
	uint16_t NumPixels();
	
private:
	uint32_t Wheel(uint8_t WheelPos);
	
	Adafruit_Protomatter				*m_pRGBmatrixPanel;
	uint32_t							*m_pTXBuffer;
	uint32_t							m_nRainbowIteration;
	int 								m_width; 
	int 								m_height;
	int 								m_brightness;
};


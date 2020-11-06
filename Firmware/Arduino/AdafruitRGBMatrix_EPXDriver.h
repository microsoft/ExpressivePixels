// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#include "stdlib.h"
#include "memory.h"
#include "CLEDDriverBase.h"
#include <Arduino.h>
#include <RGBmatrixPanel.h>


class CAdafruitRGBMatrix_EPXDriver : public CLEDDriverBase
{
	
public:
	CAdafruitRGBMatrix_EPXDriver(int width, int height, uint8_t pinClk, uint8_t pinOE, uint8_t pinLAT, uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinD, uint8_t *prgbpins);	
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
	int 								brightness;
};


// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "stdlib.h"
#include "string.h"
#include "CLEDDriverBase.h"



class Adafruit_NeopixelDriver : public CLEDDriverBase
{
	
public:
	Adafruit_NeopixelDriver(int dataPin, uint16_t numPixels);	
	
	~Adafruit_NeopixelDriver()
	{
	}
			
	void Initialize();
	void Rainbow(uint8_t wait);
	void RainbowCycle(uint8_t wait);
	void Show();

        void SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);
        void SetPixelColor(uint16_t idx, uint32_t color);
        void SetBrightness(uint8_t brightness);
        void Clear();
	
private:
	uint32_t Wheel(uint8_t WheelPos);
		
	uint32_t  m_nRainbowIteration;
        void      *m_pvAdafruitNeopixel;
};


// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include "AdafruitNeoPixel_EPXDriver.h"

EPX_OPTIMIZEFORDEBUGGING_ON


CAdafruitNeoPixel_EPXDriver::CAdafruitNeoPixel_EPXDriver(int dataPin, uint16_t numPixels) : CLEDDriverBase(numPixels)
{
	m_nRainbowIteration = 0;
	m_AdaFruitNeoPixel = new Adafruit_NeoPixel(numPixels, dataPin, NEO_GRB + NEO_KHZ800);
}



void CAdafruitNeoPixel_EPXDriver::Initialize()
{
	m_AdaFruitNeoPixel->begin();
}



void CAdafruitNeoPixel_EPXDriver::SetBrightness(uint8_t brightness)
{
	m_AdaFruitNeoPixel->setBrightness(brightness);
}



uint16_t CAdafruitNeoPixel_EPXDriver::NumPixels()
{
	return m_AdaFruitNeoPixel->numPixels();
}



void CAdafruitNeoPixel_EPXDriver::SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
	m_AdaFruitNeoPixel->setPixelColor(idx, r, g, b);
}



void CAdafruitNeoPixel_EPXDriver::SetPixelColor(uint16_t idx, uint32_t color)
{
	m_AdaFruitNeoPixel->setPixelColor(idx, color);
}


void CAdafruitNeoPixel_EPXDriver::Clear()
{
	m_AdaFruitNeoPixel->clear();
}



void CAdafruitNeoPixel_EPXDriver::Show()
{
	m_AdaFruitNeoPixel->show();
}



uint32_t CAdafruitNeoPixel_EPXDriver::Wheel(uint8_t WheelPos) 
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



void CAdafruitNeoPixel_EPXDriver::Rainbow(uint8_t wait) 
{
	uint16_t i, j;

	for (j = 0; j < 256; j++) 
	{
		for (i = 0; i < m_numPixels; i++) 
			// SetPixelColor(i, Wheel((i + j) & 255));
			m_AdaFruitNeoPixel->setPixelColor(i, Wheel((i + j) & 255));
		m_AdaFruitNeoPixel->show();
		delay(wait);
	}
}



// Slightly different, this makes the rainbow equally distributed throughout
void CAdafruitNeoPixel_EPXDriver::RainbowCycle(uint8_t wait) 
{
	uint16_t i;

	if(m_nRainbowIteration >= 256 * 5)
		m_nRainbowIteration = 0;

	// 5 cycles of all colors on wheel
	for(i = 0 ; i < m_numPixels; i++) 
		//SetPixelColor(i, Wheel(((i * 256 / m_numPixels) + j) & 255));
		m_AdaFruitNeoPixel->setPixelColor(i, Wheel((i + m_nRainbowIteration) & 255));	
	m_AdaFruitNeoPixel->show();
	delay(wait);

	m_nRainbowIteration++;
}


// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include "AdafruitDotStar_EPXDriver.h"

EPX_OPTIMIZEFORDEBUGGING_ON


CAdafruitDotStar_EPXDriver::CAdafruitDotStar_EPXDriver(int dataPin, int clockPin, uint16_t numPixels) : CLEDDriverBase(numPixels)
{
	m_nRainbowIteration = 0;
	m_AdaFruitDotStar = new Adafruit_DotStar(numPixels, dataPin, clockPin);
}



void CAdafruitDotStar_EPXDriver::Initialize()
{
	m_AdaFruitDotStar->begin();
}



void CAdafruitDotStar_EPXDriver::SetBrightness(uint8_t brightness)
{
	m_AdaFruitDotStar->setBrightness(brightness);
}



uint16_t CAdafruitDotStar_EPXDriver::NumPixels()
{
	return m_AdaFruitDotStar->numPixels();
}



void CAdafruitDotStar_EPXDriver::SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
	m_AdaFruitDotStar->setPixelColor(idx, g, r, b);
}



void CAdafruitDotStar_EPXDriver::SetPixelColor(uint16_t idx, uint32_t color)
{
	m_AdaFruitDotStar->setPixelColor(idx, color);
}


void CAdafruitDotStar_EPXDriver::Clear()
{
	m_AdaFruitDotStar->clear();
}



void CAdafruitDotStar_EPXDriver::Show()
{
	m_AdaFruitDotStar->show();
}



uint32_t CAdafruitDotStar_EPXDriver::Wheel(uint8_t WheelPos) 
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



void CAdafruitDotStar_EPXDriver::Rainbow(uint8_t wait) 
{
	uint16_t i, j;

	for (j = 0; j < 256; j++) 
	{
		for (i = 0; i < m_numPixels; i++) 
			// SetPixelColor(i, Wheel((i + j) & 255));
			m_AdaFruitDotStar->setPixelColor(i, Wheel((i + j) & 255));
		m_AdaFruitDotStar->show();
		delay(wait);
	}
}



// Slightly different, this makes the rainbow equally distributed throughout
void CAdafruitDotStar_EPXDriver::RainbowCycle(uint8_t wait) 
{
	uint16_t i;

	if(m_nRainbowIteration >= 256 * 5)
		m_nRainbowIteration = 0;

	// 5 cycles of all colors on wheel
	for(i = 0 ; i < m_numPixels; i++) 
		//SetPixelColor(i, Wheel(((i * 256 / m_numPixels) + j) & 255));
		m_AdaFruitDotStar->setPixelColor(i, Wheel((i + m_nRainbowIteration) & 255));	
	m_AdaFruitDotStar->show();
	delay(wait);

	m_nRainbowIteration++;
}


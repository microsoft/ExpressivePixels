// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "EPXPlatform_Runtime.h"
#include "EPXVariant.h"

#ifdef VARIANTDISPLAYDRIVER_ADANEOPIXEL
#include "Adafruit_NeopixelDriver.h"
#include "..\Adafruit_NeoPixel\Adafruit_NeoPixel.h"



Adafruit_NeopixelDriver::Adafruit_NeopixelDriver(int dataPin, uint16_t numPixels) : CLEDDriverBase(numPixels)
{
      Adafruit_NeoPixel *pNeopixel = new Adafruit_NeoPixel(numPixels, dataPin);
      m_pvAdafruitNeopixel = pNeopixel;
}



void Adafruit_NeopixelDriver::Initialize()
{
        Adafruit_NeoPixel *pNeopixel = (Adafruit_NeoPixel *) m_pvAdafruitNeopixel;
        pNeopixel->begin();

	CLEDDriverBase::Initialize();
}



void Adafruit_NeopixelDriver::SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b) 
{
        Adafruit_NeoPixel *pNeopixel = (Adafruit_NeoPixel *) m_pvAdafruitNeopixel;
        pNeopixel->setPixelColor(idx, r, g, b);
}



void Adafruit_NeopixelDriver::SetPixelColor(uint16_t idx, uint32_t color)
{
        Adafruit_NeoPixel *pNeopixel = (Adafruit_NeoPixel *) m_pvAdafruitNeopixel;
        pNeopixel->setPixelColor(idx, color);
}


	
void Adafruit_NeopixelDriver::SetBrightness(uint8_t brightness)
{
        Adafruit_NeoPixel *pNeopixel = (Adafruit_NeoPixel *) m_pvAdafruitNeopixel;
        pNeopixel->setBrightness(brightness);
}



void Adafruit_NeopixelDriver::Clear()
{
        Adafruit_NeoPixel *pNeopixel = (Adafruit_NeoPixel *) m_pvAdafruitNeopixel;
        pNeopixel->clear();
}



void Adafruit_NeopixelDriver::Show()
{
        Adafruit_NeoPixel *pNeopixel = (Adafruit_NeoPixel *) m_pvAdafruitNeopixel;
        pNeopixel->show();
}



uint32_t Adafruit_NeopixelDriver::Wheel(uint8_t WheelPos) 
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



void Adafruit_NeopixelDriver::Rainbow(uint8_t wait) 
{
	uint16_t i, j;

	for (j = 0; j < 256; j++) 
	{
		for (i = 0; i < m_numPixels; i++) 
			SetPixelColor(i, Wheel((i + j) & 255));
		Show();
		delay(wait);
	}
}



// Slightly different, this makes the rainbow equally distributed throughout
void Adafruit_NeopixelDriver::RainbowCycle(uint8_t wait) 
{
	uint16_t i;

	if (m_nRainbowIteration >= 256 * 5)
		m_nRainbowIteration = 0;

	// 5 cycles of all colors on wheel
	for(i = 0 ; i < m_numPixels ; i++) 
		SetPixelColor(i, Wheel(((i * 256 / m_numPixels) + m_nRainbowIteration) & 255));
	Show();
	delay(wait);

	m_nRainbowIteration++;

}
#endif


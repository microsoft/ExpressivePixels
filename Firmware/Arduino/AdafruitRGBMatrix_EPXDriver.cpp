// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include "AdafruitRGBMatrix_EPXDriver.h"

EPX_OPTIMIZEFORDEBUGGING_ON

#include <RGBmatrixPanel.h> // Hardware-specific library

#define CLK  13
#define OE   1  // TX
#define LAT  0  // RX
#define A   A5
#define B   A4
#define C   A3
#define D   A2
uint8_t rgbpins[] = { 6,5,9,11,10,12 };

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64, rgbpins);
// RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 32s, rgbpins);

CAdafruitRGBMatrix_EPXDriver::CAdafruitRGBMatrix_EPXDriver(int width, int height) : CLEDDriverBase(width * height)
{
	m_width = width;
	m_height = height;
	m_nRainbowIteration = 0;
}



void CAdafruitRGBMatrix_EPXDriver::Initialize()
{
	matrix.begin();
}



void CAdafruitRGBMatrix_EPXDriver::SetBrightness(uint8_t brightness)
{
	
}



uint16_t CAdafruitRGBMatrix_EPXDriver::NumPixels()
{
	return m_width * m_height;
}



void CAdafruitRGBMatrix_EPXDriver::SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
	int x, y;

	y = idx / m_width;
	x = idx - (m_width * y);
	matrix.drawPixel(x, y, matrix.Color888(r, g, b));
}



void CAdafruitRGBMatrix_EPXDriver::SetPixelColor(uint16_t idx, uint32_t color)
{
	
}


void CAdafruitRGBMatrix_EPXDriver::Clear()
{
	
}



void CAdafruitRGBMatrix_EPXDriver::Show()
{
	
}



uint32_t CAdafruitRGBMatrix_EPXDriver::Wheel(uint8_t WheelPos) 
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



void CAdafruitRGBMatrix_EPXDriver::Rainbow(uint8_t wait) 
{
	uint16_t i, j;
/*
	for (j = 0; j < 256; j++) 
	{
		for (i = 0; i < m_numPixels; i++) 			
			m_AdaFruitNeoPixel->setPixelColor(i, Wheel((i + j) & 255));
		//m_AdaFruitNeoPixel->show();
		delay(wait);
	}
	*/
}



// Slightly different, this makes the rainbow equally distributed throughout
void CAdafruitRGBMatrix_EPXDriver::RainbowCycle(uint8_t wait) 
{
	/*
	uint16_t i;

	if(m_nRainbowIteration >= 256 * 5)
		m_nRainbowIteration = 0;

	// 5 cycles of all colors on wheel
	for(i = 0 ; i < m_numPixels; i++) 
		//SetPixelColor(i, Wheel(((i * 256 / m_numPixels) + j) & 255));
		m_AdaFruitNeoPixel->setPixelColor(i, Wheel((i + m_nRainbowIteration) & 255));	
	m_AdaFruitNeoPixel->show();
	delay(wait);
*/
	m_nRainbowIteration++;
}


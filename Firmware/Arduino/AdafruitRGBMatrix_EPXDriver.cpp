// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include "AdafruitRGBMatrix_EPXDriver.h"

EPX_OPTIMIZEFORDEBUGGING_ON

CAdafruitRGBMatrix_EPXDriver::CAdafruitRGBMatrix_EPXDriver(int width, int height, uint8_t pinCLK, uint8_t pinOE, uint8_t pinLAT, uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinD, uint8_t *prgbpins)
	: CLEDDriverBase(width * height)
{
	m_width = width;
	m_height = height;
	m_nRainbowIteration = 0;
	m_pRGBmatrixPanel = new RGBmatrixPanel(pinA, pinB, pinC, pinD, pinCLK, pinOE, pinLAT, false, width, prgbpins);
}



CAdafruitRGBMatrix_EPXDriver::~CAdafruitRGBMatrix_EPXDriver()
{
	if (m_pTXBuffer != NULL)
		free(m_pTXBuffer);
	if (m_pRGBmatrixPanel != NULL)
		delete m_pRGBmatrixPanel;
}



void CAdafruitRGBMatrix_EPXDriver::Initialize()
{
	m_pRGBmatrixPanel->begin();
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
	m_pRGBmatrixPanel->drawPixel(x, y, m_pRGBmatrixPanel->Color888(r, g, b));
}



void CAdafruitRGBMatrix_EPXDriver::SetPixelColor(uint16_t idx, uint32_t color)
{
	SetPixel(idx, (uint8_t)(color >> 16), (uint8_t)(color >>  8), (uint8_t) color);	
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

	for (j = 0; j < 256; j++) 
	{
		for (i = 0; i < m_numPixels; i++) 			
			SetPixelColor(i, Wheel((i + j) & 255));
		delay(wait);
	}	
}



// Slightly different, this makes the rainbow equally distributed throughout
void CAdafruitRGBMatrix_EPXDriver::RainbowCycle(uint8_t wait) 
{
	uint16_t i;

	if(m_nRainbowIteration >= 256 * 5)
		m_nRainbowIteration = 0;

	// 5 cycles of all colors on wheel
	for(i = 0 ; i < m_numPixels; i++) 
		//SetPixelColor(i, Wheel(((i * 256 / m_numPixels) + j) & 255));
		SetPixelColor(i, Wheel((i + m_nRainbowIteration) & 255));	
	delay(wait);

	m_nRainbowIteration++;
}


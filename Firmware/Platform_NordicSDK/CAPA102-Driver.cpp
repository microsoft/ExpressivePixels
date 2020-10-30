// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "EPXPlatform_Runtime.h"
#include <..\Adafruit_DotStar\Adafruit_DotStar.h>
#include "CAPA102-Driver.h"

Adafruit_DotStar *g_pStripDotStar;


void CAPA102::Initialize()
{
	g_pStripDotStar = new Adafruit_DotStar(m_numPixels, m_dataPin, m_clockPin, DOTSTAR_BGR);
	g_pStripDotStar->begin();
	g_pStripDotStar->setBrightness(DEFAULT_BRIGHTNESS);
	g_pStripDotStar->show();
}



void CAPA102::SetBrightness(uint8_t brightness) 
{ 
	g_pStripDotStar->setBrightness(brightness);	
}



void CAPA102::Clear()
{
	g_pStripDotStar->clear();
}



void CAPA102::SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b) 
{
	g_pStripDotStar->setPixelColor(idx, r, g, b);
}



void CAPA102::SetPixelColor(uint16_t idx, uint32_t color)
{
	g_pStripDotStar->setPixelColor(idx, color);
}



void CAPA102::Show()
{
	g_pStripDotStar->show();
}




// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include "AdafruitRGBMatrix_EPXDriver.h"

EPX_OPTIMIZEFORDEBUGGING_ON

/* ----------------------------------------------------------------------
The RGB matrix must be wired to VERY SPECIFIC pins, different for each
microcontroller board. This first section sets that up for a number of
supported boards.
------------------------------------------------------------------------- */
#if defined(_VARIANT_MATRIXPORTAL_M4_) // MatrixPortal M4
	uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
	uint8_t addrPins[] = {17, 18, 19, 20};
	uint8_t clockPin   = 14;
	uint8_t latchPin   = 15;
	uint8_t oePin      = 16;
#elif defined(_VARIANT_FEATHER_M4_) // Feather M4 + RGB Matrix FeatherWing
	uint8_t rgbPins[]  = {6, 5, 9, 11, 10, 12};
	uint8_t addrPins[] = {A5, A4, A3, A2};
	uint8_t clockPin   = 13;
	uint8_t latchPin   = 0;
	uint8_t oePin      = 1;
#elif defined(__SAMD51__) // M4 Metro Variants (Express, AirLift)
	uint8_t rgbPins[]  = {6, 5, 9, 11, 10, 12};
	uint8_t addrPins[] = {A5, A4, A3, A2};
	uint8_t clockPin   = 13;
	uint8_t latchPin   = 0;
	uint8_t oePin      = 1;
#elif defined(_SAMD21_) // Feather M0 variants
	uint8_t rgbPins[]  = {6, 7, 10, 11, 12, 13};
	uint8_t addrPins[] = {0, 1, 2, 3};
	uint8_t clockPin   = SDA;
	uint8_t latchPin   = 4;
	uint8_t oePin      = 5;
#elif defined(NRF52_SERIES) // Special nRF52840 FeatherWing pinout
	uint8_t rgbPins[]  = {6, A5, A1, A0, A4, 11};
	uint8_t addrPins[] = {10, 5, 13, 9};
	uint8_t clockPin   = 12;
	uint8_t latchPin   = PIN_SERIAL1_RX;
	uint8_t oePin      = PIN_SERIAL1_TX;
#elif defined(ESP32)
  // 'Safe' pins, not overlapping any peripherals:
  // GPIO.out: 4, 12, 13, 14, 15, 21, 27, GPIO.out1: 32, 33
  // Peripheral-overlapping pins, sorted from 'most expendible':
  // 16, 17 (RX, TX)
  // 25, 26 (A0, A1)
  // 18, 5, 9 (MOSI, SCK, MISO)
  // 22, 23 (SCL, SDA)
  uint8_t rgbPins[]  = {4, 12, 13, 14, 15, 21};
  uint8_t addrPins[] = {16, 17, 25, 26};
  uint8_t clockPin   = 27; // Must be on same port as rgbPins
  uint8_t latchPin   = 32;
  uint8_t oePin      = 33;
#elif defined(ARDUINO_TEENSY40)
  uint8_t rgbPins[]  = {15, 16, 17, 20, 21, 22}; // A1-A3, A6-A8, skip SDA,SCL
  uint8_t addrPins[] = {2, 3, 4, 5};
  uint8_t clockPin   = 23; // A9
  uint8_t latchPin   = 6;
  uint8_t oePin      = 9;
#elif defined(ARDUINO_TEENSY41)
  uint8_t rgbPins[]  = {26, 27, 38, 20, 21, 22}; // A12-14, A6-A8
  uint8_t addrPins[] = {2, 3, 4, 5};
  uint8_t clockPin   = 23; // A9
  uint8_t latchPin   = 6;
  uint8_t oePin      = 9;
#endif


CAdafruitRGBMatrix_EPXDriver::CAdafruitRGBMatrix_EPXDriver(int width, int height)
	: CLEDDriverBase(width * height)
{
	m_width = width;
	m_height = height;
	m_nRainbowIteration = 0;
	brightness = 255;
	m_pRGBmatrixPanel = new Adafruit_Protomatter(
		m_width,					// Width of matrix (or matrix chain) in pixels
		4,							// Bit depth, 1-6
		1, rgbPins,					// # of matrix chains, array of 6 RGB pins for each
		4, addrPins,				// # of address pins (height is inferred), array of pins
		clockPin, latchPin, oePin,	// Other matrix control pins
		true);						// Allow double-buffering
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
	ProtomatterStatus status = m_pRGBmatrixPanel->begin();
	Serial.print("Protomatter begin() status: ");
	Serial.println((int)status);
	if(status != PROTOMATTER_OK) {
		// DO NOT CONTINUE if matrix setup encountered an error.
		for(;;);
	}
}



void CAdafruitRGBMatrix_EPXDriver::SetBrightness(uint8_t brightness)
{
	// This is not a protomatter function, mocked implementation.
	if (b  != brightness) {
		brightness = b;
	}
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
	m_pRGBmatrixPanel->fillScreen(0);
}



void CAdafruitRGBMatrix_EPXDriver::Show()
{
	m_pRGBmatrixPanel->show();
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
		Show();
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
	Show();
	delay(wait);

	m_nRainbowIteration++;
}


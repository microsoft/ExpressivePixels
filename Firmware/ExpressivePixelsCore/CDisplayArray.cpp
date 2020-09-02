// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "CDisplayArray.h"
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_GPIO.h"

EPX_OPTIMIZEFORDEBUGGING_ON


CDisplayArray::CDisplayArray()
{
	m_Powered = false;
	m_brightness = 15;
	m_chaserPosition = 0;
	PowerManagementReset();
}



void CDisplayArray::PowerOff()
{
	if (m_Powered)
	{
		DEBUGLOGLN("DISPLAYARRAY POWERING DOWN");
		m_Powered = false;
		if (m_nDataPin != -1)
		{
			digitalWrite(m_nDataPin, LOW);
			digitalWrite(m_nClockPin, LOW);
		}
		digitalWrite(m_nPowerPin, LOW);
	}
}



void CDisplayArray::PowerOn()
{		
	if (!m_Powered)
	{
		DEBUGLOGLN("DISPLAYARRAY POWERING UP");
		m_Powered = true;	
		if (m_nDataPin != -1)
		{
			digitalWrite(m_nDataPin, LOW);
			digitalWrite(m_nClockPin, LOW);
		}	
		
		digitalWrite(m_nPowerPin, HIGH);
	
		delay(10);
	}
}



void CDisplayArray::Initialize(CLEDDriverBase *pCLEDControllerBase, int width, int height, int powerPin)
{ 
	m_pLEDController = pCLEDControllerBase;	
	m_width = width;
	m_height = height;		
	m_nPowerPin = powerPin;
	if (powerPin != -1)
		pinMode(m_nPowerPin, OUTPUT);

	if(m_pLEDController != NULL)
		m_pLEDController->Initialize();
	SetBrightness(DISPLAYARRAY_DEFAULTBRIGHTNESS);
	Show();
}



void CDisplayArray::PowerManagementReset()
{
	m_lastClearRequest = 0;	
}



void CDisplayArray::Process()
{
	if (m_lastClearRequest > 0 && (millis() - m_lastClearRequest > DISPLAYARRAY_CLEARTIMEOUT_MS))
	{
		PowerOff();		
		m_lastClearRequest = 0;
	}
}



uint8_t CDisplayArray::GetBrightness()
{
	return m_brightness;
}



void CDisplayArray::SetBrightness(uint8_t brightness, bool overrideClamp)
{
	if (!overrideClamp && brightness < DISPLAYARRAY_MINBRIGHTNESS)
		brightness = DISPLAYARRAY_MINBRIGHTNESS;		
	m_brightness = brightness;
	if (m_pLEDController != NULL)
		m_pLEDController->SetBrightness((uint8_t) (((float) 256 / (float) 100) * m_brightness)); // Convert from 0-100 to 0-255 driver range
}



void CDisplayArray::Clear()
{
	if (m_pLEDController != NULL)
		m_pLEDController->Clear();		
	m_lastClearRequest = millis();	
}



void CDisplayArray::Show(bool fromClear)
{
	if (!fromClear)
		PowerOn();
	if (m_Powered && m_pLEDController != NULL)
		m_pLEDController->Show();
}



void CDisplayArray::SetPixelColor(uint16_t i, uint32_t color)
{	
	if (m_pLEDController != NULL)
		m_pLEDController->SetPixel(i, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)color);
	PowerManagementReset();
}



void CDisplayArray::SetPixelRGB(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	if (m_pLEDController != NULL)
		m_pLEDController->SetPixel(i, r, g, b);
	PowerManagementReset();
}




void CDisplayArray::PreviewColor(uint32_t color)
{
	if (m_pLEDController != NULL)
	{		
		for (uint16_t idx = 0; idx < m_pLEDController->NumPixels(); idx++)
			SetPixelColor(idx, color);
		Show();
	}
	PowerManagementReset();
}



void CDisplayArray::RainbowCycle(uint8_t wait) 
{
	PowerOn();
	m_pLEDController->RainbowCycle(wait);
	PowerManagementReset();
}



void CDisplayArray::Chase(bool reset)
{
	if (!m_Powered || m_pLEDController == NULL)
		return;

	if (reset)
		m_chaserPosition = 0;

	m_pLEDController->Clear();
	m_pLEDController->SetPixel(m_chaserPosition, 0x00, 0x00, 0xFF);
	m_pLEDController->Show();

	if (m_chaserPosition < m_pLEDController->NumPixels() - 1)
		m_chaserPosition++;
	else
		m_chaserPosition = 0;
	PowerManagementReset();
}



void CDisplayArray::ShowSingleFrame(char *pFrame, uint16_t pixelCount)
{
	uint8_t		byteRed, byteGreen, byteBlue;
	uint16_t	idx = 0;

	// Extract the pixel count
	if(pixelCount == 0)
		pixelCount = 256;

	// Process pixels
	for(int i = 0 ; i < pixelCount ; i++)
	{
		byteRed = *pFrame++;
		byteGreen = *pFrame++;
		byteBlue = *pFrame++;
		SetPixelRGB(idx, byteRed, byteGreen, byteBlue);
		idx++;
	}
	Show();
	PowerManagementReset();
}



void CDisplayArray::ShowSinglePixel(uint16_t idx, uint32_t color)
{
	SetPixelColor(idx, color);
	Show();
	PowerManagementReset();
}


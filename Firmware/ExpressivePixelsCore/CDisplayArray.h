// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementation for Display array
 **/
#pragma once
#include <stdint.h>
#include <string.h>
#include "CLEDDriverBase.h"

#define DISPLAYARRAY_BYTESPERPIXEL		3
#define DISPLAYARRAY_DEFAULTBRIGHTNESS	25
#define DISPLAYARRAY_MINBRIGHTNESS		5
#define DISPLAYARRAY_CLEARTIMEOUT_MS	4000

// Dimension structure
typedef struct
{
	uint8_t width;
	uint8_t height;	
} DISPLAYINFO;



// Class definition for DisplayArray
class CDisplayArray
{
public:
	CDisplayArray();
	
	bool IsPowered() { return m_Powered; }
	inline int Width() { return m_width; }
	inline int Height() { return m_height; }
	uint8_t GetBrightness();
	uint16_t TotalPixels() { return m_pLEDController->NumPixels(); }
	static inline uint32_t ColorFromBytes(uint8_t r, uint8_t g, uint8_t b)
	{			
		return (uint32_t)(((uint32_t) r << 16) | ((uint32_t) g << 8) | ((uint32_t) b << 0));							
	}	
	void Chase(bool reset);
	void Clear();
	inline void GetPixelBytes(uint32_t color, uint8_t *pr, uint8_t *pg, uint8_t *pb)
	{
		*pr = (uint8_t)(color >> 16);
		*pg = (uint8_t)(color >> 8);
		*pb = (uint8_t)color;
	}
	void Initialize(CLEDDriverBase *pCLEDControllerBase, int width, int height, int powerPin = -1);
	void PreviewColor(uint32_t color);
	bool Process();
	void RainbowCycle(uint8_t wait);
	void SetBrightness(uint8_t brightness, bool overrideClamp = false);
	void SetPixelColor(uint16_t i, uint32_t color);
	void SetPixelRGB(uint16_t i, uint8_t r, uint8_t g, uint8_t b);
	void ShowSingleFrame(char *pFrame, uint16_t pixelCount);
	void ShowSinglePixel(uint16_t	idx, uint32_t color);
	void Show(bool fromClear = false);
	void PowerOff();
	void PowerOn();
	void PowerManagementReset();

private:	
	CLEDDriverBase	*m_pLEDController;	// Reference to underlying display driver

	bool			m_Powered;			// True if display is powered on
	int				m_nPowerPin;		// GPIO pin for display hardware power module
	int				m_nDataPin;			// GPIO pin for display hardware data line
	int				m_nClockPin;		// GPIO pin for display hardware clock line
	int				m_width, m_height;	// Physical dimensions of display

	uint8_t			m_brightness;		// Current brightness of display
	
	uint32_t		m_lastClearRequest;	// Last time display was cleared

	// Chaser control
	int				m_chaserPosition;	// Chaser position 
};



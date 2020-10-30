// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include <stdint.h>
#include "stdlib.h"
#include "string.h"
#include "EPXPlatform_Runtime.h"


#define DEFAULT_BRIGHTNESS 20

typedef struct 
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} LEDDRIVERPIXEL;



class CLEDDriverBase
{
public:
	CLEDDriverBase(uint16_t numPixels)
	{
		m_brightness = DEFAULT_BRIGHTNESS;
		m_numPixels = numPixels;
		m_pPixels = NULL;	
	}
	
	
	~CLEDDriverBase()
	{
		if (m_pPixels != NULL)
			TFREE(m_pPixels);
	}
	
	virtual uint16_t NumPixels() { return m_numPixels; }
	virtual void SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b) 
	{
		if (idx < m_numPixels)
		{
			m_pPixels[idx].r = r;
			m_pPixels[idx].g = g;
			m_pPixels[idx].b = b;
		}
	}
	virtual void SetPixelColor(uint16_t idx, uint32_t color)
	{
		if (idx < m_numPixels)
		{
			m_pPixels[idx].r = (uint8_t)(color >> 16);
			m_pPixels[idx].g = (uint8_t)(color >>  8);
			m_pPixels[idx].b = (uint8_t) color;
		}
	}
	virtual void Initialize()
	{
		m_pPixels = (LEDDRIVERPIXEL *) TMALLOC(sizeof(LEDDRIVERPIXEL) * m_numPixels);
		if (m_pPixels != NULL)
			memset(m_pPixels, 0x00, sizeof(LEDDRIVERPIXEL) * m_numPixels);
	}
	
	virtual uint8_t GetBrightness() { return m_brightness; }
	
	virtual void SetBrightness(uint8_t brightness) { m_brightness = brightness; }

	virtual void Show() = 0;
	
	virtual void RainbowCycle(uint8_t wait) = 0;
	
	
	virtual void Clear()
	{
		memset(m_pPixels, 0x00, sizeof(LEDDRIVERPIXEL) * m_numPixels);
	}
	uint32_t IntColor(uint8_t r, uint8_t g, uint8_t b) { return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b; }
	
protected:
	LEDDRIVERPIXEL	*m_pPixels;
	uint8_t			m_brightness;
	uint16_t		m_numPixels;	
};



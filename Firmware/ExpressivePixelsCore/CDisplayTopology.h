// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include <stdint.h>
#include <string.h>
#include "CDisplayArray.h"

#define TOPOLOGYPIXEL_UNASSIGNED	0xFFFF

class CDisplayTopology
{
public:
	CDisplayTopology()
	{
		m_pTopologyMatrix = 0;
	}
		
	void SetMatrix(uint16_t *pTopologyMatrix, uint16_t displayWidth, uint16_t displayHeight)
	{ 
		m_pTopologyMatrix = pTopologyMatrix;		
		m_displayWidth = displayWidth;
		m_displayHeight = displayHeight;	
		m_topologyPixels = m_displayWidth * m_displayHeight;
	}
	
	
	
	inline uint16_t Map(uint16_t pos) 
	{ 
		if (pos < m_topologyPixels)
			return m_pTopologyMatrix[pos];
		return TOPOLOGYPIXEL_UNASSIGNED;
	}

	uint16_t MapRotated(uint16_t pos, int rotation);

	inline uint16_t TotalPixels() { return m_topologyPixels; }
	
private:
	uint16_t	m_displayWidth, m_displayHeight;
	uint16_t	m_topologyPixels;
	uint16_t	*m_pTopologyMatrix;
};


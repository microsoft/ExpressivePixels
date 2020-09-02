// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "CDisplayTopology.h"



uint16_t CDisplayTopology::MapRotated(uint16_t pos, int rotation) 
{ 
	if (pos < m_topologyPixels)
	{
		int srcX, srcY;
		
		srcY = pos / m_displayWidth;
		srcX = pos - (srcY * m_displayWidth);				
		switch (rotation)
		{
			case 90:
				return m_pTopologyMatrix[((m_displayWidth - srcX - 1) * m_displayWidth) + srcY];
								
			case 180:
			return m_pTopologyMatrix[((m_displayHeight - 1 - srcY) * m_displayWidth) + (m_displayWidth - 1 - srcX)];
			
			case 270:
				return m_pTopologyMatrix[(srcX * m_displayWidth) + (m_displayHeight - srcY - 1)];
		}		
		return m_pTopologyMatrix[pos];
	}
	return TOPOLOGYPIXEL_UNASSIGNED;
}




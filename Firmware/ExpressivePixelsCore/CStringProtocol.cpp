// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <string.h>
#include "EPXApp.h"
#include "EPXPlatform_Runtime.h"
#include "CStringProtocol.h"

EPX_OPTIMIZEFORDEBUGGING_ON

CStringProtocol::CStringProtocol()
{
	m_stringBuilderLength = 0;
}



void CStringProtocol::Reset()
{
	m_stringBuilderLength = 0;
	StringPayloadReset();
}


int g_cntr = 0;

void CStringProtocol::StringProtocolProcess()
{
	if (m_pCActiveStringProtocolSerialChannel != NULL)
	{
		while (m_pCActiveStringProtocolSerialChannel->available())
		{
			char cVal = m_pCActiveStringProtocolSerialChannel->read();
			if (cVal == 0x00)
			{
				if (++g_cntr == 3)
					g_cntr++;
				StringPayloadFinalized();				
				Reset();
			}
			else
			{
				StringPayloadParse(cVal);
				
				// If the max length of the stringbuilder has been reached just reset the process
				if (m_stringBuilderLength == MAX_STRING_PROTOCOL_LENGTH - 1)
					Reset();
				else
					m_stringBuilderLength++;
			}
		}
	}	
}



void CStringProtocol::StringProtocolSend(uint8_t *payload, uint16_t payloadLength)
{
	m_pCActiveStringProtocolSerialChannel->write(payload, payloadLength, true);	
}

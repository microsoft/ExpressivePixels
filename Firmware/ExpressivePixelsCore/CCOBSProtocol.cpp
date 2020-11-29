// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <string.h>
#include "EPXApp.h"
#include "EPXPlatform_Runtime.h"
#include "COBSCodec.h"
#include "CCOBSProtocol.h"

EPX_OPTIMIZEFORDEBUGGING_ON

static uint8_t g_magicNAK[8] = {0xA1, 0x85, 0x29, 0xB8, 0xBC, 0xED, 0xFE, 0x62 };




CCOBSProtocol::CCOBSProtocol()
{
	m_pCActiveSerialChannel = NULL;
	m_ProtocolVersion = 0;
	ProtocolReset();
}



void CCOBSProtocol::ProtocolReset()
{
	m_progressCompletionTracking = false;
	m_lastProgressCompletion = 0;
	m_ProtocolPacketReceiveRemaining = 0;
	m_channelMaxCacheLoad = 0;
	m_COBS_StagingFillIndex = 0;
}



void CCOBSProtocol::ProtocolProcess()
{		
	unsigned long curMillis = millis();

	// Process frames
	if(m_pCActiveSerialChannel != NULL)
	{
		if (m_pCActiveSerialChannel->available() > m_channelMaxCacheLoad)
			m_channelMaxCacheLoad = m_pCActiveSerialChannel->available();

		while (m_pCActiveSerialChannel->available())
		{		
			m_COBS_StagingBuffer[m_COBS_StagingFillIndex++] = m_pCActiveSerialChannel->read();
			if (m_COBS_StagingBuffer[m_COBS_StagingFillIndex - 1] == 0x00)
			{
				size_t decodedFrameLength = COBS_Decode(m_COBS_StagingBuffer, m_COBS_StagingFillIndex - 1, m_COBS_ProcessedBuffer);
				if (decodedFrameLength > 0)
				{
					EPXPROTOCOL_0 *pEPXProtocol0 = (EPXPROTOCOL_0 *) m_COBS_ProcessedBuffer;
					switch (pEPXProtocol0->frameType)					
					{
					case EPX_FRAMETYPE_HEADERPLUSDATA:
						{
							EPXPROTOCOL_HEADER *pEPXHeader = (EPXPROTOCOL_HEADER *) pEPXProtocol0;
							m_ProtocolFormat = pEPXHeader->format;
							m_ProtocolFlags = pEPXHeader->flags;
							
							//DEBUGLOGLN("EPX_FRAMETYPE_HEADERPLUSDATA %d", pEPXHeader->length);
														
							memcpy(&m_ProtocolPacketReceiveSize, &pEPXHeader->length, sizeof(uint16_t));						
							m_ProtocolPacketReceiveRemaining = m_ProtocolPacketReceiveSize;
							ProtocolDecode((uint8_t *)(pEPXHeader + 1), decodedFrameLength - sizeof(EPXPROTOCOL_HEADER));
						}
						break;
					
					case EPX_FRAMETYPE_HEADERPLUSDATA32:
					{
						EPXPROTOCOL_HEADER32* pEPXHeader32 = (EPXPROTOCOL_HEADER32*)pEPXProtocol0;
						m_ProtocolFormat = pEPXHeader32->format;
						m_ProtocolFlags = pEPXHeader32->flags;

						m_ProtocolPacketReceiveSize = (uint32_t)pEPXHeader32->length;
						m_ProtocolPacketReceiveRemaining = m_ProtocolPacketReceiveSize;
						ProtocolDecode((uint8_t*)(pEPXHeader32 + 1), decodedFrameLength - sizeof(EPXPROTOCOL_HEADER32));
					}
					break;

					case EPX_FRAMETYPE_DATA:
						ProtocolDecode((uint8_t *)(pEPXProtocol0 + 1), decodedFrameLength - sizeof(EPXPROTOCOL_0));
						break;
					}
					
					// Process flow control if requested
					if (m_ProtocolFlags & EPX_PROTOCOLFLAG_ACKRQ)
						ProtocolSendACK();
				}
				else
				{
					DEBUGLOGLN("COBSDecode Failure");
					
				}
				m_COBS_StagingFillIndex = 0;
			}
			if (m_COBS_StagingFillIndex == COBS_MAXFRAME_SIZE)
			{
				DEBUGLOGLN("ProtocolDecode m_COBS_StagingFillIndex == COBS_MAXFRAME_SIZE");	
				m_COBS_StagingFillIndex = 0;
			}
		}		
	}
}



void CCOBSProtocol::ProtocolDecode(uint8_t *p, uint32_t length)
{
	// If more was received than expected for this packet
	if(length > m_ProtocolPacketReceiveRemaining)
	{
		DEBUGLOGLN("ProtocolDecode length > m_ProtocolPacketReceiveRemaining");
		// Reset everything
		ProtocolReset();
		PayloadReset();
		return;
	}
				
	m_ProtocolPacketReceiveRemaining -= length;	
	if (m_progressCompletionTracking)
	{					
		int delta = m_ProtocolPacketReceiveSize / 10;
		int currentProgressCompletion = (m_ProtocolPacketReceiveSize - m_ProtocolPacketReceiveRemaining) / delta;
		if (currentProgressCompletion > m_lastProgressCompletion)
		{
		//	DEBUGLOGLN("Completion %d %d", currentProgressCompletion, m_ProtocolPacketReceiveRemaining);
			m_lastProgressCompletion = currentProgressCompletion;
			SendTransmissionCompletionUpdate(currentProgressCompletion);
		}
	}
	
	while (length--)
		PayloadParse(m_ProtocolFormat, *p++);

	if (m_ProtocolPacketReceiveRemaining == 0)
	{
		PayloadFinalized(m_ProtocolFormat);
		ProtocolReset();
		PayloadReset();
		return;
	}		
}




void CCOBSProtocol::ProtocolEncodeAndSend(uint8_t payloadFormat, uint8_t *payload, uint16_t payloadLength)
{
	if (m_pCActiveSerialChannel != NULL)
	{		
		uint8_t frameType = EPX_FRAMETYPE_HEADERPLUSDATA;
		int  lengthRemaining = payloadLength;
		int currentOffset = 0;
		int frameHeaderLength = sizeof(EPXPROTOCOL_HEADER);
		uint8_t cobsStagingBuffer[256], cobsProcessedBuffer[256];
	
		while (currentOffset < lengthRemaining)
		{
			uint16_t lengthStagingFrame = epxmin(MAX_COBS_DATAFRAMESIZE, frameHeaderLength +  lengthRemaining - currentOffset);
			uint16_t lengthEncodedFrame = lengthStagingFrame + (1 + 1);   // (COBS code + zero byte terminator)
		
			cobsStagingBuffer[0] = frameType;
			if (currentOffset == 0)
			{
				// Add the payload type to the first frame's header
				cobsStagingBuffer[1] = payloadFormat;

				// Add the payload length to the first frame's header
				memcpy(&cobsStagingBuffer[2], &payloadLength, sizeof(uint16_t));
				
				// Flags
				cobsStagingBuffer[4] = 0;
			}

			int payloadBytesToCopy = epxmin(lengthStagingFrame - frameHeaderLength, payloadLength);
			memcpy(&cobsStagingBuffer[frameHeaderLength], payload + currentOffset, payloadBytesToCopy);
			currentOffset += payloadBytesToCopy;
		
			COBS_Encode(cobsStagingBuffer, lengthStagingFrame, cobsProcessedBuffer);
			cobsProcessedBuffer[lengthEncodedFrame - 1] = 0x00;			
			m_pCActiveSerialChannel->write((void *) cobsProcessedBuffer, lengthEncodedFrame, false);

			// Next frame is just a data frame
			frameType = EPX_FRAMETYPE_DATA;
			frameHeaderLength = sizeof(EPXPROTOCOL_0);
		}
	}
}



void CCOBSProtocol::ProtocolSendACK()
{
	ProtocolEncodeAndSend(EPX_PROTOCOLFORMAT_BINARY_ACK, g_magicNAK, sizeof(g_magicNAK));
}


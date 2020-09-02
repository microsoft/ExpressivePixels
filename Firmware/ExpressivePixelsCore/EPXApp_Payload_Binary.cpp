// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <stdlib.h>
#include "EPXApp.h"

EPX_OPTIMIZEFORDEBUGGING_ON



void CExpressivePixelsApp::PayloadParseBinary(uint8_t data)
{
	if (!m_bDynamicPayloadFilling && m_PayloadBinaryFillPos < MAX_COMMAND_VALUE)
		m_szPayloadCommandValue[m_PayloadBinaryFillPos++] = data;	
	
	if (m_PayloadActiveCommand == 0)
	{	
		// Wait for enough of the protocol header to be received
		if (m_PayloadBinaryFillPos >= sizeof(EPXAPP_PROTOCOL_HEADER))
		{
			// Start tracking the payload command type
			EPXAPP_PROTOCOL_HEADER *pHeader = (EPXAPP_PROTOCOL_HEADER *) m_szPayloadCommandValue;
			m_PayloadActiveCommand = pHeader->command;
		}
	}
	else
	{
		if (m_bDynamicPayloadFilling)
		{
			switch (m_PayloadActiveCommand)
			{
			case PAYLOADCOMMAND_UPLOAD_FRAME8:
				if (m_StagedAnimation.pRAMFrames != NULL && m_PayloadParseCommandSequenceFillPos < m_StagedAnimation.Meta.cbFrames)
					m_StagedAnimation.pRAMFrames[m_PayloadParseCommandSequenceFillPos++] = data;
				break;
						
			case PAYLOADCOMMAND_STORE_ANIMATION8:
				{
					EPXAPP_PROTOCOL_ANIMATIONPAYLOAD *pAnimationPayload = (EPXAPP_PROTOCOL_ANIMATIONPAYLOAD *) m_szPayloadCommandValue;

					m_CAppStorage.SequenceWriteData(&m_StagedAnimation, &data, sizeof(data));						
					m_animationPayloadStateMachine.sectionBytesRemaining--;
					if (m_animationPayloadStateMachine.sectionBytesRemaining == 0)
					{
						switch (m_animationPayloadStateMachine.state)
						{										
						case ANIMATIONPAYLOAD_STATE_NAME:
							// Finalize string
							m_StagedAnimation.pszName[m_animationPayloadStateMachine.sectionFillPos++] = data;
							m_StagedAnimation.pszName[m_animationPayloadStateMachine.sectionFillPos] = 0x00;
							
							m_animationPayloadStateMachine.state = ANIMATIONPAYLOAD_STATE_PALETTE;
							m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_PALLETESIZE, (uint8_t *) &pAnimationPayload->paletteSize, sizeof(pAnimationPayload->paletteSize));
							m_CAppStorage.SequenceWriteToken(&m_StagedAnimation, SEQUENCETOKEN_PALLETEBYTES);
							m_animationPayloadStateMachine.sectionFillPos = 0;
							m_animationPayloadStateMachine.sectionBytesRemaining = pAnimationPayload->paletteSize * sizeof(PALETTE_ENTRY);
							break;
										
						case ANIMATIONPAYLOAD_STATE_PALETTE:
							m_animationPayloadStateMachine.state = ANIMATIONPAYLOAD_STATE_FRAMES;
							m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_FRAMESBYTELEN, (uint8_t *) &pAnimationPayload->framesPayloadSize, sizeof(pAnimationPayload->framesPayloadSize));
							m_CAppStorage.SequenceWriteToken(&m_StagedAnimation, SEQUENCETOKEN_FRAMESBYTES);
							m_animationPayloadStateMachine.sectionFillPos = 0;
							m_animationPayloadStateMachine.sectionBytesRemaining = pAnimationPayload->framesPayloadSize;
							break;
						}
					}
					else
					{
						switch (m_animationPayloadStateMachine.state)
						{										
							case ANIMATIONPAYLOAD_STATE_NAME:
								m_StagedAnimation.pszName[m_animationPayloadStateMachine.sectionFillPos++] = data;
								break;
						}
					}
				}
				break;
			}			
		}
		else
		{			
			// Header has been received, now track the payload data
			switch(m_PayloadActiveCommand)
			{
			case PAYLOADCOMMAND_UPLOAD_FRAME8:
				if (!m_bDynamicPayloadFilling && m_PayloadBinaryFillPos >= sizeof(EPXAPP_PROTOCOL_DEVICEFRAMEPAYLOAD))
				{
					EPXAPP_PROTOCOL_DEVICEFRAMEPAYLOAD *pFramePayload = (EPXAPP_PROTOCOL_DEVICEFRAMEPAYLOAD *) m_szPayloadCommandValue;				
					m_StagedAnimation.Meta.cbFrames = (pFramePayload->width * pFramePayload->height * 3);
					m_StagedAnimation.pRAMFrames = (uint8_t *) malloc(m_StagedAnimation.Meta.cbFrames);
					m_PayloadParseCommandSequenceFillPos = 0;
					m_bDynamicPayloadFilling = true;
				}			
				break;
		
			case PAYLOADCOMMAND_STORE_ANIMATION8:
				if (!m_bDynamicPayloadFilling && m_PayloadBinaryFillPos >= sizeof(EPXAPP_PROTOCOL_ANIMATIONPAYLOAD))
				{
					EPXAPP_PROTOCOL_ANIMATIONPAYLOAD *pAnimationPayload = (EPXAPP_PROTOCOL_ANIMATIONPAYLOAD *) m_szPayloadCommandValue;
							
					CompletionTrackingEnable();
					uint8_t guidLen = sizeof(pAnimationPayload->id);
					BytesToHex(pAnimationPayload->id, sizeof(pAnimationPayload->id), m_StagedAnimation.szID, sizeof(m_StagedAnimation.szID));					
					m_StagedAnimation.utcTimeStamp = pAnimationPayload->utcTimestamp;
					
					guidLen = strlen(m_StagedAnimation.szID);
					m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_GUIDLEN, &guidLen, sizeof(guidLen));
					m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_GUID, (uint8_t *) m_StagedAnimation.szID, guidLen);
					m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_LOOPCOUNT, &pAnimationPayload->loopCount, sizeof(pAnimationPayload->loopCount));
					m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_FRAMERATE, &pAnimationPayload->frameRate, sizeof(pAnimationPayload->frameRate));
					m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_FRAMECOUNT, (uint8_t *) &pAnimationPayload->frameCount, sizeof(pAnimationPayload->frameCount));
					m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_UTCTIMESTAMP, (uint8_t *) &pAnimationPayload->utcTimestamp, sizeof(pAnimationPayload->utcTimestamp));
																
					m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_NAMELEN, &pAnimationPayload->nameLength, sizeof(pAnimationPayload->nameLength));
					m_animationPayloadStateMachine.sectionBytesRemaining = pAnimationPayload->nameLength;							
					m_StagedAnimation.pszName = (char *) malloc(pAnimationPayload->nameLength + 1);					
					m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_NAME);
									
					// Kick of the state machine for the remainder of the payload
					m_animationPayloadStateMachine.state = ANIMATIONPAYLOAD_STATE_NAME;
					m_bDynamicPayloadFilling = true;
				}
				break;
			}		
		}
	}
}


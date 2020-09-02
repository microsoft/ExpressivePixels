// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <stdlib.h>
#include "EPXApp.h"
#include "EPXPlatform_Crypto.h"
#include "EPXPlatform_Settings.h"
#include "CDisplayArray.h"
#include "COBSCodec.h"

EPX_OPTIMIZEFORDEBUGGING_ON

void CExpressivePixelsApp::PayloadReset()
{
	m_bDynamicPayloadFilling = false;
	m_PayloadActiveCommand = 0;
	m_PayloadActiveTransactionID = 0;
	m_PayloadStreamingJSONParser.reset();	
	m_PayloadBinaryFillPos = 0;
	m_nPayloadCommandValue = 0;
	m_nPayloadCommandValue2 = 0;
	m_szPayloadCommandValue[0] = 0x00;
	m_PayloadParseCommandSequenceFillPos = 0;

	// Free just in case the AnimatonEngine didn't manage to attach
	CAnimationManager::ExpressivePixelSequenceFree(&m_StagedAnimation);
	
	// Free state machine
	memset(&m_animationPayloadStateMachine, 0x00, sizeof(m_animationPayloadStateMachine));
}



void CExpressivePixelsApp::PayloadParse(uint8_t protocolFormat, uint8_t data)
{
	switch (protocolFormat)
	{
	case EPX_PROTOCOLFORMAT_BINARY:
		PayloadParseBinary(data);
		break;
		
	case EPX_PROTOCOLFORMAT_JSON:
		m_PayloadStreamingJSONParser.parse(data); 
		break;
	}
	
}



void CExpressivePixelsApp::PayloadProcessFromJSON(const char *pszJSON)
{
	if (pszJSON != NULL)
	{
		while (*pszJSON != 0x00)
		{
			m_PayloadStreamingJSONParser.parse(*pszJSON);
			pszJSON++;
		}

		PayloadExecute(EPX_PROTOCOLFORMAT_JSON);
		PayloadReset();
	}
}



void CExpressivePixelsApp::PayloadFinalized(uint8_t format)
{
	PayloadExecute(format);
}



void CExpressivePixelsApp::PayloadExecute(uint8_t format)
{	
	LogActiveCommand();
	switch (m_PayloadActiveCommand)
	{
	case PAYLOADCOMMAND_CONNECT_HEADERRQ:
		{
			EPXString response;

			response += JSON_OPENOBJECT;
				response += JSON_KEYVALUE_STRINGPAIR_CONTINUED(JSON_STATUS, JSON_SUCCESS);
				response += JSON_KEYVALUE_STRINGPAIR_CONTINUED((const char *) JSONKEY_TRANSACTIONID, EPXString((int) m_PayloadActiveTransactionID));
				response += JSON_KEYOBJECTOPEN(JSON_DATA);
				response += GetDeviceResponseInfo();
				response += JSON_CLOSEOBJECT;
			response += JSON_CLOSEOBJECT;

			// Send response back to host
			DataChannelSendResponseJSON(response);			
		}
		break;

	case PAYLOADCOMMAND_CLEARDISPLAY:
		m_CAppStorage.AutoPlaylistClear();
		m_CAnimator.Clear();
		break;

	case PAYLOADCOMMAND_DISPLAY_BRIGHTNESS:	
		SetBrightness((uint8_t) m_nPayloadCommandValue);
		break;

	case PAYLOADCOMMAND_ENUMERATE_ANIMATIONS:
		{
			EPXString response;

			m_CAppStorage.EnumerateSequencesJSON(response, m_PayloadActiveTransactionID);
			DataChannelSendResponseJSON(response);   // Send response back to host
		}
		break;

	case PAYLOADCOMMAND_PREVIEW_COLOR:
		{
			char *pszHex = m_szPayloadCommandValue;

			// Stop any running animation
			m_CAppStorage.AutoPlaylistClear();
			m_CAnimator.Stop();

			// Extract triplet hex values from json value
			unsigned char byteRed = HexToByte(pszHex, 2);
			unsigned char byteGreen = HexToByte(pszHex + 2, 2);
			unsigned char byteBlue = HexToByte(pszHex + 4, 2);

			uint32_t color = (uint32_t)(((uint32_t)byteRed << 16) | ((uint32_t)byteGreen << 8) | ((uint32_t)byteBlue << 0));
			m_CDisplayArray.PreviewColor(color);
		}
		break;

	case PAYLOADCOMMAND_REMOVE_ANIMATION:
		m_CAnimator.Clear();
		m_CAppStorage.SequenceDelete(m_szPayloadCommandValue);
		break;

	case PAYLOADCOMMAND_PLAY_STORED_ANIMATION8_BYNAME:
		{
			char *pszID = m_CAppStorage.SequenceIDFromName(m_szPayloadCommandValue);
			if (pszID == NULL)
				break;
			strcpy(m_szPayloadCommandValue, pszID);
		}
		// Fall through
				
	case PAYLOADCOMMAND_PLAY_STORED_ANIMATION8 :
	case PAYLOADCOMMAND_UPLOAD_ANIMATION8 :
		if(m_CAnimator.CanPlay())
		{
			EXPRESSIVEPIXEL_SEQUENCE sequence;
				
			m_CAppStorage.AutoPlaylistClear();
			m_CAnimator.Clear();			
			if (m_PayloadActiveCommand == PAYLOADCOMMAND_UPLOAD_ANIMATION8)
				m_CAppStorage.SequenceWriteClose(&m_StagedAnimation);
			
			if (m_CAppStorage.SequenceRead(m_PayloadActiveCommand == PAYLOADCOMMAND_UPLOAD_ANIMATION8 ? (char *) PLAYNOWID : m_szPayloadCommandValue, &sequence))
			{
				if (m_nPayloadCommandValue > 0)
					sequence.Meta.loopCount = m_nPayloadCommandValue; 
				if (m_nPayloadCommandValue2)	
					sequence.runUntilComplete = true;
				m_CAnimator.Activate(&sequence);
			}
		}
		break;
		
	case PAYLOADCOMMAND_REQUEST_THUMBNAIL:
		{
			int						 width = m_CDisplayArray.Width();
			int						 height = m_CDisplayArray.Height();
			uint16_t				 payloadLength;
			BINARY_CHANNEL_RESPONSE	 *pChannelResponse;
			EPX_THUMBNAIL_HEADER	 *pThumbnailHeader;
			uint8_t					 *pFrameBytes;

			payloadLength = sizeof(BINARY_CHANNEL_RESPONSE) + sizeof(EPX_THUMBNAIL_HEADER) + (width * height * DISPLAYARRAY_BYTESPERPIXEL);			
			uint8_t	*pPayload = (uint8_t *) malloc(payloadLength);
			if (pPayload != NULL)
			{
				ACTIVE_ANIMATIONSEQUENCE activeAnimation;
			
				memset(pPayload, 0x00, payloadLength);
				pChannelResponse = (BINARY_CHANNEL_RESPONSE	*) pPayload;
				pChannelResponse->transactionID = m_PayloadActiveTransactionID;

				pThumbnailHeader = (EPX_THUMBNAIL_HEADER *)(pChannelResponse + 1);
				pFrameBytes = (uint8_t *)(pThumbnailHeader + 1);
				
				memset(&activeAnimation, 0x00, sizeof(activeAnimation));
				if (m_CAppStorage.SequenceRead(m_szPayloadCommandValue, &activeAnimation.Sequence, true))
				{
					memcpy(&pThumbnailHeader->guid, &g_displayDesignGUID, sizeof(g_displayDesignGUID));
					pThumbnailHeader->utcTimeStamp = activeAnimation.Sequence.utcTimeStamp;
					pThumbnailHeader->width = width;
					pThumbnailHeader->height = height;
					m_CAnimator.RenderFrameToBytes(&activeAnimation, pFrameBytes);					
					CAnimationManager::ExpressivePixelSequenceFree(&activeAnimation.Sequence);				
				}
				else
					payloadLength = sizeof(BINARY_CHANNEL_RESPONSE);
				DataChannelSendResponse(pPayload, payloadLength);
				free(pPayload);
			}
		}
		break;
		
	case PAYLOADCOMMAND_STORE_ANIMATION8:
		m_CAppStorage.AutoPlaylistClear();
		m_CAnimator.Clear();		
		m_CAppStorage.SequenceWriteClose(&m_StagedAnimation);
		CAnimationManager::ExpressivePixelSequenceFree(&m_StagedAnimation);
		break;

	case PAYLOADCOMMAND_UPLOAD_FRAME8:
		m_CAppStorage.AutoPlaylistClear();
		m_CAnimator.Stop();
		if(m_StagedAnimation.pRAMFrames != NULL)
		{			
			m_CAnimator.ShowSingleFrame(m_StagedAnimation.pRAMFrames, m_StagedAnimation.Meta.cbFrames / 3);  // 3 bytes per pixel
			CAnimationManager::ExpressivePixelSequenceFree(&m_StagedAnimation);
		}
		break;

	case PAYLOADCOMMAND_UPLOAD_PIXEL8:
		{
			int				pixelPosition;
			unsigned char	byteRed, byteGreen, byteBlue;
			uint32_t		color;
			uint16_t		pixelIndex;
			
			if (format == EPX_PROTOCOLFORMAT_BINARY)
			{
				EPXAPP_PROTOCOL_DEVICEPIXELPAYLOAD *pUploadPixelPayload = (EPXAPP_PROTOCOL_DEVICEPIXELPAYLOAD *) m_szPayloadCommandValue;							
				pixelIndex = pUploadPixelPayload->pixelIndex;
				byteRed = pUploadPixelPayload->pixR;
				byteGreen = pUploadPixelPayload->pixG;
				byteBlue = pUploadPixelPayload->pixB;
			}
			else
			{
				char *pszHex = m_szPayloadCommandValue;

				// Extract triplet hex values from json value
				byteRed = HexToByte(pszHex, 2);
				byteGreen = HexToByte(pszHex + 2, 2);
				byteBlue = HexToByte(pszHex + 4, 2);
				pixelIndex = m_nPayloadCommandValue;
			}
			color = (uint32_t)(((uint32_t)byteRed << 16) | ((uint32_t)byteGreen << 8) | ((uint32_t)byteBlue << 0));					
			m_CAnimator.ShowSinglePixel(pixelIndex, color);
		}
		break;

	case PAYLOADCOMMAND_TTY:
		ExecuteTTYCommand();
		break;
		
	case PAYLOADCOMMAND_SETDEVICENAME:
		strncpy(g_appSettings.szDeviceName, m_szPayloadCommandValue, sizeof(g_appSettings.szDeviceName));
		DEBUGLOGLN("SETDEVICENAME %s", g_appSettings.szDeviceName);		
		CSettings::WriteString((const char *)SETTINGSKEY_DEVICENAME, g_appSettings.szDeviceName);
		m_bRebootOnDisconnect = true;
		break;
		
	case PAYLOADCOMMAND_SETKEY:
		if (strlen(m_szPayloadCommandValue) == 0)
			ClearAESKey();
		else if (strlen(m_szPayloadCommandValue) / 2 == EPX_AES_KEY_BYTE_SIZE)
		{
			char *pszCur = m_szPayloadCommandValue;
			int bytePos = 0;
			
			while (*pszCur != 0x00)
			{
				m_aesKey[bytePos++] = HexToByte(pszCur, 2);				
				pszCur += 2;
			}
			
			DEBUGLOGLN("SETKEY");		
			CSettings::Write((const char *)SETTINGSKEY_AESKEY, m_aesKey, sizeof(m_aesKey));
		}
		break;
	}
}



void CExpressivePixelsApp::DataChannelPurge()
{
	int bytesPurged = 0;
	while (m_pCActiveSerialChannel != NULL && m_pCActiveSerialChannel->available())
	{
		bytesPurged++;
		m_pCActiveSerialChannel->read();
	}

	DEBUGLOGLN("PayloadChannelPurge SERIAL %s bytes", EPXString(bytesPurged).c_str());
}




void CExpressivePixelsApp::DataChannelSendResponseJSON(EPXString &response)
{
	ProtocolEncodeAndSend(EPX_PROTOCOLFORMAT_JSON, (uint8_t *) response.c_str(), response.length());
}



void CExpressivePixelsApp::DataChannelSendResponse(void *pData, uint16_t payloadLength)
{
	ProtocolEncodeAndSend(EPX_PROTOCOLFORMAT_BINARY, (uint8_t *) pData, payloadLength);
}



void CExpressivePixelsApp::SendAuthenticationChallenge()
{
	EPXString response;
	char szNONCE[(EPX_NONCE_SIZE * 2) + 1];
	
	BytesToHex(m_currentNONCE, sizeof(m_currentNONCE), (char *) szNONCE, sizeof(szNONCE));	
	szNONCE[EPX_NONCE_SIZE * 2] = 0x00;
	response += JSON_OPENOBJECT;
	response += JSON_KEYVALUE_STRINGPAIR(JSON_CHALLENGE, EPXString(szNONCE).c_str());
	response += JSON_CLOSEOBJECT;
	DEBUGLOGLN("\tSending challenge");
	DataChannelSendResponseJSON(response);
}



void CExpressivePixelsApp::SendTransmissionCompletionUpdate(uint8_t progress)
{
	EPXString response;

	response += JSON_OPENOBJECT;
	response += JSON_KEYVALUE_VALUEPAIR(JSON_PROGRESS, EPXString(progress).c_str());
	response += JSON_CLOSEOBJECT;
	DataChannelSendResponseJSON(response);
}



char g_szActiveCommands[PAYLOADCOMMAND_KNOWN_MAX][48] = 
	{
		"", "PAYLOADCOMMAND_CONNECT_HEADERRQ", "PAYLOADCOMMAND_CLEARDISPLAY", "PAYLOADCOMMAND_DISPLAY_BRIGHTNESS", "PAYLOADCOMMAND_ENUMERATE_ANIMATIONS",
			"PAYLOADCOMMAND_PREVIEW_COLOR", "PAYLOADCOMMAND_UPLOAD_FRAME8", "PAYLOADCOMMAND_UPLOAD_ANIMATION8", "PAYLOADCOMMAND_UPLOAD_PIXEL8",
			"PAYLOADCOMMAND_REMOVE_ANIMATION", "PAYLOADCOMMAND_STORE_ANIMATION8", "PAYLOADCOMMAND_PLAY_STORED_ANIMATION8", "PAYLOADCOMMAND_PLAY_STORED_ANIMATION8_BYNAME",
			"PAYLOADCOMMAND_SETDEVICENAME", "PAYLOADCOMMAND_REQUEST_THUMBNAIL", "PAYLOADCOMMAND_AUTHENTICATION", "PAYLOADCOMMAND_SETKEY"
	};



void CExpressivePixelsApp::LogActiveCommand()
{
	if (m_PayloadActiveCommand < PAYLOADCOMMAND_KNOWN_MAX)	
		{DEBUGLOGLN("COMMAND %s", g_szActiveCommands[m_PayloadActiveCommand]); }
	else
		{DEBUGLOGLN("COMMAND %d", m_PayloadActiveCommand); }
}


// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <stdlib.h>
#include <string.h>
#include "EPXApp.h"

EPX_OPTIMIZEFORDEBUGGING_ON

uint32_t _transStart = 0, _transEnd = 0;

/**** JsonListener inplementation ****/
void CExpressivePixelsApp::key(char *key)
{
	m_PayloadJSONListnerTracking_Key = key;

	if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_FRAMESHEX) == 0 ||
		stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PALETTEHEX) == 0 ||
		stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PIXELSHEX) == 0)
		m_PayloadStreamingJSONParser.setHEXByteMode();
}



void CExpressivePixelsApp::byteAsHexValue(char *value)
{
	if (m_PayloadActiveCommand == PAYLOADCOMMAND_UPLOAD_ANIMATION8 || m_PayloadActiveCommand == PAYLOADCOMMAND_STORE_ANIMATION8)
	{
		if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_FRAMESHEX) == 0)
		{
			uint8_t byteVal = HexToByte(value, 2);
			if (m_StagedAnimation.pFile != NULL)
				m_CAppStorage.SequenceWriteData(&m_StagedAnimation, &byteVal, sizeof(byteVal));
			else if (m_StagedAnimation.pRAMFrames != NULL && m_PayloadParseCommandSequenceFillPos < m_StagedAnimation.Meta.cbFrames)
				m_StagedAnimation.pRAMFrames[m_PayloadParseCommandSequenceFillPos++] = byteVal;
		}
		else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PALETTEHEX) == 0)
		{
			uint8_t byteVal = HexToByte(value, 2);
			m_PayloadParsePaletteRGB[m_PayloadParsePaletteBytePos] = byteVal;
			m_PayloadParsePaletteBytePos++;
			if (m_PayloadParsePaletteBytePos == 3)
			{
				PALETTE_ENTRY paletteEntry;
				
				paletteEntry.r = m_PayloadParsePaletteRGB[0];
				paletteEntry.g = m_PayloadParsePaletteRGB[1];
				paletteEntry.b = m_PayloadParsePaletteRGB[2];
				
				// uint32_t color = (uint32_t)(((uint32_t)m_PayloadParsePaletteRGB[0] << 16) | ((uint32_t)m_PayloadParsePaletteRGB[1] << 8) | ((uint32_t)m_PayloadParsePaletteRGB[2] << 0));							
				if (m_StagedAnimation.pFile != NULL)
					m_CAppStorage.SequenceWriteData(&m_StagedAnimation, &paletteEntry, sizeof(paletteEntry));
				else if(m_StagedAnimation.pPalette != NULL && m_PayloadParseCommandSequenceFillPos < m_StagedAnimation.Meta.cbPallete)
					m_StagedAnimation.pPalette[m_PayloadParseCommandSequenceFillPos++] = paletteEntry;
				m_PayloadParsePaletteBytePos = 0;
			}		
		}
	}
	else if (m_PayloadActiveCommand == PAYLOADCOMMAND_UPLOAD_FRAME8)
	{
		if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PIXELSHEX) == 0)
		{
			if (m_StagedAnimation.pRAMFrames != NULL && m_PayloadParseCommandSequenceFillPos < m_StagedAnimation.Meta.cbFrames)
			{
				uint8_t byteVal = HexToByte(value, 2);
				m_StagedAnimation.pRAMFrames[m_PayloadParseCommandSequenceFillPos++] = byteVal;
			}
		}
	}
}



void CExpressivePixelsApp::value(char *value)
{
	//DEBUGLOGLN("JSON key %s", m_PayloadJSONListnerTracking_Key.c_str());
	if(!m_bAuthenticated)
	{
		if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *) JSONKEY_AUTHENTICATIONRESPONSE) == 0)
			Authenticate(epx_strupr(value));
	}
	else
	{
		m_PayloadJSONListnerTracking_Value = value;
		if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_CONSOLECOMMAND) == 0)
			m_TTYValue = m_PayloadJSONListnerTracking_Value;
		/**** COMMAND Key ****/	
		else if(stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_COMMAND) == 0 ||
			stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_CMD) == 0)
		{
			if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_CONSOLE_COMMAND) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_TTY;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_CLEAR_DISPLAY) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_CLEARDISPLAY;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_SETDEVICENAME) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_SETDEVICENAME;			
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_SETKEY) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_SETKEY;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_CONNECT_HEADERRQ) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_CONNECT_HEADERRQ;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_ENUMERATE_ANIMATIONS) == 0 ||
					 stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_ENUMERATE_ANIMATIONS2) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_ENUMERATE_ANIMATIONS;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_PREVIEW_COLOR) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_PREVIEW_COLOR;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_DISPLAY_BRIGHTNESS) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_DISPLAY_BRIGHTNESS;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_UPLOAD_FRAME8) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_UPLOAD_FRAME8;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_UPLOAD_ANIMATION8) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_UPLOAD_ANIMATION8;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_UPLOAD_PIXEL8) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_UPLOAD_PIXEL8;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_STORE_ANIMATION8) == 0)
			{
				_transStart = millis();
				m_PayloadActiveCommand = PAYLOADCOMMAND_STORE_ANIMATION8;
			}
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_REMOVE_ANIMATION) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_REMOVE_ANIMATION;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_PLAY_STORED_ANIMATION8) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_PLAY_STORED_ANIMATION8;		
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_PLAY_STORED_ANIMATION8_BYNAME) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_PLAY_STORED_ANIMATION8_BYNAME;
			else if (stricmp(m_PayloadJSONListnerTracking_Value.c_str(), (const char *)COMMAND_REQUEST_THUMBNAIL) == 0)
				m_PayloadActiveCommand = PAYLOADCOMMAND_REQUEST_THUMBNAIL;		
			memset(m_szPayloadCommandValue, 0x00, sizeof(m_szPayloadCommandValue));
		}
		else if (strcasecmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_TRANSACTIONID) == 0)
			m_PayloadActiveTransactionID = atoi(value);
		else if (strcasecmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_DEVICENAME) == 0)
			strncpy(m_szPayloadCommandValue, value, MAX_COMMAND_VALUE);		
		else if(strcasecmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_KEY) == 0)
			strncpy(m_szPayloadCommandValue, value, MAX_COMMAND_VALUE);
		else if (m_PayloadActiveCommand == PAYLOADCOMMAND_DISPLAY_BRIGHTNESS && stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_BRIGHTNESS) == 0)
			m_nPayloadCommandValue = atoi(m_PayloadJSONListnerTracking_Value.c_str());
		else if (m_PayloadActiveCommand == PAYLOADCOMMAND_PREVIEW_COLOR && stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PREVIEWCOLOR) == 0 && strlen(value) == 6)
			strncpy(m_szPayloadCommandValue, value, epxmin(strlen(value), MAX_COMMAND_VALUE - 1));
		else if ((m_PayloadActiveCommand == PAYLOADCOMMAND_REMOVE_ANIMATION || m_PayloadActiveCommand == PAYLOADCOMMAND_REQUEST_THUMBNAIL) && stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_ID) == 0)
			strncpy(m_szPayloadCommandValue, value, epxmin(strlen(value), MAX_COMMAND_VALUE - 1));
		else if (m_PayloadActiveCommand == PAYLOADCOMMAND_PLAY_STORED_ANIMATION8 || m_PayloadActiveCommand == PAYLOADCOMMAND_PLAY_STORED_ANIMATION8_BYNAME)
		{
			if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *) JSONKEY_FORCEDLOOPCOUNT) == 0)
				m_nPayloadCommandValue = atoi(value);
			if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *) JSONKEY_RUNUNTILCOMPLETE) == 0)
				m_nPayloadCommandValue2 = atoi(value);
			else
			{
				if (m_PayloadActiveCommand == PAYLOADCOMMAND_PLAY_STORED_ANIMATION8 && stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_ID) == 0)
					strncpy(m_szPayloadCommandValue, value, epxmin(strlen(value), MAX_COMMAND_VALUE - 1));	
				else if (m_PayloadActiveCommand == PAYLOADCOMMAND_PLAY_STORED_ANIMATION8_BYNAME && stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_NAME) == 0)
					strncpy(m_szPayloadCommandValue, value, epxmin(strlen(value), MAX_COMMAND_VALUE - 1));
			}
		}
		else if (m_PayloadActiveCommand == PAYLOADCOMMAND_UPLOAD_FRAME8)
		{
			if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PIXELSHEXLENGTH) == 0)
			{
				m_StagedAnimation.framesHexLength = atoi(value);
				m_StagedAnimation.Meta.cbFrames = m_StagedAnimation.framesHexLength / 2;
				m_StagedAnimation.pRAMFrames = (uint8_t *)TMALLOC(m_StagedAnimation.Meta.cbFrames);
				m_PayloadParseCommandSequenceFillPos = 0;
			}
			else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PIXELSHEX) == 0)
			{			
				m_PayloadParseCommandSequenceFillPos = 0;
				m_CAppStorage.SequenceWriteCacheFlush(&m_StagedAnimation);
			}
		}		
		else if (m_PayloadActiveCommand == PAYLOADCOMMAND_UPLOAD_ANIMATION8 || m_PayloadActiveCommand == PAYLOADCOMMAND_STORE_ANIMATION8)
		{		
			//DEBUGLOGLN("KEY %s VALUE %s", m_PayloadJSONListnerTracking_Key.c_str(), value);
			CompletionTrackingEnable();
			if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_ID) == 0)
			{
				// Play Now gets unique zero ID
				if(m_PayloadActiveCommand == PAYLOADCOMMAND_UPLOAD_ANIMATION8)
					strcpy(m_StagedAnimation.szID, (char *) PLAYNOWID);
				else
				// ID opens the file	
					strcpy(m_StagedAnimation.szID, value);

				uint8_t guidLen = strlen(m_StagedAnimation.szID);
				m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_GUIDLEN, &guidLen, sizeof(guidLen));
				m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_GUID, (uint8_t *) m_StagedAnimation.szID, guidLen);
			}
			else
			{
				// Only write if the file has been opened
				if(m_StagedAnimation.pFile != NULL)
				{
					if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PALETTEHEXLENGTH) == 0)
					{
						m_StagedAnimation.palleteHexLength = atoi(value);
						m_StagedAnimation.Meta.cbPallete = (m_StagedAnimation.palleteHexLength / 3) / 2;
						m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_PALLETESIZE, (uint8_t *) &m_StagedAnimation.Meta.cbPallete, sizeof(m_StagedAnimation.Meta.cbPallete));
						m_CAppStorage.SequenceWriteToken(&m_StagedAnimation, SEQUENCETOKEN_PALLETEBYTES);
						m_PayloadParsePaletteBytePos = 0;
						m_PayloadParseCommandSequenceFillPos = 0;
					}
					else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_LOOPCOUNT) == 0)
					{
						m_StagedAnimation.Meta.loopCount = atoi(value);					
						m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_LOOPCOUNT, &m_StagedAnimation.Meta.loopCount, sizeof(m_StagedAnimation.Meta.loopCount));
					}
					else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_FRAMERATE) == 0)
					{
						m_StagedAnimation.Meta.frameRate = atoi(value);
						m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_FRAMERATE, &m_StagedAnimation.Meta.frameRate, sizeof(m_StagedAnimation.Meta.frameRate));
					}
					else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_FRAMECOUNT) == 0)
					{
						m_StagedAnimation.Meta.frameCount = atoi(value);
						m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_FRAMECOUNT, (uint8_t *) &m_StagedAnimation.Meta.frameCount, sizeof(m_StagedAnimation.Meta.frameCount));
					}
					else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_UTCTIMESTAMP) == 0)
					{
						m_StagedAnimation.utcTimeStamp = atoi(value);
						m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_UTCTIMESTAMP, (uint8_t *) &m_StagedAnimation.utcTimeStamp, sizeof(m_StagedAnimation.utcTimeStamp));
					}
					else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_NAME) == 0)
					{
						uint8_t nameLen = strlen(value);
					
						m_StagedAnimation.pszName = (char *) TMALLOC(nameLen + 1);
						strcpy(m_StagedAnimation.pszName, value);
						m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_NAMELEN, &nameLen, sizeof(nameLen));
						m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_NAME, (uint8_t *) value, nameLen);
					}		
					else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_FRAMESHEXLENGTH) == 0)
					{
						m_StagedAnimation.framesHexLength = atoi(value);
						m_StagedAnimation.Meta.cbFrames = m_StagedAnimation.framesHexLength / 2;
						m_CAppStorage.SequenceWriteSection(&m_StagedAnimation, SEQUENCETOKEN_FRAMESBYTELEN, (uint8_t *) &m_StagedAnimation.Meta.cbFrames, sizeof(m_StagedAnimation.Meta.cbFrames));
						m_CAppStorage.SequenceWriteToken(&m_StagedAnimation, SEQUENCETOKEN_FRAMESBYTES);
						m_PayloadParseCommandSequenceFillPos = 0;
					}
					else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_FRAMESHEX) == 0)
					{
						m_PayloadParseCommandSequenceFillPos = 0;
						m_CAppStorage.SequenceWriteCacheFlush(&m_StagedAnimation);
						_transEnd = millis();
						DEBUGLOGLN("TRANSTIME %d", _transEnd - _transStart);
					}
				}
			}		
		}
		else if (m_PayloadActiveCommand == PAYLOADCOMMAND_UPLOAD_PIXEL8)
		{
			if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PIXELINDEX) == 0)
				m_nPayloadCommandValue = atoi(value);
			else if (stricmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PIXELHEX) == 0)
				strncpy(m_szPayloadCommandValue, value, epxmin(strlen(value), MAX_COMMAND_VALUE - 1));
		}	
		/***********************************/
		/* Simple String Protocol commands */
		/***********************************/
		else if(strcasecmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_PLAY) == 0)
		{
			m_PayloadActiveCommand = PAYLOADCOMMAND_PLAY_STORED_ANIMATION8_BYNAME;
			strncpy(m_szPayloadCommandValue, value, epxmin(strlen(value), MAX_COMMAND_VALUE - 1));			
		}		
		else if(strcasecmp(m_PayloadJSONListnerTracking_Key.c_str(), (const char *)JSONKEY_BRIGHTNESS) == 0)
		{
			m_PayloadActiveCommand = PAYLOADCOMMAND_DISPLAY_BRIGHTNESS;
			m_nPayloadCommandValue = atoi(m_PayloadJSONListnerTracking_Value.c_str());
		}

	}
}

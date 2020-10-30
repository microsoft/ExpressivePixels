// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <string.h>
#include "EPXPlatform_Runtime.h"
#include "EPXApp.h"
#include "EPXApp_CStorage.h"
#include "EPXApp_ChannelConstants.h"
#include  "EPXVariant.h"

EPX_OPTIMIZEFORDEBUGGING_ON

const char		*SEQUENCES_FOLDER = "/SEQUENCES";


CExpressivePixelsStorage::CExpressivePixelsStorage()
{
	m_pAutoPlayList = NULL;
	m_writeCachePosition = 0;
	m_cbTotalBytesWritten = 0;
	m_pSequenceFileListHead = NULL;
}



CExpressivePixelsStorage::~CExpressivePixelsStorage()
{
	CStorage::FreeEnumFolderList(&m_pSequenceFileListHead);
}



void CExpressivePixelsStorage::Initialize(bool format)
{
	if (CStorage::Initialize(GPIO_PIN_FLASHRAM_MISO, GPIO_PIN_FLASHRAM_MOSI, GPIO_PIN_FLASHRAM_SCLK, GPIO_PIN_FLASHRAM_CS, format))
	{
		CStorage::CreateFolder((char *)SEQUENCES_FOLDER);
		
		// Generate the file list
		m_pSequenceFileListHead = EnumerateSequences();
	}
}



void CExpressivePixelsStorage::Power(bool on)
{
	CStorage::Power(on);
}
	


bool CExpressivePixelsStorage::Format()
{
	return CStorage::Format();
}



bool CExpressivePixelsStorage::DeleteFile(const char *pszFilename)
{
	return CStorage::DeleteFile(pszFilename);
}



bool CExpressivePixelsStorage::EnumerateAutoPlaylist()
{
	m_pAutoPlayList = (PAUTOPLAYPLAYLIST)TMALLOC(sizeof(AUTOPLAYPLAYLIST));
	if (m_pAutoPlayList != NULL)
	{
		memset(m_pAutoPlayList, 0x00, sizeof(AUTOPLAYPLAYLIST));
		m_pAutoPlayList->pSequenceListHead = m_pSequenceFileListHead;
		if (m_pAutoPlayList->pSequenceListHead != NULL)
		{
			m_pAutoPlayList->pSequenceListCurrent = m_pAutoPlayList->pSequenceListHead;
			return true;
		}		
	}
	return false;
}



bool CExpressivePixelsStorage::AutoPlaylistIterate(bool playOnce, bool *pbRestarted)
{
	if (pbRestarted != NULL)
		*pbRestarted = false;
	if (m_pAutoPlayList != NULL && m_pAutoPlayList->pSequenceListCurrent != NULL)
	{
		if (m_pAutoPlayList->pSequenceListCurrent->pNext == NULL)
		{
			if (playOnce)
				return false;			
			if (pbRestarted != NULL)
				*pbRestarted = true;
			m_pAutoPlayList->pSequenceListCurrent = m_pAutoPlayList->pSequenceListHead;
		}
		else
			m_pAutoPlayList->pSequenceListCurrent = m_pAutoPlayList->pSequenceListCurrent->pNext;
		return true;
	}
	return false;
}



PPERSISTED_SEQUENCE_LIST CExpressivePixelsStorage::AutoPlaylistCurrent()
{
	if (m_pAutoPlayList != NULL && m_pAutoPlayList->pSequenceListCurrent != NULL)
		return m_pAutoPlayList->pSequenceListCurrent;
	return NULL;
}



void CExpressivePixelsStorage::AutoPlaylistClear()
{
	if (m_pAutoPlayList != NULL)
	{
		TFREE(m_pAutoPlayList);
		m_pAutoPlayList = NULL;
	}
}



PPERSISTED_SEQUENCE_LIST CExpressivePixelsStorage::EnumerateSequences()
{
	PPERSISTED_SEQUENCE_LIST pCur, pSequenceFileListHead;
	
	CStorage::EnumFolder(SEQUENCES_FOLDER, &pSequenceFileListHead);
	if (pSequenceFileListHead != NULL)
	{
		pCur = pSequenceFileListHead;
		while (pCur != NULL)
		{
			void *pFile = SequenceOpen(pCur->pszFilename);
			if (pFile != NULL)
			{
				// Read the UTC timestamp
				SequenceReadTokenData(pFile, SEQUENCETOKEN_UTCTIMESTAMP, &pCur->utcTimestamp, sizeof(pCur->utcTimestamp));

				// Extract the name and guid from the file
				pCur->pszName = SequenceReadTokenString(pFile, SEQUENCETOKEN_NAME);
				pCur->pszGUID = SequenceReadTokenString(pFile, SEQUENCETOKEN_GUID);										
				if (pCur->pszGUID == NULL)
				{					
					pCur->pszGUID = (char *) TMALLOC(strlen(pCur->pszFilename) + 1);
					if (pCur->pszGUID != NULL)
						strcpy(pCur->pszGUID, pCur->pszFilename);
				}		
				
				// Print the file name and mention if it's a directory.
				DEBUGLOGLN("FN %s, Name %s, GUID %s : %d bytes", pCur->pszFilename, pCur->pszName, pCur->pszGUID, (uint16_t) pCur->size);

				CStorage::FreeEnumFolderListItemFilename(pCur);
				SequenceClose(pFile);
			}
			pCur = pCur->pNext;
		}
	}
	return pSequenceFileListHead;
}



void CExpressivePixelsStorage::EnumerateSequencesJSON(EPXString &response, uint32_t activeTransactionID, char *pszInnerContainerObject)
{
	PPERSISTED_SEQUENCE_LIST pCur, pSequenceFileListHead;
	
	response += JSON_OPENOBJECT;
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED(JSON_STATUS, JSON_SUCCESS);
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED((const char *) JSONKEY_TRANSACTIONID, EPXString(activeTransactionID));
	
	uint8_t pctUsed = (uint8_t)(((float) CStorage::UsedSpace()/ (float) CStorage::Capacity()) * 100);
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED(T(JSONKEY_STORAGEUSED), EPXString((uint32_t) pctUsed));	
	
	if (pszInnerContainerObject != NULL)
		response += JSON_KEYOBJECTOPEN("info");		
	response += JSON_KEYOBJECTOPENARRAY("Sequences");

	pSequenceFileListHead = m_pSequenceFileListHead;
	if (pSequenceFileListHead != NULL)
	{
		pCur = pSequenceFileListHead;
		DEBUGLOGLN("SEQUENCE LIST:");
		while (pCur != NULL)
		{
			// Extract the name from the file
			if(stricmp(pCur->pszGUID, (char *) PLAYNOWID) != 0)
			{
				// Generate the JSON
				response += JSON_OPENOBJECT;
				response += JSON_KEYVALUE_STRINGPAIR_CONTINUED("ID", pCur->pszGUID);
				if (pCur->pszName != NULL)
					response += JSON_KEYVALUE_STRINGPAIR_CONTINUED("Filename", pCur->pszName);
				response += JSON_KEYVALUE_VALUEPAIR_CONTINUED("Size", pCur->size);
				response += JSON_KEYVALUE_VALUEPAIR("UTC", pCur->utcTimestamp);
				response += (pCur->pNext == NULL ? JSON_CLOSEOBJECT : JSON_CLOSEOBJECT_CONTINUED);
					
				DEBUGLOGLN("- Name %s ID# %s, Size %d bytes, UTC %d", pCur->pszName, pCur->pszGUID, pCur->size, pCur->utcTimestamp);
			}
			pCur = pCur->pNext;
		}
	}
	response += JSON_CLOSEARRAY;
	if (pszInnerContainerObject != NULL)
		response += JSON_CLOSEOBJECT;
	response += JSON_CLOSEOBJECT;
}



char *CExpressivePixelsStorage::SequenceIDFromName(char *pszName)
{
	PPERSISTED_SEQUENCE_LIST pCur = m_pSequenceFileListHead;
	
	pCur = m_pSequenceFileListHead;
	while (pCur != NULL)
	{
		if (stricmp(pCur->pszName, pszName) == 0)
			return pCur->pszGUID;
		pCur = pCur->pNext;
	}
	return NULL;
}



EPXString CExpressivePixelsStorage::SequenceFilenameFromID(const char *pszGUID, int cb)
{
	return EPXString(SEQUENCES_FOLDER) + "/" + pszGUID;
}



void *CExpressivePixelsStorage::SequenceOpen(const char *pszFilename, int *pFileSize)
{
	if (pszFilename != NULL)
	{
		EPXString fullFilename = SequenceFilenameFromID(pszFilename);
		return CStorage::OpenFile((char *)fullFilename.c_str(), pFileSize);
	}
	return NULL;	
}



void CExpressivePixelsStorage::SequenceClose(void *pFile)
{
	if (pFile != NULL)
		CStorage::Close(pFile);
}



uint16_t CExpressivePixelsStorage::SequenceReadTokenData(void *pFile, uint8_t tokenType, void *pData, uint16_t cb)
{
	if (pFile != NULL)
	{
		uint8_t readTokenType;
		uint32_t skipBytes;

		uint32_t fileSize = CStorage::FileSize(pFile);
		uint32_t originalFileSize = fileSize;
		CStorage::Seek(pFile, 0);
		while (fileSize > 0)
		{
			if (!ReadBytes(pFile, &fileSize, &readTokenType, sizeof(readTokenType)))
				break;

			if (readTokenType == tokenType)
			{
				if (ReadBytes(pFile, &fileSize, (uint8_t *) pData, cb))
					return cb;
				else
					return 0;
			}
			else
			{
				if (!SequenceReadTokenTraverse(pFile, readTokenType, originalFileSize, &fileSize, &skipBytes))
					return 0;
			}
		}
	}
	return 0;
}



char *CExpressivePixelsStorage::SequenceReadTokenString(void *pFile, uint8_t tokenTypeString)
{
	if (pFile != NULL)
	{
		size_t  bytesRead = 0;
		uint8_t readTokenType;
		uint8_t tokenTypeStringLen = tokenTypeString + 1;
		uint8_t nameLen = 0;
		uint32_t skipBytes = 0;

		uint32_t fileSize = CStorage::FileSize(pFile);
		uint32_t originalFileSize = fileSize;
		CStorage::Seek(pFile, 0);
		while (fileSize > 0)
		{
			if (!ReadBytes(pFile, &fileSize, &readTokenType, sizeof(readTokenType)))			
				break;
			
			if (readTokenType == tokenTypeStringLen)
			{
				// Read the string length
				if(!ReadBytes(pFile, &fileSize, &nameLen, sizeof(nameLen)))			
					return NULL;
				
				// Read the string token
				if(!ReadBytes(pFile, &fileSize, &readTokenType, sizeof(readTokenType)))			
					return NULL;
				
				if (readTokenType == tokenTypeString)
				{
					if (nameLen <= fileSize)
					{
						char *psz = (char *)TMALLOC(nameLen + 1);
						if (psz == NULL)
							break;						
						if (!ReadBytes(pFile, &fileSize, (uint8_t *) psz, nameLen))
						{
							TFREE(psz);
							break;
						}
						psz[nameLen] = 0x00;
						return psz;
					}
					else
						break;
				}
			}

			if (!SequenceReadTokenTraverse(pFile, readTokenType, originalFileSize, &fileSize, &skipBytes))
				return NULL;
		}
	}
	return NULL;
}



bool CExpressivePixelsStorage::SequenceReadTokenTraverse(void *pFile, uint8_t tokenType, uint32_t originalFileSize, uint32_t *pFileSize, uint32_t *pSkipBytes)
{
	size_t   bytesRead;
	uint8_t  skip8Len = 0;
	uint16_t skip16Len = 0;
	uint32_t skip32Len = 0;
	
	switch (tokenType)
	{
	case SEQUENCETOKEN_NAMELEN:
		if (!ReadBytes(pFile, pFileSize, &skip8Len, sizeof(skip8Len)))
			return false;
		*pSkipBytes = skip8Len;
		break;
						
	case SEQUENCETOKEN_GUIDLEN:
		if (!ReadBytes(pFile, pFileSize, &skip8Len, sizeof(skip8Len)))
			return false;
		*pSkipBytes = skip8Len;
		break;
																		
	case SEQUENCETOKEN_PALLETESIZE:
		if (!ReadBytes(pFile, pFileSize, (uint8_t *)&skip16Len, sizeof(skip16Len)))
			return false;
		*pSkipBytes = skip16Len * sizeof(PALETTE_ENTRY);
		break;

	case SEQUENCETOKEN_FRAMESBYTELEN:
		if (!ReadBytes(pFile, pFileSize, (uint8_t *)&skip32Len, sizeof(skip32Len)))
			return false;
		*pSkipBytes = skip32Len;
		break;

	case SEQUENCETOKEN_FRAMECOUNT:
		if (!ReadBytes(pFile, pFileSize, (uint8_t *)&skip16Len, sizeof(skip16Len)))
			return false;
		break;

	case SEQUENCETOKEN_FRAMERATE:
		if (!ReadBytes(pFile, pFileSize, &skip8Len, sizeof(skip8Len)))
			return false;
		break;

	case SEQUENCETOKEN_LOOPCOUNT:
		if (!ReadBytes(pFile, pFileSize, &skip8Len, sizeof(skip8Len)))
			return false;
		break;
					
	case SEQUENCETOKEN_UTCTIMESTAMP:
		if (!ReadBytes(pFile, pFileSize, (uint8_t *)&skip32Len, sizeof(skip32Len)))
			return false;
		break;
				
	default:
		if (*pSkipBytes > 0)
		{
			int currentPos = originalFileSize - (*pFileSize);					
			currentPos += (*pSkipBytes);
			CStorage::Seek(pFile, currentPos);						
			(*pFileSize) -= (*pSkipBytes);					
			*pSkipBytes = 0;
		}
		break;
	}	
	return true;
}



bool CExpressivePixelsStorage::ReadBytes(void *pFile, uint32_t *pFileSize, uint8_t *pBuf, uint32_t cbToRead)
{
	size_t bytesRead = CStorage::ReadFile(pFile, pBuf, cbToRead);
	if (bytesRead != cbToRead)
		return false;
	(*pFileSize) -= bytesRead;
	return true;
}



bool CExpressivePixelsStorage::SequenceRead(const char *pszGUID, EXPRESSIVEPIXEL_SEQUENCE *pSequence, bool playFromFile)
{
	bool success = false;
	uint8_t skipLen = 0, guidLen = 0;
	uint32_t fileSize;
	
	if (m_bForceDirectFromFile)
		playFromFile = true;
	memset(pSequence, 0x00, sizeof(EXPRESSIVEPIXEL_SEQUENCE));
	
	void *pFile = SequenceOpen(pszGUID, (int *) &fileSize);
	if (pFile != NULL)
	{
		uint8_t tokenType;
		size_t bytesRead = 0;

		uint32_t originalFileSize = fileSize;
		while (fileSize > 0)
		{
			if (!ReadBytes(pFile, &fileSize, &tokenType, sizeof(tokenType)))
				break;

			switch (tokenType)
			{
			case SEQUENCETOKEN_NAMELEN:
				if (!ReadBytes(pFile, &fileSize, &pSequence->nameLen, sizeof(pSequence->nameLen)))
					goto error;
				break;
						
			case SEQUENCETOKEN_NAME:					
				pSequence->pszName = (char *)TMALLOC(pSequence->nameLen + 1);
				if (pSequence->pszName == NULL)
					goto error;
				if (!ReadBytes(pFile, &fileSize, (uint8_t *) pSequence->pszName, pSequence->nameLen))
					goto error;
				pSequence->pszName[pSequence->nameLen] = 0x00;
				break;

			case SEQUENCETOKEN_GUIDLEN:
				if (!ReadBytes(pFile, &fileSize, &guidLen, sizeof(guidLen)))
					goto error;
				break;
				
			case SEQUENCETOKEN_GUID:
				if (!ReadBytes(pFile, &fileSize, (uint8_t *)pSequence->szID, guidLen))
					goto error;
				pSequence->szID[guidLen] = 0x00;
				break;
														
			case SEQUENCETOKEN_PALLETESIZE:
				if (!ReadBytes(pFile, &fileSize, (uint8_t *)&pSequence->Meta.cbPallete, sizeof(pSequence->Meta.cbPallete)))
					goto error;
				break;

			case SEQUENCETOKEN_PALLETEBYTES:
				pSequence->pRAMPalette = (PALETTE_ENTRY *)TMALLOC(pSequence->Meta.cbPallete * sizeof(PALETTE_ENTRY));
				if (pSequence->pRAMPalette == NULL)
					goto error;				
				if (!ReadBytes(pFile, &fileSize, (uint8_t *) pSequence->pRAMPalette, pSequence->Meta.cbPallete * sizeof(PALETTE_ENTRY)))
					goto error;
				break;

			case SEQUENCETOKEN_FRAMESBYTELEN:
				if (!ReadBytes(pFile, &fileSize, (uint8_t *) &pSequence->Meta.cbFrames, sizeof(pSequence->Meta.cbFrames)))
					goto error;
				break;

			case SEQUENCETOKEN_FRAMESBYTES:
				if (playFromFile)
				{
					pSequence->frameBytesStartOffset = CStorage::Position(pFile);
					fileSize -= pSequence->Meta.cbFrames;
				}
				else
				{
					pSequence->pRAMFrames = (uint8_t *)TMALLOC(pSequence->Meta.cbFrames);
					if (pSequence->pRAMFrames == NULL)
						goto error;
					if (!ReadBytes(pFile, &fileSize, (uint8_t *) pSequence->pRAMFrames, pSequence->Meta.cbFrames))
						goto error;
				}
				break;

			case SEQUENCETOKEN_FRAMECOUNT:
				if (!ReadBytes(pFile, &fileSize, (uint8_t *)&pSequence->Meta.frameCount, sizeof(pSequence->Meta.frameCount)))
					goto error;
				break;

			case SEQUENCETOKEN_FRAMERATE:
				if (!ReadBytes(pFile, &fileSize, (uint8_t *)&pSequence->Meta.frameRate, sizeof(pSequence->Meta.frameRate)))
					goto error;
				break;

			case SEQUENCETOKEN_LOOPCOUNT:
				if (!ReadBytes(pFile, &fileSize, (uint8_t *)&pSequence->Meta.loopCount, sizeof(pSequence->Meta.loopCount)))
					goto error;
				break;
					
			case SEQUENCETOKEN_UTCTIMESTAMP:
				if (!ReadBytes(pFile, &fileSize, (uint8_t *)&pSequence->utcTimeStamp, sizeof(pSequence->utcTimeStamp)))
					goto error;
				break;
				
			default:
				if (skipLen > 0)
				{
					int currentPos = originalFileSize - fileSize;					
					currentPos += skipLen;
					CStorage::Seek(pFile, currentPos);						
					fileSize -= skipLen;					
					skipLen = 0;
				}
				break;
			}
		}
		success = true;
error:
		if (playFromFile)
			pSequence->pFile = pFile;
		else
			CStorage::Close(pFile);
	}
	return success;
}



bool CExpressivePixelsStorage::SequenceWriteSection(EXPRESSIVEPIXEL_SEQUENCE *pSequence, uint8_t section, uint8_t *pData, uint16_t cb)
{
	bool success = false;
	
	if (pSequence->szID[0] != 0x00)
	{
		if (pSequence->pFile == NULL)
		{
			m_cbTotalBytesWritten = 0;
			EPXString fullFilename = SequenceFilenameFromID(pSequence->szID);
			pSequence->pFile = CStorage::CreateFile((char *) fullFilename.c_str());
			m_writeCachePosition = 0;
		}
		
		if (pSequence->pFile != NULL && section > 0)
		{
			if (SequenceWriteTokenAndData(pSequence->pFile, section, pData, cb))
				success = true;
		}
	}
	return success;
}



void CExpressivePixelsStorage::SequenceWriteClose(EXPRESSIVEPIXEL_SEQUENCE *pSequence)
{
	if (pSequence->pFile != NULL)
	{
		// Update the internal sequence list
		UpdateSequenceList(UPDATESEQUENCELIST_ADDORUPDATE, pSequence->szID, pSequence->pszName, CStorage::FileSize(pSequence->pFile), pSequence->utcTimeStamp);

		DEBUGLOGLN("CExpressivePixelsStorage::SequenceWriteClose WRITTEN %d", m_cbTotalBytesWritten);
		CStorage::Close(pSequence->pFile);
		pSequence->pFile = NULL;
	}
}



bool CExpressivePixelsStorage::SequenceWriteTokenAndData(void *pFile, uint8_t token, void *pBuf, uint16_t cb)
{
	size_t bytesWritten = CStorage::WriteFile(pFile, (uint8_t *)&token, sizeof(uint8_t));
	if (bytesWritten == sizeof(uint8_t))
	{
		// Return if just writing the token
		if(pBuf == NULL || cb == 0)
			return true;
		
		bytesWritten = CStorage::WriteFile(pFile, (uint8_t *)pBuf, cb);
		m_cbTotalBytesWritten += bytesWritten;
		return (bytesWritten == cb);
	}
	return false;
}

	

bool CExpressivePixelsStorage::SequenceWriteToken(EXPRESSIVEPIXEL_SEQUENCE *pSequence, uint8_t token)
{
	if (pSequence->pFile != NULL)
	{
		size_t bytesWritten = CStorage::WriteFile(pSequence->pFile, (uint8_t *)&token, sizeof(uint8_t));
		m_cbTotalBytesWritten += bytesWritten;
		return (bytesWritten == sizeof(uint8_t));
	}
	return false;
}



bool CExpressivePixelsStorage::SequenceWriteData(EXPRESSIVEPIXEL_SEQUENCE *pSequence, void *pBuf, uint16_t cb)
{
	if (pSequence->pFile != NULL)
	{
		size_t  bytesWritten = CStorage::WriteFile(pSequence->pFile, (uint8_t *)pBuf, cb);
		m_cbTotalBytesWritten += bytesWritten;
		return cb == bytesWritten;

		/*
		// Cache and/or flush all data in buffer
		while (cb > 0)
		{
			uint16_t cacheBytesRemaining = STORAGEMANAGER_WRITECACHESIZE - m_writeCachePosition;
			uint16_t cacheBytes = min(cacheBytesRemaining, cb);
			
			memcpy(m_writeCache, pBuf, cacheBytes);
			cacheBytesRemaining -= cacheBytes;
			m_writeCachePosition += cacheBytes;
			if (cacheBytesRemaining == 0)
				SequenceWriteCacheFlush(pSequence);
			cb -= cacheBytes;
		}				
		return (true);	*/
	}
	return false;
}



bool CExpressivePixelsStorage::SequenceWriteCacheFlush(EXPRESSIVEPIXEL_SEQUENCE *pSequence)
{
	bool success = false;
	if (pSequence->pFile != NULL)
	{
		if (m_writeCachePosition > 0)
		{
			DEBUGLOGLN("FLUSHING CACHE");
			size_t  bytesWritten = CStorage::WriteFile(pSequence->pFile, (uint8_t *)m_writeCache, m_writeCachePosition);
			success = bytesWritten == m_writeCachePosition;
			m_writeCachePosition = 0;
		}
		else
			success = true;
	}
	return success;
}



bool CExpressivePixelsStorage::SequenceDelete(char *pszGUID)
{
	DEBUGLOGLN("CExpressivePixelsStorage::SequenceDelete ID %s", pszGUID);

	/**** Determine FileID ****/
	EPXString fullFilename = SequenceFilenameFromID(pszGUID);
	DEBUGLOGLN("CExpressivePixelsStorage::SequenceDelete %s", fullFilename.c_str());
	if (DeleteFile(fullFilename.c_str()))
	{
		UpdateSequenceList(UPDATESEQUENCELIST_DELETE, pszGUID);
		DEBUGLOGLN("CExpressivePixelsStorage::SequenceDelete DELETED");
		return true;
	}
	else
		DEBUGLOGLN("CExpressivePixelsStorage::SequenceDelete FAILED");
	return false;
}



void CExpressivePixelsStorage::UpdateSequenceListItem(PPERSISTED_SEQUENCE_LIST pItem, char *pszGUID, char *pszName, uint16_t size, uint32_t utc)
{
	// Update the GUID
	if(pszGUID != NULL)
	{
		pItem->pszGUID = (char *) TMALLOC(strlen(pszGUID) + 1);
		if (pItem->pszGUID != NULL)
			strcpy(pItem->pszGUID, pszGUID);		
	}

	// Update size and utc
	pItem->size = size;
	pItem->utcTimestamp = utc;	
	
	// Update the name last of all
	if(pItem->pszName != NULL)
	{
		// Check to see if it hasn't been changed
		if(strcmp(pItem->pszName, pszName) == 0)
			return;	
		TFREE(pItem->pszName);				
	}
	pItem->pszName = (char *) TMALLOC(strlen(pszName) + 1);
	if (pItem->pszName != NULL)
		strcpy(pItem->pszName, pszName);
				
}



void CExpressivePixelsStorage::UpdateSequenceList(UpdateSequenceListOperations operation, char *pszGUID, char *pszName, uint16_t size, uint32_t utc)
{
	PPERSISTED_SEQUENCE_LIST pCur = m_pSequenceFileListHead, pPrev = NULL;
	
	switch (operation)
	{
	case UPDATESEQUENCELIST_ADDORUPDATE:
		// First look for existing entry
		while(pCur != NULL)
		{
			if (stricmp(pCur->pszGUID, pszGUID) == 0)
			{
				UpdateSequenceListItem(pCur, NULL, pszName, size, utc);
				return;
			}			
			pPrev = pCur;
			pCur = pCur->pNext;
		}
		
		// Doesn't exist so add a new one
		pCur = (PPERSISTED_SEQUENCE_LIST) TMALLOC(sizeof(PERSISTED_SEQUENCE_LIST));
		if (pCur != NULL)
		{
			memset(pCur, 0x00, sizeof(PERSISTED_SEQUENCE_LIST));
			UpdateSequenceListItem(pCur, pszGUID, pszName, size, utc);
			
			if (m_pSequenceFileListHead == NULL)
				m_pSequenceFileListHead = pCur;
			else if (pPrev != NULL)
				pPrev->pNext = pCur;
		}
		break;
			
	case UPDATESEQUENCELIST_DELETE:
		while (pCur != NULL)
		{
			if (stricmp(pCur->pszGUID, pszGUID) == 0)
			{
				if (pCur == m_pSequenceFileListHead)
					m_pSequenceFileListHead = pCur->pNext;
				else
					pPrev->pNext = pCur->pNext;
				CStorage::FreeEnumFolderListItem(pCur);				
				return;
			}			
			pPrev = pCur;
			pCur = pCur->pNext;
		}
		break;
	}
}




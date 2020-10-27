// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXString.h"
#include "EPXPlatform_CStorage.h"
#include "CAnimationManager.h"

#define SETTINGS_FILENAME "SETTINGS.JSON"

#define STORAGEMANAGER_WRITECACHESIZE	512


enum UpdateSequenceListOperations : uint8_t
{
	UPDATESEQUENCELIST_ADDORUPDATE,
	UPDATESEQUENCELIST_DELETE
};

typedef struct
{
	PPERSISTED_SEQUENCE_LIST pSequenceListHead;
	PPERSISTED_SEQUENCE_LIST pSequenceListCurrent;
} AUTOPLAYPLAYLIST, *PAUTOPLAYPLAYLIST;



class CExpressivePixelsStorage
{
public:
	CExpressivePixelsStorage();
	~CExpressivePixelsStorage();

	void AutoPlaylistClear();
	void Initialize(bool format = false);
	void EnumerateSequencesJSON(EPXString &response, uint32_t activeTransactionID, char *pszInnerContainerObject = NULL);
	bool AutoPlaylistIterate(bool playOnce = false, bool *pbRestarted = NULL);
	bool IsAutoPlayActive() { return m_pAutoPlayList != NULL; }
	bool DeleteFile(const char *pszFilename);
	bool EnumerateAutoPlaylist();
	bool Format();
	bool SequenceDelete(char *pszGUID);
	bool SequenceRead(const char *pszGUID, EXPRESSIVEPIXEL_SEQUENCE *pSequence, bool playFromFile = false);
	bool SequenceWriteData(EXPRESSIVEPIXEL_SEQUENCE *pSequence, void *pBuf, uint16_t cb);
	bool SequenceWriteCacheFlush(EXPRESSIVEPIXEL_SEQUENCE *pSequence);

	bool SequenceWriteSection(EXPRESSIVEPIXEL_SEQUENCE *pSequence, uint8_t section = 0, uint8_t *pData = NULL, uint16_t cb = 0);
	bool SequenceWriteToken(EXPRESSIVEPIXEL_SEQUENCE *pSequence, uint8_t token);

	char *SequenceIDFromName(char *pszName);
	char *SequenceReadTokenString(void *pFile, uint8_t tokenType);
	PPERSISTED_SEQUENCE_LIST AutoPlaylistCurrent();
	PPERSISTED_SEQUENCE_LIST FirstStoredSequence() { return m_pSequenceFileListHead; }
	
	uint16_t SequenceReadTokenData(void *pFile, uint8_t tokenType, void *pData, uint16_t cb);
	
	static void Power(bool on);
	void SequenceClose(void *pFile);
	void *SequenceOpen(const char *pszFilename, int *fileSize = NULL);
	void SequenceWriteClose(EXPRESSIVEPIXEL_SEQUENCE *pSequence);
	void SetReadDirectFromFile(bool bDirect) { m_bForceDirectFromFile = bDirect; }

private:
	CStorage				 m_CStorage;
	PPERSISTED_SEQUENCE_LIST m_pSequenceFileListHead = NULL;

	bool ReadBytes(void *pFile, uint32_t *pFileSize, uint8_t *pBuf, uint32_t cbToRead);
	bool SequenceReadTokenTraverse(void *pFile, uint8_t tokenType, uint32_t originalFileSize, uint32_t *pFileSize, uint32_t *pSkipBytes);
	bool SequenceWriteTokenAndData(void *pFile, uint8_t token, void *pBuf, uint16_t cb);
	PPERSISTED_SEQUENCE_LIST EnumerateSequences();
	EPXString SequenceFilenameFromID(const char *pszGUID, int cb = 0);
	void UpdateSequenceList(UpdateSequenceListOperations operation, char *pszGUID, char *pszName = NULL, uint16_t size = 0, uint32_t utc = 0);
	void UpdateSequenceListItem(PPERSISTED_SEQUENCE_LIST pItem, char *pszGUID, char *pszName, uint16_t size, uint32_t utc);

	bool				m_bForceDirectFromFile;
	uint8_t				m_writeCache[STORAGEMANAGER_WRITECACHESIZE];
	uint16_t			m_writeCachePosition;
	uint32_t			m_cbTotalBytesWritten;
	PAUTOPLAYPLAYLIST	m_pAutoPlayList;
};



// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#include <Adafruit_SPIFlash.h>


typedef struct _PERSISTED_SEQUENCE_LIST
{
	char *pszFilename;
	char *pszGUID;
	char *pszName;
	uint16_t size;
	uint32_t utcTimestamp;
	struct _PERSISTED_SEQUENCE_LIST *pNext;
} PERSISTED_SEQUENCE_LIST, *PPERSISTED_SEQUENCE_LIST;



class BaseStorageClass
{
public:
	virtual bool Initialize() = 0;

	virtual bool CreateFolder(char *pszFolder) = 0;
	virtual bool DeleteFile(const char *pszFilename) = 0;
	virtual bool FileExists(const char *pszFilename) = 0;
	virtual bool Format() = 0;
	virtual size_t ReadFile(File *file, uint8_t *pBuffer, int bufferLen) = 0;
	virtual size_t WriteFile(File *file, uint8_t *pBuffer, int bufferLen) = 0;
	virtual File OpenFile(char *pszFilename, uint8_t mode) = 0;
	virtual File *OpenFileDyn(char *pszFilename, uint8_t mode) = 0;
	virtual uint32_t Position(File *file) = 0;
	virtual void Close(File *file) = 0;
	virtual void EnumFolder(const char *pszFolder, PPERSISTED_SEQUENCE_LIST *ppFileList) = 0;


	void ReleaseSequenceFileList(PPERSISTED_SEQUENCE_LIST pFileList);
};



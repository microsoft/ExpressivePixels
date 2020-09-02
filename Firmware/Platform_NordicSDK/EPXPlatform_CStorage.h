// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementation declarations for Storage capability on Nordic SDK
 *
 **/
#pragma once
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_CStorageBase.h"


class CStorage
{
public:
	static bool Initialize(uint16_t pinMISO, uint16_t pinMOSI, uint16_t pinSCLK, uint16_t pinCS, bool format = false);
	static bool CreateFolder(char *pszFolder);
	static bool DeleteFile(const char *pszFilename);
	static bool FileExists(const char *pszFilename);
	static bool Format();
	static bool Seek(void *pFile, uint32_t pos);
	static uint32_t FileSize(void *pFile);
	static void EnumFolder(const char *pszFolder, PPERSISTED_SEQUENCE_LIST *ppFileList);
	static void FreeEnumFolderList(PPERSISTED_SEQUENCE_LIST *ppList);
	static void FreeEnumFolderListItem(PPERSISTED_SEQUENCE_LIST pItem);
	static void FreeEnumFolderListItemFilename(PPERSISTED_SEQUENCE_LIST pItem);
	static void FlashChip();
	
	static uint16_t ReadFile(void *pFile, void *pData, uint16_t cbToRead);
	static uint16_t WriteFile(void *pFile, void *pData, uint16_t cbToRead);
	static uint32_t Capacity();
	static uint32_t UsedSpace();
	static uint32_t Position(void *pFile);
	
	static void Close(void *pFile);
	static void *CreateFile(const char *pszFilename);
	static void *OpenFile(const char *pszFilename, int *pFileSize = NULL);
	static void Power(bool on);
};


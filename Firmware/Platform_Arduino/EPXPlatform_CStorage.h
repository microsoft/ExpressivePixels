// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_CStorageBase.h"

#define ADAFSFORK_FILENAMEMAX	13

class CStorage
{
public:
	static bool Initialize(uint16_t pinMISO, uint16_t pinMOSI, uint16_t pinSCLK, uint16_t pinCS, bool format = false);
	static bool CreateFolder(const char *pszFolder);
	static bool DeleteFile(const char *pszFilename);
	static bool FileExists(const char *pszFilename);
	static bool Format();
	static bool Seek(void *pFile, uint32_t pos);
	static uint32_t FileSize(void *pFile);
	static void EnumFolder(const char *pszFolder, PPERSISTED_SEQUENCE_LIST *ppFileList);
	static void FreeEnumFolderList(PPERSISTED_SEQUENCE_LIST *ppList);
	static void FreeEnumFolderListItem(PPERSISTED_SEQUENCE_LIST pItem);
	static void FreeEnumFolderListItemFilename(PPERSISTED_SEQUENCE_LIST pItem);
	
	static uint16_t ReadFile(void *pFile, void *pData, uint16_t cbToRead);
	static uint16_t WriteFile(void *pFile, void *pData, uint16_t cbToRead);
	static uint32_t Capacity();
	static uint32_t UsedSpace();
	static uint32_t Position(void *pFile);
	
	static char *TrimFilename(const char *pszInFilename);
	static void Close(void *pFile);
	static void *CreateFile(const char *pszFilename);
	static void *OpenFile(const char *pszFilename, int *pFileSize = NULL);
	static void Power(bool on);

	static const int FilenameMax() { return ADAFSFORK_FILENAMEMAX; }

	static bool m_lfsInitialized;
	static int m_nvmPageSize;
	static int m_nvmPageCount;
	static int m_nvmCapacity;
	static int m_nvmRowSize;
	static int m_deviceBlocks;

	static void NVMErase(const volatile void* flash_ptr, uint32_t size);
	static void NVMRead(const volatile void* flash_ptr, void* data, uint32_t size);
	static void NVMEraseRow(const volatile void* flash_ptr);
	static inline uint32_t NVMReadWord(const void* data);
	static void NVMWrite(const volatile void* flash_ptr, const void* data, uint32_t size);

};


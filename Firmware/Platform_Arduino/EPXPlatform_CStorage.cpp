// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_CStorage.h"
#include "EPXVariant.h"
#include "Adafruit_SPIFlash.h"
#include "lfs.h"

EPX_OPTIMIZEFORDEBUGGING_ON


// On-board external flash (QSPI or SPI) macros should already
// defined in your board variant if supported
// - EXTERNAL_FLASH_USE_QSPI
// - EXTERNAL_FLASH_USE_CS/EXTERNAL_FLASH_USE_SPI
#if defined(EXTERNAL_FLASH_USE_QSPI)
Adafruit_FlashTransport_QSPI flashTransport;

#elif defined(EXTERNAL_FLASH_USE_SPI)
Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

#else
#define NO_FLASH_STORAGE
#endif

#ifndef NO_FLASH_STORAGE
#define LFS_BLOCK_SIZE		SFLASH_SECTOR_SIZE
#define LFS_DEVICE_READSIZE SFLASH_PAGE_SIZE
#define LFS_LOOK_AHEAD_SIZE	SFLASH_PAGE_SIZE

Adafruit_SPIFlash			flash(&flashTransport);
static bool					g_lfsInitialized = false;
static lfs_t				g_lfs;
static lfs_config			g_lfsCFG;


int user_provided_block_device_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size);
int user_provided_block_device_prog(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size);
int user_provided_block_device_erase(const struct lfs_config* c, lfs_block_t block);
int user_provided_block_device_sync(const struct lfs_config* c);




int user_provided_block_device_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size)
{
	flash.readBuffer((block * c->block_size + off), (uint8_t*)buffer, size);
	return 0;
}



int user_provided_block_device_prog(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size)
{
	flash.writeBuffer((block * c->block_size + off), (uint8_t*)buffer, size);
	return 0;
}



int user_provided_block_device_erase(const struct lfs_config* c, lfs_block_t block)
{
	flash.eraseSector(block);
	return 0;
}



int user_provided_block_device_sync(const struct lfs_config* c)
{
	return 0;
}
#endif



bool CStorage::Initialize(uint16_t pinMISO, uint16_t pinMOSI, uint16_t pinSCLK, uint16_t pinCS, bool format)
{
#ifndef NO_FLASH_STORAGE
	DEBUGLOGLN("Initializing flash storage");

	// Init external flash
	if (!flash.begin())
	{
		DEBUGLOGLN("Error, failed to initialize flash chip!");
		return false;
	}
	DEBUGLOGLN("Flash initialized");
	DEBUGLOGLN("Flash chip JEDEC ID: 0x%d", flash.getJEDECID());

	// If initialize has format request erase the chip before mounting
	if (format)
		flash.eraseChip();

	// mount the filesystem		
	DEBUGLOGLN("\tMounting LittleFS... PageSize %d, SectorSize %d, Blocksize %d", SFLASH_PAGE_SIZE, SFLASH_SECTOR_SIZE, SFLASH_BLOCK_SIZE);
	memset(&g_lfsCFG, 0x00, sizeof(g_lfsCFG));
	g_lfsCFG.read = user_provided_block_device_read;
	g_lfsCFG.prog = user_provided_block_device_prog;
	g_lfsCFG.erase = user_provided_block_device_erase;
	g_lfsCFG.sync = user_provided_block_device_sync;

	// block device configuration
	g_lfsCFG.read_size = LFS_DEVICE_READSIZE;
	g_lfsCFG.prog_size = LFS_DEVICE_READSIZE;
	g_lfsCFG.cache_size = LFS_BLOCK_SIZE,
	g_lfsCFG.block_size = LFS_BLOCK_SIZE;
	g_lfsCFG.block_cycles = 500;
	g_lfsCFG.lookahead_size = LFS_LOOK_AHEAD_SIZE;

	// Set the number of blocks based on the reported chip		
	g_lfsCFG.block_count = flash.numPages();

	// Now mount the device
	int err = lfs_mount(&g_lfs, &g_lfsCFG);

	// reformat if we can't mount the filesystem
	// this should only happen on the first boot
	if (err)
	{
		DEBUGLOGLN("\t**** FAILED to mount file system ****");
		flash.eraseChip();

		DEBUGLOGLN("\tFormatting LFS");
		err = lfs_format(&g_lfs, &g_lfsCFG);
		if (err)
		{
			DEBUGLOGLN("\t**** Format FAILURE ****");
		}
		DEBUGLOGLN("\tRe-Mounting LittleFS...");
		err = lfs_mount(&g_lfs, &g_lfsCFG);
		if (err)
		{
			DEBUGLOGLN("\t**** FAILED to re-mount file system ****");
		}
		else
		{
			g_lfsInitialized = true;
			uint32_t freeSpace = UsedSpace();
			DEBUGLOGLN("\tMounted file system, %d/%d USED BLOCKS", freeSpace, g_lfsCFG.block_count);
			return true;
		}
	}
	else
	{
		g_lfsInitialized = true;
		uint32_t freeSpace = UsedSpace();
		DEBUGLOGLN("\tMounted file system, %d/%d USED BLOCKS", freeSpace, g_lfsCFG.block_count);
		return true;
	}
#endif
	return false;
}



void CStorage::Power(bool on)
{
	
}
	


bool CStorage::Format()
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized)
	{
		DEBUGLOGLN("Formatting LittleFS...");
		flash.eraseChip();

		DEBUGLOGLN("\tFormatting LFS");
		int err = lfs_format(&g_lfs, &g_lfsCFG);
		if (err)
		{
			DEBUGLOGLN("\t**** Format FAILURE ****");
		}
		DEBUGLOGLN("\tMounting LittleFS...");
		err = lfs_mount(&g_lfs, &g_lfsCFG);
		if (err)
		{
			DEBUGLOGLN("\t**** FAILED to mount file system ****");
		}
		else
		{
			DEBUGLOGLN("\tMounted file system");
			return true;
		}
	}
#endif
	return false;
}



uint32_t CStorage::Capacity()
{
#ifdef NO_FLASH_STORAGE
	return 0;
#else
	return g_lfsCFG.block_count;
#endif
}



uint32_t CStorage::UsedSpace()
{
#ifndef NO_FLASH_STORAGE
#if defined (NRF52832_XXAA) || defined (NRF52840_XXAA) 
	return 0; // Bluefruit embeds an older version of LittleFS
#else
	if (g_lfsInitialized)
	{
		lfs_ssize_t lfsSize = lfs_fs_size(&g_lfs);
		return lfsSize;
	}
#endif
#endif
	return 0;
}



bool CStorage::CreateFolder(const char *pszFolder)
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized)
	{		
		char* pszFilenameUpr = epx_strupr((char*)pszFolder);
		int result = -1;
		if (pszFilenameUpr != NULL)
		{
			result = lfs_mkdir(&g_lfs, pszFilenameUpr);
			TFREE(pszFilenameUpr);
		}
		return result == 0;
	}
#endif
	return false;
}



void CStorage::EnumFolder(const char *pszFolder, PPERSISTED_SEQUENCE_LIST *ppFileList)
{
	*ppFileList = NULL;
#ifndef NO_FLASH_STORAGE
	lfs_dir_t lfdDir;
	
	DEBUGLOGLN("CStorage::EnumFolder %s", pszFolder);	
	char* pszFilenameUpr = epx_strupr((char*)pszFolder);
	if (g_lfsInitialized && pszFilenameUpr != NULL && lfs_dir_open(&g_lfs, &lfdDir, pszFilenameUpr) == 0)
	{
		int							result = 1;
		PPERSISTED_SEQUENCE_LIST	pLast = NULL;

		while (result > 0)
		{
			struct lfs_info info;

			result = lfs_dir_read(&g_lfs, &lfdDir, &info);
			if (result > 0 && info.type == LFS_TYPE_REG)
			{
				DEBUGLOGLN("\t> %s", info.name);
				PPERSISTED_SEQUENCE_LIST pNewEntry = (PPERSISTED_SEQUENCE_LIST)TMALLOC(sizeof(PERSISTED_SEQUENCE_LIST));
				if (pNewEntry != NULL)
				{
					memset(pNewEntry, 0x00, sizeof(PERSISTED_SEQUENCE_LIST));
					pNewEntry->pszFilename = (char*)TMALLOC(strlen(info.name) + 1);
					if (pNewEntry->pszFilename != NULL)
					{
						strcpy(pNewEntry->pszFilename, info.name);
						pNewEntry->size = (uint16_t)info.size;
						if (pLast == NULL)
							*ppFileList = pNewEntry;
						else
							pLast->pNext = pNewEntry;
						pLast = pNewEntry;
					}
				}
			}
		}
		lfs_dir_close(&g_lfs, &lfdDir);
	}
	if (pszFilenameUpr != NULL)
		TFREE(pszFilenameUpr);
#endif
}	
			
			

void CStorage::Close(void* pFile)
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized && pFile != NULL)
	{
		lfs_file_close(&g_lfs, (lfs_file_t*)pFile);
		TFREE(pFile);
	}
#endif
}



void *CStorage::CreateFile(const char *pszFilename)
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized)
	{
		lfs_file_t* pFile = (lfs_file_t*)TMALLOC(sizeof(lfs_file_t));
		if (pFile != NULL)
		{
			char* pszFilenameUpr = epx_strupr((char*)pszFilename);
			int result = -1;

			if (pszFilenameUpr != NULL)
			{
				result = lfs_file_open(&g_lfs, pFile, (const char*)pszFilenameUpr, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
				if (result < 0)
					DEBUGLOGLN("\t**** CreateFile lfs_file_open FAILED %s %d", pszFilenameUpr, result);
				TFREE(pszFilenameUpr);
			}
			if (result >= 0)
				return pFile;
			TFREE(pFile);
		}
	}
#endif
	return NULL;
}



void *CStorage::OpenFile(const char *pszFilename, int *pFileSize)
{	
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized)
	{
		lfs_file_t* pFile = (lfs_file_t*)TMALLOC(sizeof(lfs_file_t));
		if (pFile != NULL)
		{
			char* pszFilenameUpr = epx_strupr((char*)pszFilename);
			int result = -1;

			if (pszFilenameUpr != NULL)
			{
				result = lfs_file_open(&g_lfs, pFile, (const char*)pszFilenameUpr, LFS_O_RDONLY);
				if (result < 0)
					DEBUGLOGLN("\t**** OpenFile lfs_file_open FAILED %d", result);
				TFREE(pszFilenameUpr);
			}
			if (result >= 0)
			{
				if (pFileSize != NULL)
					*pFileSize = lfs_file_size(&g_lfs, pFile);
				return pFile;
			}
			TFREE(pFile);
		}
	}
#endif
	return NULL;
}



bool CStorage::DeleteFile(const char *pszFilename)
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized)
	{
		char* pszFilenameUpr = epx_strupr((char*)pszFilename);
		int result = -1;

		if (pszFilenameUpr != NULL)
		{
			result = lfs_remove(&g_lfs, (const char*)pszFilenameUpr);
			if (result < 0)
				DEBUGLOGLN("\t**** DeleteFile lfs_remove FAILED %d", result);
			TFREE(pszFilenameUpr);
		}
		return result >= 0;
	}
#endif
	return false;
}



uint16_t CStorage::ReadFile(void* pFile, void* pData, uint16_t cbToRead)
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized && pFile != NULL)
		return lfs_file_read(&g_lfs, (lfs_file_t*)pFile, pData, cbToRead);
#endif
	return 0;
}



uint16_t CStorage::WriteFile(void* pFile, void* pData, uint16_t cbToWrite)
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized && pFile != NULL)
		return lfs_file_write(&g_lfs, (lfs_file_t*)pFile, pData, cbToWrite);
#endif
	return 0;
}



uint32_t CStorage::FileSize(void *pFile)
{	
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized  && pFile != NULL)
		return (uint32_t)lfs_file_size(&g_lfs, (lfs_file_t*)pFile);
#endif
	return 0;
}



bool CStorage::FileExists(const char *pszFilename)
{
#ifndef NO_FLASH_STORAGE
	struct lfs_info info;

	if (g_lfsInitialized)
	{
		char* pszFilenameUpr = epx_strupr((char*)pszFilename);
		int result = -1;

		if (pszFilenameUpr != NULL)
		{
			result = lfs_stat(&g_lfs, (const char*)pszFilenameUpr, &info);
			TFREE(pszFilenameUpr);
		}
		return result >= 0;
	}
#endif
	return false;
}

	

uint32_t CStorage::Position(void *pFile)
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized && pFile != NULL)
		return lfs_file_tell(&g_lfs, (lfs_file_t*)pFile);
#endif
	return 0;
}



bool CStorage::Seek(void *pFile, uint32_t pos)
{
#ifndef NO_FLASH_STORAGE
	if (g_lfsInitialized && pFile != NULL)
		lfs_file_seek(&g_lfs, (lfs_file_t*)pFile, pos, LFS_SEEK_SET);
#endif
	return false;
}


			
void CStorage::FreeEnumFolderList(PPERSISTED_SEQUENCE_LIST *ppList)
{
#ifndef NO_FLASH_STORAGE
	PPERSISTED_SEQUENCE_LIST pCur, pHead = *ppList;

	while (pHead != NULL)
	{
		pCur = pHead;
		pHead = pCur->pNext;
		FreeEnumFolderListItem(pCur);
	}
	*ppList = NULL;
#endif
}



void CStorage::FreeEnumFolderListItem(PPERSISTED_SEQUENCE_LIST pItem)
{
	if (pItem != NULL)
	{
		if (pItem->pszFilename != NULL)
			free(pItem->pszFilename);		
		if (pItem->pszGUID != NULL)
			free(pItem->pszGUID);
		if (pItem->pszName != NULL)
			free(pItem->pszName);
		free(pItem);
	}
}



void CStorage::FreeEnumFolderListItemFilename(PPERSISTED_SEQUENCE_LIST pItem)
{
	if (pItem != NULL)
	{
		if (pItem->pszFilename != NULL)
		{
			free(pItem->pszFilename);
			pItem->pszFilename = NULL;		
		}
	}
}

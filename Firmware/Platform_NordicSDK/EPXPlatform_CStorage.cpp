// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <stdlib.h>
#include <string.h>
#include "EPXPlatform_SPIClass.h"
#include "EPXPlatform_CStorage.h"
#include "EPXPlatform_CFlashStorageDevice.h"
#include "EPXVariant.h"
#include "lfs.h"

#define FLASH_TOTAL_BLOCKS	64
#define LFS_BLOCK_SIZE		4096
#define LFS_DEVICE_READSIZE 256
#define LFS_LOOK_AHEAD_SIZE	256

static SPIClass				*g_pSPIFlash = NULL;
static CFlashStorageDevice *g_pFlashStorageDevice;	
static bool					g_lfsInitialized = false;
static lfs_t				g_lfs;
static lfs_config			g_lfsCFG;
static uint16_t				g_deviceBlocks = 0;

int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block);
int user_provided_block_device_sync(const struct lfs_config *c);




int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) 
{
	g_pFlashStorageDevice->ReadBuffer((block * c->block_size + off), (uint8_t *)buffer, size);
	return 0;
}



int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) 
{
	g_pFlashStorageDevice->WriteBuffer((block * c->block_size + off), (uint8_t *)buffer, size);
	return 0;
}



int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block) 
{
	g_pFlashStorageDevice->EraseUnit(block * c->block_size);	
	return 0;
}



int user_provided_block_device_sync(const struct lfs_config *c) 
{
	return 0;
}



bool CStorage::Initialize(uint16_t pinMISO, uint16_t pinMOSI, uint16_t pinSCLK, uint16_t pinCS, bool format)
{
	if (g_pSPIFlash == NULL)
	{		
		DEBUGLOGLN("Initializing flash storage");
		
		// Initialize SPI for flash IC
		g_pSPIFlash = new SPIClass(NRF_SPI0, pinMISO, pinSCLK, pinMOSI);
		g_pSPIFlash->begin();
		
		g_pFlashStorageDevice = new CFlashStorageDevice(pinCS, g_pSPIFlash);
		if(!g_pFlashStorageDevice->Initialize())
                  return false;
							
		//format = true;
		// If initialize has format request erase the chip before mounting
		if (format)
			FlashChip();
		
		// mount the filesystem		
		DEBUGLOGLN("\tMounting LittleFS...");
		memset(&g_lfsCFG, 0x00, sizeof(g_lfsCFG));
		g_lfsCFG.read = user_provided_block_device_read;
		g_lfsCFG.prog = user_provided_block_device_prog;
		g_lfsCFG.erase = user_provided_block_device_erase;
		g_lfsCFG.sync  = user_provided_block_device_sync;

		// block device configuration
		g_lfsCFG.read_size = LFS_DEVICE_READSIZE;
		g_lfsCFG.prog_size = LFS_DEVICE_READSIZE;
		g_lfsCFG.cache_size = LFS_BLOCK_SIZE,
		g_lfsCFG.block_size = LFS_BLOCK_SIZE;
		g_lfsCFG.block_cycles = 500,
		g_lfsCFG.lookahead_size = LFS_LOOK_AHEAD_SIZE;

		// Set the number of blocks based on the reported chip		
		g_lfsCFG.block_count = g_pFlashStorageDevice->BlockCount();
		g_deviceBlocks = g_lfsCFG.block_count;

		// Now mount the device
		int err = lfs_mount(&g_lfs, &g_lfsCFG);

		// reformat if we can't mount the filesystem
		// this should only happen on the first boot
		if(err) 
		{
			DEBUGLOGLN("\t**** FAILED to mount file system ****");
			FlashChip();
			
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
				uint32_t freeSpace = UsedSpace();
				DEBUGLOGLN("\tMounted file system, %d/%d USED BLOCKS", freeSpace, g_deviceBlocks);
				g_lfsInitialized = true;
				return true;
			}
		}		
		else
		{
			uint32_t freeSpace = UsedSpace();
			DEBUGLOGLN("\tMounted file system, %d/%d USED BLOCKS", freeSpace, g_deviceBlocks);
			g_lfsInitialized = true;
			return true;
		}
	}
	else
		return true;
	return false;
}



void CStorage::Power(bool on)
{
	if (g_lfsInitialized)
	{		
		if (on)
		{
			DEBUGLOGLN("CStorage::PowerUp");
			g_pFlashStorageDevice->PowerUp();
			delay(25);
		}
		else
			g_pFlashStorageDevice->PowerDown();
	}
}
	



void CStorage::FlashChip()
{
	DEBUGLOGLN("\tErasing flash chip STARTING");
	g_pFlashStorageDevice->ChipErase();
	DEBUGLOGLN("\tErasing flash chip COMPLETE");
	
}



bool CStorage::Format()
{
	if (g_lfsInitialized)
	{
		DEBUGLOGLN("Formatting LittleFS...");
		FlashChip();
		
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
			g_lfsInitialized = true;
			return true;
		}
	}
	return false;
}



uint32_t CStorage::Capacity()
{
	return FLASH_TOTAL_BLOCKS;
}



uint32_t CStorage::UsedSpace()
{
	
	lfs_ssize_t lfsSize = lfs_fs_size(&g_lfs);
	return lfsSize;
}



bool CStorage::CreateFolder(char *pszFolder)
{
	if (g_lfsInitialized)
		return lfs_mkdir(&g_lfs, pszFolder) == 0;
	return false;
}



void CStorage::EnumFolder(const char *pszFolder, PPERSISTED_SEQUENCE_LIST *ppFileList)
{
	lfs_dir_t lfdDir;
	
	*ppFileList = NULL;	
	if (g_lfsInitialized && lfs_dir_open(&g_lfs, &lfdDir, pszFolder) == 0)
	{
		int							result = 1;
		PPERSISTED_SEQUENCE_LIST	pLast = NULL;

		while (result > 0)
		{
			struct lfs_info info;
			
			result = lfs_dir_read(&g_lfs, &lfdDir, &info);
			if (result > 0 && info.type == LFS_TYPE_REG)
			{
				PPERSISTED_SEQUENCE_LIST pNewEntry = (PPERSISTED_SEQUENCE_LIST)malloc(sizeof(PERSISTED_SEQUENCE_LIST));
				if (pNewEntry != NULL)
				{
					memset(pNewEntry, 0x00, sizeof(PERSISTED_SEQUENCE_LIST));
					pNewEntry->pszFilename = (char *)malloc(strlen(info.name) + 1);
					if (pNewEntry->pszFilename != NULL)
					{
						strcpy(pNewEntry->pszFilename, info.name);
						pNewEntry->size = (uint16_t) info.size;
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
}	
			
			


void CStorage::Close(void *pFile)
{
	if (g_lfsInitialized && pFile != NULL)
	{
		lfs_file_close(&g_lfs, (lfs_file_t *) pFile);
		free(pFile);
	}
}



void *CStorage::CreateFile(const char *pszFilename)
{
	if (g_lfsInitialized)
	{
		lfs_file_t *pFile = (lfs_file_t *) malloc(sizeof(lfs_file_t));
		if (pFile != NULL)
		{			
			int result = lfs_file_open(&g_lfs, pFile, (const char *) epx_strupr((char *) pszFilename), LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
			if (result >= 0)
				return pFile;
			free(pFile);
		}
	}
	return NULL;
}



void *CStorage::OpenFile(const char *pszFilename, int *pFileSize)
{
	if (g_lfsInitialized)
	{
		lfs_file_t *pFile = (lfs_file_t *) malloc(sizeof(lfs_file_t));
		if (pFile != NULL)
		{			
			int result = lfs_file_open(&g_lfs, pFile, (const char *) epx_strupr((char *) pszFilename), LFS_O_RDONLY);
			if (result >= 0)
			{
				if (pFileSize != NULL)
					*pFileSize = lfs_file_size(&g_lfs, pFile);
				return pFile;
			}
			free(pFile);
		}		
	}
	return NULL;
}



bool CStorage::DeleteFile(const char *pszFilename)
{
	if (g_lfsInitialized)
		return lfs_remove(&g_lfs, (const char *) epx_strupr((char *) pszFilename)) >= 0;	
	return false;
}



uint16_t CStorage::ReadFile(void *pFile, void *pData, uint16_t cbToRead)
{
	if (g_lfsInitialized && pFile != NULL)
		return lfs_file_read(&g_lfs, (lfs_file_t *) pFile, pData, cbToRead);
	return 0;
}



uint16_t CStorage::WriteFile(void *pFile, void *pData, uint16_t cbToWrite)
{
	if (g_lfsInitialized && pFile != NULL)
		return lfs_file_write(&g_lfs, (lfs_file_t *) pFile, pData, cbToWrite);
	return 0;
}



uint32_t CStorage::FileSize(void *pFile)
{	
	if (pFile != NULL)
		return (uint32_t) lfs_file_size(&g_lfs, (lfs_file_t *) pFile);
	return 0;
}



bool CStorage::FileExists(const char *pszFilename)
{
	struct lfs_info info;
	
	if (g_lfsInitialized)
		return lfs_stat(&g_lfs, (const char *) epx_strupr((char *) pszFilename), &info) >= 0;
	return false;
}

	

uint32_t CStorage::Position(void *pFile)
{
	if (g_lfsInitialized && pFile != NULL)
		return lfs_file_tell(&g_lfs, (lfs_file_t *) pFile);
	return 0;
}



bool CStorage::Seek(void *pFile, uint32_t pos)
{
	if (g_lfsInitialized && pFile != NULL)
		lfs_file_seek(&g_lfs, (lfs_file_t *) pFile, pos, LFS_SEEK_SET);
	return false;
}


			
void CStorage::FreeEnumFolderList(PPERSISTED_SEQUENCE_LIST *ppList)
{
	PPERSISTED_SEQUENCE_LIST pCur, pHead = *ppList;

	while (pHead != NULL)
	{
		pCur = pHead;
		pHead = pCur->pNext;
		FreeEnumFolderListItem(pCur);
	}
	*ppList = NULL;
}



void CStorage::FreeEnumFolderListItem(PPERSISTED_SEQUENCE_LIST pItem)
{
	if (pItem != NULL)
	{
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



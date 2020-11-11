// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <SPI.h>
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include "EPXPlatform_CStorage.h"
#include "EPXVariant.h"

// Since SdFat doesn't fully support FAT12 such as format a new flash
// We will use Elm Cham's fatfs f_mkfs() to format
#include "ff.h"
#include "diskio.h"

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
  #error No QSPI/SPI flash are defined on your board variant.h !
#endif

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

static char g_szPlatformFilename[64];
uint32_t g_fileBytesWritten = 0;



bool CStorage::Initialize(uint16_t pinMISO, uint16_t pinMOSI, uint16_t pinSCLK, uint16_t pinCS, bool format)
{
	DEBUGLOGLN("Initializing flash storage");

	// Init external flash
	if(!flash.begin())
	{
		DEBUGLOGLN("Error, failed to initialize flash chip!");
		return false;
	}
	DEBUGLOGLN("Flash initialized");
	DEBUGLOGLN("Flash chip JEDEC ID: 0x%d", flash.getJEDECID());
	
Format();

	// Init file system on the flash
	if(!fatfs.begin(&flash))
	{
		DEBUGLOGLN("**ERROR** - failed to mount filesystem");
		if(!Format())
			return false;
	}

	DEBUGLOGLN("Filesystem mounted");	
	return true;
}



void CStorage::Power(bool on)
{
	
}
	


bool CStorage::Format()
{
	// Call fatfs begin and passed flash object to initialize file system
	DEBUGLOGLN("Creating and formatting FAT filesystem (this takes ~60 seconds)...");

	// Make filesystem.
	uint8_t buf[512] = {0};          // Working buffer for f_fdisk function.    
	FRESULT r = f_mkfs("", FM_FAT | FM_SFD, 0, buf, sizeof(buf));
	if (r != FR_OK) {
		DEBUGLOGLN("Error, f_mkfs failed with error code: %d", r);
		return false;
	}

	// sync to make sure all data is written to flash
	flash.syncBlocks();
	
	DEBUGLOGLN("Formatted flash!");

	// Check new filesystem
	if (!fatfs.begin(&flash)) {
		DEBUGLOGLN("Error, failed to mount newly formatted filesystem!");
		return false;
	}
	return true;
}



void CStorage::Close(void *pFile)
{
	if (pFile != NULL)
		((File *) pFile)->close();
}




uint32_t CStorage::Capacity()
{
	return 0;
}



uint32_t CStorage::UsedSpace()
{
	return 0;
}



bool CStorage::CreateFolder(char *pszFolder)
{
	if (!fatfs.exists(pszFolder))
	{
		if (fatfs.mkdir(pszFolder))
			return true;
	}
	return false;
}



void CStorage::EnumFolder(const char *pszFolder, PPERSISTED_SEQUENCE_LIST *ppFileList)
{
	char szFilename[16];
	PPERSISTED_SEQUENCE_LIST pLast = NULL;

	*ppFileList = NULL;

	// You can open a directory to list all the children (files and directories).
	// Just like the SD library the File type represents either a file or directory.
	File testDir = fatfs.open(pszFolder);
	if (!testDir) {
		DEBUGLOGLN("Error, failed to open directory!");
		return;
	}
	if (!testDir.isDirectory())
	{
		DEBUGLOGLN("Error, expected test to be a directory!");
		return;
	}

	File child = testDir.openNextFile();
	while (child)
	{
		child.getName(szFilename, sizeof(szFilename));

		if (ppFileList != NULL)
		{
			PPERSISTED_SEQUENCE_LIST pNewEntry = (PPERSISTED_SEQUENCE_LIST)malloc(sizeof(PERSISTED_SEQUENCE_LIST));
			if (pNewEntry != NULL)
			{
				memset(pNewEntry, 0x00, sizeof(PERSISTED_SEQUENCE_LIST));
				pNewEntry->pszFilename = (char *)malloc(strlen(szFilename) + 1);
				if (pNewEntry->pszFilename != NULL)
				{
					strcpy(pNewEntry->pszFilename, szFilename);
					pNewEntry->size = (uint16_t)child.size();

					if (pLast == NULL)
						*ppFileList = pNewEntry;
					else
						pLast->pNext = pNewEntry;
					pLast = pNewEntry;
				}
			}
		}

		// Keep calling openNextFile to get a new file.
		// When you're done enumerating files an unopened one will
		// be returned (i.e. testing it for true/false like at the
		// top of this while loop will fail).
		child = testDir.openNextFile();
	}

	// If you want to list the files in the directory again call
	// rewindDirectory().  Then openNextFile will start from the
	// top again.
	testDir.rewindDirectory();
}	
			
			

uint16_t CStorage::ReadFile(void *pFile, void *pData, uint16_t cbToRead)
{
	size_t bytesRead = 0;

	if (pFile != NULL)
		bytesRead = ((File *) pFile)->readBytes((uint8_t *) pData, cbToRead);
	return bytesRead;		
}



uint16_t CStorage::WriteFile(void *pFile, void *pData, uint16_t cbToWrite)
{
	size_t bytesWritten = 0;

	if (pFile != NULL)
	{
		bytesWritten = ((File *) pFile)->write(pData, cbToWrite);
		g_fileBytesWritten += bytesWritten;
	}
	return bytesWritten;
}



char *CStorage::TrimFilename(const char *pszInFilename)
{
	char *pszStart = g_szPlatformFilename;

	strcpy(g_szPlatformFilename, pszInFilename);
	if(*pszStart == '/')
		 pszStart++;

	char *pszSep = strchr(pszStart, '/');
	if(pszSep != NULL)
	{
		pszSep++;
		if(strlen(pszSep) > FilenameMax())
			pszSep[FilenameMax()] = 0x00;
	}
	return g_szPlatformFilename;
}



void *CStorage::CreateFile(const char *pszFilename)
{
	g_fileBytesWritten = 0;
	File* pFile = new File();
	if(pFile != NULL)
	{
		if(pFile->open(fatfs.vwd(), TrimFilename(pszFilename), (O_RDWR | O_CREAT)))
			return pFile;
		delete pFile;
	}
	return NULL;
}



void *CStorage::OpenFile(const char *pszFilename, int *pFileSize)
{
	File* pFile = new File();
	if(pFile != NULL)
	{
		if(pFile->open(fatfs.vwd(), TrimFilename(pszFilename), FILE_READ))
		{
			if(pFileSize != NULL)
				*pFileSize = pFile->size();		
			return pFile;
		}
		delete pFile;
	}
	return NULL;
}



bool CStorage::DeleteFile(const char *pszFilename)
{
	if (fatfs.exists(pszFilename))
	{
		if (!fatfs.remove(TrimFilename(pszFilename)))
		{
			DEBUGLOGLN("**ERROR** CStorage::DeleteFile %s", TrimFilename(pszFilename));
		}
		else
			return true;
	}
	return false;
}




uint32_t CStorage::FileSize(void *pFile)
{	
	if (pFile != NULL)
		return ((File *) pFile)->size();
	return 0;
}



bool CStorage::FileExists(const char *pszFilename)
{
	return fatfs.exists(TrimFilename(pszFilename));
}

	

uint32_t CStorage::Position(void *pFile)
{
	if (pFile != NULL)
		return ((File *) pFile)->position();
	return 0;
}



bool CStorage::Seek(void *pFile, uint32_t pos)
{
	if (pFile != NULL)
		return ((File *) pFile)->seek(pos);
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



//--------------------------------------------------------------------+
// fatfs diskio
//--------------------------------------------------------------------+
extern "C"
{

DSTATUS disk_status ( BYTE pdrv )
{
  (void) pdrv;
	return 0;
}

DSTATUS disk_initialize ( BYTE pdrv )
{
  (void) pdrv;
	return 0;
}

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
  (void) pdrv;
	return flash.readBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
  (void) pdrv;
  return flash.writeBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  (void) pdrv;

  switch ( cmd )
  {
    case CTRL_SYNC:
      flash.syncBlocks();
      return RES_OK;

    case GET_SECTOR_COUNT:
      *((DWORD*) buff) = flash.size()/512;
      return RES_OK;

    case GET_SECTOR_SIZE:
      *((WORD*) buff) = 512;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *((DWORD*) buff) = 8;    // erase block size in units of sector size
      return RES_OK;

    default:
      return RES_PARERR;
  }
}

}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once
/*
 * Class implementation for flash chip storage access on Nordic SDK
 *
 **/
#include "EPXPlatform_SPIClass.h"

// Flash status bits
#define  WINBOND_STAT_BUSY         0x01   // Erase/Write in Progress
#define  WINBOND_STAT_WRTEN        0x02   // Write Enable Latch

#define  WINBOND_CMD_STATREAD		0x02
#define  WINBOND_CMD_DATAWRITE		0x01
#define  WINBOND_CMD_DATAREAD      0x03
#define  WINBOND_CMD_READY         0x01


#define WINBOND_CMD_WRITEENABLE		0x06   // Write Enabled
#define WINBOND_CMD_WRITEDISABLE	0x04   // Write Disabled
#define WINBOND_CMD_READSTAT1		0x05   // Read Status Register 1
#define WINBOND_CMD_READSTAT2		0x35   // Read Status Register 2
#define WINBOND_CMD_CHIPERASE		0xC7   // Chip Erase
#define WINBOND_CMD_JEDECID			0x9F   // JEDEC ID
#define WINBOND_CMD_POWERDOWN		0xB9   // Power Down
#define WINBOND_CMD_RELEASPOWERDOWN 0xAB   // Release Power Down/Device ID
#define WINBOND_CMD_SECTORERASE4K   0x20   // Sector Erase (4KB)
#define WINBOND_CMD_PAGEPROG		0x02   // Page Program

// Supported flash chip type JEDEC identifiers
typedef enum
{
	CHIPTYPE_W25X20CL = 0xEF120000,
        CHIPTYPE_W25Q80DV = 0xEF140000,
        CHIPTYPE_GD25Q16 = 0xC81540C8
} enumChipTypes;



class CFlashStorageDevice
{
public:
				CFlashStorageDevice(int8_t ss, SPIClass *spiinterface);
	
	bool		ChipErase();
	bool		EraseUnit(uint32_t address);
	bool		Initialize();
	uint32_t	ChipGetJEDECID();
	uint32_t	ReadBuffer(uint32_t address, uint8_t *buffer, uint32_t len);
	uint32_t	WriteBuffer(uint32_t address, uint8_t *buffer, uint32_t len);
	void		PowerDown();
	void		PowerUp(bool overrideCache = false);
        uint32_t    BlockCount() { return m_Blocks; }
	uint32_t	DeviceID() { return m_deviceID; }

private:
	inline void	ActivateChip();
	inline void DeActivateChip();
	
	bool		ChipWaitForReady(uint32_t timeout = 1000);
	uint8_t		ChipReadStatus();
	uint8_t		ChipRead();
	void		ChipRead(uint8_t *data, uint16_t length);
	void		ChipWriteEnable(bool enable);
	void		ChipWrite(uint8_t data);
	void		ChipWrite(uint8_t *data, uint16_t length);
	
	uint32_t	WritePage(uint32_t address, uint8_t *buffer, uint32_t len);
	
	bool		m_bPoweredDown;			// True if chip in powereddown state
	SPIClass	*m_spi;					// Reference to system SPI capability
	int8_t		m_cs;					// Flash chip select line
	
    uint32_t	m_deviceID;				// JEDEC flash chip ID
    uint32_t    m_Blocks;				// Number of blocks available on flash chip
	uint32_t	m_PageSize;				// Page size on flash chip
	uint32_t	m_Pages;				// Number of pages available on flash chip
	uint32_t	m_Capacity;				// Byte capacity available on flash chip
};


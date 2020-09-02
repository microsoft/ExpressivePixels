#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "EPXPlatform_CFlashStorageDevice.h"
#include "EPXPlatform_Runtime.h"



CFlashStorageDevice::CFlashStorageDevice(int8_t chipSelect, SPIClass *spiinterface)
{	
	m_cs = chipSelect;
	m_spi = spiinterface;     // default to built in SPI
	m_bPoweredDown = false;
	
	m_PageSize = 0;
	m_Pages = 0;
	m_Capacity = 0;
}



bool CFlashStorageDevice::Initialize()
{
	pinMode(m_cs, OUTPUT);
	digitalWrite(m_cs, HIGH);  

	PowerUp(true);
	m_deviceID = ChipGetJEDECID();
	DEBUGLOGLN("\tFlash chip JEDEC ID: 0x%x", m_deviceID);
	if (m_deviceID == 0xFFFFFFFF)
		return false;

	if (m_deviceID == CHIPTYPE_W25X20CL) 
	{
          m_PageSize = 256;
          m_Pages = 2048;
          m_Blocks = 64;
          m_Capacity =  m_Pages * m_PageSize;
          DEBUGLOGLN("\tW25X20CL initialized - Capacity %d bytes", m_Capacity);
	}
	else if (m_deviceID == CHIPTYPE_W25Q80DV)
	{
          m_PageSize = 256;
          m_Pages = 4096;
          m_Blocks = 256;
          m_Capacity =  m_Pages * m_PageSize;
          DEBUGLOGLN("\tW25Q80DV initialized - Capacity %d bytes", m_Capacity);
	}
	else if (m_deviceID == CHIPTYPE_GD25Q16)
	{
          m_PageSize = 256;
          m_Pages = 8192;
          m_Blocks = 512;
          m_Capacity =  m_Pages * m_PageSize;
          DEBUGLOGLN("\tW25Q80DV initialized - Capacity %d bytes", m_Capacity);
	}
	else 
	{
          DEBUGLOGLN("\tUnknown flash chip");
          m_PageSize = 0;
          return false;
	}
	return true;
}



void CFlashStorageDevice::ActivateChip()
{	
	digitalWrite(m_cs, LOW);
}



void CFlashStorageDevice::DeActivateChip()
{
	digitalWrite(m_cs, HIGH);	
}



uint8_t CFlashStorageDevice::ChipReadStatus()
{
	ActivateChip();
	ChipWrite(WINBOND_CMD_READSTAT1);      // Send read status 1 cmd
	uint8_t status = ChipRead();                     // Dummy write
	DeActivateChip();
	return status & (WINBOND_STAT_BUSY | WINBOND_STAT_WRTEN);
}



void CFlashStorageDevice::ChipWrite(uint8_t data) 
{
	ChipWrite(&data, 1);
}



void CFlashStorageDevice::ChipWrite(uint8_t *data, uint16_t length) 
{
	uint8_t c;
  
	while (length--) 
	{
		c = *data;
		data++;
		m_spi->transfer(c);
	}
}
  


uint8_t CFlashStorageDevice::ChipRead() 
{
	uint8_t data;
	ChipRead(&data, 1);
	return data;
}



void CFlashStorageDevice::ChipRead(uint8_t *data, uint16_t length) 
{
	uint8_t x = 0;

	while (length--) 
	{
		x = m_spi->transfer(0xFF);
		*data = x;
		data++;
	}
}



bool CFlashStorageDevice::ChipWaitForReady(uint32_t timeout)
{
	uint8_t status;

	while (timeout > 0)
	{
		status = ChipReadStatus() & WINBOND_STAT_BUSY;
		if (status == 0)
		{
			return false;
		}
		nrf_delay_ms(1);
		timeout--;
	}

	return true;
}



void CFlashStorageDevice::ChipWriteEnable(bool enable)
{
	ActivateChip();
	ChipWrite(enable ? WINBOND_CMD_WRITEENABLE : WINBOND_CMD_WRITEDISABLE);
	DeActivateChip();
}



uint32_t CFlashStorageDevice::ChipGetJEDECID()
{
	uint32_t id;
	
	ActivateChip();
		
	ChipWrite(WINBOND_CMD_JEDECID); 
	id = ChipRead(); id <<= 8;
	ChipWrite(0x00);               // Dummy write
	id |= ChipRead(); id <<= 8;
	ChipWrite(0x00);               // Dummy write
	id |= ChipRead(); id <<= 8;
	ChipWrite(0x00);               // Dummy write
	id |= ChipRead();
	
	DeActivateChip();
	return id;
}



void CFlashStorageDevice::PowerDown()
{
	if (!m_bPoweredDown)
	{
		ActivateChip();
		ChipWrite(WINBOND_CMD_POWERDOWN);		
		DeActivateChip();
		m_bPoweredDown = true;
	}
}



void CFlashStorageDevice::PowerUp(bool overrideCache)
{
	if (m_bPoweredDown || overrideCache)
	{
		ActivateChip();
		ChipWrite(WINBOND_CMD_RELEASPOWERDOWN);		
		DeActivateChip();
		m_bPoweredDown = false;
	}
}



uint32_t CFlashStorageDevice::ReadBuffer(uint32_t address, uint8_t *buffer, uint32_t len)
{
	// Make sure the address is valid
	if(address >= m_Capacity)
		return 0;

	// Wait until the device is ready or a timeout occurs
	if(ChipWaitForReady())
	  return 0;

	ActivateChip();

	ChipWrite(WINBOND_CMD_DATAREAD);         
	ChipWrite((address >> 16) & 0xFF);        // address upper 8
	ChipWrite((address >> 8) & 0xFF);         // address mid 8
	ChipWrite(address & 0xFF);                // address lower 8	

	if ((address + len) > m_Capacity)
		len = m_Capacity - address;	

	// Fill response buffer
	ChipRead(buffer, len);

	DeActivateChip();
	return len;
}



uint32_t CFlashStorageDevice::WritePage(uint32_t address, uint8_t *buffer, uint32_t len)
{
	uint8_t status;

	// Make sure that the supplied data is no larger than the page size
	if(len > m_PageSize)
		return 0;

	// Make sure that the data won't wrap around to the beginning of the sector
	if((address % m_PageSize) + len > m_PageSize)
		// If you try to write to a page beyond the last byte, it will
		// wrap around to the start of the page, almost certainly
		// messing up your data
		return 0;

	// Wait until the device is ready or a timeout occurs
	if(ChipWaitForReady())
	  return 0;

	// Make sure the chip is write enabled
	ChipWriteEnable(1);

	// Make sure the write enable latch is actually set
	status = ChipReadStatus();
	if (!(status & WINBOND_STAT_WRTEN))
		// Throw a write protection error (write enable latch not set)
		return 0;

	ActivateChip();

	// Send page write command (0x02) plus 24-bit address
	ChipWrite(WINBOND_CMD_PAGEPROG);         // 0x02
	ChipWrite((address >> 16) & 0xFF);       // address upper 8
	ChipWrite((address >> 8) & 0xFF);        // address mid 8
	if(len == m_PageSize)
		// If len = 256 bytes, lower 8 bits must be 0 (see datasheet 11.2.17)
		ChipWrite(0);
	else
		ChipWrite(address & 0xFF);             // address lower 8
	

	// Transfer data
	ChipWrite(buffer, len);

	DeActivateChip();
	return (len);
}



uint32_t CFlashStorageDevice::WriteBuffer(uint32_t address, uint8_t *buffer, uint32_t len)
{
	uint32_t bytestowrite;
	uint32_t bufferoffset;
	uint32_t results;
	uint32_t byteswritten;

	// If the data is only on one page we can take a shortcut
	if((address % m_PageSize) + len <= m_PageSize)
		// Only one page ... write and be done with it
		return WritePage(address, buffer, len);

	// Block spans multiple pages
	byteswritten = 0;
	bufferoffset = 0;
	while (len)
	{
		// Figure out how many bytes need to be written to this page
		bytestowrite = m_PageSize - (address % m_PageSize);
		
		// Write the current page
		results = WritePage(address, buffer + bufferoffset, bytestowrite);
		byteswritten += results;
		
		// Abort if we returned an error
		if(!results)
		   return byteswritten;    // Something bad happened ... but return what we've written so far
		
		// Adjust address and len, and buffer offset
		address += bytestowrite;
		len -= bytestowrite;
		bufferoffset += bytestowrite;
		
		// If the next page is the last one, write it and exit
		// otherwise stay in the the loop and keep writing
		if(len <= m_PageSize)
		{
			// Write the last frame and then quit
			results = WritePage(address, buffer + bufferoffset, len);
			byteswritten += results;
			// Abort if we returned an error
			if(!results)
			  return byteswritten;    // Something bad happened ... but return what we've written so far
			// set len to zero to gracefully exit loop
			len = 0;
		}
	}

	// Return the number of bytes written
	return byteswritten;
}



bool CFlashStorageDevice::ChipErase()
{
	// Wait until the device is ready or a timeout occurs
	if(ChipWaitForReady()) return false;

	// Make sure the chip is write enabled
	ChipWriteEnable(1);

	// Make sure the write enable latch is actually set
	uint8_t status;
	status = ChipReadStatus();
	if (!(status & WINBOND_STAT_WRTEN))
	{
		// Throw a write protection error (write enable latch not set)
		return false;
	}

	// Send the erase chip command
	ActivateChip();
	ChipWrite(WINBOND_CMD_CHIPERASE); 
	DeActivateChip();

	// Wait until the busy bit is cleared before exiting
	// This can take up to 10 seconds according to the datasheet!
	if(ChipWaitForReady(12000))
	  return false;
  
	return true;
}



bool CFlashStorageDevice::EraseUnit(uint32_t address)
{
	// Wait until the device is ready or a timeout occurs
	if(ChipWaitForReady())    
		return false;

	// Make sure the chip is write enabled
	ChipWriteEnable(1);

	// Make sure the write enable latch is actually set
	uint8_t status;
	status = ChipReadStatus();
	if (!(status & WINBOND_STAT_WRTEN))
		// Throw a write protection error (write enable latch not set)
		return false;

	// Send the erase sector command;
	ActivateChip();
	ChipWrite(WINBOND_CMD_SECTORERASE4K); 
	ChipWrite((address >> 16) & 0xFF);        // address upper 8
	ChipWrite((address >> 8) & 0xFF);         // address mid 8
	ChipWrite(address & 0xFF);                // address lower 8
	DeActivateChip();

	// Wait until the busy bit is cleared before exiting
	// This can take up to 400ms according to the datasheet
	if(ChipWaitForReady(500))    
		return false;

	return true;
}

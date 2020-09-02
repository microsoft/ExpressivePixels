// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * SPI implementation on Nordic SDK
 *
 **/
#pragma once
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "EPXPlatform_GPIO.h"

#define F_CPU 48000000L
// SPI_HAS_TRANSACTION means SPI has
//   - beginTransaction()
//   - endTransaction()
//   - usingInterrupt()
//   - SPISetting(clock, bitOrder, dataMode)
#define SPI_HAS_TRANSACTION 1

#define SPI_MODE0 0x00
#define SPI_MODE1 0x01
#define SPI_MODE2 0x02
#define SPI_MODE3 0x03

#define SPI_MIN_CLOCK_DIVIDER 2


enum BitOrder {
	LSBFIRST = 0,
	MSBFIRST = 1
};

typedef enum
{
	MSB_FIRST = 0,
	LSB_FIRST
} SercomDataOrder;


typedef enum
{
	SERCOM_SPI_MODE_0 = 0,
	// CPOL : 0  | CPHA : 0
SERCOM_SPI_MODE_1,
	// CPOL : 0  | CPHA : 1
SERCOM_SPI_MODE_2, 
	// CPOL : 1  | CPHA : 0
SERCOM_SPI_MODE_3		// CPOL : 1  | CPHA : 1
} SercomSpiClockMode;



class SPISettings 
{
public:
	SPISettings(uint32_t clock, BitOrder bitOrder, uint8_t dataMode) {
		if (__builtin_constant_p(clock)) {
			init_AlwaysInline(clock, bitOrder, dataMode);
		}
		else {
			init_MightInline(clock, bitOrder, dataMode);
		}
	}

	// Default speed set to 4MHz, SPI mode set to MODE 0 and Bit order set to MSB first.
	SPISettings() { init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0); }

private:
	void init_MightInline(uint32_t clock, BitOrder bitOrder, uint8_t dataMode) {
		init_AlwaysInline(clock, bitOrder, dataMode);
	}

	void init_AlwaysInline(uint32_t clock, BitOrder bitOrder, uint8_t dataMode) __attribute__((__always_inline__)) {
		this->clockFreq = (clock >= (F_CPU / SPI_MIN_CLOCK_DIVIDER) ? F_CPU / SPI_MIN_CLOCK_DIVIDER : clock);

		this->bitOrder = (bitOrder == MSBFIRST ? MSB_FIRST : LSB_FIRST);

		switch (dataMode)
		{
		case SPI_MODE0:
			this->dataMode = SERCOM_SPI_MODE_0; break;
		case SPI_MODE1:
			this->dataMode = SERCOM_SPI_MODE_1; break;
		case SPI_MODE2:
			this->dataMode = SERCOM_SPI_MODE_2; break;
		case SPI_MODE3:
			this->dataMode = SERCOM_SPI_MODE_3; break;
		default:
			this->dataMode = SERCOM_SPI_MODE_0; break;
		}
	}

	uint32_t clockFreq;
	SercomSpiClockMode dataMode;
	SercomDataOrder bitOrder;

	friend class SPIClass;
};





class SPIClass {
public:
	SPIClass(NRF_SPI_Type *p_spi, uint8_t uc_pinMISO, uint8_t uc_pinSCK, uint8_t uc_pinMOSI);
	
	uint8_t transfer(uint8_t data);
	uint8_t transfer(uint8_t *pTXData, uint16_t cbTX, uint8_t *pRXData = NULL, uint16_t cbRX = 0);

	// Transaction Functions
	void beginTransaction(SPISettings settings);
	void endTransaction(void);

	void begin();
	void end();

	void setBitOrder(BitOrder order);
	void setDataMode(uint8_t uc_mode);
	void setFrequency(unsigned long freq);

private:
	void config(SPISettings settings);

	NRF_SPI_Type *_p_spi;
	uint8_t _uc_pinSS;
	uint8_t _uc_pinMiso;
	uint8_t _uc_pinMosi;
	uint8_t _uc_pinSCK;

	uint8_t _dataMode;
	uint32_t _bitOrder;
};





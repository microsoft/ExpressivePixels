// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "EPXPlatform_SPIClass.h"

const SPISettings DEFAULT_SPI_SETTINGS = SPISettings();



SPIClass::SPIClass(NRF_SPI_Type *p_spi, uint8_t uc_pinMISO, uint8_t uc_pinSCK, uint8_t uc_pinMOSI)
{
	_p_spi = p_spi;

	// pins
	_uc_pinMiso = uc_pinMISO;
	_uc_pinSCK = uc_pinSCK;
	_uc_pinMosi = uc_pinMOSI;

	_dataMode = SPI_MODE0;
	_bitOrder = SPI_CONFIG_ORDER_MsbFirst;
}



void SPIClass::begin()
{
	_p_spi->PSELSCK  = _uc_pinSCK;
	_p_spi->PSELMOSI = _uc_pinMosi;
	
	if (_uc_pinMiso != 0)
	{
		_p_spi->PSELMISO = _uc_pinMiso;
		nrf_gpio_pin_clear(_p_spi->PSELMISO);
		nrf_gpio_cfg_input(_p_spi->PSELMISO, NRF_GPIO_PIN_NOPULL);
	}
	nrf_gpio_cfg_output(_p_spi->PSELMOSI);

	nrf_gpio_pin_clear(_p_spi->PSELSCK);
	nrf_gpio_cfg_output(_p_spi->PSELSCK);	
	NRF_GPIO->PIN_CNF[_p_spi->PSELSCK] =
	    (GPIO_PIN_CNF_DIR_Output        << GPIO_PIN_CNF_DIR_Pos)
	  | (GPIO_PIN_CNF_INPUT_Connect     << GPIO_PIN_CNF_INPUT_Pos)
	  | (GPIO_PIN_CNF_PULL_Disabled     << GPIO_PIN_CNF_PULL_Pos)
	  | (GPIO_PIN_CNF_DRIVE_S0S1        << GPIO_PIN_CNF_DRIVE_Pos)
	  | (GPIO_PIN_CNF_SENSE_Disabled    << GPIO_PIN_CNF_SENSE_Pos);
	

	config(DEFAULT_SPI_SETTINGS);
}



void SPIClass::config(SPISettings settings)
{
	bool lsbFirst = false;
	uint32_t config;
	
	_p_spi->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);
	switch (settings.dataMode)
	{
	case SPI_MODE0:
		config = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
		break;
	case SPI_MODE1:
		config = (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
		break;
	case SPI_MODE2:
		config = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
		break;
	case SPI_MODE3:
		config = (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
		break;
	default:
		config = 0;
		break;
	}
	_dataMode = settings.dataMode;	
	_bitOrder = lsbFirst ? SPI_CONFIG_ORDER_LsbFirst : SPI_CONFIG_ORDER_MsbFirst;
	
	_p_spi->CONFIG = (config | (_bitOrder << SPI_CONFIG_ORDER_Pos));
	_p_spi->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M8;
	_p_spi->EVENTS_READY = 0U;
	_p_spi->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);
}



void SPIClass::end()
{
	_p_spi->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);
}



void SPIClass::beginTransaction(SPISettings settings)
{
	config(settings);
}



void SPIClass::endTransaction(void)
{
	_p_spi->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);
}



void SPIClass::setBitOrder(BitOrder order)
{
	this->_bitOrder = (order == MSBFIRST ? SPI_CONFIG_ORDER_MsbFirst : SPI_CONFIG_ORDER_LsbFirst);

	uint32_t config = this->_bitOrder;

	switch (this->_dataMode) {
	default:
	case SPI_MODE0:
		config |= (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
		config |= (SPI_CONFIG_CPHA_Leading    << SPI_CONFIG_CPHA_Pos);
		break;

	case SPI_MODE1:
		config |= (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
		config |= (SPI_CONFIG_CPHA_Trailing   << SPI_CONFIG_CPHA_Pos);
		break;

	case SPI_MODE2:
		config |= (SPI_CONFIG_CPOL_ActiveLow  << SPI_CONFIG_CPOL_Pos);
		config |= (SPI_CONFIG_CPHA_Leading    << SPI_CONFIG_CPHA_Pos);
		break;

	case SPI_MODE3:
		config |= (SPI_CONFIG_CPOL_ActiveLow  << SPI_CONFIG_CPOL_Pos);
		config |= (SPI_CONFIG_CPHA_Trailing   << SPI_CONFIG_CPHA_Pos);
		break;
	}
	_p_spi->CONFIG = config;
}



void SPIClass::setDataMode(uint8_t mode)
{
	this->_dataMode = mode;

	uint32_t config = this->_bitOrder;

	switch (this->_dataMode) {
	default:
	case SPI_MODE0:
		config |= (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
		config |= (SPI_CONFIG_CPHA_Leading    << SPI_CONFIG_CPHA_Pos);
		break;

	case SPI_MODE1:
		config |= (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
		config |= (SPI_CONFIG_CPHA_Trailing   << SPI_CONFIG_CPHA_Pos);
		break;

	case SPI_MODE2:
		config |= (SPI_CONFIG_CPOL_ActiveLow  << SPI_CONFIG_CPOL_Pos);
		config |= (SPI_CONFIG_CPHA_Leading    << SPI_CONFIG_CPHA_Pos);
		break;

	case SPI_MODE3:
		config |= (SPI_CONFIG_CPOL_ActiveLow  << SPI_CONFIG_CPOL_Pos);
		config |= (SPI_CONFIG_CPHA_Trailing   << SPI_CONFIG_CPHA_Pos);
		break;
	}
	_p_spi->CONFIG = config;
}



void SPIClass::setFrequency(unsigned long freq)
{
	_p_spi->FREQUENCY = freq;	
}



uint8_t SPIClass::transfer(uint8_t data)
{
	_p_spi->TXD = data;
	while (!_p_spi->EVENTS_READY);
	data = _p_spi->RXD;
	_p_spi->EVENTS_READY = 0x0UL;
	return data;
}



uint8_t SPIClass::transfer(uint8_t *pTXData, uint16_t cbTX, uint8_t *pRXData, uint16_t cbRX)
{
	uint8_t data;
	
	while (cbTX--)
	{
		_p_spi->TXD = *pTXData;
		while (!_p_spi->EVENTS_READY);
		data = _p_spi->RXD; // SPI needs to be read even if the device is write only		
		if(pRXData != NULL && cbRX > 0)
		{
			*pRXData = data;
			cbRX--;
		}
		_p_spi->EVENTS_READY = 0x0UL;
		pTXData++;
	}
	return 0;
}



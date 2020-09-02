// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * UART implementation on Nordic SDK
 *
 **/
#pragma once
#include "stdint.h"
#include "nrf_uart.h"
#include "EPXPlatform_CByteQueue.h"



class EPXUart
{
public:
	static const uint16_t SysExMaxSize;
	static CByteQueue UartFIFO;
	
	EPXUart(uint32_t rxPin, uint32_t txPin, uint32_t baud);
	
	void begin(unsigned long baud)
	{
		
	}

	
	
	int available(void)
	{
		return UartFIFO.available();
	}
	
	
	
	int read(void)
	{
		// return -1 when data is unvailable (arduino api)
		if(UartFIFO.available())
			return UartFIFO.pop();
		return -1;
	}
	
	
	uint16_t write(uint8_t c) 
	{
		// return uart_write_char(_uart, c);
		return 0;
	}
};

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "app_uart.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "EPXPlatform_Uart.h"


#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */


const uint16_t EPXUart::SysExMaxSize = 64;
CByteQueue EPXUart::UartFIFO;


void uart_event_handle(app_uart_evt_t * p_event)
{
	if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
	{
		NRF_LOG_INFO("uart_event_handle ERROR %d", p_event->data.error_communication);
	}
	else if (p_event->evt_type == APP_UART_FIFO_ERROR)
	{
		NRF_LOG_INFO("uart_event_handle FIFO %d", p_event->data.error_code);
	}
	else if (p_event->evt_type == APP_UART_DATA_READY)
	{
		uint8_t data;		
		app_uart_get(&data);				
		//NRF_LOG_INFO("UART DATA %c", data);		
		EPXUart::UartFIFO.push(data);
	}
}



EPXUart::EPXUart(uint32_t rxPin, uint32_t txPin, uint32_t baud)
{
	uint32_t err_code;

	const app_uart_comm_params_t comm_params =
	{
		rxPin == 0 ? UART_PIN_DISCONNECTED : rxPin,
		txPin == 0 ? UART_PIN_DISCONNECTED : txPin,
		UART_PIN_DISCONNECTED,
		UART_PIN_DISCONNECTED,
		APP_UART_FLOW_CONTROL_DISABLED,
		false,		
		baud
	};

	APP_UART_FIFO_INIT(&comm_params,
		UART_RX_BUF_SIZE,
		UART_TX_BUF_SIZE,
		uart_event_handle,
		APP_IRQ_PRIORITY_LOWEST,
		err_code);
	APP_ERROR_CHECK(err_code);
}


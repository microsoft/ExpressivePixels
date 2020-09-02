// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementation declarations for ByteQueue capability on Nordic SDK
 *
 **/
#pragma once
#include "app_util_platform.h"
#include "nrf_queue.h"

#define BYTEQUEUEBUFFERSIZE		1024

class CByteQueue
{
public:
	CByteQueue();

	bool	push(uint8_t val);
	int		available();
	int		pop();

private:
#ifdef NORDICSDK		
	uint8_t m_queueBuffer[BYTEQUEUEBUFFERSIZE + 1];
	nrf_queue_cb_t m_queuecb;
	nrf_queue_t m_queue;
#else
	bool	_buffer_overflow;

	uint8_t _receive_buffer[BYTEQUEUEBUFFERSIZE];
	uint8_t _receive_buffer_tail;
	uint8_t _receive_buffer_head;
#endif	
};


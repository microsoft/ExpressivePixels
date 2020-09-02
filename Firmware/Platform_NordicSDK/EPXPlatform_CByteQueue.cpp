// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_CByteQueue.h"

//NRF_QUEUE_DEF(uint8_t, BLESerialCacheQueue, 1024, NRF_QUEUE_MODE_NO_OVERFLOW);

/****************************************************************************/
/* BLESerialCache															*/
/****************************************************************************/
CByteQueue::CByteQueue()
{
#ifdef NORDICSDK	
	memset(&m_queuecb, 0x00, sizeof(m_queuecb));
	memset(&m_queue, 0x00, sizeof(m_queue));
	m_queue.p_cb = &m_queuecb;
	m_queue.element_size = sizeof(uint8_t);
	m_queue.p_buffer = m_queueBuffer;
	m_queue.size = BYTEQUEUEBUFFERSIZE;
	m_queue.mode = NRF_QUEUE_MODE_NO_OVERFLOW;
#else	
	_buffer_overflow = false;
	_receive_buffer_tail = 0;
	_receive_buffer_head = 0;
#endif	
}



bool CByteQueue::push(uint8_t val)
{
#ifdef NORDICSDK	
	uint32_t errno = nrf_queue_push(&m_queue, &val);
	return errno  == NRF_SUCCESS;
#else	
	uint8_t next = (_receive_buffer_tail + 1) % BYTEQUEUEBUFFERSIZE;
	if (next != _receive_buffer_head)
	{
		// save new data in buffer: tail points to where byte goes
		_receive_buffer[_receive_buffer_tail] = val;   // save new byte
		_receive_buffer_tail = next;
		return true;
	}
	_buffer_overflow = true;
	return false;
#endif	
}



// Read data from buffer
int CByteQueue::pop()
{
#ifdef NORDICSDK	
	uint8_t d = 0;
	uint32_t errno = nrf_queue_pop(&m_queue, &d);
	return d;
#else	
	// Empty buffer?
	if(_receive_buffer_head == _receive_buffer_tail)
		return - 1;

	// Read from "head"
	uint8_t d = _receive_buffer[_receive_buffer_head];   // grab next byte
	_receive_buffer_head = (_receive_buffer_head + 1) % BYTEQUEUEBUFFERSIZE;
	return d;
#endif	
}



int CByteQueue::available()
{
#ifdef NORDICSDK		
	return nrf_queue_utilization_get(&m_queue);
#else	
	return (_receive_buffer_tail + BYTEQUEUEBUFFERSIZE - _receive_buffer_head) % BYTEQUEUEBUFFERSIZE;
#endif	
}

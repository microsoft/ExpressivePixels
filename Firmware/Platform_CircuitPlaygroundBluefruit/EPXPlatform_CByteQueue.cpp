// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_CByteQueue.h"


CByteQueue::CByteQueue()
{
	_buffer_overflow = false;
	_receive_buffer_tail = 0;
	_receive_buffer_head = 0;
}



bool CByteQueue::push(uint8_t val)
{
	uint16_t next = (_receive_buffer_tail + 1) % BYTEQUEUEBUFFERSIZE;
	if (next != _receive_buffer_head)
	{
		// save new data in buffer: tail points to where byte goes
		_receive_buffer[_receive_buffer_tail] = val;   // save new byte
		_receive_buffer_tail = next;
		return true;
	}
	_buffer_overflow = true;
	return false;
}



// Read data from buffer
int CByteQueue::pop()
{
	// Empty buffer?
	if(_receive_buffer_head == _receive_buffer_tail)
		return - 1;

	// Read from "head"
	uint8_t d = _receive_buffer[_receive_buffer_head];   // grab next byte
	_receive_buffer_head = (_receive_buffer_head + 1) % BYTEQUEUEBUFFERSIZE;
	return d;
}



int CByteQueue::available()
{
	return (_receive_buffer_tail + BYTEQUEUEBUFFERSIZE - _receive_buffer_head) % BYTEQUEUEBUFFERSIZE;
}

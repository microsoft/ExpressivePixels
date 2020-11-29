// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#include "EPXPlatform_Runtime.h"

#define BYTEQUEUEBUFFERSIZE		1024

class CByteQueue
{
public:
	CByteQueue();

	bool	push(uint8_t val);
	int		available();
	int		pop();

private:
	bool	_buffer_overflow;

	uint8_t _receive_buffer[BYTEQUEUEBUFFERSIZE];
	uint16_t _receive_buffer_tail;
	uint16_t _receive_buffer_head;
};
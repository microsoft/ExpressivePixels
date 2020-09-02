// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include <stdint.h>
#include <stdlib.h>

class CSerialChannelBase
{
public:
	virtual char *channelName() = 0;
	virtual int	available() = 0;
	virtual int	read() = 0;
	virtual size_t write(void *pvPayload, uint16_t cb) = 0;
};



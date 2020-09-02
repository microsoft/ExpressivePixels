// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_CByteQueue.h"
#include "CSerialChannelBase.h"

#define SERIALCHANNEL_PURGE_THRESHOLD	(BYTEQUEUEBUFFERSIZE / 2)

class CSerialChannel : public CSerialChannelBase
{
public:
	CSerialChannel() {}

	CByteQueue *SerialCache() { return &m_SerialCache; }

	/**** CSerialChannelBase ****/
	char *channelName() { return (char *) "Serial"; }
	int	available() { return m_SerialCache.available(); }
	int	read() { return m_SerialCache.pop(); }
	size_t write(void *pvPayload, uint16_t cb);
	
private:
	CByteQueue				m_SerialCache;
};


#pragma once
#include "CSerialChannelBase.h"

#define MAX_STRING_PROTOCOL_LENGTH	237

class CStringProtocol
{
public:
	CStringProtocol();

	void Reset();
	void SetActiveStringProtocolSerialChannel(CSerialChannelBase *pCSerialChannelBase) { m_pCActiveStringProtocolSerialChannel = pCSerialChannelBase; }
	void StringProtocolProcess();
	void StringProtocolSend(uint8_t *payload, uint16_t payloadLength);
	
	virtual void StringPayloadFinalized() {}
	virtual void StringPayloadParse(uint8_t data) {}
	virtual void StringPayloadReset() {}

private:
	CSerialChannelBase	*m_pCActiveStringProtocolSerialChannel;  // Reference to active serial channel class
	uint16_t			m_stringBuilderLength;
};


// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * COBS protocol processing implementation
 **/
#pragma once
#include "CSerialChannelBase.h"

#define COBS_MAXFRAME_SIZE					255	// Total COBS window size
#define MAX_COBS_DATAFRAMESIZE				253 // Total payload storage size within COBS window

// Type of COBs frame
enum EPXFrameType { EPX_FRAMETYPE_HEADERPLUSDATA, EPX_FRAMETYPE_DATA };

// Type of data protocol stored within COBS packet
enum EPXProtocolFormat { EPX_PROTOCOLFORMAT_BINARY, EPX_PROTOCOLFORMAT_JSON, EPX_PROTOCOLFORMAT_BINARY_ACK };

// Control properties of COBS frame
enum EPXProtocolFlags { EPX_PROTOCOLFLAG_ACKRQ = 0x1 };


#pragma pack(push)
#pragma pack(1)

// Core COBS header
typedef struct
{
	uint8_t frameType; // COBS packet frame type 
} EPXPROTOCOL_0;



// First logical packet header 
typedef struct
{
	EPXPROTOCOL_0 epxProtocol0; // Core COBS header	
	uint8_t format;				// Data format of COBS frame
	uint16_t length;			// Payload stored in entire logical packet (across multiple COBS packets)
	uint8_t flags;				// Per packet control flags
} EPXPROTOCOL_HEADER;

#pragma pack(pop) 



// Class implementation of COBS protocol
class CCOBSProtocol
{
public:
	CCOBSProtocol();

	void						ProtocolProcess();
	void						ProtocolEncodeAndSend(uint8_t payloadFormat, uint8_t *payload, uint16_t payloadLength);
	uint8_t						ProtcolFormat() { return m_ProtocolFormat; }
	void						CompletionTrackingEnable() { m_progressCompletionTracking = true; }
	void						SetActiveSerialChannel(CSerialChannelBase *pCSerialChannelBase) { m_pCActiveSerialChannel = pCSerialChannelBase; }

	
	virtual void				PayloadFinalized(uint8_t format) {}
	virtual void				PayloadParse(uint8_t format, uint8_t data) {}
	virtual void				PayloadReset() {}
	virtual void				SendTransmissionCompletionUpdate(uint8_t progress) {}
	
	
	CSerialChannelBase			*m_pCActiveSerialChannel; // Reference to active serial channel class

private:
	void						ProtocolDecode(uint8_t *p, uint16_t length);
	void						ProtocolReset();
	void						ProtocolSendACK();
	
	bool						m_progressCompletionTracking;	// Progress for reporting back to connected host
	int							m_lastProgressCompletion;		// Previous progress for reporting back to connected host

	uint16_t					m_channelMaxCacheLoad;			// Debug tracking for unprocessor byte queue load
	uint8_t						m_ProtocolFormat;				// Current protocol format being decoded
	uint8_t						m_ProtocolFlags;				// Current flags for active logical payload
	uint8_t						m_ProtocolVersion;				// For future use
	uint8_t						m_COBS_StagingFillIndex;		// Fill position of streamed in frame
	uint8_t						m_COBS_StagingBuffer[COBS_MAXFRAME_SIZE + 1]; // Working buffer for streaming frame
	uint8_t						m_COBS_ProcessedBuffer[COBS_MAXFRAME_SIZE]; // Decoded buffer for streaming frame
	
	uint16_t					m_ProtocolPacketReceiveSize;	// Logical size of logical incoming frame
	uint16_t					m_ProtocolPacketReceiveRemaining; // Logical size of bytes remaining for incoming frame

	
};

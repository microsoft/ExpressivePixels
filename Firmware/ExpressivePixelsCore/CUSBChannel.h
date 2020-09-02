// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_CByteQueue.h"
#include "CSerialChannelBase.h"
#include "EPXGlobal.h"

#define USBCHANNEL_PURGE_THRESHOLD	(BYTEQUEUEBUFFERSIZE / 2)

typedef void(*PFN_EPXUSBCHANNNEL_PURGEREQUEST)(void *pinstance);


class CUSBChannel : public CSerialChannelBase
{
public:
	CUSBChannel();
	
	bool Initialize();
	CByteQueue *SerialCache() { return &m_ByteCache; }
	void Activate();
	void Process();
	void SetDeviceName(char *pszName);
	void SetAppInstance(void *pInst) { m_pAppInstance = pInst; }
	
	/**** CSerialChannelBase ****/
	char *channelName() { return (char *) "USB"; }
	int	available() { return m_ByteCache.available(); }
	int	read() { return m_ByteCache.pop(); }
	size_t write(void *pvPayload, uint16_t cb);

	
	void SetPowerStateChangedHandler(PFN_EPX_POWERSTATE_CHANGED pfnEPXPowerStateChanged) { m_pfnEPXPowerStateChanged = pfnEPXPowerStateChanged; }
	void SetConnectionStateChangedHandler(PFN_EPX_CONNECTIONSTATE_CHANGED pfnEPXConnectionStateChanged) { m_pfnEPXConnectionStateChanged = pfnEPXConnectionStateChanged; }
	void SetCommunicationReadyHandler(PFN_EPX_COMMUNICATION_READY pfnEPXCommunicationReady) { m_pfnEPXCommunicationReady = pfnEPXCommunicationReady; }	
	void SetPurgeRequestHandler(PFN_EPXUSBCHANNNEL_PURGEREQUEST pfnPurgeRequestHandler) { m_pfnPurgeRequestHandler = pfnPurgeRequestHandler; }
	
	
	
private:
	static void USB_ConnectionStageChanged(void *pinstance, bool connected);
	static void USB_PowerStageChanged(void *pinstance, bool connected);
	static void USB_ByteReceived(void *pinstance, uint8_t data);
	static void USB_CommunicationReady(void *pinstance);

	void							*m_pAppInstance;
	PFN_EPX_POWERSTATE_CHANGED		m_pfnEPXPowerStateChanged;
	PFN_EPX_CONNECTIONSTATE_CHANGED	m_pfnEPXConnectionStateChanged;	
	PFN_EPX_COMMUNICATION_READY		m_pfnEPXCommunicationReady;
	PFN_EPXUSBCHANNNEL_PURGEREQUEST	m_pfnPurgeRequestHandler;
	CByteQueue						m_ByteCache;
};


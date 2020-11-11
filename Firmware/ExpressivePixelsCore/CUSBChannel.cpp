// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_USB.h"
#include "CUSBChannel.h"

EPX_OPTIMIZEFORDEBUGGING_ON

CUSBChannel::CUSBChannel()
{
	m_pfnEPXPowerStateChanged = NULL;
	m_pfnEPXConnectionStateChanged = NULL;
	m_pfnPurgeRequestHandler = NULL;
}



bool CUSBChannel::Initialize()
{
	return EPXPlatform_USB_Initialize(this, USB_PowerStageChanged, USB_ConnectionStageChanged, USB_CommunicationReady, USB_ByteReceived);
}



void CUSBChannel::SetDeviceName(char *pszName)
{
	EPXPlatform_USB_SetDeviceName(pszName);
}



void CUSBChannel::Activate()
{
	EPXPlatform_USB_Activate();
}




void CUSBChannel::Process()
{
	EPXPlatform_USB_Process();
}



void CUSBChannel::USB_ByteReceived(void *pinstance, uint8_t data)
{
	CUSBChannel *pthis = (CUSBChannel *) pinstance; 
	pthis->m_ByteCache.push(data);
		
	// Force processing if cache growing beyond threshold
	if(pthis->m_ByteCache.available() > USBCHANNEL_PURGE_THRESHOLD)
	{
		DEBUGLOGLN("ExpressivePixelsSerialChannelCache FORCE FLUSH %d", pthis->m_ByteCache.available());
		(*pthis->m_pfnPurgeRequestHandler)(pthis);
	}
}



size_t CUSBChannel::write(void *pvPayload, uint16_t cb, bool altChannel)
{
	return EPXPlatform_USB_Write((uint8_t *)pvPayload, cb);
}



void CUSBChannel::USB_PowerStageChanged(void *pinstance, bool connected)
{
	CUSBChannel *pthis = (CUSBChannel *) pinstance; 

	if (connected)
		(*pthis->m_pfnEPXPowerStateChanged)(pthis->m_pAppInstance, EPXAPP_POWERSTATE_ON_USB, true);
	else
		(*pthis->m_pfnEPXPowerStateChanged)(pthis->m_pAppInstance, EPXAPP_POWERSTATE_ON_USB, false);
}



void CUSBChannel::USB_ConnectionStageChanged(void *pinstance, bool connected)
{
	CUSBChannel *pthis = (CUSBChannel *) pinstance; 
	
	if (connected)
		(*pthis->m_pfnEPXConnectionStateChanged)(pthis->m_pAppInstance, EPXAPP_CONNECTIONCHANNEL_USB, true);		
	else
		(*pthis->m_pfnEPXConnectionStateChanged)(pthis->m_pAppInstance, EPXAPP_CONNECTIONCHANNEL_USB, false);		
}



void CUSBChannel::USB_CommunicationReady(void *pinstance)
{
	CUSBChannel *pthis = (CUSBChannel *) pinstance; 	
	(*pthis->m_pfnEPXCommunicationReady)(pthis->m_pAppInstance, false);
}


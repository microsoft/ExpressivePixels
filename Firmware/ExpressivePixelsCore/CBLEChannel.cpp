// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "CBLEChannel.h"

EPX_OPTIMIZEFORDEBUGGING_ON

CBLEChannel g_CBLEChannel;



CBLEChannel::CBLEChannel()
{
	m_pfnEPXPowerStateChanged = NULL;
	m_pfnEPXConnectionStateChanged = NULL;
	m_pfnEPXBeaconReceived = NULL;
}



bool CBLEChannel::Initialize(char *pszDefaultBLEName)
{
	EPXPlatform_BLE_Initialize(this, pszDefaultBLEName, BLE_ConnectionStageChanged, BLE_CommunicationReady, BLE_ByteReceived);
	EPXPlatform_BLE_SetBeaconReceivedHandler(BLE_BeaconReceived);
	return true;	
}



void CBLEChannel::Start()
{
	EPXPlatform_BLE_Start();	
}



void CBLEChannel::Disconnect()
{
	EPXPlatform_BLE_Disconnect();
}



void CBLEChannel::SetManufacturerPayload(uint8_t * pPayload, uint8_t cbPayload)
{
	EPXPlatform_BLE_SetManufacturerPayload(pPayload, cbPayload);	
}



void CBLEChannel::SetDeviceName(char *pszName)
{
	strncpy(m_BLEDeviceName, pszName, BLEMAX_DEVICENAME - 1);
	EPXPlatform_BLE_SetDeviceName(m_BLEDeviceName);
}



char *CBLEChannel::GetRealizedDeviceName()
{
	return EPXPlatform_BLE_GetRealizedDeviceName();
}



uint32_t CBLEChannel::GetLastBeaconReceived()
{	
	return EPXPlatform_BLE_GetLastBeaconReceived();
}



void CBLEChannel::SetBeaconActivationEntries(BEACONACTIVATIONITEM ** ppBeaconActivationEntries)
{ 
	EPXPlatform_BLE_SetBeaconActivationEntries(ppBeaconActivationEntries);
}



void CBLEChannel::SetBeaconActivation(bool on)
{
	EPXPlatform_BLE_SetBeaconActivation(on);
}



void CBLEChannel::UpdateAdvertisingDeviceData()
{
	EPXPlatform_BLE_AdvertizingUpdate();
}




void CBLEChannel::BLE_ByteReceived(void *pinstance, bool altChannel, uint8_t data)
{
	CBLEChannel *pthis = (CBLEChannel *) pinstance; 
	pthis->m_BLESerialCache.push(data);
}



void CBLEChannel::BLE_ConnectionStageChanged(void *pinstance, bool connected)
{
	CBLEChannel *pthis = (CBLEChannel *) pinstance; 
	
	if (connected)
	{
		(*pthis->m_pfnEPXPowerStateChanged)(pthis->m_pAppInstance, EPXAPP_POWERSTATE_ON_BLE, true);
		(*pthis->m_pfnEPXConnectionStateChanged)(pthis->m_pAppInstance, EPXAPP_CONNECTIONCHANNEL_BLE, true);		
	}
	else
	{
		(*pthis->m_pfnEPXConnectionStateChanged)(pthis->m_pAppInstance, EPXAPP_CONNECTIONCHANNEL_BLE, false);		
		(*pthis->m_pfnEPXPowerStateChanged)(pthis->m_pAppInstance, EPXAPP_POWERSTATE_ON_BLE, false);
	}
}



void CBLEChannel::BLE_CommunicationReady(void *pinstance, bool altChannel)
{
	CBLEChannel *pthis = (CBLEChannel *) pinstance; 	
	(*pthis->m_pfnEPXCommunicationReady)(pthis->m_pAppInstance, altChannel);
}



void CBLEChannel::BLE_BeaconReceived(void *pinstance, char *pszHost, uint8_t beaconData)
{
	CBLEChannel *pthis = (CBLEChannel *) pinstance; 
	
	if (pthis->m_pfnEPXBeaconReceived != NULL)
		(*pthis->m_pfnEPXBeaconReceived)(pthis->m_pAppInstance, pszHost, beaconData);
}



size_t CBLEChannel::write(void *pvPayload, uint16_t cb, bool altChannel)
{
	return EPXPlatform_BLE_SendBytes(pvPayload, cb, altChannel);
}


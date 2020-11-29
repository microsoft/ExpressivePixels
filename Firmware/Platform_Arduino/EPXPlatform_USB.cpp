// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "Arduino.h"
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_USB.h"

EPX_OPTIMIZEFORDEBUGGING_ON

static bool											g_USBConnected = false;

void												*g_USBHostInstance = NULL;
static PFN_EPXPLATFORM_USB_CONNECTIONSTATECHANGED	g_pfnUSBConnectionStateChanged = NULL;
static PFN_EPXPLATFORM_USB_COMMUNICATIONREADY		g_pfnUSBCommunicationReady = NULL;
static PFN_EPXPLATFORM_USB_POWERSTATECHANGED		g_pfnUSBPowerStateChanged = NULL;
static PFN_EPXPLATFORM_USB_BYTERECEIVED				g_pfnUSBByteReceived = NULL;



size_t EPXPlatform_USB_Write(uint8_t *p, uint16_t cb)
{
	return Serial.write(p, cb);
}



void EPXPlatform_USB_SetDeviceName(char *pszDeviceName)
{
}



bool EPXPlatform_USB_Initialize(void *pinstance, PFN_EPXPLATFORM_USB_POWERSTATECHANGED pfnUSBPowerStateChanged, PFN_EPXPLATFORM_USB_CONNECTIONSTATECHANGED pfnConnectionStateChanged, PFN_EPXPLATFORM_USB_COMMUNICATIONREADY pfnCommunicationReady, PFN_EPXPLATFORM_USB_BYTERECEIVED pfnByteReceived)
{
	g_USBHostInstance = pinstance;
	g_pfnUSBPowerStateChanged = pfnUSBPowerStateChanged;
	g_pfnUSBCommunicationReady = pfnCommunicationReady;
	g_pfnUSBConnectionStateChanged = pfnConnectionStateChanged;
	g_pfnUSBByteReceived = pfnByteReceived;	
	return true;
}



bool EPXPlatform_USB_Activate()
{
	(*g_pfnUSBPowerStateChanged)(g_USBHostInstance, true);
	return true;
}



void EPXPlatform_USB_Process()
{
	if(Serial && !g_USBConnected)
	{
		g_USBConnected = true;		
		(*g_pfnUSBConnectionStateChanged)(g_USBHostInstance, true);	
		(*g_pfnUSBCommunicationReady)(g_USBHostInstance);			
		return;		
	}
	else if(!Serial && g_USBConnected)
	{
		g_USBConnected = false;
		(*g_pfnUSBConnectionStateChanged)(g_USBHostInstance, false);			
		return;
	}

	int processSize = min(1024, Serial.available());
	while(processSize--)
		(*g_pfnUSBByteReceived)(g_USBHostInstance, Serial.read());
}


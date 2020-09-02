// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "Arduino.h"
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_USB.h"

static bool											g_USBConnected = false;
static char											m_cdc_data_array[64];
unsigned long										g_usbBytesReceived = 0;
unsigned long										g_cUSBRXReceived = 0;
int													g_USBTXWaiting = 0;

void												*g_USBHostInstance = NULL;
static PFN_EPXPLATFORM_USB_CONNECTIONSTATECHANGED	g_pfnUSBConnectionStateChanged = NULL;
static PFN_EPXPLATFORM_USB_POWERSTATECHANGED		g_pfnUSBPowerStateChanged = NULL;
static PFN_EPXPLATFORM_USB_BYTERECEIVED				g_pfnUSBByteReceived = NULL;



size_t EPXPlatform_USB_Write(uint8_t *p, uint16_t cb)
{
	return Serial.write(p, cb);
}



bool EPXPlatform_USB_Initialize(void *pinstance, PFN_EPXPLATFORM_USB_POWERSTATECHANGED pfnUSBPowerStateChanged, PFN_EPXPLATFORM_USB_CONNECTIONSTATECHANGED pfnConnectionStateChanged, PFN_EPXPLATFORM_USB_BYTERECEIVED pfnByteReceived)
{
	g_USBHostInstance = pinstance;
	g_pfnUSBPowerStateChanged = pfnUSBPowerStateChanged;
	g_pfnUSBConnectionStateChanged = pfnConnectionStateChanged;
	g_pfnUSBByteReceived = pfnByteReceived;
	
	Serial.begin(115200);
	return true;
}



bool EPXPlatform_USB_Activate()
{
}



void EPXPlatform_USB_Process()
{
	if(Serial && !g_USBConnected)
	{
		g_USBConnected = true;
		(*g_pfnUSBPowerStateChanged)(g_USBHostInstance, true);
		(*g_pfnUSBConnectionStateChanged)(g_USBHostInstance, true);	
		return;		
	}
	else if(!Serial && g_USBConnected)
	{
		g_USBConnected = false;
		(*g_pfnUSBConnectionStateChanged)(g_USBHostInstance, false);			
		(*g_pfnUSBPowerStateChanged)(g_USBHostInstance, false);
		return;
	}

	int processSize = min(128, Serial.available());
	while(processSize--)
		(*g_pfnUSBByteReceived)(g_USBHostInstance, Serial.read());

/*
    if(processSize > 0)
    {
		char szFmt[129];
		for(int i = 0; i < processSize;i++)
		{
			uint8_t val = Serial.read();
			szFmt[i] = val;
			(*g_pfnUSBByteReceived)(g_USBHostInstance, val);			
		}
		szFmt[processSize - 1] = 0x00;
		DEBUGLOGLN(szFmt);
	}
	*/
}


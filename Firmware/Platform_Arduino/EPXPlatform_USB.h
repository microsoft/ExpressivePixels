// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	typedef void(*PFN_EPXPLATFORM_USB_BYTERECEIVED)(void *pinstance, uint8_t data);
	typedef void(*PFN_EPXPLATFORM_USB_CONNECTIONSTATECHANGED)(void *pinstance, bool connected);
	typedef void(*PFN_EPXPLATFORM_USB_COMMUNICATIONREADY)(void *pinstance);
	typedef void(*PFN_EPXPLATFORM_USB_POWERSTATECHANGED)(void *pinstance, bool powered);


	
	bool		EPXPlatform_USB_Initialize(void *pinstance, PFN_EPXPLATFORM_USB_POWERSTATECHANGED g_pfnUSBPowerStateChanged, PFN_EPXPLATFORM_USB_CONNECTIONSTATECHANGED pfnConnectionStateChanged, PFN_EPXPLATFORM_USB_COMMUNICATIONREADY pfnCommunicationReady, PFN_EPXPLATFORM_USB_BYTERECEIVED pfnByteReceived);
	size_t		EPXPlatform_USB_Write(uint8_t *p, uint16_t cb);
	bool		EPXPlatform_USB_Activate();
	void		EPXPlatform_USB_SetDeviceName(char *pszDeviceName);
	void		EPXPlatform_USB_Process();
	
	
#ifdef __cplusplus
}
#endif



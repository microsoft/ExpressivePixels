// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"
#include "CBLEBeaconActivation.h"

// #define DISABLE_BLE_ADVERTISING
#define BLEMAX_DEVICENAME	18


typedef struct _tagBeaconActivationItem
{
	char szBeaconHostName[32];
	uint8_t	beaconHostAddr[6];	
	uint8_t	m_beaconActivationBit;
	char szAnimationName[32];
	struct _tagBeaconActivationItem *pNext;
} BEACONACTIVATIONITEM;


#ifdef __cplusplus
extern "C" {
#endif

	typedef void(*PFN_EPXPLATFORM_BLE_BYTERECEIVED)(void *pinstance, bool altChannel, uint8_t data);
	typedef void(*PFN_EPXPLATFORM_BLE_COMMUNICATIONREADY)(void *pinstance, bool altChannel);
	typedef void(*PFN_EPXPLATFORM_BLE_CONNECTIONSTATECHANGED)(void *pinstance, bool connected);
	typedef void(*PFN_EPXPLATFORM_BLE_BEACONRECEIVED)(void *pinstance, char *pszHost, uint8_t beaconData);


	void		EPXPlatform_BLE_AdvertizingUpdate();
	void		EPXPlatform_BLE_Disconnect();
	void		EPXPlatform_BLE_Initialize(void *pinstance, char *pszDEFAULT_BLE_NAME, PFN_EPXPLATFORM_BLE_CONNECTIONSTATECHANGED pfnConnectionStateChanged, PFN_EPXPLATFORM_BLE_COMMUNICATIONREADY pfnCommunicationReady, PFN_EPXPLATFORM_BLE_BYTERECEIVED pfnByteReceived);
	void		EPXPlatform_BLE_SetManufacturerPayload(uint8_t *p, uint8_t cb);
	size_t		EPXPlatform_BLE_SendBytes(void *pvPayload, uint16_t cb, bool altChannel);
	uint32_t	EPXPlatform_BLE_GetLastBeaconReceived();
	char 		*EPXPlatform_BLE_GetRealizedDeviceName();
	void		EPXPlatform_BLE_SetBeaconActivation(bool on);
	void		EPXPlatform_BLE_SetBeaconActivationEntries(BEACONACTIVATIONITEM ** ppBeaconActivationEntries);
	void		EPXPlatform_BLE_SetBeaconReceivedHandler(PFN_EPXPLATFORM_BLE_BEACONRECEIVED pfnBLEBeaconReceived);
	void		EPXPlatform_BLE_SetDeviceName(char *pszDeviceName);
	void		EPXPlatform_BLE_Start();

#ifdef __cplusplus
}
#endif


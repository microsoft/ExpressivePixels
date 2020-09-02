// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"
#include "CBLEBeaconActivation.h"

#define DISABLE_BLE_ADVERTISING
#define BLEMAX_DEVICENAME	18


#ifdef __cplusplus
extern "C" {
#endif

	typedef void(*PFN_EPXPLATFORM_BLE_BYTERECEIVED)(void *pinstance, uint8_t data);
	typedef void(*PFN_EPXPLATFORM_BLE_CONNECTIONSTATECHANGED)(void *pinstance, bool connected);
	typedef void(*PFN_EPXPLATFORM_BLE_BEACONRECEIVED)(void *pinstance, char *pszHost, uint8_t beaconData);


	void		EPXPlatform_BLE_AdvertizingUpdate();
	void		EPXPlatform_BLE_Initialize(void *pinstance, PFN_EPXPLATFORM_BLE_CONNECTIONSTATECHANGED pfnConnectionStateChanged, PFN_EPXPLATFORM_BLE_BYTERECEIVED pfnByteReceived);
	void		EPXPlatform_BLE_SetManufacturerPayload(uint8_t *p, uint8_t cb);
	size_t		EPXPlatform_BLE_SendBytes(void *pvPayload, uint16_t cb);
	uint32_t	EPXPlatform_BLE_GetLastBeaconReceived();
	void		EPXPlatform_BLE_SetBeaconActivation(bool on);
	void		EPXPlatform_BLE_SetBeaconActivationEntries(BEACONACTIVATIONITEM ** ppBeaconActivationEntries);
	void		EPXPlatform_BLE_SetBeaconReceivedHandler(PFN_EPXPLATFORM_BLE_BEACONRECEIVED pfnBLEBeaconReceived);
	void		EPXPlatform_BLE_SetDeviceName(char *pszDeviceName);

#ifdef __cplusplus
}
#endif


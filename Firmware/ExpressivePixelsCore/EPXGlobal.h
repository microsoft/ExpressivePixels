// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"

typedef struct 
{
	uint8_t guid[16];
} EPX_GUID;


typedef struct
{
	const char *pszPinName;
	uint16_t gpioPin;	
} SWITCHACTIVATION_GPIOBUTTON_MAPPING;


enum ConnectionChannel
{
	EPXAPP_CONNECTIONCHANNEL_NONE = 0x0,
	EPXAPP_CONNECTIONCHANNEL_USB  = 0x1,
	EPXAPP_CONNECTIONCHANNEL_BLE  = 0x2
};



enum PowerStates
{
	EPXAPP_POWERSTATE_NONE   = 0x0,
	EPXAPP_POWERSTATE_ON_USB = 0x1,
	EPXAPP_POWERSTATE_ON_BLE = 0x2
};



typedef void(*PFN_EPX_TRACE_EVENT)(void *pinstance, char *pszTrace);
typedef void(*PFN_EPX_POWERSTATE_CHANGED)(void *pinstance, uint8_t state, bool set);
typedef void(*PFN_EPX_COMMUNICATION_READY)(void *pinstance);
typedef void(*PFN_EPX_CONNECTIONSTATE_CHANGED)(void *pinstance, uint8_t state, bool set);
typedef void(*PFN_EPX_BEACONRECEIVED)(void *pinstance, char *pszHost, uint8_t beaconData);


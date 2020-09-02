// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "EPXString.h"

typedef void(*PFN_ACIVATE_ANIMATION)(void *pContext, char *pszAnimationName, uint8_t triggerPowerMode);

class ITriggerSource
{
public:
	virtual void Initialize() = 0;	
	virtual void Process() = 0;	
	virtual bool ConsoleCommandProcess(const char *pszCommand, bool *pbResponseSuccess, EPXString &innerResponse) = 0;
	virtual EPXString AppendConsoleHelpSyntax() = 0;
	virtual void SetAnimationActivationHandler(void *pEPXHost, PFN_ACIVATE_ANIMATION pfnActivateAnimation) = 0;
	virtual void SetTraceEventHandler(void *pEPXHost, PFN_EPX_TRACE_EVENT pfnTraceEvent) = 0;
};


typedef struct _tagTriggerSourceItem
{
	ITriggerSource *pITriggerSource;
	struct _tagTriggerSourceItem *pNext;
} TRIGGERSOURCEITEM;



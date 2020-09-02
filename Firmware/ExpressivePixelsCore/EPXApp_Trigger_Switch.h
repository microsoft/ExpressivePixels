// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"
#include "EPXGlobal.h"
#include "EPXApp_ITriggerSource.h"

#define TTYTOKEN_SWITCHACTIVATION			F("SWITCHACTIVATION")
#define SWITCHACTIVATION_ENTRY_FILENAME		"SwitchActivations.dat"
#define TRIGGERSWITCH_DEBOUNCE_MILLIS		1000

typedef void(*PFN_ACIVATE_ANIMATION)(void *pContext, char *pszAnimationName, uint8_t triggerPowerMode);


typedef struct _tagSwitchTriggerActivationItem
{
	uint16_t	gpioPin;	
	const char *pszPin;
	char		szAnimationName[32];
	uint32_t	triggerTime;
	struct _tagSwitchTriggerActivationItem *pNext;
} SWITCHACTIVATIONITEM;




class CSwitchActivation : public ITriggerSource
{
public:
	CSwitchActivation();
	~CSwitchActivation();

	
	void Initialize();
	void Process();
	virtual bool ConsoleCommandProcess(const char *pszCommand, bool *pbResponseSuccess, EPXString &innerResponse);
	EPXString AppendConsoleHelpSyntax();
	
	bool AddEntry(char *pszPin, char *pszAnimationName);
	bool Load();	
	bool RemoveAll();
	bool RemoveEntry(char *pszPin);
	SWITCHACTIVATIONITEM **EntriesReference() { return &m_itemsHead; }
	SWITCHACTIVATIONITEM *FindEntry(uint16_t gpioPin);
	SWITCHACTIVATIONITEM *FirstEntry() { return m_itemsHead; }
	
	static void SubsystemInitialize(int numMappings, SWITCHACTIVATION_GPIOBUTTON_MAPPING *pMappings);
	void Clear();
	void SetAnimationActivationHandler(void *pEPXHost, PFN_ACIVATE_ANIMATION pfnActivateAnimation);
	void SetTraceEventHandler(void *pEPXHost, PFN_EPX_TRACE_EVENT pfnTraceEvent);
	void SetTrace(bool on);
	
private:
	bool Save();
	static void SystemGPIOEventHandler(void *pinstance, uint8_t event, uint16_t pin, uint16_t value);
	SWITCHACTIVATION_GPIOBUTTON_MAPPING *MapLogicalPin(char *pszPin);
	SWITCHACTIVATION_GPIOBUTTON_MAPPING *MapPhysicalPin(uint16_t pin);
	
	bool					m_bTracing;
	SWITCHACTIVATIONITEM	*m_itemsHead;
	PFN_ACIVATE_ANIMATION	m_pfnActivateAnimation;
	PFN_EPX_TRACE_EVENT		m_pfnTraceEvent;
	uint8_t					m_queuedButtonPin;
};

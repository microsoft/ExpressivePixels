// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"
#include "EPXGlobal.h"
#include "EPXApp_ITriggerSource.h"

#ifdef EPXMIDI

#define MIDIACTIVATION_ENTRY_FILENAME		"MIDIActivations.dat"

typedef struct _tagMIDIActivationItem
{
	uint8_t	channel;	
	uint8_t	note;
	char szAnimationName[32];
	struct _tagMIDIActivationItem *pNext;
} MIDIACTIVATIONITEM;




class CMIDIActivation
{
public:
	CMIDIActivation();
	~CMIDIActivation();

	bool AddEntry(char *pszHost, char *pszActivationBit, char *pszAnimationName);
	bool Load();	
	bool RemoveAll();
	bool RemoveEntry(char *pszHost, char *pszActivationBit);
	MIDIACTIVATIONITEM **EntriesReference() { return &m_itemsHead; }
	MIDIACTIVATIONITEM *FindEntry(char *pszHost, char *pszNote);
	MIDIACTIVATIONITEM *FindEntry(uint8_t channel, uint8_t note);
	MIDIACTIVATIONITEM *FirstEntry() { return m_itemsHead; }
	void Initialize();
	void Clear();
	void Process();
	void SetAnimationActivationHandler(void *pEPXHost, PFN_ACIVATE_ANIMATION pfnActivateAnimation);
	void SetTraceEventHandler(void *pEPXHost, PFN_EPX_TRACE_EVENT pfnTraceEvent);
	void SetTrace(bool on);
	
private:
	bool Save();
	
	static void NoteOff(uint8_t inChannel, uint8_t inNumber, uint8_t inVelocity);
	static void NoteOn(uint8_t inChannel, uint8_t inNumber, uint8_t inVelocity);

	bool					m_bTracing;
	MIDIACTIVATIONITEM		*m_itemsHead;
	PFN_ACIVATE_ANIMATION	m_pfnActivateAnimation;
	PFN_EPX_TRACE_EVENT		m_pfnTraceEvent;
};
#endif


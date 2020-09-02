// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXVariant.h"
#include "EPXApp_MIDI.h"
#include "EPXApp.h"


#ifdef EPXMIDI
#include "MIDI.h"

EPX_OPTIMIZEFORDEBUGGING_ON

EPXUart MIDISerial(GPIO_PIN_D0, 0, NRF_UART_BAUDRATE_31250);
void *g_pMIDIHostContext = NULL;
void *g_pfnEPXHostContext = NULL;


MIDI_CREATE_DEFAULT_INSTANCE();


CMIDIActivation::CMIDIActivation()
{
	m_bTracing = false;
	m_itemsHead = NULL;
	m_pfnActivateAnimation = NULL;
}



CMIDIActivation::~CMIDIActivation()
{
	Clear();	
}


void CMIDIActivation::Initialize()
{	
	// Initiate MIDI communications, listen to all channels
	g_pMIDIHostContext = this;
	MIDI.begin(MIDI_CHANNEL_OMNI);
	MIDI.setHandleNoteOn(NoteOn);
	MIDI.setHandleNoteOff(NoteOff); 
	
	Load();
}



void CMIDIActivation::SetAnimationActivationHandler(void *pEPXHost, PFN_ACIVATE_ANIMATION pfnActivateAnimation)
{
	g_pfnEPXHostContext = pEPXHost;
	m_pfnActivateAnimation = pfnActivateAnimation;
}



void CMIDIActivation::SetTraceEventHandler(void *pEPXHost, PFN_EPX_TRACE_EVENT pfnTraceEvent)
{
	g_pfnEPXHostContext = pEPXHost;
	m_pfnTraceEvent = pfnTraceEvent;
}



void CMIDIActivation::Process()
{
	MIDI.read();
}



bool CMIDIActivation::AddEntry(char *pszChannel, char *pszNote, char *pszAnimationName)
{
	MIDIACTIVATIONITEM *pCur = NULL, *pNew;
	
	pCur = FindEntry(pszChannel, pszNote);
	if (pCur != NULL)
	{
		strcpy(pCur->szAnimationName, pszAnimationName);		
		return Save();
	}
	
	pNew = (MIDIACTIVATIONITEM *) malloc(sizeof(MIDIACTIVATIONITEM));
	if (pNew == NULL)
		return false;
	memset(pNew, 0x00, sizeof(MIDIACTIVATIONITEM));
	pNew->channel = atoi(pszChannel);
	pNew->note = atoi(pszNote);
	strcpy(pNew->szAnimationName, pszAnimationName);
	
	// Move to the end of the list
	if(m_itemsHead == NULL)
		m_itemsHead = pNew;
	else
	{
		pCur = m_itemsHead;
		while (pCur->pNext != NULL)
			pCur = pCur->pNext;
		pCur->pNext = pNew;
	}
	return Save();
}



bool CMIDIActivation::RemoveEntry(char *pszChannel, char *pszNote)
{
	MIDIACTIVATIONITEM *pCur = m_itemsHead, *pDelete, *pLast;
	
	while (pCur != NULL)
	{
		if (atoi(pszChannel) == pCur->channel && atoi(pszNote) == pCur->note)
		{
			pDelete = pCur;
			if (pCur == m_itemsHead)
				m_itemsHead = pCur->pNext;
			else
				pLast->pNext = pCur->pNext;
			free(pDelete);
			return Save();
		}
		pLast = pCur;
		pCur = pCur->pNext;
	}
	return false;
}



bool CMIDIActivation::RemoveAll()
{
	Clear();
	return Save();
}



MIDIACTIVATIONITEM *CMIDIActivation::FindEntry(char *pszChannel, char *pszNote)
{
	return FindEntry(atoi(pszChannel), atoi(pszNote));
}



MIDIACTIVATIONITEM *CMIDIActivation::FindEntry(uint8_t channel, uint8_t note)
{
	MIDIACTIVATIONITEM *pCur = m_itemsHead;
	
	while (pCur != NULL)
	{
		if (pCur->channel == channel && pCur->note == note)
			return pCur;
		pCur = pCur->pNext;
	}
	return NULL;
}



void CMIDIActivation::Clear()
{
	MIDIACTIVATIONITEM *pCur = m_itemsHead;
	
	while (m_itemsHead != NULL)
	{
		pCur = m_itemsHead;
		m_itemsHead = pCur->pNext;
		free(pCur);
	}
}



void CMIDIActivation::SetTrace(bool on)
{
	m_bTracing = on;
}



bool CMIDIActivation::Load()
{
	bool success = false;
	int	 fileSize;
	void *pFile;
	
	Clear();
	pFile = CStorage::OpenFile(MIDIACTIVATION_ENTRY_FILENAME, &fileSize);
	if (pFile != NULL)
	{
		MIDIACTIVATIONITEM *pNew, *pLatest = NULL;
		
		while (fileSize > 0)
		{
			pNew = (MIDIACTIVATIONITEM *) malloc(sizeof(MIDIACTIVATIONITEM));
			
			int bytesRead = CStorage::ReadFile(pFile, pNew, sizeof(MIDIACTIVATIONITEM));
			if (bytesRead != sizeof(MIDIACTIVATIONITEM))
				goto done;
			pNew->pNext = NULL;
			
			if (m_itemsHead == NULL)
				m_itemsHead = pLatest = pNew;
			else
			{
				pLatest->pNext = pNew;
				pLatest = pNew;
			}
			fileSize -=	sizeof(MIDIACTIVATIONITEM);
		}		
		success = true;
done:		
		CStorage::Close(pFile);
	}
	return success;
}



bool CMIDIActivation::Save()
{
	bool success = false;
	
	if (m_itemsHead == NULL)
	{		
		CStorage::DeleteFile(MIDIACTIVATION_ENTRY_FILENAME);
		success = true;
	}
	else
	{
		MIDIACTIVATIONITEM *pCur = m_itemsHead;
		void *pFile;
				
		pFile = CStorage::CreateFile(MIDIACTIVATION_ENTRY_FILENAME);
		if (pFile != NULL)
		{
			while (pCur != NULL)
			{				
				int bytesWritten = CStorage::WriteFile(pFile, pCur, sizeof(MIDIACTIVATIONITEM));
				if (bytesWritten != sizeof(MIDIACTIVATIONITEM))
					goto done;				
				pCur = pCur->pNext;	
			}
			success = true;
done:			
			CStorage::Close(pFile);
		}
	}
	return success;
}



void CMIDIActivation::NoteOn(uint8_t inChannel, uint8_t inNumber, uint8_t inVelocity)
{
	CMIDIActivation *pCMIDIActivation = (CMIDIActivation *) g_pMIDIHostContext;
		
	/*
	if (inNumber == 65)
	{
		inNumber = inVelocity;
		inVelocity = 65;
	}
	*/
	
	DEBUGLOGLN("CMIDIActivation::NoteOn Ch %d, Note %d", inChannel, inNumber);

	if (pCMIDIActivation->m_bTracing && pCMIDIActivation->m_pfnTraceEvent != NULL)
	{
		char szChannel[16], szNote[16];
		
		sprintf(szChannel, "%d", inChannel);
		sprintf(szNote, "%d", inNumber);
		EPXString strFmt = EPXString("MIDI Event : Channel ") + szChannel + ", " + szNote;
		(*pCMIDIActivation->m_pfnTraceEvent)(g_pfnEPXHostContext, (char *) strFmt.c_str());
	}
	
	if (pCMIDIActivation != NULL)
	{
		MIDIACTIVATIONITEM *activationItem;
		activationItem = pCMIDIActivation->FindEntry(inChannel, inNumber);
		if (activationItem != NULL && pCMIDIActivation->m_pfnActivateAnimation != NULL)
		{
			DEBUGLOGLN("CMIDIActivation::NoteOn ACTIVATING ANIMATION %s", activationItem->szAnimationName);
			(*pCMIDIActivation->m_pfnActivateAnimation)(g_pfnEPXHostContext, activationItem->szAnimationName, EPXAPP_TRIGGERPOWERMODE_ALWAYSON);
		}
	}
}



void CMIDIActivation::NoteOff(uint8_t inChannel, uint8_t inNumber, uint8_t inVelocity)
{
}

#endif


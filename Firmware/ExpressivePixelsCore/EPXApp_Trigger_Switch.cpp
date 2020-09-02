// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXVariant.h"
#include "EPXPlatform_GPIO.h"
#include "EPXApp_Trigger_Switch.h"
#include "EPXApp.h"
#include "EPXPlatform_GPIO.h"

EPX_OPTIMIZEFORDEBUGGING_ON

static int									g_numGPIOButtonMappings = 0;
static SWITCHACTIVATION_GPIOBUTTON_MAPPING *g_pGPIOButtonMappings;
static void									*g_pSwitchTriggerHostContext = NULL;
static void									*g_pfnEPXHostContext = NULL;



CSwitchActivation::CSwitchActivation()
{
	m_bTracing = false;
	m_itemsHead = NULL;
	m_pfnActivateAnimation = NULL;
	m_queuedButtonPin = 0;
}



CSwitchActivation::~CSwitchActivation()
{
	Clear();	
}



void CSwitchActivation::SubsystemInitialize(int numMappings, SWITCHACTIVATION_GPIOBUTTON_MAPPING *pMappings)
{
	g_numGPIOButtonMappings = numMappings;
	g_pGPIOButtonMappings = pMappings;
}



void CSwitchActivation::Initialize()
{	
	g_pSwitchTriggerHostContext = this;	
	EPXPlatform_GPIO_Initialize(this, SystemGPIOEventHandler);	
	Load();
}



EPXString CSwitchActivation::AppendConsoleHelpSyntax()
{
	EPXString body;
	
	body += "- SWITCHACTIVATION ADD <";
	
	
	for (int i = 0; i < g_numGPIOButtonMappings; i++)
	{
		if (i > 0)
			body += "|";
		body += g_pGPIOButtonMappings[i].pszPinName;
	}
	body += "> <ANIMATION NAME> (Add switch activation entry)\n";
	body += "- SWITCHACTIVATION DELETE <";
	for(int i = 0 ; i < g_numGPIOButtonMappings ; i++)
	{
		if (i > 0)
			body += "|";
		body += g_pGPIOButtonMappings[i].pszPinName;
	}		
	body += "> (Remove switch activation entry)\n";
	body += "- SWITCHACTIVATION DELETEALL (Remove all switch activation entries)\n";
	body += "- SWITCHACTIVATION LIST (List switch activations)\n";
	return body;
}



bool CSwitchActivation::ConsoleCommandProcess(const char *pszCommand, bool *pbResponseSuccess, EPXString &innerResponse)
{
	bool syntax = false;
	char *pszPostToken;
	
	if (CExpressivePixelsApp::CompareToken(pszCommand, (const char *)TTYTOKEN_SWITCHACTIVATION, &pszPostToken))
	{
		char *pszAction = CExpressivePixelsApp::ExtractNextParameter(pszPostToken, &pszPostToken);
		if (pszAction != NULL)
		{
			if (!stricmp(pszAction, (const char *)TTYTOKEN_ADD))
			{
				char *pszPin = CExpressivePixelsApp::ExtractNextParameter(pszPostToken, &pszPostToken);
				if (pszPin != NULL)
				{
					if (pszPostToken != NULL)						
					{
						syntax = true;									
						*pbResponseSuccess = AddEntry(pszPin, pszPostToken);
					}
				}
				if (!syntax)
				{
					innerResponse += JSON_KEYOBJECTOPEN("info");	
					innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Syntax error");
					innerResponse += JSON_CLOSEOBJECT;
				}
			}
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_DELETE))
			{
				char *pszPin = CExpressivePixelsApp::ExtractNextParameter(pszPostToken, &pszPostToken);
				if (pszPin != NULL)
				{
					syntax = true;						
					*pbResponseSuccess = RemoveEntry(pszPin);
				}				
				if (!syntax)
				{
					innerResponse += JSON_KEYOBJECTOPEN("info");	
					innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Syntax error");
					innerResponse += JSON_CLOSEOBJECT;
				}
			}
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_DELETEALL))
				*pbResponseSuccess = RemoveAll();			
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_LIST))
			{
				SWITCHACTIVATIONITEM *pCur = FirstEntry();
				
				innerResponse += JSON_KEYOBJECTOPEN("info");				
				innerResponse += JSON_KEYOBJECTOPENARRAY("Switch Triggers");				
				while (pCur != NULL)
				{
					innerResponse += JSON_QUOTEDVALUE(EPXString("PinName ") + pCur->pszPin + " -> " + pCur->szAnimationName);
					if (pCur->pNext != NULL)
						innerResponse += EPXString(JSON_CONTINUATION) + JSON_LINESEPARATOR;
					pCur = pCur->pNext;
				}				
				innerResponse += EPXString(JSON_CLOSEARRAY);
				innerResponse += JSON_CLOSEOBJECT;
				*pbResponseSuccess = true;
			}
			else
			{
				innerResponse += JSON_KEYOBJECTOPEN("info");	
				innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Unknown SWITCHACTIVATION action");
				innerResponse += JSON_CLOSEOBJECT;
			}
		}
		return true;
	}
	return false;
}



void CSwitchActivation::Process()
{
	if (m_queuedButtonPin > 0)
	{
		SWITCHACTIVATIONITEM *pSwitchItem = (SWITCHACTIVATIONITEM *) FindEntry(m_queuedButtonPin);
		if (pSwitchItem != NULL && m_pfnActivateAnimation != NULL)
		{
			DEBUGLOGLN("SwitchActivation PIN ACTIVATING ANIMATION %s", pSwitchItem->szAnimationName);
			
			if (millisPassed(pSwitchItem->triggerTime) > TRIGGERSWITCH_DEBOUNCE_MILLIS)
			{					
				(*m_pfnActivateAnimation)(g_pfnEPXHostContext, pSwitchItem->szAnimationName, EPXAPP_TRIGGERPOWERMODE_SINGLEANIMATION);
				pSwitchItem->triggerTime = millis();
			}
			else
			{
				DEBUGLOGLN("CSwitchActivation::Process DEBOUNCED");				
			}
		}
		m_queuedButtonPin = 0;
	}
}



void CSwitchActivation::SetAnimationActivationHandler(void *pEPXHost, PFN_ACIVATE_ANIMATION pfnActivateAnimation)
{
	g_pfnEPXHostContext = pEPXHost;
	m_pfnActivateAnimation = pfnActivateAnimation;
}



void CSwitchActivation::SetTraceEventHandler(void *pEPXHost, PFN_EPX_TRACE_EVENT pfnTraceEvent)
{
	g_pfnEPXHostContext = pEPXHost;
	m_pfnTraceEvent = pfnTraceEvent;
}




SWITCHACTIVATION_GPIOBUTTON_MAPPING *CSwitchActivation::MapLogicalPin(char *pszPin)
{
	for (int i = 0; i < g_numGPIOButtonMappings; i++)
	{
		if (stricmp(pszPin, g_pGPIOButtonMappings[i].pszPinName) == 0)
			return &g_pGPIOButtonMappings[i];		
	}
	return NULL;
}



SWITCHACTIVATION_GPIOBUTTON_MAPPING *CSwitchActivation::MapPhysicalPin(uint16_t pin)
{
	for (int i = 0; i < g_numGPIOButtonMappings; i++)
	{
		if (pin == g_pGPIOButtonMappings[i].gpioPin)
			return &g_pGPIOButtonMappings[i];		
	}
	return NULL;
}



bool CSwitchActivation::AddEntry(char *pszPin, char *pszAnimationName)
{
	SWITCHACTIVATIONITEM *pCur = NULL, *pNew;
	
	SWITCHACTIVATION_GPIOBUTTON_MAPPING *pMapping = MapLogicalPin(pszPin);
	if (pMapping == NULL)
		return false;
	
	pCur = FindEntry(pMapping->gpioPin);
	if (pCur != NULL)
	{
		strcpy(pCur->szAnimationName, pszAnimationName);		
		return Save();
	}
	
	pNew = (SWITCHACTIVATIONITEM *) malloc(sizeof(SWITCHACTIVATIONITEM));
	if (pNew == NULL)
		return false;
	memset(pNew, 0x00, sizeof(SWITCHACTIVATIONITEM));
	pNew->gpioPin = pMapping->gpioPin;
	pNew->pszPin = (char *) pMapping->pszPinName;
	strcpy(pNew->szAnimationName, pszAnimationName);
	
	// Register the button with GPIO 
	EPXPlatform_GPIO_ButtonClickConfigure(pNew->gpioPin);			
	
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



bool CSwitchActivation::RemoveEntry(char *pszPin)
{
	SWITCHACTIVATIONITEM *pCur = m_itemsHead, *pDelete, *pLast;
	
	SWITCHACTIVATION_GPIOBUTTON_MAPPING *pMapping = MapLogicalPin(pszPin);
	if (pMapping == NULL)
		return false;
	
	while (pCur != NULL)
	{
		if (pMapping->gpioPin == pCur->gpioPin)
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



bool CSwitchActivation::RemoveAll()
{
	Clear();
	return Save();
}



SWITCHACTIVATIONITEM *CSwitchActivation::FindEntry(uint16_t gpioPin)
{
	SWITCHACTIVATIONITEM *pCur = m_itemsHead;
	
	while (pCur != NULL)
	{
		if (pCur->gpioPin == gpioPin)
			return pCur;
		pCur = pCur->pNext;
	}
	return NULL;
}



void CSwitchActivation::Clear()
{
	SWITCHACTIVATIONITEM *pCur = m_itemsHead;
	
	while (m_itemsHead != NULL)
	{
		pCur = m_itemsHead;
		m_itemsHead = pCur->pNext;
		free(pCur);
	}
}



void CSwitchActivation::SetTrace(bool on)
{
	m_bTracing = on;
}



bool CSwitchActivation::Load()
{
	bool success = false;
	int	 fileSize;
	void *pFile;
	
	Clear();
	pFile = CStorage::OpenFile(SWITCHACTIVATION_ENTRY_FILENAME, &fileSize);
	if (pFile != NULL)
	{
		SWITCHACTIVATIONITEM *pNew, *pLatest = NULL;
		
		while (fileSize > 0)
		{
			pNew = (SWITCHACTIVATIONITEM *) malloc(sizeof(SWITCHACTIVATIONITEM));
			
			int bytesRead = CStorage::ReadFile(pFile, pNew, sizeof(SWITCHACTIVATIONITEM));
			if (bytesRead != sizeof(SWITCHACTIVATIONITEM))
				goto done;
			pNew->pNext = NULL;
			
			SWITCHACTIVATION_GPIOBUTTON_MAPPING *pMapping = MapPhysicalPin(pNew->gpioPin);
			if (pMapping == NULL)
				pNew->pszPin = NULL;
			else
				pNew->pszPin = pMapping->pszPinName;
			pNew->triggerTime = 0;	
			
			// Register the button with GPIO 
			EPXPlatform_GPIO_ButtonClickConfigure(pNew->gpioPin);			
			
			if (m_itemsHead == NULL)
				m_itemsHead = pLatest = pNew;
			else
			{
				pLatest->pNext = pNew;
				pLatest = pNew;
			}
			fileSize -=	sizeof(SWITCHACTIVATIONITEM);
		}		
		success = true;
done:		
		CStorage::Close(pFile);
	}
	return success;
}



bool CSwitchActivation::Save()
{
	bool success = false;
	
	if (m_itemsHead == NULL)
	{		
		CStorage::DeleteFile(SWITCHACTIVATION_ENTRY_FILENAME);
		success = true;
	}
	else
	{
		SWITCHACTIVATIONITEM *pCur = m_itemsHead;
		void *pFile;
				
		pFile = CStorage::CreateFile(SWITCHACTIVATION_ENTRY_FILENAME);
		if (pFile != NULL)
		{
			while (pCur != NULL)
			{				
				int bytesWritten = CStorage::WriteFile(pFile, pCur, sizeof(SWITCHACTIVATIONITEM));
				if (bytesWritten != sizeof(SWITCHACTIVATIONITEM))
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



void CSwitchActivation::SystemGPIOEventHandler(void *pinstance, uint8_t event, uint16_t pin, uint16_t value)
{
	CSwitchActivation *pthis = (CSwitchActivation *) pinstance;	
	
	if (event == EPXGPIO_BUTTON_PUSHED)
		pthis->m_queuedButtonPin = pin;
}



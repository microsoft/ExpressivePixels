// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <string.h>
#include "EPXVariant.h"
#include "EPXApp.h"
#include "CBLEBeaconActivation.h"

EPX_OPTIMIZEFORDEBUGGING_ON

CBeaconActivation::CBeaconActivation()
{
	m_itemsHead = NULL;
}



CBeaconActivation::~CBeaconActivation()
{
	Clear();	
}



bool CBeaconActivation::AddEntry(char *pszHost, char *pszActivationBit, char *pszAnimationName)
{
	BEACONACTIVATIONITEM *pCur = NULL, *pNew;
	
	pCur = FindEntry(pszHost, pszActivationBit);
	if (pCur != NULL)
	{
		strcpy(pCur->szAnimationName, pszAnimationName);		
		return Save();
	}
	
	pNew = (BEACONACTIVATIONITEM *) malloc(sizeof(BEACONACTIVATIONITEM));
	if (pNew == NULL)
		return false;
	memset(pNew, 0x00, sizeof(BEACONACTIVATIONITEM));
	strcpy(pNew->szBeaconHostName, pszHost);
	pNew->m_beaconActivationBit = atoi(pszActivationBit);
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



bool CBeaconActivation::RemoveEntry(char *pszHost, char *pszActivationBit)
{
	BEACONACTIVATIONITEM *pCur = m_itemsHead, *pDelete, *pLast;
	
	while (pCur != NULL)
	{
		if (stricmp(pCur->szBeaconHostName, pszHost) == 0 && pCur->m_beaconActivationBit == atoi(pszActivationBit))
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



bool CBeaconActivation::RemoveAll()
{
	Clear();
	return Save();
}



BEACONACTIVATIONITEM *CBeaconActivation::FindEntry(char *pszHost, char *pszActivationBit)
{
	return FindEntry(pszHost, atoi(pszActivationBit));
}



BEACONACTIVATIONITEM *CBeaconActivation::FindEntry(char *pszHost, uint8_t activationBit)
{
	BEACONACTIVATIONITEM *pCur = m_itemsHead;
	
	while (pCur != NULL)
	{
		if (stricmp(pCur->szBeaconHostName, pszHost) == 0 && (pCur->m_beaconActivationBit & activationBit) != 0)
			return pCur;
		pCur = pCur->pNext;
	}
	return NULL;
}




void CBeaconActivation::Clear()
{
	BEACONACTIVATIONITEM *pCur = m_itemsHead;
	
	while (m_itemsHead != NULL)
	{
		pCur = m_itemsHead;
		m_itemsHead = pCur->pNext;
		free(pCur);
	}
}



bool CBeaconActivation::Load()
{
	bool success = false;
	int	 fileSize;
	void *pFile;
	
	Clear();
	pFile = CStorage::OpenFile(BEACONACTIVATION_ENTRY_FILENAME, &fileSize);
	if (pFile != NULL)
	{
		BEACONACTIVATIONITEM *pNew, *pLatest = NULL;
		
		while (fileSize > 0)
		{
			pNew = (BEACONACTIVATIONITEM *) malloc(sizeof(BEACONACTIVATIONITEM));
			
			int bytesRead = CStorage::ReadFile(pFile, pNew, sizeof(BEACONACTIVATIONITEM));
			if (bytesRead != sizeof(BEACONACTIVATIONITEM))
				goto done;
			pNew->pNext = NULL;
			
			if (m_itemsHead == NULL)
				m_itemsHead = pLatest = pNew;
			else
			{
				pLatest->pNext = pNew;
				pLatest = pNew;
			}
			fileSize -=	sizeof(BEACONACTIVATIONITEM);
		}		
		success = true;
done:		
		CStorage::Close(pFile);
	}
	return success;
}



bool CBeaconActivation::Save()
{
	bool success = false;
	
	if (m_itemsHead == NULL)
	{		
		CStorage::DeleteFile(BEACONACTIVATION_ENTRY_FILENAME);
		success = true;
	}
	else
	{
		BEACONACTIVATIONITEM *pCur = m_itemsHead;
		void *pFile;
				
		pFile = CStorage::CreateFile(BEACONACTIVATION_ENTRY_FILENAME);
		if (pFile != NULL)
		{
			while (pCur != NULL)
			{				
				int bytesWritten = CStorage::WriteFile(pFile, pCur, sizeof(BEACONACTIVATIONITEM));
				if (bytesWritten != sizeof(BEACONACTIVATIONITEM))
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

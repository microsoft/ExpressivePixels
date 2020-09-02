// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <string.h>
#include "EPXString.h"
#include "EPXPlatform_Settings.h"
#include "EPXPlatform_CStorage.h"



void CSettings::Initialize()
{
	CStorage::CreateFolder((char *) SETTINGS_FOLDER);
}



void CSettings::ClearAll()
{
	
}



uint16_t CSettings::Write(const char *pszSettingsKey, void *pBuffer, uint16_t cb)
{
	uint16_t bytesWritten = 0;
	
	EPXString filename = EPXString((const char *) SETTINGS_FOLDER) + "/" + pszSettingsKey; 
	void *pFile = CStorage::CreateFile(filename.c_str());
	if (pFile != NULL)
	{
		bytesWritten = CStorage::WriteFile(pFile, pBuffer, cb);
		CStorage::Close(pFile);
	}
	return bytesWritten;
}



uint16_t CSettings::Read(const char *pszSettingsKey, void *pBuffer, uint16_t cb)
{
	uint16_t bytesRead = 0;
	
	EPXString filename = EPXString((const char *) SETTINGS_FOLDER) + "/" + pszSettingsKey; 	
	memset(pBuffer, 0x00, cb);	
	if (CStorage::FileExists(filename.c_str()))
	{
		void *pFile = CStorage::OpenFile(filename.c_str());
		if (pFile != NULL)
		{
			bytesRead = CStorage::ReadFile(pFile, pBuffer, cb);
			CStorage::Close(pFile);
		}
	}
	return bytesRead;
}



bool CSettings::WriteString(const char *pszSettingsKey, char *pszValue, uint16_t valueLength)
{
	bool success = false;
	uint16_t bytesWritten = 0;

	EPXString filename = EPXString((const char *) SETTINGS_FOLDER) + "/" + pszSettingsKey; 
	void *pFile = CStorage::CreateFile(filename.c_str());
	if (pFile != NULL)
	{
		uint16_t length = valueLength == 0 ? strlen(pszValue) : valueLength;		
		bytesWritten = CStorage::WriteFile(pFile, &length, sizeof(length));
		if (bytesWritten == sizeof(length))
		{
			bytesWritten = CStorage::WriteFile(pFile, pszValue, length);	
			if (bytesWritten == length)
				success = true;
		}
		CStorage::Close(pFile);
	}
	return success;
}



bool CSettings::ReadString(const char *pszSettingsKey, char *pszValue, uint16_t bufferLength)
{
	bool success = false;
	uint16_t bytesRead = 0;

	EPXString filename = EPXString((const char *) SETTINGS_FOLDER) + "/" + pszSettingsKey; 
	memset(pszValue, 0x00, bufferLength);	
	
	if (CStorage::FileExists(filename.c_str()))
	{
		void *pFile = CStorage::OpenFile(filename.c_str());
		if (pFile != NULL)
		{
			uint16_t length = strlen(pszValue);
		
			bytesRead = CStorage::ReadFile(pFile, &length, sizeof(length));
			if (bytesRead == sizeof(length))
			{
				uint16_t bytesToRead = epxmin(length, bufferLength - 1);
						
				bytesRead = CStorage::ReadFile(pFile, pszValue, bytesToRead);
				if (bytesRead == bytesToRead)
				{
					pszValue[bytesRead] = 0x00;
					success = true;
				}
			}
			CStorage::Close(pFile);
		}
	}
	return success;
}



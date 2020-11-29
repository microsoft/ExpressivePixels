// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"

#define SETTINGS_FOLDER		F("/SETTINGS")

class CSettings
{
public:
	static void		Initialize();

	static bool		WriteString(const char *pszSettingsKey, char *pszValue, uint16_t valueLength = 0);
	static bool		ReadString(const char *pszSettingsKey, char *pszValue, uint16_t bufferLength);
	static uint16_t Read(const char *pszSettingsKey, void *pBuffer, uint16_t cb);
	static uint16_t Write(const char *pszSettingsKey, void *pBuffer, uint16_t cb);
	static void		ClearAll();
};



// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.


#pragma once

typedef struct _PERSISTED_SEQUENCE_LIST
{
	char *pszFilename;
	char *pszGUID;
	char *pszName;
	uint16_t size;
	uint32_t utcTimestamp;
	struct _PERSISTED_SEQUENCE_LIST *pNext;
} PERSISTED_SEQUENCE_LIST, *PPERSISTED_SEQUENCE_LIST;



// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementing Bluetooth beacon triggering
 **/

#pragma once
#include <stdint.h>
#include "EPXPlatform_BLE.h"

#define BEACONACTIVATION_ENTRY_FILENAME		"BeaconActivations.dat" // Storage file for persisted activations


#ifdef __cplusplus

// Beacon activation classe
class CBeaconActivation
{
public:
	CBeaconActivation();
	~CBeaconActivation();

	bool AddEntry(char *pszHost, char *pszActivationBit, char *pszAnimationName);
	bool Load();	
	bool RemoveAll();
	bool RemoveEntry(char *pszHost, char *pszActivationBit);
	BEACONACTIVATIONITEM **EntriesReference() { return &m_itemsHead; }
	BEACONACTIVATIONITEM *FindEntry(char *pszHost, char *pszActivationBit);
	BEACONACTIVATIONITEM *FindEntry(char *pszHost, uint8_t activationBit);
	BEACONACTIVATIONITEM *FirstEntry() { return m_itemsHead; }
	void Clear();
	
private:
	bool Save();


	BEACONACTIVATIONITEM		*m_itemsHead; // Linked list of beacon activations
};
#endif

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementing Bluetooth connectivity capability
 **/
#pragma once
#include "EPXPlatform_CByteQueue.h"
#include "CSerialChannelBase.h"
#include "CBLEBeaconActivation.h"
#include "EPXPlatform_BLE.h"
#include "EPXGlobal.h"

#define DEVICE_NAME_MAX_SIZE  18 // Max device name length



// Bluetooth capability class
class CBLEChannel : public CSerialChannelBase
{
public:
	CBLEChannel();
	bool Initialize(char *pszDefaultBLEName);
	CByteQueue *SerialCache() { return &m_BLESerialCache; }
	char *GetDeviceName() { return m_BLEDeviceName; }
	char *GetRealizedDeviceName();
	uint32_t GetLastBeaconReceived();
	void Disconnect();
	void SetAppInstance(void *pInst) { m_pAppInstance = pInst; }	
	void SetBeaconActivation(bool on);
	void SetBeaconActivationEntries(BEACONACTIVATIONITEM ** ppBeaconActivationEntries);	
	void SetDeviceName(char *pszName);
	void SetManufacturerPayload(uint8_t * pPayload, uint8_t cbPayload);
	void Start();
	void UpdateAdvertisingDeviceData();

	/**** CSerialChannelBase ****/
	char *channelName() { return (char *) "BLE"; }
	int	available() { return m_BLESerialCache.available(); }
	int	read() { return m_BLESerialCache.pop(); }
	size_t write(void *pvPayload, uint16_t cb, bool altChannel);
	
	void SetPowerStateChangedHandler(PFN_EPX_POWERSTATE_CHANGED pfnEPXPowerStateChanged) { m_pfnEPXPowerStateChanged = pfnEPXPowerStateChanged; }
	void SetCommunicationsReadyHandler(PFN_EPX_COMMUNICATION_READY pfnEPXCommunicationReady) { m_pfnEPXCommunicationReady = pfnEPXCommunicationReady; }
	void SetConnectionStateChangedHandler(PFN_EPX_CONNECTIONSTATE_CHANGED pfnEPXConnectionStateChanged) { m_pfnEPXConnectionStateChanged = pfnEPXConnectionStateChanged; }			
	void SetBeaconReceivedHandler(PFN_EPX_BEACONRECEIVED pfnEPXBeaconReceived) { m_pfnEPXBeaconReceived = pfnEPXBeaconReceived; }
	
	
	
private:
	static void BLE_CommunicationReady(void *pinstance, bool altChannel);
	static void BLE_ConnectionStageChanged(void *pinstance, bool connected);
	static void BLE_ByteReceived(void *pinstance, bool altChannel, uint8_t data);
	static void BLE_BeaconReceived(void *pinstance, char *pszHost, uint8_t beaconData);
	
	CByteQueue						m_BLESerialCache;					// Queue of incoming received bytes
	char							m_BLEDeviceName[BLEMAX_DEVICENAME];	// Device name 
	void							*m_pAppInstance;					// Reference to host application class instance
	PFN_EPX_POWERSTATE_CHANGED		m_pfnEPXPowerStateChanged;			// Callback function for Power State Change
	PFN_EPX_COMMUNICATION_READY		m_pfnEPXCommunicationReady;			// Callback function for Communication Ready event
	PFN_EPX_CONNECTIONSTATE_CHANGED	m_pfnEPXConnectionStateChanged;		// Callback function for BLE Connection State Change
	PFN_EPX_BEACONRECEIVED			m_pfnEPXBeaconReceived;				// Callback function for BLE Beacon activation
};

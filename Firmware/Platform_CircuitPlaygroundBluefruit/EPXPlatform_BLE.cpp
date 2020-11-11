// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <stdint.h>
#include <string.h>

#include <bluefruit.h>
#include "EPXApp.h"
#include "EPXPlatform_BLE.h"
#include "EPXPlatform_Runtime.h"
#include "EPXVariant.h"

EPX_OPTIMIZEFORDEBUGGING_ON

#define BLE_CONN_HANDLE_INVALID 			0xFFFF  // Invalid Connection Handle.

BLEDis										g_bledis;
BLEUart										g_bleuart;

static uint8_t								g_beaconData = 0;
static uint32_t								g_lastBeaconReceived = 0;
static uint8_t								g_emptyBeaconHostAddr[32] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static BEACONACTIVATIONITEM					**g_ppBeaconActivationEntries = NULL;

static char									g_szBLEDeviceName[BLEMAX_DEVICENAME] = "";
static char									*g_pszDEFAULT_BLE_NAME = NULL;
char										g_szRealizedDeviceName[BLEMAX_DEVICENAME] = "";
static uint8_t								*g_manufacturerPayload = NULL;
static uint8_t								g_cbManufacturerPayloadLength = 0;
static uint16_t 							g_conn_handle = BLE_CONN_HANDLE_INVALID;

void										*g_hostInstance = NULL;
PFN_EPXPLATFORM_BLE_CONNECTIONSTATECHANGED	g_pfnConnectionStateChanged = NULL;
PFN_EPXPLATFORM_BLE_COMMUNICATIONREADY		g_pfnCommunicationReady = NULL;
PFN_EPXPLATFORM_BLE_BYTERECEIVED			g_pfnByteReceived = NULL;
PFN_EPXPLATFORM_BLE_BEACONRECEIVED			g_pfnBLEBeaconReceived = NULL;



// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{	
	// Get the reference to current connection
	g_conn_handle = conn_handle;
	BLEConnection* connection = Bluefruit.Connection(conn_handle);

	char central_name[32] = { 0 };
	connection->getPeerName(central_name, sizeof(central_name));

	DEBUGLOGLN("Connected to %s", central_name);
	(*g_pfnConnectionStateChanged)(g_hostInstance, true);

	if (g_pfnCommunicationReady != NULL)			
		(*g_pfnCommunicationReady)(g_hostInstance, false);
}



/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
	DEBUGLOGLN("BLE Disconnected");
	(*g_pfnConnectionStateChanged)(g_hostInstance, false);

	g_conn_handle = BLE_CONN_HANDLE_INVALID;
}
	


void EPXPlatform_BLE_UART_RXHandler(uint16_t conn_hdl)
{
	while (g_bleuart.available())
	{
		uint8_t ch;
		ch = (uint8_t) g_bleuart.read();
		(*g_pfnByteReceived)(g_hostInstance, ch, false);
	}
}




uint32_t EPXPlatform_BLE_GetLastBeaconReceived()
{
	return g_lastBeaconReceived;
}



void EPXPlatform_BLE_SetBeaconActivationEntries(BEACONACTIVATIONITEM ** ppBeaconActivationEntries)
{
	g_ppBeaconActivationEntries = ppBeaconActivationEntries;
}



void EPXPlatform_BLE_SetBeaconActivation(bool on)
{
#ifndef DISABLE_BLE_ADVERTISING
	if (on)
	{
		//scan_start();
		g_lastBeaconReceived = millis();
	}
	else
	{
		//scan_stop();
		g_lastBeaconReceived = 0;
		delay(250);  // Wait for all processing to stop
	}
#endif
}



void EPXPlatform_BLE_SetDeviceName(char *pszDeviceName)
{
	strcpy(g_szBLEDeviceName, pszDeviceName);
}

	

char *EPXPlatform_BLE_GetRealizedDeviceName()
{
	return g_szRealizedDeviceName;
}


	
void EPXPlatform_BLE_Initialize(void *pinstance, char *pszDEFAULT_BLE_NAME, PFN_EPXPLATFORM_BLE_CONNECTIONSTATECHANGED pfnConnectionStateChanged, PFN_EPXPLATFORM_BLE_COMMUNICATIONREADY pfnCommunicationReady, PFN_EPXPLATFORM_BLE_BYTERECEIVED pfnByteReceived)
{
	char szDeviceName[BLEMAX_DEVICENAME];

	g_hostInstance = pinstance;
	g_pszDEFAULT_BLE_NAME = pszDEFAULT_BLE_NAME;
	g_pfnConnectionStateChanged = pfnConnectionStateChanged;
	g_pfnCommunicationReady = pfnCommunicationReady;
	g_pfnByteReceived = pfnByteReceived;
	
	Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
	
	Bluefruit.begin();

	// Set the advertising name
	strncpy(szDeviceName, (char *)(g_szBLEDeviceName[0] != 0x00 ? g_szBLEDeviceName : g_pszDEFAULT_BLE_NAME), BLEMAX_DEVICENAME);
		
	// If the device hasn't been previously named then append the last two bytes of MAC address for somewhat uniqueness
	if(g_szBLEDeviceName[0] == 0x00)
	{
		const char *pszUniqueID = getMcuUniqueID();
		sprintf(&szDeviceName[strlen(szDeviceName)], " %s", &pszUniqueID[strlen(pszUniqueID) - 4]);
	}
	szDeviceName[BLEMAX_DEVICENAME - 1] = 0x00;
	strcpy(g_szRealizedDeviceName, szDeviceName);

	DEBUGLOGLN("BLEDevice Name %s", szDeviceName);
	Bluefruit.setName(szDeviceName);
	Bluefruit.setAppearance(1990);
	Bluefruit.Periph.setConnectCallback(connect_callback);
	Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
	
	// Configure and Start BLE Uart Service
	g_bleuart.setRxCallback(EPXPlatform_BLE_UART_RXHandler);
	// g_bleuart.setRxOverflowCallback(rx_overflow_callback_t fp)
	g_bleuart.begin();
	g_bledis.begin();
	
#ifdef BEACONACTIVATION
//	scan_init();
#endif
			
#ifndef DISABLE_BLE_ADVERTISING	
  // Set up and start advertising
  // Advertising packet
	Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
	Bluefruit.Advertising.addTxPower();
	Bluefruit.Advertising.addName();
	Bluefruit.Advertising.addAppearance(1990);

	Bluefruit.ScanResponse.addService(g_bleuart);
	Bluefruit.ScanResponse.addManufacturerData(g_manufacturerPayload, g_cbManufacturerPayloadLength);
	
	/* Start Advertising
	 * - Enable auto advertising if disconnected
	 * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
	 * - Timeout for fast mode is 30 seconds
	 * - Start(timeout) with timeout = 0 will advertise forever (until connected)
	 */
	Bluefruit.Advertising.restartOnDisconnect(true);
	Bluefruit.Advertising.setInterval(32, 244);     // in unit of 0.625 ms
	Bluefruit.Advertising.setFastTimeout(30);       // number of seconds in fast mode
#endif				
}



void EPXPlatform_BLE_Start()
{
#ifndef DISABLE_BLE_ADVERTISING	
	Bluefruit.Advertising.start(0);                 // 0 = Don't stop advertising after n seconds
#endif				
}



void EPXPlatform_BLE_Disconnect()
{
	if (g_conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		BLEConnection* connection = Bluefruit.Connection(g_conn_handle);
		connection->disconnect();
		g_conn_handle = BLE_CONN_HANDLE_INVALID;
	}
}



void EPXPlatform_BLE_SetBeaconReceivedHandler(PFN_EPXPLATFORM_BLE_BEACONRECEIVED pfnBLEBeaconReceived)
{
	g_pfnBLEBeaconReceived = pfnBLEBeaconReceived;
}



void EPXPlatform_BLE_SetManufacturerPayload(uint8_t *p, uint8_t cb)
{
	g_manufacturerPayload = p;
	g_cbManufacturerPayloadLength = cb;
}



void EPXPlatform_BLE_AdvertizingUpdate()
{
//	advertising_Update();
}



size_t EPXPlatform_BLE_SendBytes(void *pvPayload, uint16_t cb, bool altChannel)
{
	uint8_t *pPayload = (uint8_t *)pvPayload;
	size_t requestBytesToWrite = cb, bytesWritten = 0;

	g_bleuart.write(pPayload, requestBytesToWrite);
	return bytesWritten;
}


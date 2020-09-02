// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXVariant.h"
#include "EPXPlatform_Settings.h"
#include "EPXPlatform_USB.h"
#include "EPXPlatform_GPIO.h"
#include "EPXApp.h"

EPX_OPTIMIZEFORDEBUGGING_ON

CExpressivePixelsApp	g_ExpressivePixelsApp;
APPSETTINGS				g_appSettings;
uint8_t					g_emptyAESKey[EPX_AES_KEY_BYTE_SIZE] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

extern uint8_t bootAnimation[];


CExpressivePixelsApp::CExpressivePixelsApp() :	m_CAnimator(&m_CDisplayArray)
{
	m_pCActiveSerialChannel = NULL;
	m_triggerSources = NULL;
	memcpy(m_aesKey, g_emptyAESKey, sizeof(m_aesKey));
	memset(&g_appSettings, 0x00, sizeof(g_appSettings));
	memset(&m_animationPayloadStateMachine, 0x00, sizeof(m_animationPayloadStateMachine));
	memset(&m_StagedAnimation, 0x00, sizeof(m_StagedAnimation));

	m_pActiveBeaconActivation = NULL;
	m_pPendingBeaconActivationHost = NULL;
	m_bMIDIActivation = false;	
	m_bAuthenticated = false;
	m_pendingCommunicationReady = false;
	m_bAuthenticationPending = false;
	m_bBeaconActivation = false;
	m_bBeaconActivationAlwaysOn = false;
	m_bBeaconStartupActivation = false;
	m_lastBeaconData = 0;
	m_ledToggle = false;
	m_powerState = EPXAPP_POWERSTATE_NONE;
	m_triggerPowerMode = EPXAPP_TRIGGERPOWERMODE_OFF;
	m_FeatureMillis = 0;
	m_batteryLevelPct = 0;
	m_batteryLevelMV = 0;
	m_lastLEDTimer = 0;
	m_lastSampleBatteryTimer = 0;
	m_BootMillis = 0;
	m_lastClearBLEKeyButtonPushed = 0;
	m_lastAuthChallengeTime = 0;
	m_bRebootOnDisconnect = false;
	m_PendingFeatureButtonPushed = false;
	m_bAutoPlayOnUSBPower = false;
	m_bInvokedAutoPlay = false;
	m_bPowerOnState = false;
	m_renderMode = RENDERMODE_ANIMATE;
	m_connectedChannel = EPXAPP_CONNECTIONCHANNEL_NONE;
	m_pendingPowerStateChange = EPXAPP_PENDINGPOWERSTATE_NONE;
		
	PayloadReset();
	m_PayloadStreamingJSONParser.setListener(this);
	
	m_CBLEChannel.SetAppInstance(this);		
	m_CBLEChannel.SetPowerStateChangedHandler(SystemPowerStateChanged);
	m_CBLEChannel.SetCommunicationsReadyHandler(SystemCommunicationReady);
	m_CBLEChannel.SetConnectionStateChangedHandler(SystemConnectionStateChanged);
	m_CBLEChannel.SetBeaconReceivedHandler(BLE_BeaconReceived);
	m_CBLEChannel.SetManufacturerPayload((uint8_t *) &m_manufacturerPayload, sizeof(m_manufacturerPayload));
	SetActiveSerialChannel(&m_CBLEChannel); // Set BLE channel as primary
	
	m_CUSBChannel.SetAppInstance(this);		
	m_CUSBChannel.SetPowerStateChangedHandler(SystemPowerStateChanged);
	m_CUSBChannel.SetConnectionStateChangedHandler(SystemConnectionStateChanged);
	m_CUSBChannel.SetCommunicationReadyHandler(SystemCommunicationReady);
	m_CUSBChannel.SetPurgeRequestHandler(USBQueue_PurgeRequestHandler);
}



bool CExpressivePixelsApp::AppInitialize(CLEDDriverBase *pLEDDriver, uint16_t *pArrayMatrix, uint16_t arrayWidth, uint16_t arrayHeight)
{
	// Configure hardware
	EPXPlatform_GPIO_Initialize(this, SystemGPIOEventHandler);	
	EPXPlatform_GPIO_PinConfigure(GPIO_PIN_FEATURE, EPXGPIO_INPUT);	
	EPXPlatform_GPIO_ButtonClickConfigure(GPIO_PIN_FEATURE);
	pinMode(GPIO_PIN_3V3_ACCEN, OUTPUT);
	if (GPIO_PIN_BOOSTER_ENABLE > 0)
		pinMode(GPIO_PIN_BOOSTER_ENABLE, OUTPUT);
	pinMode(GPIO_PIN_STATUSLED, OUTPUT);
	digitalWrite(GPIO_PIN_STATUSLED, HIGH);
	delay(250);
	digitalWrite(GPIO_PIN_STATUSLED, LOW);

	// Check state of Feature button at boot to do a 'default' system restore
	bool bFeatureButtonPressedOnBoot = !EPXPlatform_GPIO_PinRead(GPIO_PIN_FEATURE);
		
	// Configure display and array
	m_CDisplayTopology.SetMatrix(pArrayMatrix, arrayWidth, arrayHeight);
	m_CAnimator.SetTopology(&m_CDisplayTopology);
	m_CDisplayArray.Initialize(pLEDDriver, arrayWidth, arrayHeight, DISPLAYARRAY_POWERPIN);
	
	// Get remaining free memory for diagnostics
	m_bootUpRAM = freeRam();

	DataChannelPurge();
	DEBUGLOGLN("** BOOTUP RAM ** %s", EPXString(freeRam()).c_str());

#ifdef VARIANTCAPABILITY_BATTERY_MONITORING	
	// Initialize Battery Monitor
	m_BatteryMonitor.Initialize();
	SampleBattery();
#endif

	m_CAppStorage.Initialize(bFeatureButtonPressedOnBoot);
	m_CAppStorage.SetReadDirectFromFile(true);   // On by default
	CSettings::Initialize();

	DEBUGLOGLN("** INITIALIZED **");

	/**** Read persisted app settings ****/
	// Setting - load Bluetooth advertising name, and USB device descriptor name
	CSettings::ReadString((const char *) SETTINGSKEY_DEVICENAME, g_appSettings.szDeviceName, sizeof(g_appSettings.szDeviceName));
	m_CBLEChannel.SetDeviceName(g_appSettings.szDeviceName);
	
	// Setting - load AES Key
	CSettings::Read((const char *) SETTINGSKEY_AESKEY, m_aesKey, sizeof(m_aesKey));
		
	// Setting - load autoplay
	CSettings::Read((const char *) SETTINGSKEY_AUTOPLAYONUSBPOWER, &m_bAutoPlayOnUSBPower, sizeof(m_bAutoPlayOnUSBPower));
	DEBUGLOGLN("AUTOPLAYONUSBPOWER %s", m_bAutoPlayOnUSBPower ? "ON" : "OFF");
	
	// Setting - load last set brightness
	uint8_t bootBrightness;
	CSettings::Read((const char *) SETTINGSKEY_BRIGHTNESS, &bootBrightness, sizeof(bootBrightness));
	if (bootBrightness > 0)
		m_CDisplayArray.SetBrightness(bootBrightness);
	DEBUGLOGLN("BOOT BRIGHTNESS %s", EPXString((int) bootBrightness).c_str());
	
	// Setting - load display rotation
	int rotateBy = 0;
	CSettings::Read((const char *) SETTINGSKEY_ROTATEBY, &rotateBy, sizeof(rotateBy));	
	m_CAnimator.SetRotation(rotateBy);
	
	// Setting - load BeaconActivation enabled
	CSettings::Read((const char *) SETTINGSKEY_BEACONACTIVATION, &m_bBeaconStartupActivation, sizeof(m_bBeaconStartupActivation));
	DEBUGLOGLN("BEACONSTARTUPACTIVATION %s", m_bBeaconStartupActivation ? "ENABLED" : "DISABLED");

	// Setting - load BeaconActivation AlwaysOn
	CSettings::Read((const char *) SETTINGSKEY_BEACONACTIVATIONALWAYSON, &m_bBeaconActivationAlwaysOn, sizeof(m_bBeaconActivationAlwaysOn));	
	DEBUGLOGLN("BEACONSTARTUPACTIVATION ALWAYSON %s", m_bBeaconActivationAlwaysOn ? "ON" : "OFF");
	
	// Initialize and start USB Channel **BEFORE** the Bluetooth Channel
	m_CUSBChannel.Initialize();
	
	// Initialize and start Bluetooth Channel
	m_CBLEChannel.Initialize(g_szDEFAULT_BLE_NAME);
	m_CBLEChannel.SetBeaconActivationEntries(m_beaconActivation.EntriesReference());
	m_beaconActivation.Load();
	
	// Initialize crypto for auhentication
	EPXPlatform_Crypto_Initialize();
	
	// USB device name is based on realized BLE name (that may autogenerate from MAC address)
	m_CUSBChannel.SetDeviceName(m_CBLEChannel.GetRealizedDeviceName());
		
	// Initialize trigger sources
	RegisterTriggerSource(&m_CSwitchActivation);
	
	m_CBLEChannel.Start();
	
#ifdef EPXMIDI
	// Initialize MIDI
	digitalWrite(GPIO_PIN_3V3_ACCEN, HIGH);
	m_CMIDIActivation.Initialize();
	m_CMIDIActivation.SetAnimationActivationHandler(this, ActivateSequenceByNameEvent);
	m_CMIDIActivation.SetTraceEventHandler(this, TraceEventHandler);
	m_bMIDIActivation = true;
	m_bAutoPlayOnUSBPower = false;
#endif	
	
	DEBUGLOGLN("** WAITING RAM ** %s", EPXString(freeRam()).c_str());
	m_BootMillis = millis();

	m_pendingPowerStateChange = EPXAPP_PENDINGPOWERSTATE_BLE_OFF;

	// Register all configured buttons
	EPXPlatform_GPIO_ButtonClickFinalize();
	
	ChannelsInitialized();
	m_CUSBChannel.Activate();	
	return true;
}



bool CExpressivePixelsApp::IsAESKeySet()
{
	return memcmp(m_aesKey, g_emptyAESKey, sizeof(g_emptyAESKey)) != 0;	
}



void CExpressivePixelsApp::ClearAESKey()
{
	DEBUGLOGLN("ClearAESKey");	
	
	m_CAppStorage.Power(true);
	memcpy(m_aesKey, g_emptyAESKey, sizeof(m_aesKey));			
	CSettings::Write((const char *)SETTINGSKEY_AESKEY, m_aesKey, sizeof(m_aesKey));
	if (!m_bPowerOnState)
		m_CAppStorage.Power(false);
	
	int flashes = 8;
	while (flashes--)
	{		
		digitalWrite(GPIO_PIN_STATUSLED, (flashes % 2) == 0 ? HIGH : LOW);		
		delay(250);
	}
}



void CExpressivePixelsApp::Authenticate(char *pszChallengeResponseHex)
{
	DEBUGLOGLN("\tResponse received");
	m_lastAuthChallengeTime = 0;
	if (strlen(pszChallengeResponseHex) == EPX_NONCE_SIZE * 2)
	{
		int idx;
		uint8_t responseNONCE[EPX_NONCE_SIZE];
		
		idx = 0;
		while (*pszChallengeResponseHex != 0x00)
		{
			responseNONCE[idx++] = HexToByte(pszChallengeResponseHex, 2);
			pszChallengeResponseHex += 2;
		}
		
		uint8_t *pEncryptedLocalNONCE = EPXPlatform_Crypto_Encrypt(m_aesKey, m_currentNONCE, sizeof(m_currentNONCE));
		if (pEncryptedLocalNONCE != NULL)
		{
			if (memcmp(pEncryptedLocalNONCE, responseNONCE, EPX_NONCE_SIZE) == 0)		
			{
				DEBUGLOGLN("\tAuthentication SUCCEEDED - keys compare");				
				m_bAuthenticated = true;
			}
			else
			{
				DEBUGLOGLN("\tAuthentication FAILED - keys don't compare FAILED");			
			}
			free(pEncryptedLocalNONCE);
		}
		else
		{
			DEBUGLOGLN("\tDecryption FAILED");			
		}
	}
	
	// Authentication failed - force client disconnect
	if(!m_bAuthenticated)
	{
		m_CBLEChannel.Disconnect();
		DEBUGLOGLN("\tDISCONNECTED");			
	}
}



void CExpressivePixelsApp::RegisterTriggerSource(ITriggerSource *pTriggerSource)
{
	TRIGGERSOURCEITEM *pNewTriggerSource;

	pNewTriggerSource = (TRIGGERSOURCEITEM *) malloc(sizeof(TRIGGERSOURCEITEM));
	if (pNewTriggerSource != NULL)
	{
		memset(pNewTriggerSource, 0x00, sizeof(TRIGGERSOURCEITEM));
		pNewTriggerSource->pITriggerSource = pTriggerSource;
		
		// Initialize the trigger source
		pNewTriggerSource->pITriggerSource->Initialize();	
		pNewTriggerSource->pITriggerSource->SetAnimationActivationHandler(this, ActivateSequenceByNameEvent);
		pNewTriggerSource->pITriggerSource->SetTraceEventHandler(this, TraceEventHandler);

		// Update the list of sources
		if(m_triggerSources == NULL)
			m_triggerSources = pNewTriggerSource;
		else
		{
			// Advance to the end
			TRIGGERSOURCEITEM *pLastTriggerSource = m_triggerSources;			
			while (pLastTriggerSource->pNext != NULL)
				pLastTriggerSource = pLastTriggerSource->pNext;			
			pLastTriggerSource->pNext = pNewTriggerSource;
		}
	}	
}



void CExpressivePixelsApp::ChannelsInitialized()
{
	if (m_bBeaconStartupActivation)
	{
		m_bBeaconActivation = true;
		m_CBLEChannel.SetBeaconActivation(m_bBeaconActivation);
		PowerManage(true);
	}
	else if (m_bMIDIActivation)
		PowerManage(true);
	else
		PowerManage(false, true);
}



void CExpressivePixelsApp::UpdateAdvertisingData()
{
	m_manufacturerPayload.firmwareVersionHi = VERSION_MAJOR;
	m_manufacturerPayload.firmwareVersionLo = VERSION_MINOR;
	m_manufacturerPayload.batteryLevel = m_batteryLevelPct;
	m_CBLEChannel.UpdateAdvertisingDeviceData();
}



void CExpressivePixelsApp::SystemPowerStateChanged(void *pinstance, uint8_t state, bool set)
{
	CExpressivePixelsApp *pthis = (CExpressivePixelsApp *) pinstance;
	
	// Called on an interrupt so app's idle processing needs to pick this up to avoid race conditions
	if(state == EPXAPP_POWERSTATE_ON_BLE)
		pthis->m_pendingPowerStateChange = set ? EPXAPP_PENDINGPOWERSTATE_BLE_ON : EPXAPP_PENDINGPOWERSTATE_BLE_OFF;
	if (state == EPXAPP_POWERSTATE_ON_USB)
		pthis->m_pendingPowerStateChange = set ? EPXAPP_PENDINGPOWERSTATE_USB_ON : EPXAPP_PENDINGPOWERSTATE_USB_OFF;
}



void CExpressivePixelsApp::SystemCommunicationReady(void *pinstance)
{
	CExpressivePixelsApp *pthis = (CExpressivePixelsApp *) pinstance;
	pthis->m_pendingCommunicationReady = true;		
}



void CExpressivePixelsApp::SystemConnectionStateChanged(void *pinstance, uint8_t state, bool set)
{
	CExpressivePixelsApp *pthis = (CExpressivePixelsApp *) pinstance;
	
	// Called on an interrupt so app's idle processing needs to pick this up to avoid race conditions
	if(!set)
	{
		switch (state)
		{
		case EPXAPP_CONNECTIONCHANNEL_USB:
			pthis->m_pendingConnectionStateChange = EPXAPP_PENDINGCONNECTIONSTATE_DISCONNECTED_USB;
			break;
		
		case EPXAPP_CONNECTIONCHANNEL_BLE:
			pthis->m_pendingConnectionStateChange = EPXAPP_PENDINGCONNECTIONSTATE_DISCONNECTED_BLE;
			break;
			
		default:
			pthis->m_pendingConnectionStateChange = EPXAPP_PENDINGCONNECTIONSTATE_NONE;
			break;
		}
	}
	else
	{
		switch (state)
		{
		case EPXAPP_CONNECTIONCHANNEL_USB:
			// Switch primary communication channel 
			g_ExpressivePixelsApp.SetActiveSerialChannel(&pthis->m_CUSBChannel);
			pthis->m_pendingConnectionStateChange = EPXAPP_PENDINGCONNECTIONSTATE_CONNECTED_USB;
			break;
			
		case EPXAPP_CONNECTIONCHANNEL_BLE:
			g_ExpressivePixelsApp.SetActiveSerialChannel(&pthis->m_CBLEChannel);
			pthis->m_pendingConnectionStateChange = EPXAPP_PENDINGCONNECTIONSTATE_CONNECTED_BLE;
			break;
			
		default:
			pthis->m_pendingConnectionStateChange = EPXAPP_PENDINGCONNECTIONSTATE_NONE;
			break;
		}
	}
}



void CExpressivePixelsApp::SystemGPIOEventHandler(void *pinstance, uint8_t event, uint16_t pin, uint16_t value)
{
	CExpressivePixelsApp *pthis = (CExpressivePixelsApp *) pinstance;	
	
	if (event == EPXGPIO_BUTTON_PUSHED)
	{
		switch (pin)
		{
		case GPIO_PIN_FEATURE:
			pthis->m_lastClearBLEKeyButtonPushed = millis();
			break;
		}	
	}
	else if (event == EPXGPIO_BUTTON_RELEASED)
	{
		switch (pin)
		{
			case GPIO_PIN_FEATURE:
				if (pthis->m_lastClearBLEKeyButtonPushed != 0)
				{
					pthis->m_lastClearBLEKeyButtonPushed = 0;
					pthis->FeatureButtonPushed();
				}
				break;
		}	
	}
}



void CExpressivePixelsApp::TraceEventHandler(void *pinstance, char *pszTrace)
{
	CExpressivePixelsApp *pthis = (CExpressivePixelsApp *) pinstance;	
	EPXString response;
	
	response += JSON_OPENOBJECT;
	response += JSON_KEYVALUE_STRINGPAIR(JSON_TRACE, pszTrace);
	response += JSON_CLOSEOBJECT;
	pthis->DataChannelSendResponseJSON(response);
}



void CExpressivePixelsApp::USBQueue_PurgeRequestHandler(void *pinstance)
{
	CExpressivePixelsApp *pthis = (CExpressivePixelsApp *) pinstance;	
	pthis->AppProcess(true);	
}



bool CExpressivePixelsApp::PowerManage(bool on, bool force)
{
	if (on != m_bPowerOnState || force)
	{
		m_bPowerOnState = on;
		
		// Power control storage flash chip
		m_CAppStorage.Power(on);

		// Power control PWM for battery sampling
		m_BatteryMonitor.Power(on);
		
		if (on)
		{
			if (GPIO_PIN_BOOSTER_ENABLE > 0)
				digitalWrite(GPIO_PIN_BOOSTER_ENABLE, HIGH);
			delay(100);   	
#ifdef VARIANTCAPABILITY_BATTERY_MONITORING		
			SampleBattery();
#endif			
		}
		else
		{
			m_CDisplayArray.PowerOff();
			if (GPIO_PIN_BOOSTER_ENABLE > 0)
				digitalWrite(GPIO_PIN_BOOSTER_ENABLE, LOW);
			m_ledToggle = false;		
			digitalWrite(GPIO_PIN_STATUSLED, LOW);		
		}	
		return true;
	}
	return false;
}



void CExpressivePixelsApp::BLE_BeaconReceived(void *pinstance, char *pszHost, uint8_t beaconData)
{
	CExpressivePixelsApp *pthis = (CExpressivePixelsApp *) pinstance;	
	
	pthis->m_pPendingBeaconActivationHost = pszHost;
	pthis->m_pendingBeaconData = beaconData;
}



bool CExpressivePixelsApp::ProcessConnectionStateChange()
{
	bool bNewConnectionEstablished = false;
	
	// Process a change in connection state
	switch(m_pendingConnectionStateChange)
	{
	case EPXAPP_PENDINGCONNECTIONSTATE_DISCONNECTED_USB:
		if (m_connectedChannel == EPXAPP_CONNECTIONCHANNEL_BLE)
			break;
		// Fall through
	case EPXAPP_PENDINGCONNECTIONSTATE_DISCONNECTED_BLE:
		if (m_connectedChannel != EPXAPP_CONNECTIONCHANNEL_NONE)
		{
			m_CAnimator.Clear();
			if (!m_bBeaconActivation && !m_bMIDIActivation)
				PowerManage(false);
			m_lastAuthChallengeTime = 0;
			m_connectedChannel = EPXAPP_CONNECTIONCHANNEL_NONE;
			m_bAuthenticated = false;
			m_pendingCommunicationReady = false;
			if (m_bRebootOnDisconnect)
				EPXPlatform_Runtime_Reboot(REBOOTTYPE_NONE);
		}
		break;
		
	case EPXAPP_PENDINGCONNECTIONSTATE_CONNECTED_BLE:
	case EPXAPP_PENDINGCONNECTIONSTATE_CONNECTED_USB:
		m_bAuthenticated = false;
		PowerManage(true);
		AutoPlayClear();
		m_CAnimator.Clear();
		uint8_t newConnectedChannel = m_pendingConnectionStateChange == EPXAPP_PENDINGCONNECTIONSTATE_CONNECTED_BLE ? EPXAPP_CONNECTIONCHANNEL_BLE : EPXAPP_CONNECTIONCHANNEL_USB;
		if (m_connectedChannel != newConnectedChannel)
			bNewConnectionEstablished = true;
		m_connectedChannel = newConnectedChannel;	
		
		if (m_connectedChannel == EPXAPP_CONNECTIONCHANNEL_USB)
			SystemCommunicationReady(this);
		break;
	}
	m_pendingConnectionStateChange = EPXAPP_PENDINGCONNECTIONSTATE_NONE;
	return bNewConnectionEstablished;
}



void CExpressivePixelsApp::ProcessPowerStateChange()
{
	// Process a change in power state
	switch(m_pendingPowerStateChange)
	{		
	case EPXAPP_PENDINGPOWERSTATE_BLE_ON:
	case EPXAPP_PENDINGPOWERSTATE_USB_ON:
		DEBUGLOGLN("POWERSTATECHANGED %s", m_pendingPowerStateChange == EPXAPP_PENDINGPOWERSTATE_BLE_ON ? "BLE ON" : "USB ON");		
		if (m_pendingPowerStateChange == EPXAPP_PENDINGPOWERSTATE_BLE_ON)		
			m_powerState |= EPXAPP_POWERSTATE_ON_BLE;
		else if (m_pendingPowerStateChange == EPXAPP_PENDINGPOWERSTATE_USB_ON)
		{
			if (m_bAutoPlayOnUSBPower && m_connectedChannel == EPXAPP_CONNECTIONCHANNEL_NONE)
				ToggleInvokedAutoPlay(true);
			m_powerState |= EPXAPP_POWERSTATE_ON_USB;
		}
		break;		
		
	case EPXAPP_PENDINGPOWERSTATE_BLE_OFF:
	case EPXAPP_PENDINGPOWERSTATE_USB_OFF:
		{
			DEBUGLOGLN("POWERSTATECHANGED %s", m_pendingPowerStateChange == EPXAPP_PENDINGPOWERSTATE_BLE_OFF ? "BLE OFF" : "USB OFF");
			if (m_pendingPowerStateChange == EPXAPP_PENDINGPOWERSTATE_BLE_OFF)		
				m_powerState &= ~EPXAPP_POWERSTATE_ON_BLE;
			else if (m_pendingPowerStateChange == EPXAPP_PENDINGPOWERSTATE_USB_OFF && m_connectedChannel != EPXAPP_CONNECTIONCHANNEL_BLE)
			{
				if (m_bAutoPlayOnUSBPower)	
					ToggleInvokedAutoPlay(false);
				m_powerState &= ~EPXAPP_POWERSTATE_ON_USB;
			}
			break;
		}
	}
	m_pendingPowerStateChange = EPXAPP_PENDINGPOWERSTATE_NONE;		
}



void CExpressivePixelsApp::ProcessPowerState()
{	
	// Power down if no channel is active
	if(m_powerState == EPXAPP_POWERSTATE_NONE && m_renderMode == RENDERMODE_ANIMATE && m_lastClearBLEKeyButtonPushed == 0 &&
		!m_bInvokedAutoPlay && !m_bBeaconActivation && !m_bMIDIActivation && m_triggerPowerMode == EPXAPP_TRIGGERPOWERMODE_OFF)
	{
		EPXPlatform_GPIO_PinWrite(GPIO_PIN_STATUSLED, EPXGPIO_LOW);		
		EPXPlatform_Runtime_MCUSleep();
	}
}



void CExpressivePixelsApp::ProcessCommunicationReady()
{
	if (m_connectedChannel == EPXAPP_CONNECTIONCHANNEL_BLE && IsAESKeySet())
	{
		DEBUGLOGLN("Authenticating...");				
		DEBUGLOGLN("\tKey set");
				
		// Generate a new nonce for every new connection
		EPXPlatform_Crypto_GenerateNONCE(m_currentNONCE);
				
		// And send the authentication challenge to the connecting client
		SendAuthenticationChallenge();			
		m_lastAuthChallengeTime = millis();
	}
	else
	{
		DEBUGLOGLN("Authentication not set");
				
		// Send an empty authentication challenge to the connecting client, also for USB communications
		memset(m_currentNONCE, 0x00, sizeof(m_currentNONCE));
		SendAuthenticationChallenge();
		m_bAuthenticated = true;
	}	
	m_pendingCommunicationReady = false;
}



void CExpressivePixelsApp::ProcessIndicators()
{
	// Toggle system awake LED
	if(millisPassed(m_lastLEDTimer) > 500)
	{
		m_ledToggle = !m_ledToggle;		
		digitalWrite(GPIO_PIN_STATUSLED, m_ledToggle ? HIGH : LOW);		
		m_lastLEDTimer = millis();	
	}
}



void CExpressivePixelsApp::ProcessTriggers()
{
	TRIGGERSOURCEITEM	*pTriggerSourceItem = m_triggerSources;
		
	while (pTriggerSourceItem != NULL)
	{		
		pTriggerSourceItem->pITriggerSource->Process();
		pTriggerSourceItem = pTriggerSourceItem->pNext;
	}
}



void CExpressivePixelsApp::ProcessRendering()
{
	switch (m_renderMode)
	{
	case RENDERMODE_ANIMATE:
		if (m_CAnimator.Process())
		{
			bool restarted;
				
			DEBUGLOGLN("Animation COMPLETE ");
			if (m_CAppStorage.IsAutoPlayActive() && m_CAppStorage.AutoPlaylistIterate(false, &restarted))
			{
				DEBUGLOGLN("Next AUTOPLAY %s", m_CAppStorage.AutoPlaylistCurrent()->pszGUID);
				ActivateSequenceFromID(m_CAppStorage.AutoPlaylistCurrent()->pszGUID);					
			}
			else
			{
				if (m_bBeaconActivation && m_lastBeaconData > 0 && m_pActiveBeaconActivation != NULL && (m_pActiveBeaconActivation->m_beaconActivationBit  & m_lastBeaconData) != 0)
					ActivateSequenceFromName(m_pActiveBeaconActivation->szAnimationName);
				else if (m_bInvokedAutoPlay)
				{					
					AutoPlayClear();					
					if (m_connectedChannel == EPXAPP_CONNECTIONCHANNEL_NONE)
						PowerManage(false);
					m_bInvokedAutoPlay = false;					
				}
				else if (m_triggerPowerMode == EPXAPP_TRIGGERPOWERMODE_SINGLEANIMATION)
				{
					if (m_connectedChannel == EPXAPP_CONNECTIONCHANNEL_NONE)
						PowerManage(false);
					m_triggerPowerMode = EPXAPP_TRIGGERPOWERMODE_OFF;					
				}
				else
					m_CAnimator.Clear();
			}
		}
		break;

	case RENDERMODE_CHASER:
		if (millis() - m_FeatureMillis > 250)
		{
			m_CDisplayArray.Chase(false);
			m_FeatureMillis = millis();
		}
		break;
		
	case RENDERMODE_RAINBOWCYCLE:
		m_CDisplayArray.RainbowCycle(10); 
		break;
	}
	
	// Perform display power management
	m_CDisplayArray.Process();
}



void CExpressivePixelsApp::ProcessBattery()
{
#ifdef VARIANTCAPABILITY_BATTERY_MONITORING	
	// Sample battery periodically - 60s
	if(millisPassed(m_lastSampleBatteryTimer) > 60000)
		SampleBattery();
#endif
}



EPXString CExpressivePixelsApp::GetDeviceResponseInfo()
{
	EPXString response, capabilityResponse;
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED("FIRMWARE", (EPXString((int) VERSION_MAJOR) + "." + EPXString((int) VERSION_MINOR)));	
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED("MODEL", EPX_DEVICEMODEL);			
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED("DEVICENAME", m_CBLEChannel.GetRealizedDeviceName());			
	response += JSON_KEYVALUE_VALUEPAIR_CONTINUED("BRIGHTNESS", EPXString(m_CDisplayArray.GetBrightness()));
	response += JSON_KEYVALUE_VALUEPAIR_CONTINUED("DISPLAYWIDTH", EPXString((int) m_CDisplayArray.Width()));
	response += JSON_KEYVALUE_VALUEPAIR_CONTINUED("DISPLAYHEIGHT", EPXString((int) m_CDisplayArray.Height()));
	response += JSON_KEYVALUE_VALUEPAIR_CONTINUED("AUTOPLAY", EPXString((int) m_bAutoPlayOnUSBPower));
	response += GetBatteryInfoResponse();
	response += JSON_CONTINUATION;

	// Specify device capabilities
#ifdef VARIANTCAPABILITY_SECURITY	
		response += JSON_KEYOBJECTOPENARRAY("CAPABILITIES");
			capabilityResponse += JSON_QUOTEDVALUE("SECURITY");
#endif	
	
#ifdef VARIANTCAPABILITY_STORAGE	
			if (capabilityResponse.length() > 0)
				capabilityResponse += JSON_CONTINUATION;
			capabilityResponse += JSON_QUOTEDVALUE("STORAGE");
#endif
	
#ifdef VARIANTCAPABILITY_PREVIEW	
			if (capabilityResponse.length() > 0)
				capabilityResponse += JSON_CONTINUATION;
			capabilityResponse += JSON_QUOTEDVALUE("PREVIEW");
#endif	
	
#ifdef VARIANTCAPABILITY_DFU
	if (capabilityResponse.length() > 0)
		capabilityResponse += JSON_CONTINUATION;
	capabilityResponse += JSON_QUOTEDVALUE("DFU");
#endif		
		response += capabilityResponse;
		response += JSON_CLOSEARRAY;			
	return response;
}



void CExpressivePixelsApp::AppProcess(bool disableUSBProcessing) 
{
	bool bNewConnectionEstablished = false;
		
	if (!disableUSBProcessing)
		m_CUSBChannel.Process();
		
	// Process connection state changes
	if (m_pendingConnectionStateChange != EPXAPP_PENDINGCONNECTIONSTATE_NONE)
		bNewConnectionEstablished = ProcessConnectionStateChange();
	
	// Process power state changes
	if (m_pendingPowerStateChange != EPXAPP_PENDINGPOWERSTATE_NONE)
		ProcessPowerStateChange();

	if (m_pendingCommunicationReady)
		ProcessCommunicationReady();
	
	// Process appropriate power state actions - eg: go to sleep, keep on sleeping
	ProcessPowerState();
	
	if (!m_bAuthenticated && m_lastAuthChallengeTime != 0 && millisPassed(m_lastAuthChallengeTime) > EPX_AUTHENTICATION_RESPONSE_TIMEOUT_MS)
	{
		m_lastAuthChallengeTime = 0;
		DEBUGLOGLN("\tAuthentication response TIMEOUT");			
		m_CBLEChannel.Disconnect();
	}
	
	// If not beacon has been received from any hosts for the expiry interval then shutdown BeaconActivation and power down
	if(m_bBeaconActivation && !m_bBeaconActivationAlwaysOn && m_connectedChannel == EPXAPP_CONNECTIONCHANNEL_NONE && millisPassed(m_CBLEChannel.GetLastBeaconReceived()) > BEACONACTIVATION_DEACTIVATION_EXPIRY_MS)
	{
		DEBUGLOGLN("ACTIVATION BEACON EXPIRED");
		m_CBLEChannel.SetBeaconActivation(false);
		m_bBeaconActivation = false;
		PowerManage(false);
		return;
	}							
							
	// Process buttons
	if(m_lastClearBLEKeyButtonPushed > 0 && millisPassed(m_lastClearBLEKeyButtonPushed) > EPX_CLEARBLEKEY_BUTTONHOLD_MS)
	{
		ClearAESKey();
		m_lastClearBLEKeyButtonPushed = 0;
	}	
	else if (m_PendingFeatureButtonPushed)
	{
		ToggleInvokedAutoPlay(!m_bInvokedAutoPlay);
		m_PendingFeatureButtonPushed = false;
	}

	// Process LED state
	ProcessIndicators();

	// Process animation triggers
	ProcessTriggers();

	// Only process if a connection is active	
	if(m_connectedChannel != EPXAPP_CONNECTIONCHANNEL_NONE || m_renderMode == RENDERMODE_RAINBOWCYCLE || m_bInvokedAutoPlay || m_bBeaconActivation || m_bMIDIActivation || m_triggerPowerMode != EPXAPP_TRIGGERPOWERMODE_OFF)
	{		
		if (m_pPendingBeaconActivationHost != NULL)
		{			
			if (m_bBeaconActivation)
			{
				AutoPlayClear();
				
				BEACONACTIVATIONITEM *pActivation = m_beaconActivation.FindEntry(m_pPendingBeaconActivationHost, m_pendingBeaconData);
				if (pActivation != NULL)
					ActivateSequenceFromName(pActivation->szAnimationName);
				m_lastBeaconData = m_pendingBeaconData;
				m_pActiveBeaconActivation = pActivation;
			}	
			m_pendingBeaconData = 0;
			m_pPendingBeaconActivationHost = NULL;
		}		

		// Process battery
		ProcessBattery();

		// Process payload channel reading from any channel 
		ProtocolProcess();
		
		// Process rendering 
		ProcessRendering();
		
#ifdef EPXMIDI
		// Process MIDI
		m_CMIDIActivation.Process();
#endif	
	}
}



void CExpressivePixelsApp::ActivateSequenceByNameEvent(void *pinstance, char *pszName, uint8_t triggerPowerMode)
{
	CExpressivePixelsApp *pthis = (CExpressivePixelsApp *) pinstance;	
	
	// Powerup the device
	pthis->m_triggerPowerMode = triggerPowerMode;
	pthis->PowerManage(true);	
	pthis->ActivateSequenceFromName(pszName);
}



#ifdef VARIANTCAPABILITY_BATTERY_MONITORING
void CExpressivePixelsApp::SampleBattery()
{
	m_lastSampleBatteryTimer = millis();
	m_BatteryMonitor.SingleSampleRequest();	
	m_BatteryMonitor.GetInfo(&m_batteryLevelMV, &m_batteryLevelPct);
	if(m_batteryLevelMV == 0)
	{
		m_batteryLevelMV = 5.0;
		m_batteryLevelPct = 100;
	}
	UpdateAdvertisingData();	
}



EPXString CExpressivePixelsApp::GetBatteryInfoResponse()
{
	EPXString response;
	uint16_t mv;
	
	m_BatteryMonitor.GetInfo(&mv, NULL);
	
	DEBUGLOGLN("BatteryInfo %d %d", mv, m_batteryLevelPct);
	
	response += JSON_KEYOBJECTOPEN((char *) JSON_BATTERY);
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED("MILLIV", EPXString(mv));
	response += JSON_KEYVALUE_STRINGPAIR("PCT", EPXString(m_batteryLevelPct));
	response += JSON_CLOSEOBJECT;
	return response;
}
#endif



void CExpressivePixelsApp::SetBrightness(uint8_t brightness)
{
	m_CDisplayArray.SetBrightness(brightness);

	// Persist as setting
	CSettings::Write((const char *) SETTINGSKEY_BRIGHTNESS, &brightness, sizeof(brightness));
}



void CExpressivePixelsApp::ExecuteTTYCommand()
{

	bool responseSuccess = false, syntax = false;
	char *pszPostToken;
	EPXString response, innerResponse;

	DEBUGLOGLN("CExpressivePixelsApp::ExecuteTTYCommand");
	DEBUGLOGLN("%s", m_TTYValue.c_str());
	
	if (stricmp(m_TTYValue.c_str(), (const char *)TTYTOKEN_HELP) == 0 || stricmp(m_TTYValue.c_str(), (const char *)TTYTOKEN_HELPQ) == 0)
	{
		EPXString body;

		body += "SYNTAX\n";
		body += "- HELP\n";
		body += "- AUTOPLAY <ON | OFF> (toggle ON/OFF auto sequence play on USB power)\n";
		body += "- CHASER <ON | OFF> (toggle 0-N led lightup)\n";
		body += "- DEEPSLEEP (places device into deep sleep)\n";
		body += "- DELETE <ID> (delete stored sequence by unique ID)\n";
		body += "- FORMAT YES (erases flash memory followed by reboot)\n";
		body += "- INFO (displays device information)\n";
		body += "- LIST (lists stored sequences)\n";
		body += "- NAME <DeviceName> (Name of device)\n";
		body += "- REBOOT <UF2 | OTA> (Reboot device, optionally into bootloader mode)\n";
		body += "- ROTATE <90 | 180 | 270> (Rotate image on display)\n";

		body += "\n>>> Trigger Systems <<<\n";
		body += "-----------------------\n";
		body += "- BEACONACTIVATION ADD <HOSTDEVICENAME> <ACTIVATION BIT> <ANIMATION NAME> (Add beacon activation entry)\n";
		body += "- BEACONACTIVATION ALWAYSON <ON | OFF> (disable beacon activation expiry sleep)\n";
		body += "- BEACONACTIVATION DELETE <HOSTDEVICENAME> <ACTIVATION BIT> (Remove beacon activation entry)\n";
		body += "- BEACONACTIVATION DELETEALL (Remove all beacon activation entries)\n";
		body += "- BEACONACTIVATION LIST (List beacon activations)\n";
		body += "- BEACONACTIVATION <ENABLED | DISABLED> (Enable or disable Beacon activation)\n";
		body += "- MIDIACTIVATION ADD <CHANNEL> <NOTE> <ANIMATION NAME> (Add MIDI activation entry)\n";
		body += "- MIDIACTIVATION DELETE <CHANNEL> <NOTE> (Remove MIDI activation entry)\n";
		body += "- MIDIACTIVATION DELETEALL (Remove all MIDI activation entries)\n";
		body += "- MIDIACTIVATION LIST (List MIDI activations)\n";
		body += "- MIDIACTIVATION TRACE <ON | OFF> (enable note tracings)\n";
		
		// Add the trigger subsystem help syntax
		TRIGGERSOURCEITEM	*pTriggerSource = m_triggerSources;
		while (pTriggerSource != NULL)
		{
			body += pTriggerSource->pITriggerSource->AppendConsoleHelpSyntax();
			pTriggerSource = pTriggerSource->pNext;
		}
		
		innerResponse += JSON_KEYVALUE_STRINGPAIR("info", body);
		responseSuccess = true;
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_CLEARSETTINGS, &pszPostToken))
	{
		CSettings::ClearAll();
		responseSuccess = true;
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_CHASER, &pszPostToken))
	{
		DEBUGLOGLN("TTY CHASER");
		if (stricmp(pszPostToken, (const char *) TOGGLE_ON) == 0)
		{
			AutoPlayClear();
			m_CAnimator.Clear();
			m_renderMode = RENDERMODE_CHASER;
			m_CDisplayArray.Chase(true);
		}
		else
		{
			m_CDisplayArray.Clear();
			m_CDisplayArray.Show();
			m_renderMode = RENDERMODE_ANIMATE;
		}
		responseSuccess = true;
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_AUTOPLAY, &pszPostToken))
	{
		char *pszAction = ExtractNextParameter(pszPostToken, &pszPostToken);
		DEBUGLOGLN("WRITING SETTINGSKEY_AUTOPLAYONUSBPOWER %s", pszPostToken);
		
		m_bAutoPlayOnUSBPower = stricmp(pszAction, (const char *)TOGGLE_ON) == 0;
		CSettings::Write((const char *)SETTINGSKEY_AUTOPLAYONUSBPOWER, &m_bAutoPlayOnUSBPower, sizeof(m_bAutoPlayOnUSBPower));
		responseSuccess = true;
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_NAME, &pszPostToken))
	{
		DEBUGLOGLN("DEVICENAME %s", pszPostToken);
		CSettings::WriteString((const char *)SETTINGSKEY_DEVICENAME, pszPostToken, strlen(pszPostToken));
		responseSuccess = true;
	}
	else if (!stricmp(m_TTYValue.c_str(), (const char *)TTYTOKEN_INFO))
	{
		innerResponse += JSON_KEYOBJECTOPEN("info");	
		innerResponse += GetDeviceResponseInfo();
		innerResponse += JSON_CLOSEOBJECT;
		responseSuccess = true;
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_BEACONACTIVATION, &pszPostToken))
	{
		char *pszAction = ExtractNextParameter(pszPostToken, &pszPostToken);
		if (pszAction != NULL)
		{
			if (!stricmp(pszAction, (const char *)TTYTOKEN_ADD))
			{
				char *pszHost = ExtractNextParameter(pszPostToken, &pszPostToken);
				if (pszHost != NULL)
				{
					char *pszActivationBit = ExtractNextParameter(pszPostToken, &pszPostToken);	
					if (pszActivationBit != NULL)
					{
						if (pszPostToken != NULL)						
						{
							syntax = true;									
							m_CBLEChannel.SetBeaconActivation(false);							
							responseSuccess = m_beaconActivation.AddEntry(pszHost, pszActivationBit, pszPostToken);
							m_CBLEChannel.SetBeaconActivation(m_bBeaconActivation);
						}
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
				char *pszHost = ExtractNextParameter(pszPostToken, &pszPostToken);
				if (pszHost != NULL)
				{
					char *pszActivationBit = ExtractNextParameter(pszPostToken, &pszPostToken);	
					if (pszActivationBit != NULL)	
					{
						syntax = true;						
						m_CBLEChannel.SetBeaconActivation(false);
						responseSuccess = m_beaconActivation.RemoveEntry(pszHost, pszActivationBit);
						m_CBLEChannel.SetBeaconActivation(m_bBeaconActivation);
					}
				}				
				if (!syntax)
				{
					innerResponse += JSON_KEYOBJECTOPEN("info");	
					innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Syntax error");
					innerResponse += JSON_CLOSEOBJECT;
				}
			}
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_DELETEALL))
			{
				m_CBLEChannel.SetBeaconActivation(false);
				responseSuccess = m_beaconActivation.RemoveAll();
				m_CBLEChannel.SetBeaconActivation(m_bBeaconActivation);
			}
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_ALWAYSON))
			{
				char *pszOn = ExtractNextParameter(pszPostToken, &pszPostToken);
				m_bBeaconActivationAlwaysOn = stricmp(pszOn, (const char *)TOGGLE_ON) == 0;
				CSettings::Write((const char *) SETTINGSKEY_BEACONACTIVATIONALWAYSON, &m_bBeaconActivationAlwaysOn, sizeof(m_bBeaconActivationAlwaysOn));
				responseSuccess = true;
			}			
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_LIST))
			{
				BEACONACTIVATIONITEM *pCur = m_beaconActivation.FirstEntry();
				
				innerResponse += JSON_KEYOBJECTOPEN("info");				
				innerResponse += JSON_KEYOBJECTOPENARRAY("Activations");				
				while (pCur != NULL)
				{
					char szBit[8];
					
					itoa(pCur->m_beaconActivationBit, szBit, 10);					
					innerResponse += JSON_QUOTEDVALUE(EPXString(pCur->szBeaconHostName) + " " + szBit + " " + pCur->szAnimationName);
					if (pCur->pNext != NULL)
						innerResponse += EPXString(JSON_CONTINUATION) + JSON_LINESEPARATOR;
					pCur = pCur->pNext;
				}				
				innerResponse += JSON_CLOSEARRAY;
				innerResponse += JSON_CLOSEOBJECT;
				responseSuccess = true;
			}
			else if (stricmp(pszAction, (const char *) TTYTOKEN_ENABLED) == 0 || stricmp(pszAction, (const char *) TTYTOKEN_DISABLED) == 0)
			{			
				DEBUGLOGLN("BEACON ACTIVATION %s", pszAction);
				m_bBeaconActivation = stricmp(pszAction, (const char *)TTYTOKEN_ENABLED) == 0;
				CSettings::Write((const char *) SETTINGSKEY_BEACONACTIVATION, &m_bBeaconActivation, sizeof(m_bBeaconActivation));
				m_CBLEChannel.SetBeaconActivation(m_bBeaconActivation);
				responseSuccess = true;
			}
			else
			{
				innerResponse += JSON_KEYOBJECTOPEN("info");	
				innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Unknown BEACONACTIVATION action");
				innerResponse += JSON_CLOSEOBJECT;
			}
		}
	}
#ifdef EPXMIDI	
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_MIDIACTIVATION, &pszPostToken))
	{
		char *pszAction = ExtractNextParameter(pszPostToken, &pszPostToken);
		if (pszAction != NULL)
		{
			if (!stricmp(pszAction, (const char *)TTYTOKEN_ADD))
			{
				char *pszChannel = ExtractNextParameter(pszPostToken, &pszPostToken);
				if (pszChannel != NULL)
				{
					char *pszNote = ExtractNextParameter(pszPostToken, &pszPostToken);	
					if (pszNote != NULL)
					{
						if (pszPostToken != NULL)						
						{
							syntax = true;									
							responseSuccess = m_CMIDIActivation.AddEntry(pszChannel, pszNote, pszPostToken);
						}
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
				char *pszHost = ExtractNextParameter(pszPostToken, &pszPostToken);
				if (pszHost != NULL)
				{
					char *pszActivationBit = ExtractNextParameter(pszPostToken, &pszPostToken);	
					if (pszActivationBit != NULL)	
					{
						syntax = true;						
						responseSuccess = m_CMIDIActivation.RemoveEntry(pszHost, pszActivationBit);
					}
				}				
				if (!syntax)
				{
					innerResponse += JSON_KEYOBJECTOPEN("info");	
					innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Syntax error");
					innerResponse += JSON_CLOSEOBJECT;
				}
			}
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_DELETEALL))
				responseSuccess = m_CMIDIActivation.RemoveAll();			
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_TRACE))
			{
				syntax = true;
				char *pszOn = ExtractNextParameter(pszPostToken, &pszPostToken);
				m_CMIDIActivation.SetTrace(stricmp(pszOn, (const char *)TOGGLE_ON) == 0);
				responseSuccess = true;
			}			
			else if (!stricmp(pszAction, (const char *)TTYTOKEN_LIST))
			{
				MIDIACTIVATIONITEM *pCur = m_CMIDIActivation.FirstEntry();
				
				innerResponse += JSON_KEYOBJECTOPEN("info");				
				innerResponse += JSON_KEYOBJECTOPENARRAY("Activations");				
				while (pCur != NULL)
				{
					char szChannel[8], szNote[8];
					
					itoa(pCur->channel, szChannel, 10);					
					itoa(pCur->note, szNote, 10);					
					innerResponse += JSON_QUOTEDVALUE(EPXString("Channel ") + szChannel + ", Note " + szNote + " -> " + pCur->szAnimationName);
					if (pCur->pNext != NULL)
						innerResponse += EPXString(JSON_CONTINUATION) + JSON_LINESEPARATOR;
					pCur = pCur->pNext;
				}				
				innerResponse += EPXString(JSON_CLOSEARRAY);
				innerResponse += JSON_CLOSEOBJECT;
				responseSuccess = true;
			}
			else
			{
				innerResponse += JSON_KEYOBJECTOPEN("info");	
				innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Unknown MIDIACTIVATION action");
				innerResponse += JSON_CLOSEOBJECT;
			}
		}
	}
#endif	
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_LIST, &pszPostToken))
	{
		pszPostToken = MoveToTokenParameter(m_TTYValue.c_str(), (const char *)TTYTOKEN_LIST);
		m_CAppStorage.EnumerateSequencesJSON(response, m_PayloadActiveTransactionID, (char *) "info");
		DataChannelSendResponseJSON(response);
		return;
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_FORMAT, &pszPostToken))
	{
		pszPostToken = MoveToTokenParameter(m_TTYValue.c_str(), (const char *)TTYTOKEN_FORMAT);
		if (stricmp(pszPostToken, (const char *)TOGGLE_YES) == 0)
		{
			m_CAppStorage.Format();
			delay(1000);
			EPXPlatform_Runtime_Reboot(REBOOTTYPE_NONE);
			responseSuccess = true;
		}
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_DELETE, &pszPostToken))
	{
		AutoPlayClear();
		m_CAnimator.Clear();
		if (m_CAppStorage.SequenceDelete(pszPostToken))
			responseSuccess = true;
		else
		{
			innerResponse += JSON_KEYOBJECTOPEN("info");	
			innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Delete file by ID not found");
			innerResponse += JSON_CLOSEOBJECT;
		}
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_DEEPSLEEP, &pszPostToken))
	{
		PowerManage(false);		
		EPXPlatform_Runtime_MCUDeepSleep();
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_REBOOT, &pszPostToken))
	{
		uint8_t rebootType = REBOOTTYPE_NONE;
		
		DEBUGLOGLN("REBOOT Type %s", pszPostToken);
		responseSuccess = true;
		if (stricmp(pszPostToken, (const char *) "DISCONNECT") == 0)
		{
			m_bRebootOnDisconnect = true;	
			goto sendResponse;
		}
		else if (stricmp(pszPostToken, (const char *) "UF2") == 0)
			rebootType = REBOOTTYPE_UF2;
		else if (stricmp(pszPostToken, (const char *) "OTA") == 0)
			rebootType = REBOOTTYPE_OTA;
		else if (stricmp(pszPostToken, (const char *) "USB") == 0)
			rebootType = REBOOTTYPE_SERIAL;
		EPXPlatform_Runtime_Reboot(rebootType);
	}
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_ROTATE, &pszPostToken))
	{
		int rotateBy = -1;
		
		// Rotation only supported on square displays
		if(m_CDisplayArray.Width() != m_CDisplayArray.Height())
		{
			innerResponse += JSON_KEYOBJECTOPEN("info");	
			innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Rotation only supported on square displays");
			innerResponse += JSON_CLOSEOBJECT;
		}		
		else
		{
			DEBUGLOGLN("ROTATE IMAGE BY %s degrees", pszPostToken);
			if (stricmp(pszPostToken, (const char *) "0") == 0)
				rotateBy = 0;
			else if (stricmp(pszPostToken, (const char *) "90") == 0)
				rotateBy = 90;
			else if (stricmp(pszPostToken, (const char *) "180") == 0)
				rotateBy = 180;
			else if (stricmp(pszPostToken, (const char *) "270") == 0)
				rotateBy = 270;
			if (rotateBy == 0 || rotateBy == 90 || rotateBy == 180 || rotateBy == 270)
			{			
				m_CAnimator.SetRotation(rotateBy);
				CSettings::Write((const char *) SETTINGSKEY_ROTATEBY, &rotateBy, sizeof(rotateBy));
				responseSuccess = true;
			}	
			else
			{
				innerResponse += JSON_KEYOBJECTOPEN("info");	
				innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Unsupported rotation angle");
				innerResponse += JSON_CLOSEOBJECT;
			}
		}
	}	
	else if (CompareToken(m_TTYValue.c_str(), (const char *)TTYTOKEN_PLAY, &pszPostToken))
	{
		EXPRESSIVEPIXEL_SEQUENCE sequence;

		AutoPlayClear();
		m_CDisplayArray.Clear();

		if (m_CAppStorage.SequenceRead(pszPostToken, &sequence))
			m_CAnimator.Activate(&sequence);
		responseSuccess = true;
	}
	else
	{
		bool bProcessed = false;
		TRIGGERSOURCEITEM	*pTriggerSourceItem = m_triggerSources;
		
		while (!bProcessed && pTriggerSourceItem != NULL)
		{
			bProcessed = pTriggerSourceItem->pITriggerSource->ConsoleCommandProcess(m_TTYValue.c_str(), &responseSuccess, innerResponse);
			pTriggerSourceItem = pTriggerSourceItem->pNext;
		}		

		if (!bProcessed)
		{	
			DEBUGLOGLN("UNKNOWN CONSOLE COMMAND");
			innerResponse += JSON_KEYOBJECTOPEN("info");	
			innerResponse += JSON_KEYVALUE_STRINGPAIR("details", "Unknown command");
			innerResponse += JSON_CLOSEOBJECT;
		}
	}
	
sendResponse:	
	response += JSON_OPENOBJECT;
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED(JSON_STATUS, responseSuccess ? JSON_SUCCESS : JSON_ERROR);
	response += JSON_KEYVALUE_STRINGPAIR_CONTINUED((const char *) JSONKEY_TRANSACTIONID, EPXString(m_PayloadActiveTransactionID));
	if (innerResponse.length() > 0)
		response += innerResponse;
	response += JSON_CLOSEOBJECT;
	//Serial.println(response.c_str());

	DataChannelSendResponseJSON(response);
}




void CExpressivePixelsApp::ToggleInvokedAutoPlay(bool bAutoPlay)
{
	m_bInvokedAutoPlay = bAutoPlay;	
	if (bAutoPlay)
	{		
		PowerManage(true);
		m_CAnimator.Clear();
		AutoPlayStart();
	}
	else	
	{
		AutoPlayClear();
		PowerManage(false);		
	}
}



void CExpressivePixelsApp::AutoPlayStart()
{
	if (m_CAppStorage.EnumerateAutoPlaylist())
	{
		DEBUGLOGLN("Starting AUTOPLAY %s", m_CAppStorage.AutoPlaylistCurrent()->pszGUID);
		m_CAnimator.SetDisableInfiniteLooping(true);
		ActivateSequenceFromID(m_CAppStorage.AutoPlaylistCurrent()->pszGUID);
	}
}



void CExpressivePixelsApp::AutoPlayClear()
{
	m_CAnimator.Clear();
	m_CAnimator.SetDisableInfiniteLooping(false);
	m_CAppStorage.AutoPlaylistClear();
	m_bInvokedAutoPlay = false;
}



void CExpressivePixelsApp::ActivateSequenceFromID(char *pszGUID)
{
	EXPRESSIVEPIXEL_SEQUENCE sequence;

	// Firstly check if an animation is running and it is infinite looper, then interpret as stopping the animation
	if(m_CAnimator.IsActiveAnimationInfinite(pszGUID))
		m_CAnimator.Clear();
	else if (m_CAppStorage.SequenceRead(pszGUID, &sequence))
		m_CAnimator.Activate(&sequence);
}



void CExpressivePixelsApp::ActivateSequenceFromName(char *pszName)
{
	char *pszID = m_CAppStorage.SequenceIDFromName(pszName);
	if (pszID != NULL)
		ActivateSequenceFromID(pszID);
}



bool CExpressivePixelsApp::CompareToken(const char *psz, const char *pszToken, char **ppszPostToken)
{
	bool isToken;
	*ppszPostToken = NULL;

	isToken = strncasecmp(psz, pszToken, strlen(pszToken)) == 0;
	if (isToken && ppszPostToken != NULL)
		*ppszPostToken = MoveToTokenParameter(psz, pszToken);
	return isToken;
}



char *CExpressivePixelsApp::MoveToTokenParameter(const char *pszTokenHead, const char *pszToken)
{
	char *pszPostToken = (char *) pszTokenHead + strlen(pszToken);
	while (*pszPostToken == ' ' || *pszPostToken == '\t') pszPostToken++;  // Trim
	return pszPostToken;
}



char *CExpressivePixelsApp::ExtractNextParameter(const char *pszTokenHead, char **ppszToken)
{
	char *pszCur = (char *) pszTokenHead;
	
	if (pszTokenHead == NULL)
		return NULL;
	
	*ppszToken = 0x00;
	while (*pszCur != ' ' && *pszCur != '\t' && *pszCur != 0x00) 
		pszCur++;
	if (*pszCur != 0x00)
		*ppszToken = pszCur + 1;	
	*pszCur = 0x00;
	return (char *) pszTokenHead;
}



extern "C"
{
	void EPXApp_Initialize(void *pLEDDriver, uint16_t *pArrayMatrix, uint16_t arrayWidth, uint16_t arrayHeight)
	{
		g_ExpressivePixelsApp.AppInitialize((CLEDDriverBase *) pLEDDriver, pArrayMatrix, arrayWidth, arrayHeight);
	}
	

	
	void EPXApp_Process()
	{
		g_ExpressivePixelsApp.AppProcess();
	}
	
	
	
}




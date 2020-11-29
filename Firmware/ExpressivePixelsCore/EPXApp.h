// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"
#include "EPXGlobal.h"


// Application specific Bluetooth payload
typedef struct
{
	uint8_t firmwareVersionHi;
	uint8_t firmwareVersionLo;
	uint8_t batteryLevel;	
	uint8_t alignment[1];
} EPX_BLE_MANUFACTURER_PAYLOAD;



// Payload structure for device/animation sync
typedef struct
{
	EPX_GUID    guid;
	uint32_t	utcTimeStamp;
	uint8_t		width;
	uint8_t		height;
	uint8_t		alignment[2];
} EPX_THUMBNAIL_HEADER;



#ifdef __cplusplus
#include "EPXString.h"
#include "EPXPlatform_CBatteryMonitor.h"
#include "EPXPlatform_Crypto.h"
#include "CLEDDriverBase.h"
#include "EPXVariant.h"
#include "EPXPlatform_Runtime.h"
#include "EPXApp_ChannelConstants.h"
#include "JsonStreamingParser.h"
#include "CAnimationManager.h"
#include "CBLEChannel.h"
#include "CUSBChannel.h"
#include "CBLEBeaconActivation.h"
#include "CDisplayTopology.h"
#include "EPXApp_CStorage.h"
#include "EPXApp_MIDI.h"
#include "EPXApp_Trigger_Switch.h"
#include "CSerialChannelBase.h"
#include "CCOBSProtocol.h"
#include "CStringProtocol.h"
	

#define	VERSION_MAJOR						1
#define VERSION_MINOR						46

#define BEACONACTIVATION_DEACTIVATION_EXPIRY_MS	30000
#define EPX_AUTHENTICATION_RESPONSE_TIMEOUT_MS  6000
#define EPX_CLEARBLEKEY_BUTTONHOLD_MS			3000

#define MAX_COMMAND_VALUE					64

#define COMMAND_CONSOLE_COMMAND		F("CONSOLE_COMMAND")
#define JSONKEY_CONSOLECOMMAND		F("ConsoleCommand")

#define JSON_BATTERY				F("BATTERY")

#define SETTINGSKEY_AESKEY			F("AESKEY")
#define SETTINGSKEY_AUTOPLAYONUSBPOWER	F("AUTOPLAYONUSBPOWER")
#define SETTINGSKEY_BEACONHOST		F("BEACONHOST")
#define SETTINGSKEY_BEACONACTIVATION F("BEACONACTIVATION")
#define SETTINGSKEY_BEACONACTIVATIONANIMATIONID F("BEACONANIMATIONID")
#define SETTINGSKEY_BEACONACTIVATIONBIT F("BEACONACTIVATIONBIT")
#define SETTINGSKEY_BEACONACTIVATIONALWAYSON F("BEACONACTIVATIONALWAYSON")
#define SETTINGSKEY_DEVICENAME		F("DEVICENAME")
#define SETTINGSKEY_BRIGHTNESS		F("BRIGHTNESS")
#define SETTINGSKEY_READDIRECTFROMFILE F("READDIRECTFROMFILE")
#define SETTINGSKEY_ROTATEBY		F("ROTATEBY")

#define TTYTOKEN_HELP				F("HELP")
#define TTYTOKEN_HELPQ				F("?")
#define TTYTOKEN_CHASER				F("CHASER")
#define TTYTOKEN_AUTOPLAY			F("AUTOPLAY")
#define TTYTOKEN_DIRECTFROMFILE		F("DIRECTFROMFILE")
#define TTYTOKEN_BRIGHTNESS			F("BRIGHTNESS")
#define TTYTOKEN_NAME				F("NAME")
#define TTYTOKEN_MIDIACTIVATION		F("MIDIACTIVATION")
#define TTYTOKEN_BEACONACTIVATION	F("BEACONACTIVATION")
#define TTYTOKEN_BEACONHOST			F("BEACONHOST")
#define TTYTOKEN_LIST				F("LIST")
#define TTYTOKEN_INFO				F("INFO")
#define TTYTOKEN_FORMAT				F("FORMAT")
#define TTYTOKEN_CLEARSETTINGS		F("CLEARSETTINGS")
#define TTYTOKEN_DELETE				F("DELETE")
#define TTYTOKEN_DELETEALL			F("DELETEALL")
#define TTYTOKEN_REBOOT				F("REBOOT")
#define TTYTOKEN_DEEPSLEEP			F("DEEPSLEEP")
#define TTYTOKEN_PLAY				F("PLAY")
#define TTYTOKEN_ADD				F("ADD")
#define TTYTOKEN_ALWAYSON			F("ALWAYSON")
#define TTYTOKEN_TRACE				F("TRACE")
#define TTYTOKEN_ENABLED			F("ENABLED")
#define TTYTOKEN_DISABLED			F("DISABLED")
#define TTYTOKEN_LOOP				F("LOOP")
#define TTYTOKEN_ROTATE				F("ROTATE")


#define TOGGLE_ON					F("ON")
#define TOGGLE_OFF 					F("OFF")
#define TOGGLE_YES					F("YES")
#define TOGGLE_NO					F("NO")



enum AppPayloadCommands
{
	PAYLOADCOMMAND_TTY = PAYLOADCOMMAND_KNOWN_MAX,
};


enum RenderModes
{
	RENDERMODE_ANIMATE,
	RENDERMODE_CHASER,
	RENDERMODE_RAINBOWCYCLE
};

enum TriggerPowerMode
{
	EPXAPP_TRIGGERPOWERMODE_OFF = 0,
	EPXAPP_TRIGGERPOWERMODE_SINGLEANIMATION = 1,
	EPXAPP_TRIGGERPOWERMODE_ALWAYSON = 2
};



enum PendingPowerStateChanges
{
	EPXAPP_PENDINGPOWERSTATE_NONE,
	EPXAPP_PENDINGPOWERSTATE_BLE_ON,
	EPXAPP_PENDINGPOWERSTATE_BLE_OFF,
	EPXAPP_PENDINGPOWERSTATE_USB_ON,
	EPXAPP_PENDINGPOWERSTATE_USB_OFF
};



enum PendingConnectionStateChanges : uint8_t
{
	EPXAPP_PENDINGCONNECTIONSTATE_NONE,
	EPXAPP_PENDINGCONNECTIONSTATE_DISCONNECTED_BLE,
	EPXAPP_PENDINGCONNECTIONSTATE_DISCONNECTED_USB,
	EPXAPP_PENDINGCONNECTIONSTATE_CONNECTED_BLE,
	EPXAPP_PENDINGCONNECTIONSTATE_CONNECTED_USB
};




class CExpressivePixelsApp : public JsonListener, public CCOBSProtocol, public CStringProtocol
{
public:
	CExpressivePixelsApp();

	void ToggleInvokedAutoPlay(bool bAutoPlay);
	void AutoPlayStart();
	void ActivateSequenceFromID(char *pszGUID);
	void ActivateSequenceFromName(char *pszName);
	bool AppInitialize(CLEDDriverBase *pLEDDriver, uint16_t *pArrayMatrix, uint16_t arrayWidth, uint16_t arrayHeight);
	void AppProcess(bool disableUSBProcessing = false);
	void ChannelsInitialized();
	void DataChannelPurge();	
	void DataChannelSendResponse(void *pData, uint16_t payloadLength);
	void DataChannelSendResponseJSON(EPXString &response);
	void FeatureButtonPushed() { m_PendingFeatureButtonPushed = true; }
	void PowerDisplay(bool bOn);
	void RegisterTriggerSource(ITriggerSource *pTriggerSource);
	bool PowerManage(bool on, bool force = false);

	int BootupRAM() { return m_bootUpRAM; }
	EPXString GetBatteryInfoResponse();

	static bool CompareToken(const char *psz, const char *pszToken, char **ppszPostToken = NULL);
	static char *ExtractNextParameter(const char *pszTokenHead, char **pszToken);
	static char *MoveToTokenParameter(const char *pszTokenHead, const char *pszToken);

private:
	bool IsAESKeySet();
	bool ProcessConnectionStateChange();
	EPXString GetDeviceResponseInfo();
	void Authenticate(char *pszChallengeResponseHex);
	void ExecuteTTYCommand();	
	void AutoPlayClear();
	void ClearAESKey();
	void ProcessBattery();
	void ProcessCommunicationReady();
	void ProcessIndicators();
	void ProcessPowerState();
	void ProcessPowerStateChange();
	void ProcessRendering();
	void ProcessTriggers();
	void SampleBattery();
	void SetBrightness(uint8_t brightness);
	void UpdateAdvertisingData();
	
	static void		TraceEventHandler(void *pinstance, char *pszTrace);
	static void		SystemGPIOEventHandler(void *pinstance, uint8_t event, uint16_t pin, uint16_t value);
	static void		SystemPowerStateChanged(void *pinstance, uint8_t state, bool set);
	static void		SystemCommunicationReady(void *pinstance, bool altChannel);
	static void		SystemConnectionStateChanged(void *pinstance, uint8_t state, bool set);
	static void		BLE_BeaconReceived(void *pinstance, char *pszHost, uint8_t beaconData);
	static void		USBQueue_PurgeRequestHandler(void *pinstance);
	static void		ActivateSequenceByNameEvent(void *pinstance, char *pszName, uint8_t triggerPowerMode);

#ifdef EPXMIDI
	CMIDIActivation		m_CMIDIActivation;
#endif	
	CSwitchActivation	m_CSwitchActivation;
	
	BatteryMonitor      m_BatteryMonitor;
	CAnimationManager	m_CAnimator;
	CBeaconActivation	m_beaconActivation;
	CBLEChannel			m_CBLEChannel;
	CUSBChannel			m_CUSBChannel;
	CDisplayArray		m_CDisplayArray;
	CDisplayTopology	m_CDisplayTopology;
	CExpressivePixelsStorage m_CAppStorage;
	char				m_szDeviceName[32];	// Device name for Bluetooth/USB identification
	EPX_BLE_MANUFACTURER_PAYLOAD m_manufacturerPayload;
	TRIGGERSOURCEITEM	*m_triggerSources;
	uint8_t				m_batteryLevelPct;
	uint16_t			m_batteryLevelMV;
	
	bool				m_bAuthenticated;
	bool				m_bAuthenticationPending;
	bool				m_bBeaconStartupActivation;	
	bool				m_bBeaconActivation;	
	bool				m_bMIDIActivation;
	bool				m_pendingCommunicationReady;
	bool				m_bBeaconActivationAlwaysOn;
	bool				m_ledToggle;
	bool				m_bRebootOnDisconnect;
	bool				m_bPowerOnState;
	bool				m_bAutoPlayOnUSBPower;
	bool				m_bInvokedAutoPlay;
	bool				m_bBLEConnected;
	bool				m_PendingFeatureButtonPushed;
	bool				m_bAlternateBLEChannel;
	int					m_bootUpRAM;
	int					m_renderMode;
	char				*m_pPendingBeaconActivationHost;
	BEACONACTIVATIONITEM *m_pActiveBeaconActivation;
	EPXString			m_TTYValue;
	unsigned long		m_FeatureMillis;
	unsigned long		m_BootMillis;
	uint8_t				m_aesKey[EPX_AES_KEY_BYTE_SIZE]; // For this simple app can keep the aes key in memory
	uint8_t				m_currentNONCE[EPX_NONCE_SIZE];
	uint8_t				m_pendingBeaconData;
	uint8_t				m_lastBeaconData;
	uint8_t				m_connectedChannel;
	uint8_t				m_powerState;
	uint8_t				m_pendingPowerStateChange;
	uint8_t				m_triggerPowerMode;
	uint8_t				m_pendingConnectionStateChange;
	uint32_t			m_lastLEDTimer;
	uint32_t			m_lastSampleBatteryTimer;	
	uint32_t			m_lastAuthChallengeTime;
	uint32_t			m_lastClearBLEKeyButtonPushed;

	
	/****************************/
	/* Payload management       */
	/****************************/
	
	void				PayloadFinalized(uint8_t format);
	void				PayloadParse(uint8_t format, uint8_t data);
	void				PayloadParseBinary(uint8_t data);
	void				PayloadProcessFromJSON(const char *pszJSON);
	void				PayloadReset();
	
	void				StringPayloadFinalized();
	void				StringPayloadParse(uint8_t data);
	void				StringPayloadReset();
	
	void				PayloadExecute(uint8_t format);
	void				LogActiveCommand();
	void				SendAuthenticationChallenge();
	void				SendTransmissionCompletionUpdate(uint8_t progress);

	bool								m_bDynamicPayloadFilling;
	char								m_szPayloadCommandValue[MAX_COMMAND_VALUE];
	ANIMATIONPAYLOADSTATEMACHINE		m_animationPayloadStateMachine;
	int									m_nPayloadCommandValue, m_nPayloadCommandValue2;
	int									m_PayloadActiveCommand;
	int									m_PayloadBinaryFillPos;
	JsonStreamingParser					m_PayloadStreamingJSONParser;
	EPXString							m_PayloadJSONListnerTracking_Key;
	EPXString							m_PayloadJSONListnerTracking_Value;
	uint8_t								m_PayloadParsePaletteBytePos;
	uint8_t								m_PayloadParsePaletteRGB[3];
	uint16_t							m_PayloadParseCommandSequenceFillPos;
	uint32_t							m_PayloadActiveTransactionID;

	EXPRESSIVEPIXEL_SEQUENCE			m_StagedAnimation;
	
	/**** JsonListener implementation ****/
	virtual void	key(char *key);
	virtual void	byteAsHexValue(char *value);
	virtual void	value(char *value);
	virtual void	whitespace(char c) {}
	virtual void	startDocument() {}
	virtual void	endArray() {}
	virtual void	endObject() {}
	virtual void	endDocument() {}
	virtual void	startArray() {}
	virtual void	startObject() {}
	/*************************************/

};
	 

extern CExpressivePixelsApp g_ExpressivePixelsApp;
extern EPX_GUID				g_displayDesignGUID;
#endif

#ifdef __cplusplus
extern "C" 
{
#endif
	void				EPXApp_Process();
	void				EPXApp_Initialize(void *pLEDDriver, uint16_t *pArrayMatrix, uint16_t arrayWidth, uint16_t arrayHeight);	 
	
	extern uint8_t		g_emptyAESKey[];
#ifdef __cplusplus
}
#endif





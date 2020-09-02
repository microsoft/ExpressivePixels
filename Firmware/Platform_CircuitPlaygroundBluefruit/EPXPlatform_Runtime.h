// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include <Arduino.h>
#define USESEGGERRTT_LOG

enum enumLogLevel { LOGLEVEL_INFO = 1, LOGLEVEL_PAYLOAD, LOGLEVEL_NONE };

enum enumRebootTypes
{
	REBOOTTYPE_NONE, REBOOTTYPE_UF2, REBOOTTYPE_OTA, REBOOTTYPE_SERIAL
};

uint8_t HexToByte(char *hex, int len);
int freeRam();


#ifndef epxmin
#define epxmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef epxmax
#define epxmax(a,b) ((a)>(b)?(a):(b))
#endif


#define BytesToUInt16(a) ((uint16_t)(((uint32_t) *a << 8) | ((uint32_t) *(a + 1) << 0)))

#ifdef USESEGGERRTT_LOG
void SEGGER_RTT_LogLn(const char *fmt, ...);
#define DEBUGLOGLN(...) SEGGER_RTT_LogLn(__VA_ARGS__); 
#else
#define DEBUGLOGLN(...) 
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void EPXPlatform_Runtime_ControlAppTimer(bool on);
	void EPXPlatform_Runtime_Initialize();
	void EPXPlatform_Runtime_Process();
	void EPXPlatform_Runtime_Reboot(uint8_t rebootType);	
	void EPXPlatform_Runtime_MCUSleep();
	
	uint32_t millisPassed(uint32_t localMillis);
	
	char *stristr(const char *subject, const char *object);
	int stricmp(const char *s1, const char *s2);
		
#ifdef __cplusplus
}
#endif


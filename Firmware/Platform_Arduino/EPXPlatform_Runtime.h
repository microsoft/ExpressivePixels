// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include <Arduino.h>
// #define USESEGGERRTT_LOG
#define EPX_OPTIMIZEFORDEBUGGING

enum enumLogLevel { LOGLEVEL_INFO = 1, LOGLEVEL_PAYLOAD, LOGLEVEL_NONE };

enum enumRebootTypes
{
	REBOOTTYPE_NONE, REBOOTTYPE_UF2, REBOOTTYPE_OTA, REBOOTTYPE_SERIAL
};

uint8_t HexToByte(char *hex, int len);
void BytesToHex(uint8_t *p, int cb, char *psz, int stringCB);
int freeRam();


#ifndef epxmin
#define epxmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef epxmax
#define epxmax(a,b) ((a)>(b)?(a):(b))
#endif


#define BytesToUInt16(a) ((uint16_t)(((uint32_t) *a << 8) | ((uint32_t) *(a + 1) << 0)))

#define T(a) ((const char *) a)

#ifdef EPX_OPTIMIZEFORDEBUGGING

	#define EPX_OPTIMIZEFORDEBUGGING_ON \
		_Pragma("GCC push_options") \
		_Pragma("GCC optimize (\"O0\")")

	#define EPX_OPTIMIZEFORDEBUGGING_OFF \
		_Pragma("#pragma GCC pop_options") \

#else
	#define EPX_OPTIMIZEFORDEBUGGING_ON
	#define EPX_OPTIMIZEFORDEBUGGING_OFF
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void EPXPlatform_Runtime_ControlAppTimer(bool on);
	void EPXPlatform_Runtime_Initialize();
	void EPXPlatform_Runtime_Process();
	void EPXPlatform_Runtime_Reboot(uint8_t rebootType);	
	void EPXPlatform_Runtime_MCUSleep();
	void EPXPlatform_Runtime_MCUDeepSleep();
	
	uint32_t millisPassed(uint32_t localMillis);

	void *tmalloc(const char *pszFile, int line, size_t siz);
	void tfree(void *buf);
    void tmallocdump();
    void tmallocstats();

	#define TMALLOC(size) tmalloc(__FILE__, __LINE__, size)
	#define TFREE(ptr) tfree((uint8_t *) ptr)

	char *stristr(const char *subject, const char *object);
	int stricmp(const char *s1, const char *s2);
	char *epx_strupr(char s[]);

#ifdef USESEGGERRTT_LOG
	void SEGGER_RTT_LogLn(const char* fmt, ...);

#define DEBUGLOGLN(...) SEGGER_RTT_LogLn( __VA_ARGS__)
#else
#define DEBUGLOGLN(...)
#endif

#ifdef __cplusplus
}
#endif


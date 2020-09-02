// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "nrf_drv_saadc.h"
#include "nrf_drv_timer.h"

/*
 * Class implementation declarations for Battery Monitor functionality on Nordic SDK
 *
 **/

class BatteryMonitor
{
public:
	BatteryMonitor();
	
	void Initialize(uint16_t samplingPeriod = 0);
	void GetInfo(uint16_t *pMV, uint8_t *pPct);
	void Power(bool on)
	{
		if (on)
			PowerOn();
		else
			PowerOff();
	}
	void PowerOn();
	void PowerOff();
	void SingleSampleRequest();
	void StartSampling();
	void StopSampling();
	
private:
	void SAADC_Init();
	void SAADC_UnInit();
	void SAADC_SamplingInit();
	
	static void SAADC_Callback(nrf_drv_saadc_evt_t const * p_event);
	static void PPI_Timerhandler(nrf_timer_event_t event_type, void * p_context);
	
	static uint8_t VoltToPercent(uint16_t mvolts);
	
	bool m_bInitialized;			// True if battery monitor has been initialized
	bool m_bSAADCInitialized;		// True if SAADC has been initialized
	uint16_t m_samplingPeriod;		// SAADC sampling period
	static uint16_t m_milliVolts;	// Last MV sampling value
	static uint8_t m_percentage;	// Last battery percent remaining value
};

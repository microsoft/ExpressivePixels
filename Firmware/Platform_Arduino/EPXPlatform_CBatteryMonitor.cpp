// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "EPXPlatform_CBatteryMonitor.h"

#define VBAT_MV_PER_LSB   (0.87890625F)   // 3.6V ADC range and 12-bit ADC resolution = 3600mV/4096
#define VBAT_MV_PER_LSB   (0.87890625F)   // 3.6V ADC range and 12-bit ADC resolution = 3600mV/4096
#define VBAT_DIVIDER      (0.59820538F)   // 1.2M + 0.806M voltage divider on VBAT = (1.2M / (0.806M + 2M)) 
#define VBAT_DIVIDER_COMP (1.70583495F)

#define SAMPLES_IN_BUFFER 1


uint16_t BatteryMonitor::m_milliVolts = 5000;
uint8_t BatteryMonitor::m_percentage = 100;



BatteryMonitor::BatteryMonitor()
{
	m_bInitialized = false;
}


void BatteryMonitor::Initialize(uint16_t samplingPeriod)
{
	m_samplingPeriod = samplingPeriod;
	m_bInitialized = true;
}



void BatteryMonitor::PowerOn()
{
}



void BatteryMonitor::PowerOff()
{


}



void BatteryMonitor::StopSampling()
{
	if (m_bInitialized)
	{		
		
	}
}



void BatteryMonitor::StartSampling()
{
	if (m_bInitialized)
	{
		
	}
}



void BatteryMonitor::GetInfo(uint16_t *pMV, uint8_t *pPct)
{
	if (pMV != NULL)
		*pMV = m_milliVolts;
	if (pPct != NULL)
		*pPct = m_percentage;
}



void BatteryMonitor::SingleSampleRequest()
{
//	m_milliVolts = (uint16_t)((float) adcValue * VBAT_MV_PER_LSB * VBAT_DIVIDER_COMP);	 
//	m_percentage = VoltToPercent(m_milliVolts);

}




/*
100.00	4.2
90		4.13
80		4.06	
70		3.99	
60		3.92	
50		3.85	
40		3.78
30		3.71	
20		3.64	
10		3.57	
0		3.5	*/
uint8_t BatteryMonitor::VoltToPercent(uint16_t mvolts) 
{
	if (mvolts >= 4200)
		return 100;
	else if (mvolts > 4130)
		return 90;
	else if (mvolts > 4060)
		return 80;
	else if (mvolts > 3990)
		return 70;
	else if (mvolts > 3920)
		return 60;
	else if (mvolts > 3850)
		return 50;
	else if (mvolts > 3780)
		return 40;
	else if (mvolts > 3710)
		return 30;
	else if (mvolts > 3640)
		return 20;
	else if (mvolts > 3570)
		return 10;
	return 0;
}



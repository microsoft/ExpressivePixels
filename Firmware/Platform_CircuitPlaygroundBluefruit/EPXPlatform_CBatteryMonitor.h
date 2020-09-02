// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

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
	
	static uint8_t VoltToPercent(uint16_t mvolts);
	
	bool m_bInitialized;
	uint16_t m_samplingPeriod;
	static uint16_t m_milliVolts; 
	static uint8_t m_percentage;
};

/*
 * AquaAnalog.h
 *
 *  Created on: Oct 23, 2019
 *      Author: doc
 */
#pragma once
#include "Arduino.h"
#include <AquaHelper.h>
#include <AquaEEPROM.h>

class AquaAnalog {
public:
	void Init();
	void Update();
	uint16_t GetADCLevel(byte canal);
	float GetADCVoltage(byte canal);
	void CheckWaterLevel(void (*GetChanalState)(bool, byte));
	float CheckPhVoltage(byte canal);
	float CheckPhVoltageSettings(byte canal);
	bool AddPhElementToStats(AquaEEPROM eeprom);
private:
};

/*
 * AquaAnalog.h
 *
 *  Created on: Oct 23, 2019
 *      Author: doc
 */
#pragma once
#include "Arduino.h"
#include <AquaHelper.h>

class AquaAnalog {
public:
	void Init();
	void Update();
	uint16_t GetADCLevel(byte canal);
	float GetADCVoltage(byte canal);
	void CheckWaterLevel(void (*GetChanalState)(bool, byte));
	uint16_t CheckPhLevel(byte canal);
	bool AddPhElementToStats();
private:
};

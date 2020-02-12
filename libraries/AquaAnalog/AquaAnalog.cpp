/*
 * AquaAnalog.cpp
 *
 *  Created on: Oct 23, 2019
 *      Author: doc
 */
#include <AquaAnalog.h>

unsigned long lastPHStateTime = 0;

/**
 * 0,1 - PH canal
 * 2,3 - Water canal
 */
uint16_t adc[MAX_ADC_CANAL] = { 0, 0, 0, 0 };

void AquaAnalog::Init() {
	for (byte i = 0; i < MAX_ADC_CANAL; i++) {
		adc[i] = 0;
	}
}

void AquaAnalog::Update() {
	if (Helper.GetTimeNow().Second % 2 == 0) {
		adc[0] = analogRead(Helper.data.nADCPins[0]);
		adc[1] = analogRead(Helper.data.nADCPins[1]);
		adc[2] = analogRead(Helper.data.nADCPins[2]);
		adc[3] = analogRead(Helper.data.nADCPins[3]);
	}
}

uint16_t AquaAnalog::GetADCLevel(byte canal) {
	return adc[canal];
}

float AquaAnalog::GetADCVoltage(byte canal) {
	return (adc[canal] * 0.1875) / 1000;
}

void AquaAnalog::CheckWaterLevel(void (*GetChanalState)(bool, byte)) {
	if (GetADCVoltage(2) > 3.0f) {
		GetChanalState(true, 1);
	} else if (GetADCVoltage(3) > 3.0f) {
		GetChanalState(true, 2);
	} else {
		GetChanalState(false, 0);
	}
}

/**
 *
 * @param canal 1 - first PH canal; 2 - second PH canal
 * @return 0 - 1024
 */
uint16_t AquaAnalog::CheckPhLevel(byte canal) {
	if (canal != 1 || canal != 2)
		return 0;
	uint16_t level = map(GetADCLevel(canal - 1), 0, MAX_LEVEL_PH, 0, 1024);
	return level;
}

bool AquaAnalog::AddPhElementToStats() {

	for (byte i = 0; i < MAX_TIMERS_PH; i++) {
		uint16_t measure = CheckPhLevel(i);
		float V6_86 = 5 / 1024.0 * Helper.data.PHTimer686[i];
		float V4_01 = 5 / 1024.0 * Helper.data.PHTimer401[i];
		float PH_step = (V6_86 - V4_01) / (Ph6_86 - Ph4_01);
		float voltage = 5 / 1024.0 * measure;
		float PH_probe = Ph6_86 - ((V6_86 - voltage) / PH_step);
		Helper.data.PHCurrent[i] = Helper.ConvertPHWordToByte(PH_probe * 100);
		if (millis() > lastPHStateTime + DELAY_PH_UPDATE_STATE) {
			lastPHStateTime = millis();
			for (byte j = 1; j < MAX_STATS; j++) {
				Helper.data.PHStats[i][j - 1] = Helper.data.PHStats[i][j];
			}
			Helper.data.PHStats[i][MAX_STATS - 1] = Helper.data.PHCurrent[i];
			return true;
		}
	}
	return false;
}

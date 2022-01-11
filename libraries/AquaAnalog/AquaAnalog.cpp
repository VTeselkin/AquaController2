/*
 * AquaAnalog.cpp
 *
 *  Created on: Oct 23, 2019
 *      Author: doc
 */
#include <AquaAnalog.h>

unsigned long lastPHStateTime = 0;
unsigned long lastPHUpdate = 0;

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

		adc[0] = analogRead(Helper.data.nADCPins[0]);
		adc[1] = analogRead(Helper.data.nADCPins[1]);
		adc[2] = analogRead(Helper.data.nADCPins[2]);
		adc[3] = analogRead(Helper.data.nADCPins[3]);
}

uint16_t AquaAnalog::GetADCLevel(byte canal) {
	return analogRead(Helper.data.nADCPins[canal]);
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

int buffer_adc[20] = {0};
int adc_temp = 0;
unsigned long int ph_sum;
/**
 *
 * @param canal 0 - first PH canal; 1 - second PH canal
 * @return 0 - 1024
 */
float AquaAnalog::CheckPhVoltage(byte canal) {
	ph_sum = 0;
	for (int i = 0; i < 20; i++) {
		buffer_adc[i] = GetADCLevel(canal);
		delay(30);

	}
	for (int i = 0; i < 19; i++) {
		for (int j = i + 1; j < 20; j++) {
			if (buffer_adc[i] > buffer_adc[j]) {
				adc_temp = buffer_adc[i];
				buffer_adc[i] = buffer_adc[j];
				buffer_adc[j] = adc_temp;
			}
		}
	}

	for (int i = 5; i < 15; i++) {
		ph_sum += buffer_adc[i];
	}

	float volt = (float) ph_sum * 3.3f / 4096.0 / 10;
	return volt;
}

bool AquaAnalog::AddPhElementToStats() {
	if (millis() > lastPHUpdate) {
		for (byte i = 0; i < 2; i++) {
			float voltage = CheckPhVoltage(i);

			float x1 = Helper.data.PHCalibrationValue[2 * i] / 100.0f;//4.11f;
			float x2 = Helper.data.PHCalibrationValue[2 * i + 1] / 100.0f;//6.86f;
			float y1 = Helper.data.PHCalibrationVoltage[2 * i]/ 100.0f;//3.15f;
			float y2 = Helper.data.PHCalibrationVoltage[2 * i + 1] / 100.0f;//2.85f;
			float PH_probe = (-(x2 -x1) * voltage - (x1*y2 - x2*y1)) / (y1 -y2);
			Helper.data.PHCurrent[i] = Helper.ConvertPHWordToByte(PH_probe * 100);
			lastPHUpdate = millis() + DELAY_PH_UPDATE;
		}
		if (millis() > lastPHStateTime) {
			byte hour = Helper.GetHourNow();
			for (byte i = 0; i < MAX_TIMERS_PH; i++) {
				Helper.data.PHStats[i][hour] = Helper.data.PHCurrent[i];
			}
			lastPHStateTime = millis() + DELAY_PH_UPDATE_STATE;
		}
		return true;
	}

	return false;
}


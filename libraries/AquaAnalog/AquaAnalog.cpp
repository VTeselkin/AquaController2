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

unsigned int buffer_adc[2][60] = { 0 };
unsigned int adc_temp = 0;
unsigned long int ph_sum;
unsigned int adc_index[2] = { 0 };
/**
 *
 * @param canal 0 - first PH canal; 1 - second PH canal
 * @return 0 - 4096
 */
float AquaAnalog::CheckPhVoltage(byte canal) {
	ph_sum = 0;

	buffer_adc[canal][adc_index[canal]] = GetADCLevel(canal);
	adc_index[canal]++;
	if (adc_index[canal] >= 60) {
		adc_index[canal] = 0;
		for (int i = 0; i < 59; i++) {
			for (int j = i + 1; j < 60; j++) {
				if (buffer_adc[canal][i] > buffer_adc[canal][j]) {
					adc_temp = buffer_adc[canal][i];
					buffer_adc[canal][i] = buffer_adc[canal][j];
					buffer_adc[canal][j] = adc_temp;
				}
			}
		}

		for (int i = 20; i < 40; i++) {
			ph_sum += buffer_adc[canal][i];
		}
		for (int i = 0; i < 60; i++) {
			buffer_adc[canal][i] = 0;
		}
		float volt = (float) ph_sum * 3.3f / 4096.0 / 20;
		return volt;
	}
	return 0;
}

/**
 *
 * @param canal 0 - first PH canal; 1 - second PH canal
 * @return 0 - 4096
 */
float AquaAnalog::CheckPhVoltageSettings(byte canal) {
	for (int i = 0; i < 60; i++) {
		buffer_adc[canal][i] = 0;
	}
	ph_sum = 0;
	for (int i = 0; i < 59; i++) {
		buffer_adc[canal][i] = GetADCLevel(canal);
		delay(10);
	}

	for (int i = 0; i < 59; i++) {
		for (int j = i + 1; j < 60; j++) {
			if (buffer_adc[canal][i] > buffer_adc[canal][j]) {
				adc_temp = buffer_adc[canal][i];
				buffer_adc[canal][i] = buffer_adc[canal][j];
				buffer_adc[canal][j] = adc_temp;
			}
		}
	}

	for (int i = 20; i < 40; i++) {
		ph_sum += buffer_adc[canal][i];
	}
	for (int i = 0; i < 60; i++) {
		buffer_adc[canal][i] = 0;
	}
	float volt = (float) ph_sum * 3.3f / 4096.0 / 20;
	return volt;

}

bool AquaAnalog::AddPhElementToStats() {
	bool res = false;
	if (millis() > lastPHUpdate) {
		lastPHUpdate = millis() + DELAY_PH_UPDATE;
		for (byte i = 0; i < MAX_TIMERS_PH; i++) {
			float voltage = CheckPhVoltage(i);
			if (voltage != 0) {
				float x1 = Helper.data.PHCalibrationValue[2 * i] / 100.0f; //4.11f;
				float x2 = Helper.data.PHCalibrationValue[2 * i + 1] / 100.0f; //6.86f;
				float y1 = Helper.data.PHCalibrationVoltage[2 * i] / 100.0f; //3.15f;
				float y2 = Helper.data.PHCalibrationVoltage[2 * i + 1] / 100.0f; //2.85f;
				float PH_probe = (-(x2 - x1) * voltage - (x1 * y2 - x2 * y1)) / (y1 - y2);
				Helper.data.PHCurrent[i] = Helper.ConvertPHWordToByte(PH_probe * 100);
				byte hour = Helper.GetHourNow();
				Helper.data.PHStats[i][hour] = Helper.data.PHCurrent[i];
				res = true;
			}
		}

	}
	return res;
}


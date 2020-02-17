/*
 * AquaCanal.cpp
 *
 *  Created on: Oct 9, 2019
 *      Author: doc
 */

#include "AquaCanal.h"

void AquaCanal::Init() {
	Wire.begin();                       // Wire must be started first
	Wire.setClock(400000);              // Supported baud rates are 100kHz, 400kHz, and 1000kHz

	const int freq = 5000;
	const int resolution = 12;

	for (byte i = 0; i < MAX_CHANALS + MAX_CHANALS_PWM; i++) {

		ledcSetup(i, freq, resolution);
		if (i < MAX_CHANALS) {
			ledcAttachPin(Helper.data.nRelayDrive[i], i);
			pinMode(Helper.data.nRelayDrive[i], OUTPUT);
		} else {
			ledcAttachPin(Helper.data.nPWMDrive[i - MAX_CHANALS], i);
			pinMode(Helper.data.nPWMDrive[i - MAX_CHANALS], OUTPUT);
		}
	}
}

void AquaCanal::SetCanal(byte canal, byte state) {
	canal = Helper.data.nPinsESP32[canal]; //Hack for ESP32
	if (state == HIGH) {
		ledcWrite(canal, MAX_PWM_POWER_CALCULATE);
	} else if (state == LOW) {
		ledcWrite(canal, 0);
	}
}

void AquaCanal::SetPWMCanal(byte canal, word level) {
	canal = Helper.data.nPinsESP32[canal]; //Hack for ESP32
	ledcWrite(canal, level);
}

byte AquaCanal::GetCanal(byte canal) {
	canal = Helper.data.nPinsESP32[canal]; //Hack for ESP32
	if (ledcRead(canal) > 0) {
		return HIGH;
	} else {
		return LOW;
	}
}

uint32_t AquaCanal::GetPWMCanalLevel(byte canal) {
	canal = Helper.data.nPinsESP32[canal]; //Hack for ESP32
	return ledcRead(canal);
}

byte AquaCanal::GetPWMCanalState(byte canal) {
	canal = Helper.data.nPinsESP32[canal]; //Hack for ESP32
	if (ledcRead(canal) > 0) {
		return HIGH;
	} else {
		return LOW;
	}
}

void AquaCanal::SetStateCanal(void (*GetChanalState)()) {
	bool res = false;
	for (byte i = 0; i < MAX_CHANALS; i++) {
		if (Helper.data.StateChanals[i] == AUTO_CHANAL) {
			byte stateCanal = GetCanal(Helper.data.nRelayDrive[i]);
			if (Helper.data.CurrentStateChanalsByTypeTimer[i] != TIMER_OFF && stateCanal == LOW) {
				SetCanal(Helper.data.nRelayDrive[i], HIGH);
				res = true;
			} else if (Helper.data.CurrentStateChanalsByTypeTimer[i] == TIMER_OFF && stateCanal == HIGH) {
				stateCanal = LOW;
				SetCanal(Helper.data.nRelayDrive[i], LOW);
				res = true;
			}
		}
	}

	if (res) {
		GetChanalState();
	}
}

void AquaCanal::SetStatePWMCanal(void (*GetChanalState)()) {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		auto canal = Helper.data.TimerPWMChanal[i];
		if (Helper.data.StatePWMChanals[i] == AUTO_CHANAL) {
			if (Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] == LOW) {
				if (Helper.data.PowerPWMChanals[canal] >= 0 && Helper.data.PowerPWMChanals[canal] < MAX_PWM_POWER_CALCULATE) {
					SetPWMOnCanal(true, i);
				}
			} else if (Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] == HIGH) {
				if (Helper.data.PowerPWMChanals[canal] <= MAX_PWM_POWER_CALCULATE && Helper.data.PowerPWMChanals[canal] > 0) {
					SetPWMOnCanal(false, i);
				}
			}
			// Manual start PWM canal
		} else if (Helper.data.StatePWMChanals[i] == ON_CHANAL) {
			if (Helper.data.PowerPWMChanals[canal] <= MAX_PWM_POWER_CALCULATE) {
				Helper.data.TimetoCheckPWMstate[i] = 0;
				Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] = TIMER_ON;
				Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[i]] = MAX_PWM_POWER_CALCULATE;
				if (GetPWMCanalLevel(canal) < MAX_PWM_POWER_CALCULATE) {
					SetPWMCanal(canal, Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[i]]);
				}
			}
			// Manual shutdown PWM canal
		} else if (Helper.data.StatePWMChanals[i] == OFF_CHANAL) {
			if (Helper.data.PowerPWMChanals[canal] > 0) {
				Helper.data.TimetoCheckPWMstate[i] = 0;
				Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] = TIMER_OFF;
				Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[i]] = 0;
				if (GetPWMCanalLevel(canal) > 0) {
					SetPWMCanal(canal, Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[i]]);
				}
			}
		}

	}
}
void AquaCanal::SetPWMOnCanal(bool isOn, byte timers) {
	// https://www.arduino.cc/reference/en/language/functions/time/millis/
	if (Helper.data.TimetoCheckPWMstate[timers] == 0 || millis() < Helper.data.TimetoCheckPWMstate[timers]) {
		Helper.data.TimetoCheckPWMstate[timers] = millis();
	}
	auto millisForOne = 1000 / (MAX_PWM_POWER_CALCULATE / Helper.data.TimerPWMDuration[timers]); //millisForOne in millisecond
	unsigned int countStep = (millis() - Helper.data.TimetoCheckPWMstate[timers]) / millisForOne;
	if (countStep > 0) {
		if (isOn)
			Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]] = countStep;
		else {
			if (MAX_PWM_POWER_CALCULATE >= countStep) {
				Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]] = MAX_PWM_POWER_CALCULATE - countStep;
			} else {
				Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]] = 0;
			}
		}
		SetPWMCanal(Helper.data.TimerPWMChanal[timers], Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]]);

	}
}


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
	ledcSetup(ledChannel, freq, resolution);

}

void AquaCanal::SetCanal(byte canal, byte state) {
	if (state == HIGH) {
		pwmController.setChannelOn(canal);
	} else if (state == LOW) {
		pwmController.setChannelOff(canal);
	}
}

void AquaCanal::SetPWMCanal(byte canal, word level) {
	pwmController.setPWM(num, on, off)(canal, level);
}

byte AquaCanal::GetCanal(byte canal) {
	if (pwmController.getChannelPWM(canal) > 0) {
		return HIGH;
	} else {
		return LOW;
	}
}

word AquaCanal::GetPWMCanalLevel(byte canal) {
	return pwmController.getChannelPWM(canal);
}

byte AquaCanal::GetPWMCanalState(byte canal) {
	if (pwmController.getChannelPWM(canal) > 0) {
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

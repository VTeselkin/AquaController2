/*
 * AquaCanal.cpp
 *
 *  Created on: Oct 9, 2019
 *      Author: doc
 */

#include "AquaCanal.h"
PCA9685 pwm;
void AquaCanal::Init() {

	pwm.setupSingleDevice(Wire, 0x40);
	pwm.setupOutputEnablePin(2);
	pwm.enableOutputs(2);
	pwm.setToFrequency(200);
	for (byte i = 0; i < MAX_CHANALS; i++) {
		pinMode(Helper.data.nRelayDrive[i], OUTPUT);
		digitalWrite(Helper.data.nRelayDrive[i], LOW);
	}
}

byte AquaCanal::GetCanal(byte canal) {
	return digitalRead(canal);
}

void AquaCanal::SetCanal(byte canal, byte state) {
	digitalWrite(canal, state);

}

void AquaCanal::SetPWMCanal(byte canal, word level) {
	pwm.setChannelPulseWidth(canal, level, 0);
}

void AquaCanal::SetPWMCanalOn(byte canal) {
	pwm.setChannelPulseWidth(canal, MAX_PWM_POWER_CALCULATE, 0);

}

void AquaCanal::SetPWMCanalOff(byte canal) {
	pwm.setChannelPulseWidth(canal, 0, 0);
}

uint32_t AquaCanal::GetPWMCanalLevel(byte canal) {
	return 0;
}

byte AquaCanal::GetPWMCanalState(byte canal) {
	if (GetPWMCanalLevel(canal) > 0) {
		return HIGH;
	} else {
		return LOW;
	}
}

void AquaCanal::SetStateCanal(void (*GetChanalState)(typeResponse type)) {

	for (byte i = 0; i < MAX_CHANALS; i++) {

		if (Helper.data.StateChanals[i] == AUTO_CHANAL) {
			if (Helper.data.CurrentStateChanalsByTypeTimer[i] != TIMER_OFF
					&& GetCanal(Helper.data.nRelayDrive[i]) == LOW) {
				SetCanal(Helper.data.nRelayDrive[i], HIGH);
				GetChanalState(CANAL);
			} else if (Helper.data.CurrentStateChanalsByTypeTimer[i] == TIMER_OFF
					&& GetCanal(Helper.data.nRelayDrive[i]) == HIGH) {
				SetCanal(Helper.data.nRelayDrive[i], LOW);
				GetChanalState(CANAL);
			}
		} else if (Helper.data.StateChanals[i] == ON_CHANAL && GetCanal(Helper.data.nRelayDrive[i]) == LOW) {
			SetCanal(Helper.data.nRelayDrive[i], HIGH);
			GetChanalState(CANAL);
		} else if (Helper.data.StateChanals[i] == OFF_CHANAL && GetCanal(Helper.data.nRelayDrive[i]) == HIGH) {
			SetCanal(Helper.data.nRelayDrive[i], LOW);
			GetChanalState(CANAL);

		}

	}

}

void AquaCanal::SetStatePWMCanal(void (*GetChanalState)(typeResponse type)) {

	for (byte i = 0; i < MAX_TIMERS; i++) {
		auto canal = Helper.data.TimerPWMChanal[i];

		if (Helper.data.StatePWMChanals[Helper.data.TimerPWMChanal[i]] == AUTO_CHANAL
				&& Helper.data.TimerPWMState[i] == ENABLE_TIMER) {

			if (Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] == TIMER_PWM) {

				if (Helper.data.PowerPWMChanals[canal] >= 0
						&& Helper.data.PowerPWMChanals[canal] < Helper.GetLevelPWM(i)) {
					SetPWMOnCanal(true, i);
				} else {
					Helper.data.TimetoCheckPWMstate[i] = millis();
				}
			} else if (Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] == TIMER_OFF) {

				if (Helper.data.PowerPWMChanals[canal] <= Helper.GetLevelPWM(i)
						&& Helper.data.PowerPWMChanals[canal] > 0) {
					SetPWMOnCanal(false, i);
				} else {
					Helper.data.TimetoCheckPWMstate[i] = millis();
				}
			}

		}

	}
	// Manual start PWM canal
	for (byte canal = 0; canal < MAX_CHANALS_PWM; canal++) {
		if (Helper.data.StatePWMChanals[canal] == ON_CHANAL) {
			if (Helper.data.PowerPWMChanals[canal] < MAX_PWM_POWER_CALCULATE) {
				Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] = TIMER_ON;
				Helper.data.PowerPWMChanals[canal] = MAX_PWM_POWER_CALCULATE;
				SetPWMCanalOn(canal);
				GetChanalState(PWMCANAL);
			}
			// Manual shutdown PWM canal
		} else if (Helper.data.StatePWMChanals[canal] == OFF_CHANAL) {
			if (Helper.data.PowerPWMChanals[canal] > 0) {
				Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] = TIMER_OFF;
				Helper.data.PowerPWMChanals[canal] = 0;
				SetPWMCanalOff(canal);
				GetChanalState(PWMCANAL);
			}
		}
	}
}

void AquaCanal::SetPWMOnCanal(bool isOn, byte timers) {
	if (Helper.data.TimetoCheckPWMLastState[timers] != isOn) {
		Helper.data.TimetoCheckPWMLastState[timers] = isOn;
		Helper.data.TimetoCheckPWMstate[timers] = millis();
	}
	// https://www.arduino.cc/reference/en/language/functions/time/millis/
	if (Helper.data.TimetoCheckPWMstate[timers] == 0 || millis() < Helper.data.TimetoCheckPWMstate[timers]) {
		Helper.data.TimetoCheckPWMstate[timers] = millis();
	}
	if (Helper.data.TimerPWMDuration[timers] == 0) {
		Helper.data.TimerPWMDuration[timers] = 1;
	}
	float millisForOne = 1000.0 / (Helper.GetLevelPWM(timers) / Helper.data.TimerPWMDuration[timers]); //millisForOne in millisecond
	unsigned int countStep = (millis() - Helper.data.TimetoCheckPWMstate[timers]) / millisForOne;
	if (countStep > 0) {
		if (isOn) {
			Serial.print("START PWM TIMER ");
			Serial.println(countStep);
			Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]] = countStep;
		} else {
			Serial.print("STOP PWM TIMER ");
			Serial.println(countStep);
			if (Helper.GetLevelPWM(timers) >= countStep) {
				Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]] = Helper.GetLevelPWM(timers)
						- countStep;
			} else {
				Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]] = 0;
			}
		}
		SetPWMCanal(Helper.data.TimerPWMChanal[timers],
				Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]]);

	}
}

void AquaCanal::SetLEDError(typeLightLED type) {
	SetDebugLED(type, 10, 0);
}
void AquaCanal::SetLEDConnect(typeLightLED type) {
	SetDebugLED(type, 11, 1);
}
void AquaCanal::SetLEDRx(typeLightLED type) {
	SetDebugLED(type, 12, 2);
}
void AquaCanal::SetLEDTx(typeLightLED type) {
	SetDebugLED(type, 13, 3);
}

void AquaCanal::SetDebugLED(typeLightLED type, byte canal, byte index) {
	_typeDebugLED[index] = type;
	switch (type) {
	case SHORT:
	case LONG:
		_timeDebugLED[index] = millis();
		break;
	case NONE:
		SetPWMCanalOff(canal);
		_stateDebugLED[index] = 0;
		return;
	case PULSE:
	case LIGHT:
		break;
	}
	_stateDebugLED[index] = 1;
	SetPWMCanalOn(canal);
}

void AquaCanal::DisableLED() {
	for (byte i = 0; i < 4; i++) {
		if (_typeDebugLED[i] != NONE && _typeDebugLED[i] != LIGHT) {
			if (_typeDebugLED[i] == SHORT && _timeDebugLED[i] + 500 < millis()) {
				SetPWMCanalOff(10);
				_typeDebugLED[i] = NONE;
				_stateDebugLED[0] = 0;
			}
			if (_typeDebugLED[i] == LONG && _typeDebugLED[i] + 1000 < millis()) {
				SetPWMCanalOff(10);
				_typeDebugLED[i] = NONE;
				_stateDebugLED[0] = 0;
			}
			if (_typeDebugLED[i] == PULSE && _timeDebugLED[i] + 1000 < millis() && _stateDebugLED[0] == 1) {
				SetPWMCanalOff(10);
				_stateDebugLED[0] = 0;
				_timeDebugLED[i] = millis();
			}
			if (_typeDebugLED[i] == PULSE && _timeDebugLED[i] + 1000 < millis() && _stateDebugLED[0] == 0) {
				SetPWMCanalOn(10);
				_stateDebugLED[0] = 1;
				_timeDebugLED[i] = millis();
			}
		}
	}
}


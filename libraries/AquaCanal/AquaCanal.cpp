/*
 * AquaCanal.cpp
 *
 *  Created on: Oct 9, 2019
 *      Author: doc
 */

#include "AquaCanal.h"
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void AquaCanal::Init() {

	const int freq = 27000000;
	const int PWMFreq = 1600;

	  pwm.begin();
	  pwm.setOscillatorFrequency(freq);
	  pwm.setPWMFreq(PWMFreq);

	for (byte i = 0; i < MAX_CHANALS; i++) {
			pinMode(Helper.data.nRelayDrive[i], OUTPUT);
			digitalWrite(Helper.data.nRelayDrive[i], LOW);
	}
	for (byte i = 0; i < MAX_CHANALS_PWM; i++) {
		pwm.setPWM(i, 0, 4096);
	}
}

byte AquaCanal::GetCanal(byte canal) {
	return digitalRead(canal);
}

void AquaCanal::SetCanal(byte canal, byte state) {
	//canal = Helper.data.nPinsESP32[canal]; //Hack for ESP32
	digitalWrite(canal, state);

}

void AquaCanal::SetPWMCanal(byte canal, word level) {
	pwm.setPWM(canal, GetPWMCanalLevel(canal), level );
}

uint32_t AquaCanal::GetPWMCanalLevel(byte canal) {
	return pwm.getPWM(canal);
}

byte AquaCanal::GetPWMCanalState(byte canal) {
	if (GetPWMCanalLevel(canal) > 0) {
		return HIGH;
	} else {
		return LOW;
	}
}

void AquaCanal::SetStateCanal(void (*GetChanalState)(String)) {

	for (byte i = 0; i < MAX_CHANALS; i++) {

		if (Helper.data.StateChanals[i] == AUTO_CHANAL) {
			if (Helper.data.CurrentStateChanalsByTypeTimer[i] != TIMER_OFF && GetCanal(Helper.data.nRelayDrive[i]) == LOW) {
				SetCanal(Helper.data.nRelayDrive[i], HIGH);
				GetChanalState("Set AUTO_CHANAL HIGH = " + i);
			} else if (Helper.data.CurrentStateChanalsByTypeTimer[i] == TIMER_OFF && GetCanal(Helper.data.nRelayDrive[i]) == HIGH) {

				SetCanal(Helper.data.nRelayDrive[i], LOW);
				GetChanalState("Set AUTO_CHANAL LOW = " + i);
			}
		} else if (Helper.data.StateChanals[i] == ON_CHANAL && GetCanal(Helper.data.nRelayDrive[i]) == LOW) {

			SetCanal(Helper.data.nRelayDrive[i], HIGH);
			GetChanalState("Set MANUAL HIGH = " + i);
		} else if (Helper.data.StateChanals[i] == OFF_CHANAL && GetCanal(Helper.data.nRelayDrive[i]) == HIGH) {

			SetCanal(Helper.data.nRelayDrive[i], LOW);
			GetChanalState("Set MANUAL LOW = " + i);
			if(GetCanal(Helper.data.nRelayDrive[i]) == HIGH){
				GetChanalState("HIGH");
			}else{
				GetChanalState("LOW");
			}

		}

	}

}

void AquaCanal::SetStatePWMCanal(void (*GetChanalState)(String)) {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		auto canal = Helper.data.TimerPWMChanal[i];
		if (Helper.data.StatePWMChanals[i] == AUTO_CHANAL) {
			if (Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] == LOW) {
				if (Helper.data.PowerPWMChanals[canal]>= 0 && Helper.data.PowerPWMChanals[canal] < MAX_PWM_POWER_CALCULATE) {
					SetPWMOnCanal(true, i);
				}
			} else if (Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] == HIGH) {
				if (Helper.data.PowerPWMChanals[canal] <= MAX_PWM_POWER_CALCULATE
						&& Helper.data.PowerPWMChanals[canal] > 0) {
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
		SetPWMCanal(Helper.data.TimerPWMChanal[timers],
				Helper.data.PowerPWMChanals[Helper.data.TimerPWMChanal[timers]]);

	}
}


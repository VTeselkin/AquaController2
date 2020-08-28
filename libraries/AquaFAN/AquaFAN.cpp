/*
 * AquaFAN.cpp
 *
 *  Created on: 20 рту. 2020 у.
 *      Author: DrJar
 */

#include "AquaFAN.h"


/**
 * Checking the status of programs for temperature sensors
 */
void AquaFAN::CheckStateFANTimer(void (*GetChanalState)(typeResponse type)) {
	if (Helper.GetTimeNow().Second % FREQURENCY_SEND_TEMP == 0) {
		if (!isWaitTemp) {
			isWaitTemp = true;
			for (byte canal = 0; canal < MAX_CHANALS_FAN; canal++) {
				bool result = false;
				byte pwmCanal = MAX_CHANALS_PWM - MAX_CHANALS_FAN + canal;
				if (Helper.data.StatePWMChanals[pwmCanal] == AUTO_CHANAL
						&& Helper.data.TempSensorState[Helper.data.FANSensor[canal]] != DISCONNECT_SENSOR) {

					if (CheckStateFAN(canal))
						result = true;
				}
				CheckCollisionsFAN(canal, result, TIMER_TEMPFAN, GetChanalState);
			}
		}
	} else {
		isWaitTemp = false;
	}
}

bool AquaFAN::CheckCollisionsFAN(byte chanal, bool isEnable, byte timerType,
		void (*GetChanalState)(typeResponse type)) {

	if (isEnable) {
		if (Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] == TIMER_OFF) {
			Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] = timerType;
			GetChanalState(CANAL);
			return true;
		} else {
			if (Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] != TIMER_TEMPFAN && timerType == TIMER_TEMPFAN) {
				Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] = timerType;
				GetChanalState(CANAL);
			}
			return false;
		}
	} else if (Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] != TIMER_OFF) {
		if (Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] == timerType) {
			Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] = TIMER_OFF;
			GetChanalState(CANAL);
			return true;
		}
	}
	return false;
}

/**
 * Checking whether the program can be run for temperature sensors
 */
bool AquaFAN::CheckStateFAN(byte canal) {

//Cooling
//-----Tmin=25------Tmax=20------/
	if (Helper.data.FANTimerMinStart[canal] > Helper.data.FANTimerMaxEnd[canal]) {
//-----Tmin=25------Tmax=20----Tcur=26--/
		if (Helper.data.TempSensor[Helper.data.FANSensor[canal]] >= Helper.data.FANTimerMinStart[canal]) {
			return true;
		}
//-----Tmin=25------Tmax=22----Tcur=24--/
		if (Helper.data.TempSensor[Helper.data.FANSensor[canal]] < Helper.data.FANTimerMinStart[canal]
				&& Helper.data.TempSensor[Helper.data.FANSensor[canal]] > Helper.data.FANTimerMaxEnd[canal]) {
			if (Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] == TIMER_TEMPFAN) {
				return true;
			}
		}

		return false;
//Warming
//-----Tmin=20------Tmax=25------/
	} else if (Helper.data.FANTimerMinStart[canal] < Helper.data.FANTimerMaxEnd[canal]) {
//--Tcur=19---Tmin=20------Tmax=25------/
		if (Helper.data.TempSensor[Helper.data.FANSensor[canal]] < Helper.data.FANTimerMinStart[canal]) {
			return true;
		}
//-----Tmin=20---Tcur=22---Tmax=25------/
		if (Helper.data.TempSensor[canal] >= Helper.data.FANTimerMinStart[canal]
				&& Helper.data.TempSensor[Helper.data.FANSensor[canal]] < Helper.data.FANTimerMaxEnd[canal]) {
			if (Helper.data.CurrentStatePWMChanalsByTypeTimer[canal] == TIMER_TEMPFAN) {
				return true;
			}
		}

		return false;

	}
	return false;

}

void AquaFAN::SetStateFANCanal(byte canal) {}


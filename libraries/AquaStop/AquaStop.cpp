/*
 * AquaStop.cpp
 *
 *  Created on: Oct 8, 2019
 *      Author: doc
 */

#include "AquaStop.h"

byte minForDisableZeroChanal = 0;


bool AquaStop::GetTemporaryStopCanal(bool isNeedEnableZeroCanal, void (*GetChanalState)()) {

		if (isNeedEnableZeroCanal) {
			if (Helper.GetTimeNow().Minute == minForDisableZeroChanal) {
				isNeedEnableZeroCanal = false;
				Helper.data.CurrentStateChanalsByTypeTimer[CHANAL_BTN_DISABLE] = TIMER_ON;
				GetChanalState();
			} else {
				if (Helper.data.CurrentStateChanalsByTypeTimer[CHANAL_BTN_DISABLE] == TIMER_ON) {
					Helper.data.CurrentStateChanalsByTypeTimer[CHANAL_BTN_DISABLE] = TIMER_OFF;
					GetChanalState();
				}
			}
		}
		return isNeedEnableZeroCanal;
}

bool AquaStop::SetTemporaryStopCanal(byte delay, bool isNeedEnableZeroCanal, void (*GetChanalState)()) {

	if (Helper.data.CurrentStateChanalsByTypeTimer[CHANAL_BTN_DISABLE] == TIMER_ON && !isNeedEnableZeroCanal) {
		Helper.data.CurrentStateChanalsByTypeTimer[CHANAL_BTN_DISABLE] = TIMER_OFF;
		minForDisableZeroChanal = Helper.GetTimeNow().Minute + delay;
		isNeedEnableZeroCanal = true;
		if (minForDisableZeroChanal > MINUTE) {
			minForDisableZeroChanal -= (MINUTE + 1);
		}
		GetChanalState();
	} else if (isNeedEnableZeroCanal) {
		Helper.data.CurrentStateChanalsByTypeTimer[CHANAL_BTN_DISABLE] = TIMER_ON;
		isNeedEnableZeroCanal = false;
		GetChanalState();
	}
	return isNeedEnableZeroCanal;
}

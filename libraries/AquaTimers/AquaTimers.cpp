/*
 * AquaTimers.cpp
 *
 *  Created on: Sep 19, 2019
 *      Author: doc
 */
#include "AquaTimers.h"

/**
 *  Every second check the status of all timers
 */
void AquaTimers::CheckStateTimer(int lastCheck, byte timerType, void (*GetChanalState)(typeResponse type),
		bool isNeedEnableZeroCanal) {
	switch (timerType) {
	case TIMER_MIN:
		CheckStateTimerHelper(Helper.data.DailyTimerState, Helper.data.DailyTimerChanal, GetChanalState, timerType,
				MAX_CHANALS, isNeedEnableZeroCanal);
		break;
	case TIMER_OTHER:
		CheckStateTimerHelper(Helper.data.HoursTimerState, Helper.data.HoursTimerCanal, GetChanalState, timerType,
				MAX_CHANALS, isNeedEnableZeroCanal);
		break;
	case TIMER_SEC:
		CheckStateTimerHelper(Helper.data.SecondTimerState, Helper.data.SecondTimerCanal, GetChanalState, timerType,
				MAX_CHANALS, isNeedEnableZeroCanal);
		break;
	case TIMER_PWM:
		CheckStateTimerHelper(Helper.data.TimerPWMState, Helper.data.TimerPWMChanal, GetChanalState, timerType,
				MAX_CHANALS_PWM, isNeedEnableZeroCanal);
		break;
	}

	if (Helper.GetTimeNow().Second != lastCheck) {
		lastCheck = second();
	}

}
void AquaTimers::CheckStateTimerHelper(byte TimerState[], byte TimerCanal[], void (*GetChanalState)(typeResponse type),
		byte timerType, byte max_canal, bool isNeedEnableZeroCanal) {
	for (byte j = 0; j < max_canal; j++) {
		bool result = false;
		for (byte i = 0; i < MAX_TIMERS; i++) {
			if (TimerState[i] == ENABLE_TIMER) {
				byte state = 0;
				if (timerType != MAX_CHANALS_PWM) {
					state = Helper.data.StateChanals[TimerCanal[i]];
				} else {
					state = Helper.data.StatePWMChanals[TimerCanal[i]];
				}
				if (TimerCanal[i] == j && state == AUTO_CHANAL) {
					switch (timerType) {
					case TIMER_MIN:
						if (CheckStateDailyTimer(i)) {
							result = true;
						}
						break;
					case TIMER_OTHER:
						if (CheckStateHoursTimer(i)) {
							result = true;
						}
						break;
					case TIMER_SEC:
						if (CheckStateSecondTimer(i)) {
							result = true;
						}
						break;
					case TIMER_PWM:
						if (CheckStatePWMTimer(i)) {
							result = true;
						}
						break;
					}

				}
			}
		}

		CheckCollisionsOtherTimer(j, result, timerType, GetChanalState, isNeedEnableZeroCanal);
	}
}
/**
 *  Check on-time minute timer
 */
bool AquaTimers::CheckStateDailyTimer(byte i) {

	word timeStart = Helper.data.DailyTimerHourStart[i] * SEC_IN_MIN + Helper.data.DailyTimerMinStart[i];
	word timeFinish = Helper.data.DailyTimerHourEnd[i] * SEC_IN_MIN + Helper.data.DailyTimerMinEnd[i];
	if (timeFinish == timeStart) {
		return false;
	}
	unsigned int currentTime = Helper.GetTimeNow().Hour * SEC_IN_MIN + Helper.GetTimeNow().Minute;
	if (timeFinish > timeStart) {
		if (currentTime >= timeStart && currentTime < timeFinish) {
			return true;
		}
		return false;
	} else {
		if (currentTime >= timeStart && currentTime < MIN_BY_DAY) {
			return true;
		}
		if (currentTime < timeFinish) {
			return true;
		}
	}
	return false;
}

/**
 * Checking the hourly timer to enable it
 */
bool AquaTimers::CheckStateHoursTimer(byte i) {

	if (Helper.data.HoursTimerMinStart[i] == Helper.data.HoursTimerMinStop[i])
		return false;
	if (Helper.data.HoursTimerMinStart[i] < Helper.data.HoursTimerMinStop[i]) {
		if (Helper.GetTimeNow().Minute >= Helper.data.HoursTimerMinStart[i]
				&& Helper.GetTimeNow().Minute < Helper.data.HoursTimerMinStop[i]) {
			return true;
		}
	} else {
		if (Helper.GetTimeNow().Minute >= Helper.data.HoursTimerMinStart[i] && Helper.GetTimeNow().Minute <= MINUTE) {
			return true;
		}
		if (Helper.GetTimeNow().Minute < Helper.data.HoursTimerMinStop[i]) {
			return true;
		}
	}
	return false;
}

/**
 * Checking the second timer to enable it
 */
bool AquaTimers::CheckStateSecondTimer(byte i) {

	unsigned long timeSecondStart = (Helper.data.SecondTimerHourStart[i] * SEC_IN_MIN * SEC_IN_MIN)
			+ (Helper.data.SecondTimerMinStart[i] * 60);
	unsigned long timeSecondFinish = timeSecondStart + Helper.data.SecondTimerDuration[i];
	unsigned long currentTime = Helper.GetTimeNow().Hour * SEC_IN_MIN * SEC_IN_MIN
			+ Helper.GetTimeNow().Minute * SEC_IN_MIN + Helper.GetTimeNow().Second;
	if (timeSecondFinish <= SECOND_BY_DAY) {
		if (currentTime >= timeSecondStart && currentTime < timeSecondFinish) {
			return true;
		}
		return false;
	} else {
		if (currentTime >= timeSecondStart && currentTime <= SECOND_BY_DAY) {
			return true;
		}
		if (currentTime < timeSecondFinish - SECOND_BY_DAY) {
			return true;
		}
		return false;
	}

}

/**
 *  Check on-time minute timer
 */
bool AquaTimers::CheckStatePWMTimer(byte i) {

	word timeStart = Helper.data.TimerPWMHourStart[i] * SEC_IN_MIN + Helper.data.TimerPWMMinStart[i];
	word timeFinish = Helper.data.TimerPWMHourEnd[i] * SEC_IN_MIN + Helper.data.TimerPWMMinEnd[i];
	if (timeFinish == timeStart) {
		return false;
	}
	unsigned int currentTime = Helper.GetTimeNow().Hour * SEC_IN_MIN + Helper.GetTimeNow().Minute;
	if (timeFinish > timeStart) {
		if (currentTime >= timeStart && currentTime < timeFinish) {
			return true;
		}
		return false;
	} else {
		if (currentTime >= timeStart && currentTime < MIN_BY_DAY) {
			return true;
		}
		if (currentTime < timeFinish) {
			return true;
		}
	}
	return false;
}

/**
 * Check channel status before making any changes.
 * If we have a conflict timer switches the channel then we change the type
 * of timer is on this channel in the property stateChanalsTimer
 */
bool AquaTimers::CheckCollisionsOtherTimer(byte chanal, bool isEnable, byte timerType,
		void (*GetChanalState)(typeResponse type), bool isNeedEnableZeroCanal) {
	if (timerType == TIMER_PWM) {
		if (isEnable) {
			if (Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] != TIMER_PWM) {
				Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] = TIMER_PWM;
				GetChanalState(PWMTIMER);
				return true;
			}
		} else if (Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] != TIMER_OFF) {
			Helper.data.CurrentStatePWMChanalsByTypeTimer[chanal] = TIMER_OFF;
			GetChanalState(PWMTIMER);
			return true;
		}
		return false;
	}
	if (chanal == CHANAL_BTN_DISABLE && isNeedEnableZeroCanal)
		return false;
	if (isEnable) {
		if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] == TIMER_OFF) {
			Helper.data.CurrentStateChanalsByTypeTimer[chanal] = timerType;
			GetChanalState(CANAL);
			return true;
		} else {
			if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] != TIMER_TEMP && timerType == TIMER_TEMP) {
				Helper.data.CurrentStateChanalsByTypeTimer[chanal] = timerType;
				GetChanalState(CANAL);
			} else if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] != TIMER_MIN && timerType == TIMER_MIN
					&& timerType != TIMER_TEMP) {
				Helper.data.CurrentStateChanalsByTypeTimer[chanal] = timerType;
				GetChanalState(CANAL);
			} else if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] != TIMER_SEC && timerType == TIMER_SEC
					&& Helper.data.CurrentStateChanalsByTypeTimer[chanal] != TIMER_MIN && timerType != TIMER_TEMP) {
				Helper.data.CurrentStateChanalsByTypeTimer[chanal] = timerType;
				GetChanalState(CANAL);
			} else if (timerType == TIMER_OTHER && Helper.data.CurrentStateChanalsByTypeTimer[chanal] == TIMER_OFF
					&& timerType != TIMER_TEMP) {
				Helper.data.CurrentStateChanalsByTypeTimer[chanal] = timerType;
				GetChanalState(CANAL);
			}

			return false;
		}
	} else if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] != TIMER_OFF) {
		if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] == timerType) {
			Helper.data.CurrentStateChanalsByTypeTimer[chanal] = TIMER_OFF;
			GetChanalState(CANAL);
			return true;
		}
	}
	return false;
}


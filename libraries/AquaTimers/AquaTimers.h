/*
 * AquaTimers.h
 *
 *  Created on: Sep 19, 2019
 *      Author: doc
 */
#pragma once

#include "Arduino.h"
#include <time.h>
#include <TimeLib.h>
#include <AquaHelper.h>



class AquaTimers {

public:

	 void CheckStateTimer(int lastCheck, byte timerType, void (*GetChanalState)(String), bool isNeedEnableZeroCanal);
	 void CheckStateTimerHelper(byte TimerState[], byte TimerCanal[],void (*GetChanalState)(String), byte timerType, byte max_canal, bool isNeedEnableZeroCanal);

private:
	 	 bool CheckCollisionsOtherTimer(byte chanal, bool isEnable, byte timerType, void (*GetChanalState)(String), bool isNeedEnableZeroCanal);
	 	 bool CheckStateDailyTimer(byte i);
		 bool CheckStateHoursTimer(byte i);
		 bool CheckStateSecondTimer(byte i);
		 bool CheckStatePWMTimer(byte i);

};

/*
 * AquaStop.h
 *
 *  Created on: Oct 8, 2019
 *      Author: doc
 */

#pragma once

#include "Arduino.h"
#include <time.h>
#include <TimeLib.h>
#include <AquaHelper.h>

class AquaStop{
public:
	bool GetTemporaryStopCanal(bool isNeedEnableZeroCanal, void (*GetChanalState)(String));
	bool SetTemporaryStopCanal(byte delay, bool isNeedEnableZeroCanal, void (*GetChanalState)());

private:
};

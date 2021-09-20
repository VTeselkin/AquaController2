/*
 * AquaNTP.h
 *
 *  Created on: Oct 31, 2019
 *      Author: doc
 */

#pragma once

#include <AquaHelper.h>
#include "Arduino.h"
#include <time.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <AquaDisplay.h>

const byte NTPServerCount = 3;

class AquaNTP{
public:
	bool SetNTPTimeToController(void (*ChangeLog)(String));
private:
};

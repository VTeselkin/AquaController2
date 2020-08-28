/*
 * AquaFAN.h
 *
 *  Created on: 20 рту. 2020 у.
 *      Author: DrJar
 */


#pragma once

#include "Arduino.h"
#include <AquaHelper.h>
#include <time.h>
#include <TimeLib.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <AquaEEPROM.h>

class AquaFAN{
public:
bool isWaitTemp = false;
void CheckStateFANTimer(void (*GetChanalState)(typeResponse type));
bool CheckStateFAN(byte canal);
private:
bool CheckCollisionsFAN( byte chanal, bool isEnable, byte timerType, void (*GetChanalState)(typeResponse type));
void SetStateFANCanal(byte canal);
};

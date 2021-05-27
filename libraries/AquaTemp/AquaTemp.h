/*
 * AquaTemp.h
 *
 *  Created on: Oct 2, 2019
 *      Author: doc
 */

#pragma once

#include "Arduino.h"
#include <AquaHelper.h>
#include <time.h>
#include <TimeLib.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <AquaEEPROM.h>

void CheckStateWarningTemp(byte sensorIndex, bool isWarming);
bool CheckStateTemp(byte sensorIndex, byte canalIndex);
bool CheckCollisionsTemp( byte chanal, bool isEnable, byte timerType, void (*GetChanalState)(typeResponse type), bool isNeedEnableZeroCanal);
bool CompareDeviceAddress(DeviceAddress &device1, DeviceAddress &device2);

class AquaTemp {
public:
void Init(AquaEEPROM aquaEEPROM);
void CheckStateTempTimer(void (*GetChanalState)(typeResponse type), bool isNeedEnableZeroCanal);
void GetTemperature( void (*GetChanalState)(typeResponse type));
bool GetTemperatureIsWarning();
byte GetTemperatereSensorIndexWarning();
void SetDalasSensor(AquaEEPROM eeprom);
void CheckStateWarningTemp( byte sensorIndex, bool isWarming);
bool CheckStateTemp(byte sensorIndex, byte canalIndex);
bool AddTempElementToStats();
byte ConvertTempWordToByte(unsigned short temp);
word ConvertTempByteToWord(unsigned short temp);
private:

};

/*
 * AquaEEPROM.h
 *
 *  Created on: Oct 10, 2019
 *      Author: doc
 */
#include "Arduino.h"
#include <AquaHelper.h>
#include <EEPROM.h>

#pragma once


const byte UTC_ADDR = 1;
class AquaEEPROM{
public:
	void Init();
	void SaveChanalState();
	void LoadChanelState();
	void SavePWMChanalState();
	void LoadPWMChanelState();
	void LoadDailyTimersReadFromERROM();
	void LoadPWMTimersReadFromERROM();
	void SavePWMTimerToERROM();
	void SaveDailyTimerToERROM();
	void LoadSecondsTimersReadFromERROM();
	void SaveSecondsTimerToERROM();
	void LoadHoursTimersReadFromERROM();
	void SaveHoursTimerToERROM();
	void LoadTempTimerFromERROM();
	void SaveTempSensorAdress(byte i, byte j);
	void SaveTempTimerToERROM();
	void SaveLcdSetings();
	void LoadLcdSetings();
	void SaveFANSettings();
	void LoadFANSettings();
	void LoadWiFiSettings();
	void SaveWifiSettings();
	void SavePHTimerToERROM();
	void LoadPHTimerToERROM();
	uint16_t SaveUTCSetting(uint16_t utc);
	uint16_t LoadUTCSetting();
private:
	void OnFirstLunch();

};

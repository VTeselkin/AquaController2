/*
 * AquaDisplay.h
 *
 *  Created on: 7 мая 2021 г.
 *      Author: DrJar
 */

#pragma once
#ifndef AquaDisplay_h
#define AquaDisplay_h
#include "Arduino.h"
#include "AquaHelper.h"
#include "EasyNextionLibrary.h"

class AquaDisplay {

public:
	static bool IsNeedSave();
	static void SetNeedSave(bool isNeedSave);
	static byte CurrentPage();
	static void Init();
	static void Update();
	static void Loop();
	static void SendLog(String log);
	static void SendLogLn(String log);
	static void ClearLog();
	static void SetLANConnection(bool isConnect);
	static void SetWANConnection(bool isConnect);
	static String GetVersion();
	static void SetDayOfWeek();
	static void SetData();
	static void SetTime();
	static void SetTemp(word temp);
	static void UpdateCanals(byte canals[], byte max_canal, String canal_name);
	static void SetPage(byte page);
	static void SetCanalState(byte i);
	static void SetPWMCanalState(byte i);
	static void SetTimerNumber(bool inc);
	static void SetTimerHourOn(bool inc);
	static void SetTimerMinutesOn(bool inc);
	static void SetTimerHourOff(bool inc);
	static void SetTimerMinutesOff(bool inc);
	static void SetTimerDelay(bool inc);
	static void SetTimerState();
	static void SetTimerCanal(bool inc);
	static void SetTimerLevel(bool inc);
	static void ChangeData(byte data[], byte max, byte index, bool inc);
	static void ChangeDataState(byte data[], byte max, byte index);
	static void CheckIndexTimer(byte &index, byte max, bool inc);
	static void UpdateDisplayTimersPWM();
	static void UpdateDisplayTimersDaily();
	static void UpdateDisplayTimersHourly();
	static void UpdateDisplayTimersSecond();

private:
	static String Format02D(byte data);
	static String Format03D(byte data);
};
#ifdef Display
#undef Display
#endif

extern AquaDisplay Display;

#endif

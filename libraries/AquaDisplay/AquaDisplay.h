/*
 * AquaDisplay.h
 *
 *  Created on: 7 мая 2021 г.
 *      Author: DrJar
 */

#pragma once
#ifndef AquaDisplay_h
#define AquaDisplay_h
#include <AquaHelper.h>
#include "Arduino.h"
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
	static typeResponse SetTimerHourOn(bool inc);
	static typeResponse SetTimerMinutesOn(bool inc);
	static typeResponse SetTimerHourOff(bool inc);
	static typeResponse SetTimerMinutesOff(bool inc);
	static typeResponse SetTimerDelay(bool inc);
	static typeResponse SetTimerState();
	static typeResponse SetTimerCanal(bool inc);
	static typeResponse SetTimerLevel(bool inc);
	static void ChangeData(byte data[], byte max, byte index, bool inc);
	static void ChangeDataCanal(byte data[], byte max, byte index, bool inc);
	static void ChangeDataState(byte data[], byte max, byte index);
	static void CheckIndexTimer(byte &index, byte max, bool inc);
	static void UpdateDisplayTimersPWM();
	static void UpdateDisplayTimersDaily();
	static void UpdateDisplayTimersHourly();
	static void UpdateDisplayTimersSecond();
	static void UpdateDisplayTimersTemp();
	static void UpdateDisplaySettings();

private:
	static String Format02D(byte data);
	static String Format03D(byte data);
	static String Format02DCanal(byte data);
	static String Format04DTemp(unsigned short temp, bool needConvert);
};
#ifdef Display
#undef Display
#endif

extern AquaDisplay Display;

#endif

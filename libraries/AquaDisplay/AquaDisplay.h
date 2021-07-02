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

class AquaDisplay{
public:
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
private:
};
#ifdef Display
#undef Display
#endif

extern AquaDisplay Display;

#endif

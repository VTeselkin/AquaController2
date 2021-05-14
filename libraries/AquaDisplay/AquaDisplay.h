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
	static void SendTime();
	static void SendLog(String log);
	static void SendLogLn(String log);
	static void ClearLog();
	static void SetLANConnection(bool isConnect);
	static void SetWANConnection(bool isConnect);
	static String GetVersion();
private:
};
#ifdef Display
#undef Display
#endif

extern AquaDisplay Display;

#endif

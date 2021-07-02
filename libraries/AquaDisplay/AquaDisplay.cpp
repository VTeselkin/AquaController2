/*
 * AquaDisplay.cpp
 *
 *  Created on: 7 мая 2021 г.
 *      Author: DrJar
 */
#include "AquaDisplay.h"
#include <AquaCanal.h>

EasyNex myNex(Serial);

void AquaDisplay::Init() {
	myNex.begin(9600);
	delay(1000);
	SetPage(2);
	ClearLog();

}

void AquaDisplay::SetPage(byte page) {
	Serial.print("page " + String(page));
	Serial.print("\xFF\xFF\xFF");
	Serial.flush();

	switch (page) {
	case 0:

		break;
	case 1:
		//page Main
		Update();
		SetTemp(NULL);
		break;
	case 2:
		//page Init
		break;
	case 3:
		UpdateCanals(Helper.data.StateChanals, MAX_CHANALS, "canal");
		//page Canal
		break;
	case 4:
		UpdateCanals(Helper.data.StatePWMChanals,MAX_CHANALS_PWM, "canal_pwm");
		//page Canal
		break;
	default:
		//page Main
		break;
	}

}

void AquaDisplay::Update() {

	myNex.writeStr("time.txt", Helper.GetFormatTimeNow(true));
	SetData();
	SetTime();
	SetDayOfWeek();
}
void AquaDisplay::Loop() {
	myNex.NextionListen();
}
String AquaDisplay::GetVersion() {
	return myNex.readStr("ver.txt");
}
byte rowLog = 0;
String oldLog = "";
void AquaDisplay::SendLogLn(String log) {
	if (log.length() > 80) {
		if (oldLog.length() == log.length()) {
			return;
		}
		oldLog = log;
		log = log.substring(0, 80) + "...";
	}
	rowLog++;
	if (rowLog > 18) {
		ClearLog();
		rowLog = 0;
		SendLogLn(log);
	}
	log.replace("\"", "");
	myNex.writeStr("log.txt+", log + "\r\n");
}

void AquaDisplay::SendLog(String log) {
	if (log.length() > 40) {
		log = log.substring(0, 40) + "...";
	}
	rowLog++;
	log.replace("\"", "");
	myNex.writeStr("log.txt+", log);

}

void AquaDisplay::ClearLog() {
	myNex.writeStr("log.txt", "");
}
bool isLasConnectLAN = false;
void AquaDisplay::SetLANConnection(bool isConnect) {
	if (isConnect && !isLasConnectLAN) {
		isLasConnectLAN = true;
		Serial.print("lan_ico.pic=3");
		Serial.print("\xFF\xFF\xFF");
	}
	if (!isConnect && isLasConnectLAN) {
		isLasConnectLAN = false;
		Serial.print("lan_ico.pic=2");
		Serial.print("\xFF\xFF\xFF");
	}
}
bool isLasConnectWAN = false;
void AquaDisplay::SetWANConnection(bool isConnect) {
	if (isConnect && !isLasConnectWAN) {
		isLasConnectWAN = true;
		Serial.print("wan_ico.pic=0");
		Serial.print("\xFF\xFF\xFF");
	}
	if (!isConnect && isLasConnectWAN) {
		isLasConnectWAN = false;
		Serial.print("wan_ico.pic=1");
		Serial.print("\xFF\xFF\xFF");
	}
}

void AquaDisplay::SetDayOfWeek() {
	myNex.writeStr("day_main.txt", Helper.GetDayOfWeek());
}

void AquaDisplay::SetData() {
	myNex.writeStr("data_main.txt", Helper.GetFormatDataNow());
}

void AquaDisplay::SetTime() {
	myNex.writeStr("btn_time.txt", Helper.GetFormatTimeNow(true));
}

String s_temp = "";
void AquaDisplay::SetTemp(word temp) {
	if (temp != NULL) {
		s_temp = "";
		byte k = temp / 100;
		byte m = temp % 100;
		if (k < 10)
			s_temp += "0";
		s_temp += k;
		s_temp += ".";
		if (m < 10) {
			s_temp += m;
		} else {
			s_temp += m / 10;
		}
	}
	myNex.writeStr("btn_temp.txt", s_temp);
}

void AquaDisplay::UpdateCanals(byte canals[], byte max_canal, String canal_name) {
	for (byte i = 1; i < max_canal + 1; i++) {
		String value = "AUTO";
		if (canals[i - 1] == 1) {
			value = "OFF";
		} else if (canals[i - 1] == 2) {
			value = "ON";
		}
		myNex.writeStr(canal_name + String(i) + ".txt", value);
	}
}

void AquaDisplay::SetCanalState(byte i) {
	Helper.data.StateChanals[i]--;
	if (Helper.data.StateChanals[i] == 0) {
		Helper.data.StateChanals[i] = 3;
	}
	Display.UpdateCanals(Helper.data.StateChanals, MAX_CHANALS, "canal");
}

void AquaDisplay::SetPWMCanalState(byte i) {
	Helper.data.StatePWMChanals[i]--;
	if (Helper.data.StatePWMChanals[i] == 0) {
		Helper.data.StatePWMChanals[i] = 3;
	}
	Display.UpdateCanals(Helper.data.StatePWMChanals, MAX_CHANALS_PWM, "canal_pwm");
}

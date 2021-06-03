/*
 * AquaDisplay.cpp
 *
 *  Created on: 7 мая 2021 г.
 *      Author: DrJar
 */
#include "AquaDisplay.h"

EasyNex myNex(Serial);

void AquaDisplay::Init() {
	myNex.begin(9600);
	delay(1000);
	Serial.print("page Init");
	Serial.print("\xFF\xFF\xFF");
	ClearLog();

}

void AquaDisplay::SetPage(byte page) {
	switch (page) {
	case 0:
		Serial.print("page Init");
		break;
	case 1:
		Serial.print("page Main");
		break;
	default:
		Serial.print("page Main");
		break;
	}
	Serial.print("\xFF\xFF\xFF");
}

void AquaDisplay::Update() {
	myNex.NextionListen();
	myNex.writeStr("time.txt", Helper.GetFormatTimeNow(true));
	SetData();
	SetTime();
	SetDayOfWeek();
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

void AquaDisplay::SetTemp(word temp) {
	String s_temp = "";
	byte k = temp / 100;
	byte m = temp % 100;
	if (k < 10)
		s_temp += "0";
	s_temp += k;
	s_temp += ".";
	if (m < 10) {
		s_temp += m;
	}else{
		s_temp += m/10;
	}
	myNex.writeStr("btn_temp.txt", s_temp);
}

void trigger1(){
	Helper.Tone();
}
void trigger2(){
	Helper.Tone();
}
void trigger3(){
	Helper.Tone();
}
void trigger4(){
	Helper.Tone();
}
void trigger5(){
	Helper.Tone();
}
void trigger6(){
	Helper.Tone();
}

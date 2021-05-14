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
	ClearLog();
}

void AquaDisplay::SendTime() {
	myNex.writeStr("time.txt", Helper.GetFormatTimeNow(true));
}

String AquaDisplay::GetVersion(){
	return myNex.readStr("ver.txt");
}
byte rowLog = 0;
String oldLog ="";
void AquaDisplay::SendLogLn(String log) {
	if(log.length() > 80){
		if(oldLog.length() == log.length()){
			return;
		}
		oldLog = log;
		log = log.substring(0, 80) +"...";
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
	if(log.length() > 40){
		log = log.substring(0, 40) +"...";
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
		myNex.writeNum("p1.pic", 3);
	}
	if (!isConnect && isLasConnectLAN) {
		isLasConnectLAN = false;
		myNex.writeNum("p1.pic", 2);
	}
}
bool isLasConnectWAN = false;
void AquaDisplay::SetWANConnection(bool isConnect) {
	if (isConnect && !isLasConnectWAN) {
		isLasConnectWAN = true;
		myNex.writeNum("p0.pic", 0);
	}
	if (!isConnect && isLasConnectWAN) {
		isLasConnectWAN = false;
		myNex.writeNum("p0.pic", 1);
	}

}

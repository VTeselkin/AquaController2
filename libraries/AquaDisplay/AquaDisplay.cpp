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

byte _currentPage = 0;
void AquaDisplay::SetPage(byte page) {
	Serial.print("page " + String(page));
	Serial.print("\xFF\xFF\xFF");
	Serial.flush();
	_currentPage = page;
	switch (page) {
	case 1:
		Update();
		SetTemp(NULL);
		break;
	case 3:
		UpdateCanals(Helper.data.StateChanals, MAX_CHANALS, "canal");
		break;
	case 4:
		UpdateCanals(Helper.data.StatePWMChanals, MAX_CHANALS_PWM, "canal_pwm");
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		break;
	default:
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


byte TimerNumberLed, TimerNumberDaily, TimerNumberHour, TimerNumberSecond = 0;
void AquaDisplay::SetTimerNumber(bool inc) {

	switch (_currentPage) {
	case 6:
		CheckIndexTimer(TimerNumberLed, MAX_TIMERS, inc);
		break;
	case 7:
		CheckIndexTimer(TimerNumberDaily, MAX_TIMERS, inc);
		break;
	case 8:
		CheckIndexTimer(TimerNumberHour, MAX_TIMERS, inc);
		break;
	case 9:
		CheckIndexTimer(TimerNumberSecond, MAX_TIMERS, inc);
		break;
	}



}

void AquaDisplay::SetTimerHourOn(bool inc) {
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMHourStart, HOUR, TimerNumberLed, inc);
		break;
	case 7:
		ChangeData(Helper.data.DailyTimerMinStart, HOUR, TimerNumberDaily, inc);
		break;
	case 9:
		ChangeData(Helper.data.SecondTimerHourStart, MINUTE, TimerNumberSecond, inc);
		break;
	}
}

void AquaDisplay::SetTimerMinutesOn(bool inc) {

	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMMinStart, MINUTE, TimerNumberLed, inc);
		break;
	case 7:
		ChangeData(Helper.data.DailyTimerMinStart, MINUTE, TimerNumberDaily, inc);
		break;
	case 8:
		ChangeData(Helper.data.HoursTimerMinStart, MINUTE, TimerNumberHour, inc);
		break;
	case 9:
		ChangeData(Helper.data.SecondTimerMinStart, MINUTE, TimerNumberSecond, inc);
		break;
	}

}
void AquaDisplay::SetTimerHourOff(bool inc) {
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMHourEnd, HOUR, TimerNumberHour, inc);
		break;
	case 7:
		ChangeData(Helper.data.DailyTimerHourEnd, HOUR, TimerNumberDaily, inc);
		break;
	}
}

void AquaDisplay::SetTimerMinutesOff(bool inc) {
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMMinEnd, MINUTE, TimerNumberLed, inc);
		break;
	case 7:
		ChangeData(Helper.data.DailyTimerMinEnd, MINUTE, TimerNumberDaily, inc);
		break;
	case 8:
		ChangeData(Helper.data.HoursTimerMinStop, MINUTE, TimerNumberHour, inc);
		break;
	}

}

void AquaDisplay::SetTimerDelay(bool inc) {

	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMDuration, SECONDS, TimerNumberLed, inc);
		break;
	case 9:
		ChangeData(Helper.data.TimerPWMDuration, SECONDS, TimerNumberSecond, inc);
		break;
	}

}

void AquaDisplay::SetTimerState() {

	switch (_currentPage) {
	case 6:
		ChangeDataState(Helper.data.TimerPWMState, ENABLE_TIMER, TimerNumberLed);
		break;
	case 7:
		ChangeDataState(Helper.data.DailyTimerState, ENABLE_TIMER, TimerNumberDaily);
		break;
	case 8:
		ChangeDataState(Helper.data.HoursTimerState, ENABLE_TIMER, TimerNumberHour);
		break;
	case 9:
		ChangeDataState(Helper.data.SecondTimerState, ENABLE_TIMER, TimerNumberSecond);
		break;
	}
}

void AquaDisplay::SetTimerCanal(bool inc) {
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMChanal, MAX_CHANALS_TIMER_PWM, TimerNumberLed, inc);
		break;
	case 7:
		ChangeData(Helper.data.DailyTimerChanal, MAX_CHANALS, TimerNumberDaily, inc);
		break;
	case 8:
		ChangeData(Helper.data.HoursTimerCanal, MAX_CHANALS, TimerNumberHour, inc);
		break;
	case 9:
		ChangeData(Helper.data.SecondTimerCanal, MAX_CHANALS, TimerNumberSecond, inc);
		break;
	}
}

void AquaDisplay::SetTimerLevel(bool inc) {
	switch (_currentPage) {
		case 6:
			ChangeData(Helper.data.TimerPWMLevel, MAX_PWM_LEVEL, TimerNumberLed, inc);
			break;
		}
}

void AquaDisplay::ChangeData(byte data[], byte max, byte index, bool inc) {
	if (data != NULL) {
		if (inc) {
			data[index]++;
			if (data[index] > max) {
				data[index] = 0;
			}
		} else {
			if (data[index] == 0) {
				data[index] = max;
			} else {
				data[index]--;
			}
		}
	}
}

void AquaDisplay::ChangeDataState(byte data[], byte max, byte index) {
	if (data != NULL) {
			if (data[index] == 0) {
				data[index] = max;
			} else {
				data[index] = 0;
			}
		}
}

void AquaDisplay::CheckIndexTimer(byte index, byte max, bool inc) {
	if (inc) {
		index++;
			if (index > max) {
				index = 0;
			}
		} else {

			if (index == 0) {
				index = max;
			} else {
				index--;
			}
		}
}

/*
 * AquaDisplay.cpp
 *
 *  Created on: 7 мая 2021 г.
 *      Author: DrJar
 */
#include <AquaHelper.h>
#include "AquaDisplay.h"
#include <AquaCanal.h>

EasyNex myNex(Serial);
byte TimerNumberLed, TimerNumberDaily, TimerNumberHour, TimerNumberSecond, TimerNumberTemp = 0;
bool _isNeedSave = false;
byte _currentPage = 0;

void AquaDisplay::Init() {
	myNex.begin(9600);
	delay(1000);
	SetPage(2);
	ClearLog();

}

void AquaDisplay::SetPage(byte page) {
	if(_currentPage == page) return;
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Serial.print("page " + String(page));
	Serial.print("\xFF\xFF\xFF");
	Serial.flush();
	_currentPage = page;
	switch (page) {
	case 1:
		Update();
		SetTemp(Helper.data.TempSensor[0]);
		SetPH(Helper.data.PHCurrent[0]);
		break;
	case 3:
		UpdateCanals(Helper.data.StateChanals, MAX_CHANALS, "canal");
		break;
	case 4:
		UpdateCanals(Helper.data.StatePWMChanals, MAX_CHANALS_PWM, "canal_pwm");
		break;
	case 6:
		UpdateDisplayTimersPWM();
		break;
	case 7:
		UpdateDisplayTimersDaily();
		break;
	case 8:
		UpdateDisplayTimersHourly();
		break;
	case 9:
		UpdateDisplayTimersSecond();
		break;
	case 10:
		UpdateDisplayTimersTemp();
		break;
	case 11:
		UpdateDisplaySettings();
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
int AquaDisplay::GetVersion() {
	auto ver = myNex.readStr("ver.txt");
	if (ver == "ERROR") {
		return 0;
	}
	return ver.toInt();
}
String AquaDisplay::GetType() {
	return myNex.readStr("type.txt");
}
byte rowLog = 0;
String oldLog = "";

void AquaDisplay::SendLogLnTime(String log) {
	log = "[" + Helper.GetFormatTimeNow(false) + "]" + log;
	SendLogLn(log);
}

void AquaDisplay::SendLogLn(String log) {
	if (log.length() > 72) {
		if (oldLog.length() == log.length()) {
			return;
		}
		oldLog = log;
		log = log.substring(0, 72) + "...";
	}
	rowLog++;
	if (rowLog > 22) {
		delay(2000);
		ClearLog();
		rowLog = 0;
		SendLogLn(log);
	}
	log.replace("\"", "");
	myNex.writeStr("log.txt+", "\r\n" + log);
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
	s_temp = Format04DTemp(temp, false);
	myNex.writeStr("btn_temp.txt", s_temp);
}

String s_ph = "";
void AquaDisplay::SetPH(word ph) {
	s_ph = Format04DPh(ph, false);
	myNex.writeStr("btn_ph.txt", s_ph);
}

void AquaDisplay::UpdateCanals(byte canals[], byte max_canal, String canal_name) {
	if (_currentPage != 3 && _currentPage != 4)
		return;
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

void AquaDisplay::UpdateDisplayTimersPWM() {
	if(_currentPage != 6) return;
	myNex.writeStr("t0.txt", Format02DCanal(TimerNumberLed));	//timer index
	if (Helper.data.TimerPWMState[TimerNumberLed] == ENABLE_TIMER) {	//state
		myNex.writeStr("b2.txt", "ON");
	} else {
		myNex.writeStr("b2.txt", "OFF");
	}
	myNex.writeStr("t4.txt", Format02D(Helper.data.TimerPWMHourStart[TimerNumberLed]));	//time on hour
	myNex.writeStr("t6.txt", Format02D(Helper.data.TimerPWMMinStart[TimerNumberLed]));	//timer on minute
	myNex.writeStr("t8.txt", Format02D(Helper.data.TimerPWMHourEnd[TimerNumberLed]));	//timer off hour
	myNex.writeStr("t10.txt", Format02D(Helper.data.TimerPWMMinEnd[TimerNumberLed]));	//timer off minute
	myNex.writeStr("t12.txt", Format03D(Helper.data.TimerPWMDuration[TimerNumberLed]));	//delay
	if (Helper.data.TimerPWMChanal[TimerNumberLed] >= MAX_CHANALS_TIMER_PWM) {
		myNex.writeStr("t20.txt", "F" + String(Helper.data.TimerPWMChanal[TimerNumberLed] - MAX_CHANALS_TIMER_PWM + 1));
	} else {
		myNex.writeStr("t20.txt", Format02DCanal(Helper.data.TimerPWMChanal[TimerNumberLed]));
	}
	myNex.writeStr("t14.txt", Format03D(Helper.data.TimerPWMLevel[TimerNumberLed]));	//level
}

void AquaDisplay::UpdateDisplayTimersDaily() {
	if(_currentPage != 7) return;
	myNex.writeStr("t0.txt", Format02DCanal(TimerNumberDaily));	//timer index
	if (Helper.data.DailyTimerState[TimerNumberDaily] == ENABLE_TIMER) {	//state
		myNex.writeStr("b2.txt", "ON");
	} else {
		myNex.writeStr("b2.txt", "OFF");
	}
	myNex.writeStr("t4.txt", Format02D(Helper.data.DailyTimerHourStart[TimerNumberDaily]));	//time on hour
	myNex.writeStr("t6.txt", Format02D(Helper.data.DailyTimerMinStart[TimerNumberDaily]));	//timer on minute
	myNex.writeStr("t8.txt", Format02D(Helper.data.DailyTimerHourEnd[TimerNumberDaily]));	//timer off hour
	myNex.writeStr("t10.txt", Format02D(Helper.data.DailyTimerMinEnd[TimerNumberDaily]));	//timer off minute
	myNex.writeStr("t20.txt", Format02DCanal(Helper.data.DailyTimerChanal[TimerNumberDaily]));	//canal
}

void AquaDisplay::UpdateDisplayTimersHourly() {
	if(_currentPage != 8) return;
	myNex.writeStr("t0.txt", Format02DCanal(TimerNumberHour));	//timer index
	if (Helper.data.HoursTimerState[TimerNumberHour] == ENABLE_TIMER) {	//state
		myNex.writeStr("b2.txt", "ON");
	} else {
		myNex.writeStr("b2.txt", "OFF");
	}
	myNex.writeStr("t6.txt", Format02D(Helper.data.HoursTimerMinStart[TimerNumberHour]));	//timer on minute
	myNex.writeStr("t10.txt", Format02D(Helper.data.HoursTimerMinStop[TimerNumberHour]));	//timer off minute
	myNex.writeStr("t20.txt", Format02DCanal(Helper.data.HoursTimerCanal[TimerNumberHour]));	//canal
}

void AquaDisplay::UpdateDisplayTimersSecond() {
	if(_currentPage != 9) return;
	myNex.writeStr("t0.txt", Format02DCanal(TimerNumberSecond));	//timer index
	if (Helper.data.SecondTimerState[TimerNumberSecond] == ENABLE_TIMER) {	//state
		myNex.writeStr("b2.txt", "ON");
	} else {
		myNex.writeStr("b2.txt", "OFF");
	}
	myNex.writeStr("t4.txt", Format02D(Helper.data.SecondTimerHourStart[TimerNumberSecond]));	//time on hour
	myNex.writeStr("t6.txt", Format02D(Helper.data.SecondTimerMinStart[TimerNumberSecond]));	//timer on minute
	myNex.writeStr("t12.txt", Format03D(Helper.data.SecondTimerDuration[TimerNumberSecond]));	//timer off minute
	myNex.writeStr("t20.txt", Format02DCanal(Helper.data.SecondTimerCanal[TimerNumberSecond]));	//canal
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

void AquaDisplay::UpdateDisplayTimersTemp() {
	if(_currentPage != 10) return;
	myNex.writeStr("t0.txt", Format02DCanal(TimerNumberTemp));	//timer index
	if (Helper.data.TempTimerState[TimerNumberTemp] == ENABLE_TIMER) {	//state
		myNex.writeStr("b2.txt", "ON");
	} else {
		myNex.writeStr("b2.txt", "OFF");
	}
	myNex.writeStr("t5.txt", Format04DTemp(Helper.data.TempSensor[TimerNumberTemp], true));	//canal
	myNex.writeStr("t4.txt", Format04DTemp(Helper.data.TempTimerMinStart[TimerNumberTemp], true));	//temp on start
	myNex.writeStr("t8.txt", Format04DTemp(Helper.data.TempTimerMaxEnd[TimerNumberTemp], true));	//temp on stop
	myNex.writeStr("t20.txt", Format02DCanal(Helper.data.TempTimerChanal[TimerNumberTemp]));	//canal
}

void AquaDisplay::UpdateDisplaySettings() {
	if(_currentPage != 11) return;
	if (Helper.data.auto_update == 1) {
		myNex.writeStr("b2.txt", "ON");
	} else {
		myNex.writeStr("b2.txt", "OFF");
	}
	if (Helper.data.ntp_update == 1) {
		myNex.writeStr("b0.txt", "ON");
	} else {
		myNex.writeStr("b0.txt", "OFF");
	}
	if (Helper.data.auto_connect == 1) {
		myNex.writeStr("b1.txt", "ON");
	} else {
		myNex.writeStr("b1.txt", "OFF");
	}
	if (Helper.data.isTone == 1) {
		myNex.writeStr("b3.txt", "ON");
	} else {
		myNex.writeStr("b3.txt", "OFF");
	}
}

void AquaDisplay::SetTimerNumber(bool inc) {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	switch (_currentPage) {
	case 6:
		CheckIndexTimer(TimerNumberLed, MAX_TIMERS, inc);
		UpdateDisplayTimersPWM();
		break;
	case 7:
		CheckIndexTimer(TimerNumberDaily, MAX_TIMERS, inc);
		UpdateDisplayTimersDaily();
		break;
	case 8:
		CheckIndexTimer(TimerNumberHour, MAX_TIMERS, inc);
		UpdateDisplayTimersHourly();
		break;
	case 9:
		CheckIndexTimer(TimerNumberSecond, MAX_TIMERS, inc);
		UpdateDisplayTimersSecond();
		break;
	case 10:
		CheckIndexTimer(TimerNumberTemp, MAX_TEMP_SENSOR, inc);
		UpdateDisplayTimersTemp();
		break;
	case 11:
		if (inc) {
			if (Helper.data.ntp_update == 1) {
				Helper.data.ntp_update = 0;
			} else {
				Helper.data.ntp_update = 1;
			}
		} else {
			if (Helper.data.auto_update == 1) {
				Helper.data.auto_update = 0;
			} else {
				Helper.data.auto_update = 1;
			}
		}
		UpdateDisplaySettings();
		break;
	}

}

typeResponse AquaDisplay::SetTimerHourOn(bool inc) {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	_isNeedSave = true;
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMHourStart, HOUR, TimerNumberLed, inc);
		myNex.writeStr("t4.txt", Format02D(Helper.data.TimerPWMHourStart[TimerNumberLed]));
		return PWMTIMER;
	case 7:
		ChangeData(Helper.data.DailyTimerHourStart, HOUR, TimerNumberDaily, inc);
		myNex.writeStr("t4.txt", Format02D(Helper.data.DailyTimerHourStart[TimerNumberDaily]));
		return TIMERDAY;
	case 9:
		ChangeData(Helper.data.SecondTimerHourStart, HOUR, TimerNumberSecond, inc);
		myNex.writeStr("t4.txt", Format02D(Helper.data.SecondTimerHourStart[TimerNumberSecond]));
		return TIMERHOUR;
	case 10:
		ChangeData(Helper.data.TempTimerMinStart, MAX_INDEX_TEMP, TimerNumberTemp, inc);
		myNex.writeStr("t4.txt", Format04DTemp(Helper.data.TempTimerMinStart[TimerNumberTemp], true));
		return TIMERTEMP;
	case 11:
		if (Helper.data.isTone == 1) {
			Helper.data.isTone = 0;
		} else {
			Helper.data.isTone = 1;
		}
		UpdateDisplaySettings();
		return SETTINGS;
	}
	return DEVICE;
}

typeResponse AquaDisplay::SetTimerMinutesOn(bool inc) {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	_isNeedSave = true;
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMMinStart, MINUTE, TimerNumberLed, inc);
		myNex.writeStr("t6.txt", Format02D(Helper.data.TimerPWMMinStart[TimerNumberLed]));
		return PWMTIMER;
	case 7:
		ChangeData(Helper.data.DailyTimerMinStart, MINUTE, TimerNumberDaily, inc);
		myNex.writeStr("t6.txt", Format02D(Helper.data.DailyTimerMinStart[TimerNumberDaily]));
		return TIMERDAY;
	case 8:
		ChangeData(Helper.data.HoursTimerMinStart, MINUTE, TimerNumberHour, inc);
		myNex.writeStr("t6.txt", Format02D(Helper.data.HoursTimerMinStart[TimerNumberHour]));
		return TIMERHOUR;
	case 9:
		ChangeData(Helper.data.SecondTimerMinStart, MINUTE, TimerNumberSecond, inc);
		myNex.writeStr("t6.txt", Format02D(Helper.data.SecondTimerMinStart[TimerNumberSecond]));
		return TIMERSEC;
	}
	return DEVICE;
}
typeResponse AquaDisplay::SetTimerHourOff(bool inc) {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	_isNeedSave = true;
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMHourEnd, HOUR, TimerNumberHour, inc);
		myNex.writeStr("t8.txt", Format02D(Helper.data.TimerPWMHourEnd[TimerNumberHour]));
		return PWMTIMER;
	case 7:
		ChangeData(Helper.data.DailyTimerHourEnd, HOUR, TimerNumberDaily, inc);
		myNex.writeStr("t8.txt", Format02D(Helper.data.DailyTimerHourEnd[TimerNumberDaily]));
		return TIMERDAY;
	case 10:
		ChangeData(Helper.data.TempTimerMaxEnd, MAX_INDEX_TEMP, TimerNumberTemp, inc);
		myNex.writeStr("t8.txt", Format04DTemp(Helper.data.TempTimerMaxEnd[TimerNumberTemp], true));
		return TIMERTEMP;
	}
	return DEVICE;
}

typeResponse AquaDisplay::SetTimerMinutesOff(bool inc) {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	_isNeedSave = true;
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMMinEnd, MINUTE, TimerNumberLed, inc);
		myNex.writeStr("t10.txt", Format02D(Helper.data.TimerPWMMinEnd[TimerNumberLed]));
		return PWMTIMER;
	case 7:
		ChangeData(Helper.data.DailyTimerMinEnd, MINUTE, TimerNumberDaily, inc);
		myNex.writeStr("t10.txt", Format02D(Helper.data.DailyTimerMinEnd[TimerNumberDaily]));
		return TIMERDAY;
	case 8:
		ChangeData(Helper.data.HoursTimerMinStop, MINUTE, TimerNumberHour, inc);
		myNex.writeStr("t10.txt", Format02D(Helper.data.HoursTimerMinStop[TimerNumberHour]));
		return TIMERHOUR;
	}
	return DEVICE;
}

typeResponse AquaDisplay::SetTimerDelay(bool inc) {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	_isNeedSave = true;
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMDuration, MINUTE_PWM, TimerNumberLed, inc);
		myNex.writeStr("t12.txt", Format03D(Helper.data.TimerPWMDuration[TimerNumberLed]));
		return PWMTIMER;
	case 9:
		ChangeData(Helper.data.SecondTimerDuration, SECONDS, TimerNumberSecond, inc);
		myNex.writeStr("t12.txt", Format03D(Helper.data.SecondTimerDuration[TimerNumberSecond]));
		return TIMERSEC;
	}
	return DEVICE;
}

typeResponse AquaDisplay::SetTimerState() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	_isNeedSave = true;
	switch (_currentPage) {
	case 6:
		ChangeDataState(Helper.data.TimerPWMState, ENABLE_TIMER, TimerNumberLed);
		if (Helper.data.TimerPWMState[TimerNumberLed] == ENABLE_TIMER) {
			myNex.writeStr("b2.txt", "ON");
		} else {
			myNex.writeStr("b2.txt", "OFF");
		}
		return PWMTIMER;
	case 7:
		ChangeDataState(Helper.data.DailyTimerState, ENABLE_TIMER, TimerNumberDaily);
		if (Helper.data.DailyTimerState[TimerNumberDaily] == ENABLE_TIMER) {
			myNex.writeStr("b2.txt", "ON");
		} else {
			myNex.writeStr("b2.txt", "OFF");
		}
		return TIMERDAY;
	case 8:
		ChangeDataState(Helper.data.HoursTimerState, ENABLE_TIMER, TimerNumberHour);
		if (Helper.data.HoursTimerState[TimerNumberHour] == ENABLE_TIMER) {
			myNex.writeStr("b2.txt", "ON");
		} else {
			myNex.writeStr("b2.txt", "OFF");
		}
		return TIMERHOUR;
	case 9:
		ChangeDataState(Helper.data.SecondTimerState, ENABLE_TIMER, TimerNumberSecond);
		if (Helper.data.SecondTimerState[TimerNumberSecond] == ENABLE_TIMER) {
			myNex.writeStr("b2.txt", "ON");
		} else {
			myNex.writeStr("b2.txt", "OFF");
		}
		return TIMERSEC;
	case 10:
		ChangeDataState(Helper.data.TempTimerState, ENABLE_TIMER, TimerNumberSecond);
		if (Helper.data.TempTimerState[TimerNumberSecond] == ENABLE_TIMER) {
			myNex.writeStr("b2.txt", "ON");
		} else {
			myNex.writeStr("b2.txt", "OFF");
		}
		return TIMERTEMP;
	case 11:
		if (Helper.data.auto_connect == 1) {
			Helper.data.auto_connect = 0;
		} else {
			Helper.data.auto_connect = 1;
		}
		UpdateDisplaySettings();
		return SETTINGS;

	}
	return DEVICE;
}

typeResponse AquaDisplay::SetTimerCanal(bool inc) {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	_isNeedSave = true;
	switch (_currentPage) {
	case 6:
		ChangeDataCanal(Helper.data.TimerPWMChanal, MAX_CHANALS_TIMER_PWM + MAX_CHANALS_FAN, TimerNumberLed, inc);
		if (Helper.data.TimerPWMChanal[TimerNumberLed] >= MAX_CHANALS_TIMER_PWM) {
			myNex.writeStr("t20.txt",
					"F" + String(Helper.data.TimerPWMChanal[TimerNumberLed] - MAX_CHANALS_TIMER_PWM + 1));
		} else {
			myNex.writeStr("t20.txt", Format02DCanal(Helper.data.TimerPWMChanal[TimerNumberLed]));
		}
		return PWMTIMER;
	case 7:
		ChangeDataCanal(Helper.data.DailyTimerChanal, MAX_CHANALS, TimerNumberDaily, inc);
		myNex.writeStr("t20.txt", Format02DCanal(Helper.data.DailyTimerChanal[TimerNumberDaily]));
		return TIMERDAY;
	case 8:
		ChangeDataCanal(Helper.data.HoursTimerCanal, MAX_CHANALS, TimerNumberHour, inc);
		myNex.writeStr("t20.txt", Format02DCanal(Helper.data.HoursTimerCanal[TimerNumberHour]));
		return TIMERHOUR;
	case 9:
		ChangeDataCanal(Helper.data.SecondTimerCanal, MAX_CHANALS, TimerNumberSecond, inc);
		myNex.writeStr("t20.txt", Format02DCanal(Helper.data.SecondTimerCanal[TimerNumberSecond]));
		return TIMERSEC;
	case 10:
		ChangeDataCanal(Helper.data.TempTimerChanal, MAX_CHANALS, TimerNumberTemp, inc);
		myNex.writeStr("t20.txt", Format02DCanal(Helper.data.TempTimerChanal[TimerNumberTemp]));
		return TIMERTEMP;
	}
	return DEVICE;
}

typeResponse AquaDisplay::SetTimerLevel(bool inc) {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	_isNeedSave = true;
	switch (_currentPage) {
	case 6:
		ChangeData(Helper.data.TimerPWMLevel, MAX_PWM_LEVEL, TimerNumberLed, inc);
		myNex.writeStr("t14.txt", String(Helper.data.TimerPWMLevel[TimerNumberLed]));
		return PWMTIMER;
	}
	return DEVICE;
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

void AquaDisplay::ChangeDataCanal(byte data[], byte max, byte index, bool inc) {
	if (data != NULL) {
		if (inc) {
			data[index]++;
			if (data[index] >= max) {
				data[index] = 0;
			}
		} else {
			if (data[index] <= 1) {
				data[index] = max - 1;
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

void AquaDisplay::CheckIndexTimer(byte &index, byte max, bool inc) {
	if (inc) {
		index++;
		if (index >= max) {
			index = 0;
		}
	} else {

		if (index < 1) {
			index = max - 1;
		} else {
			index--;
		}
	}

}
bool AquaDisplay::IsNeedSave() {
	return _isNeedSave;
}
void AquaDisplay::SetNeedSave(bool isNeedSave) {
	_isNeedSave = isNeedSave;
}
byte AquaDisplay::CurrentPage() {
	return _currentPage;
}

String AquaDisplay::Format03D(byte data) {
	char TextBuffer[4];
	sprintf(TextBuffer, "%03d", data);
	return String(TextBuffer);
}

String AquaDisplay::Format02D(byte data) {
	char TextBuffer[3];
	sprintf(TextBuffer, "%02d", data);
	return String(TextBuffer);
}

String AquaDisplay::Format02DCanal(byte data) {
	data++;
	char TextBuffer[3];
	sprintf(TextBuffer, "%02d", data);
	return String(TextBuffer);
}
String AquaDisplay::Format04DTemp(unsigned short temp, bool longRecord) {
	temp = temp * STEP + MIN_TEMP;
	return Format04D(temp, true, longRecord);
}
String AquaDisplay::Format04DPh(unsigned short ph, bool longRecord) {
	ph = ph * STEP_PH + MIN_PH;
	if (ph < 1000) {
		return Format04D(ph, false, true);
	} else {
		return Format04D(ph, false, false);
	}
}
String AquaDisplay::Format04D(unsigned short temp, bool longRecord, bool longRecordFloat) {

	s_temp = "";
	byte k = temp / 100;
	byte m = temp % 100;
	if (k < 10 && longRecord) {
		s_temp += "0";
	}
	s_temp += k;
	s_temp += ".";
	if (!longRecordFloat) {
		if (m < 10) {
			s_temp += m;
		} else {
			s_temp += m / 10;
		}
	} else {
		if (m < 10) {
			s_temp += "0";
		}
		s_temp += m;
	}
	return s_temp;
}

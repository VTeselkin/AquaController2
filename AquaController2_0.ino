/**
 * AquaController 2.0 - Software for AquaController device.
 * Original Copyright (c) 2017 Vadim Teselkin.
 * web: www.aquacontroller.ru
 * mail-to: Dr.Jarold@gmail.com
 * All right reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <AquaHelper.h>
#include <AquaTimers.h>
#include <AquaTemp.h>
#include <AquaStop.h>
#include <AquaCanal.h>
#include <AquaEEPROM.h>
#include <AquaAnalog.h>
#include <AquaWiFi.h>
#include <AquaFAN.h>
#include <AquaDisplay.h>

AquaTimers aquaTimers;
AquaTemp aquaTemp;
AquaStop aquaStop;
AquaCanal aquaCanal;
AquaEEPROM aquaEEPROM;
AquaAnalog aquaAnalog;
AquaWiFi aquaWiFi;
AquaFAN aquaFAN;

unsigned int _timerForCheck = 0;
unsigned int _hourTimerForCheck = 0;
unsigned int _secondTimerForCheck = 0;
unsigned int _pwmTimerForCheck = 0;
unsigned long _lastTimeUpdate = 0;
bool isNeedEnableZeroCanal = false;

void ScanI2C() {
	Display.SendLogLn("FirmWare : ");
	Display.SendLog(VERTION_FIRMWARE);
	Display.SendLogLn("Scanning I2C Addresses");
	uint8_t cnt = 0;
	String log = "";
	for (uint8_t i = 1; i < 127; i++) {
		Wire.beginTransmission(i);
		uint8_t ec = Wire.endTransmission(true);
		if (ec == 0) {
			if (i < 16) {
				log += "0";
			}

			if (sizeof(String(i, HEX)) > 0) {
				log += String(i, HEX);
				cnt++;
			}
		} else if (ec == 4) {
			log += "Er";
		} else {
			log += "..";
		}
		log += "  ";
		if ((i & 0x0f) == 0x0f) {
			Display.SendLogLn(log);
			log = "";
			delay(200);
		}
	}
	Display.SendLogLn("Scan Completed, ");
	Display.SendLogLn(String(cnt) + " I2C Devices found.");
}

void setup() {
	Display.Init();
	if (!Wire.begin())
		Display.SendLog("I2C Fail = " + Wire.lastError());
	ScanI2C();
	aquaEEPROM.Init();
	aquaTemp.Init(aquaEEPROM);
	aquaCanal.Init();
	aquaAnalog.Init();
	aquaWiFi.Init(ChangeWiFiLog, GetUDPWiFiPOSTRequest, SaveUTCSetting, ChandeDebugLED);
	Display.SetPage(1);
}

void loop() {
	aquaWiFi.wifiManager.process();
	isNeedEnableZeroCanal = aquaStop.GetTemporaryStopCanal(isNeedEnableZeroCanal, ChangeChanalState);
	aquaTimers.CheckStateTimer(_timerForCheck, TIMER_MIN, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_hourTimerForCheck, TIMER_OTHER, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_secondTimerForCheck, TIMER_SEC, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_pwmTimerForCheck, TIMER_PWM, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTemp.GetTemperature(ChangeTempState);
	aquaTemp.CheckStateTempTimer(ChangeChanalState, isNeedEnableZeroCanal);
	aquaCanal.SetStateCanal(ChangeChanalState);
	aquaCanal.SetStatePWMCanal(ChangeChanalState);
	aquaAnalog.Update();
	if (aquaAnalog.AddPhElementToStats()) {
		aquaWiFi.SendCacheResponse(PH, true, true);
	}
	if (aquaTemp.AddTempElementToStats()) {
		aquaWiFi.SendCacheResponse(TEMPSTATS, true, true);
	}
	aquaAnalog.CheckWaterLevel(ChangeWaterLevelStatus);
	aquaWiFi.WaitRequest();
	aquaCanal.DisableLED();
	if (millis() > _lastTimeUpdate + DELAY_TIME_UPDATE) {
		_lastTimeUpdate = millis();
		Display.Update();
		Display.SetTemp(aquaTemp.ConvertTempByteToWord(Helper.data.TempSensor[0]));

	}

	Display.Loop();
}

void ChangeChanalState(typeResponse type) {
	if (PWMTIMER) {
		aquaWiFi.SendCacheResponse(CANAL, true, true);
	} else {
		aquaWiFi.SendCacheResponse(type, true, true);
	}
}

void ChangeTempState(typeResponse type) {
	aquaWiFi.SendCacheResponse(type, true, true);
}

void ChangeWaterLevelStatus(bool warning, byte canal) {
	if (warning) {
		Helper.Tone();
	}
}

static void ChandeDebugLED(typeDebugLED led, typeLightLED type) {
	switch (led) {
	case RXLED:
		aquaCanal.SetLEDRx(type);
		break;
	case TXLED:
		aquaCanal.SetLEDTx(type);
		break;
	case WIFILED:
		aquaCanal.SetLEDConnect(type);
		break;
	case ERRLED:
		aquaCanal.SetLEDError(type);
		break;
	}
}
void ChangeWiFiLog(String log) {
	auto log2 = "[" + Helper.GetFormatTimeNow(false) + "]" + log;
	Display.SendLogLn(log2);

}

void GetUDPWiFiPOSTRequest(typeResponse type, String json) {
	if (Helper.SetPostRequest(json, SetPHSensorConfig)) {
		aquaCanal.SetStateCanal(ChangeChanalState);
		aquaCanal.SetStatePWMCanal(ChangeChanalState);
		switch (type) {
		case CANAL:
			aquaEEPROM.SaveChanalState();
			Display.UpdateCanals(Helper.data.StateChanals, MAX_CHANALS, "canal");
			break;
		case TIMERDAY:
			aquaEEPROM.SaveDailyTimerToERROM();
			break;
		case TIMERHOUR:
			aquaEEPROM.SaveHoursTimerToERROM();
			break;
		case TIMERSEC:
			aquaEEPROM.SaveSecondsTimerToERROM();
			break;
		case TIMERTEMP:
			aquaEEPROM.SaveTempTimerToERROM();
			break;
		case PHTIMER:
			aquaEEPROM.SavePHTimerToERROM();
			break;
		case TEMPSTATS:
			break;
		case DEVICE:
			break;
		case TEMPSENSOR:
			break;
		case PH:
			break;
		case PWMTIMER:
			aquaEEPROM.SavePWMTimerToERROM();
			break;
		case PWMCANAL:
			aquaEEPROM.SavePWMChanalState();
			Display.UpdateCanals(Helper.data.StatePWMChanals, MAX_CHANALS_PWM, "canal_pwm");
			break;
		case SETTINGS:
			aquaEEPROM.SaveWifiSettings();
			type = DEVICE;
			break;
		case FAN:
			aquaEEPROM.SaveFANSettings();
			break;
		}
		aquaWiFi.SendCacheResponse(type, true, false);
	} else {
		Serial.println("ERROR POST UDP");
	}
}

void SetPHSensorConfig() {
	for (byte i = 0; i < MAX_TIMERS_PH; i++) {
		if (Helper.data.PHTimer686[i] == 0) {
			Serial.print("CheckPhLevel PHTimer686 canal = ");
			Serial.println(i);
			auto level = aquaAnalog.CheckPhLevel(i);
			if (level == 0)
				level = 1;
			Helper.data.PHTimer686[i] = level;
		}
		if (Helper.data.PHTimer401[i] == 0) {
			Serial.print("CheckPhLevel PHTimer401 canal = ");
			Serial.println(i);
			auto level = aquaAnalog.CheckPhLevel(i);
			if (level == 0)
				level = 1;
			Helper.data.PHTimer401[i] = level;
		}
	}
}

uint16_t SaveUTCSetting(uint16_t utc) {
	return aquaEEPROM.SaveUTCSetting(utc);
}

//Menu LED
void trigger1() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetPage(4);
}

//Menu CANAL
void trigger2() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetPage(3);
}

//Menu TIMERS
void trigger3() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetPage(5);
}

//Menu SETTINGS
void trigger4() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
}

//Menu TIME
void trigger5() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
}

//Menu DATA
void trigger6() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
}

//Menu CANAL_1
void trigger7() {
	ChangeStateCanalFromDisplay(0);
}

//Menu CANAL_2
void trigger8() {
	ChangeStateCanalFromDisplay(1);
}

//Menu CANAL_3
void trigger9() {
	ChangeStateCanalFromDisplay(2);
}

//Menu CANAL_4
void trigger10() {
	ChangeStateCanalFromDisplay(3);
}

//Menu CANAL_5
void trigger11() {
	ChangeStateCanalFromDisplay(4);
}

//Menu CANAL_6
void trigger12() {
	ChangeStateCanalFromDisplay(5);
}

//Menu CANAL_7
void trigger13() {
	ChangeStateCanalFromDisplay(6);
}

//Menu CANAL_8
void trigger14() {
	ChangeStateCanalFromDisplay(7);
}

void ChangeStateCanalFromDisplay(byte index) {
	Display.SetCanalState(index);
	aquaEEPROM.SaveChanalState();
	aquaWiFi.SendCacheResponse(CANAL, true, true);
}

void ChangeStatePWMCanalFromDisplay(byte index) {
	Display.SetPWMCanalState(index);
	aquaEEPROM.SavePWMChanalState();
	aquaWiFi.SendCacheResponse(PWMCANAL, true, true);
}
//Menu BACK
void trigger15() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	if (Display.IsNeedSave()) {
		switch (Display.CurrentPage()) {
		case 6:
			aquaEEPROM.SavePWMTimerToERROM();
			break;
		case 7:
			aquaEEPROM.SaveDailyTimerToERROM();
			break;
		case 8:
			aquaEEPROM.SaveHoursTimerToERROM();
			break;
		case 9:
			aquaEEPROM.SaveSecondsTimerToERROM();
			break;
		}
	}
	Display.SetNeedSave(false);
	Display.SetPage(1);
}

void trigger16() {
	ChangeStatePWMCanalFromDisplay(0);
}
void trigger17() {
	ChangeStatePWMCanalFromDisplay(1);
}
void trigger18() {
	ChangeStatePWMCanalFromDisplay(2);
}
void trigger19() {
	ChangeStatePWMCanalFromDisplay(3);
}
void trigger20() {
	ChangeStatePWMCanalFromDisplay(4);
}
void trigger21() {
	ChangeStatePWMCanalFromDisplay(5);
}
void trigger22() {
	ChangeStatePWMCanalFromDisplay(6);
}
void trigger23() {
	ChangeStatePWMCanalFromDisplay(7);
}
void trigger24() {
	ChangeStatePWMCanalFromDisplay(8);
}
void trigger25() {
	ChangeStatePWMCanalFromDisplay(9);
}
//Timers LED
void trigger26() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetPage(6);
	Display.UpdateDisplayTimersPWM();
}
//Timers DAILY
void trigger27() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetPage(7);
	Display.UpdateDisplayTimersDaily();
}
//Timers HOURLY
void trigger28() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetPage(8);
	Display.UpdateDisplayTimersHourly();
}
//Timer SECOND
void trigger29() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetPage(9);
	Display.UpdateDisplayTimersSecond();
}
// Timers - Timer Number <
void trigger30() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetTimerNumber(false);
}
// Timers - Timer Number >
void trigger31() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	Display.SetTimerNumber(true);
}
// Timers - Timer Hour ON <
void trigger32() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	auto type = Display.SetTimerHourOn(false);
	Serial.println(type);
	aquaWiFi.SendCacheResponse(type, true, true);
}
// Timers - Timer Hour ON >
void trigger33() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerHourOn(true), true, true);

}
// Timers - Timer Hour OFF <
void trigger34() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerHourOff(false), true, true);
}
// Timers - Timer Hour OFF >
void trigger35() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerHourOff(true), true, true);
}
// Timers - Timer Delay <
void trigger36() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerDelay(false), true, true);
}
// Timers - Timer Delay >
void trigger37() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerDelay(true), true, true);
}
// Timers - Timer State
void trigger38() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerState(), true, true);
}
// Timers - Timer Minutes ON <
void trigger39() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerMinutesOn(false), true, true);
}
// Timers - Timer Minutes ON >
void trigger40() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerMinutesOn(true), true, true);
}
// Timers - Timer Minutes OFF <
void trigger41() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerMinutesOff(false), true, true);
}
// Timers - Timer Minutes OFF >
void trigger42() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerMinutesOff(true), true, true);
}
// Timers - Timer Canal <
void trigger43() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerCanal(false), true, true);
}
// Timers - Timer Canal >
void trigger44() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerCanal(true), true, true);
}
// Timers - Timer Level <
void trigger45() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerLevel(false), true, true);
}
// Timers - Timer Level >
void trigger46() {
	Helper.Tone();
	Canal.SetLEDRx(LONG);
	aquaWiFi.SendCacheResponse(Display.SetTimerLevel(true), true, true);
}

void trigger47() {
}

void trigger48() {
}

void trigger49() {
}

void trigger50() {
}



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
#include <esp_task_wdt.h>

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
//#include <EasyBuzzer.h>


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
unsigned int _phTimerForCheck = 0;
unsigned long _lastTimeUpdate = 0;
bool isNeedEnableZeroCanal = false;

void ScanI2C() {

	Display.SendLogLnTime("FirmWare : ");
	Display.SendLog(String(VERTION_FIRMWARE));
	Display.SendLogLnTime("Auto-detected Flash size : ");
	Display.SendLog(String(Helper.ChipSize()));
	Display.SendLog("M");
	Display.SendLogLnTime("Scanning I2C Addresses");
	uint8_t cnt = 0;
	String log = "";
	for (byte i = 1; i < 127; i++) {
		Wire.beginTransmission(i);
		uint8_t ec = Wire.endTransmission(true);
		if (ec == 0) {
			if (i < 16) {
				log += "0";
			}

			if (sizeof(String(i, HEX)) > 0) {
				Helper.data.I2C_ADRESS[cnt] = i;
				log += String(i, HEX);
				cnt++;
			}
		} else if (ec == 4) {
			log += "Er";
		} else {
			log += "..";
		}
		log += "  ";
		if (i > 8 && (i % 16 == 0)) {
			Display.SendLogLn(log);
			log = "";
			delay(200);
		}
	}
	Display.SendLogLnTime("Scan Completed, ");
	Display.SendLogLnTime(String(cnt) + " I2C Devices found.");
}

void setup() {
	esp_task_wdt_init(WDT_TIMEOUT, true);
	esp_task_wdt_add(NULL);
	EasyBuzzer.setPin(TONE_PIN);
	Display.Init();
	Display.SetPage(2);
	if (!Wire.begin())
		Display.SendLogLnTime("I2C Fail = " + Wire.lastError());
	ScanI2C();
	aquaEEPROM.Init();
	aquaTemp.Init(aquaEEPROM);
	aquaCanal.Init();
	aquaCanal.SetLEDError(SHORT);
	aquaAnalog.Init();
	aquaWiFi.Init(Display.SendLogLnTime, GetUDPWiFiPOSTRequest, SaveUTCSetting, ChandeDebugLED);
	delay(2000);
	Display.SetPage(1);
}

void loop() {
	esp_task_wdt_reset();
	aquaWiFi.wifiManager.process();
	isNeedEnableZeroCanal = aquaStop.GetTemporaryStopCanal(isNeedEnableZeroCanal, ChangeChanalState);
	aquaTimers.CheckStateTimer(_timerForCheck, TIMER_MIN, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_hourTimerForCheck, TIMER_OTHER, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_secondTimerForCheck, TIMER_SEC, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_pwmTimerForCheck, TIMER_PWM, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_phTimerForCheck, TIMER_PH, ChangeChanalState, isNeedEnableZeroCanal);
	aquaTemp.GetTemperature(ChangeTempState);
	aquaTemp.CheckStateTempTimer(ChangeChanalState, isNeedEnableZeroCanal);
	aquaCanal.SetStateCanal(ChangeChanalState);
	aquaCanal.SetStatePWMCanal(ChangeChanalState);
	aquaAnalog.Update();
	if (aquaAnalog.AddPhElementToStats()) {
		aquaWiFi.SendCacheResponse(PH, true, true); //send current PH value
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
		Display.SetTemp(Helper.data.TempSensor[0]);
		Display.SetPH(Helper.data.PHCurrent[0]);
	}
	EasyBuzzer.update();
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
			Display.UpdateDisplayTimersDaily();
			break;
		case TIMERHOUR:
			aquaEEPROM.SaveHoursTimerToERROM();
			Display.UpdateDisplayTimersHourly();
			break;
		case TIMERSEC:
			aquaEEPROM.SaveSecondsTimerToERROM();
			Display.UpdateDisplayTimersSecond();
			break;
		case TIMERTEMP:
			aquaEEPROM.SaveTempTimerToERROM();
			Display.UpdateDisplayTimersTemp();
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
			Display.UpdateDisplayTimersPWM();
			break;
		case PWMCANAL:
			aquaEEPROM.SavePWMChanalState();
			Display.UpdateCanals(Helper.data.StatePWMChanals, MAX_CHANALS_PWM, "canal_pwm");
			break;
		case SETTINGS:
			aquaEEPROM.SaveWifiSettings();
			aquaEEPROM.SaveLcdSetings();
			Display.UpdateDisplaySettings();
			type = DEVICE;
			break;
		case FAN:
			aquaEEPROM.SaveFANSettings();
			break;
		case NTP:
			break;
		case PH_SETTINGS:
			aquaEEPROM.SavePHTimerToERROM();
			break;
		}
		aquaWiFi.SendCacheResponse(type, true, false);
	} else {
		if (type == NTP || type == PH_SETTINGS) {
			Display.Update();
			return;
		}
		Serial.println("ERROR POST UDP");
	}
}

void SetPHSensorConfig(byte index, byte point, int value) {
		word voltage = aquaAnalog.CheckPhVoltage(index) * 100;
		Helper.data.PHCalibrationValue[point + 2 * index] = value;
		Helper.data.PHCalibrationVoltage[point + 2 * index] = voltage;
}

uint16_t SaveUTCSetting(uint16_t utc) {
	return aquaEEPROM.SaveUTCSetting(utc);
}

//Menu LED
void trigger1() {
	Display.SetPage(4);
}

//Menu CANAL
void trigger2() {
	Display.SetPage(3);
}

//Menu TIMERS
void trigger3() {
	Display.SetPage(5);
}

//Menu SETTINGS
void trigger4() {
	Display.SetPage(11);
}

//Menu TIME
void trigger5() {
	Display.SetPage(10);
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
		case 10:
			aquaEEPROM.SaveTempTimerToERROM();
			break;
		case 11:
			aquaEEPROM.SaveWifiSettings();
			aquaEEPROM.SaveLcdSetings();
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
	Display.SetPage(6);
}
//Timers DAILY
void trigger27() {
	Display.SetPage(7);
}
//Timers HOURLY
void trigger28() {
	Display.SetPage(8);
}
//Timer SECOND
void trigger29() {
	Display.SetPage(9);
}
// Timers - Timer Number <
void trigger30() {
	Display.SetTimerNumber(false);
}
// Timers - Timer Number >
void trigger31() {
	Display.SetTimerNumber(true);
}
// Timers - Timer Hour ON <
void trigger32() {
	aquaWiFi.SendCacheResponse(Display.SetTimerHourOn(false), true, true);
}
// Timers - Timer Hour ON >
void trigger33() {
	aquaWiFi.SendCacheResponse(Display.SetTimerHourOn(true), true, true);
}
// Timers - Timer Hour OFF <
void trigger34() {
	aquaWiFi.SendCacheResponse(Display.SetTimerHourOff(false), true, true);
}
// Timers - Timer Hour OFF >
void trigger35() {
	aquaWiFi.SendCacheResponse(Display.SetTimerHourOff(true), true, true);
}
// Timers - Timer Delay <
void trigger36() {
	aquaWiFi.SendCacheResponse(Display.SetTimerDelay(false), true, true);
}
// Timers - Timer Delay >
void trigger37() {
	aquaWiFi.SendCacheResponse(Display.SetTimerDelay(true), true, true);
}
// Timers - Timer State
void trigger38() {
	aquaWiFi.SendCacheResponse(Display.SetTimerState(), true, true);
}
// Timers - Timer Minutes ON <
void trigger39() {
	aquaWiFi.SendCacheResponse(Display.SetTimerMinutesOn(false), true, true);
}
// Timers - Timer Minutes ON >
void trigger40() {
	aquaWiFi.SendCacheResponse(Display.SetTimerMinutesOn(true), true, true);
}
// Timers - Timer Minutes OFF <
void trigger41() {
	aquaWiFi.SendCacheResponse(Display.SetTimerMinutesOff(false), true, true);
}
// Timers - Timer Minutes OFF >
void trigger42() {
	aquaWiFi.SendCacheResponse(Display.SetTimerMinutesOff(true), true, true);
}
// Timers - Timer Canal <
void trigger43() {
	aquaWiFi.SendCacheResponse(Display.SetTimerCanal(false), true, true);
}
// Timers - Timer Canal >
void trigger44() {
	aquaWiFi.SendCacheResponse(Display.SetTimerCanal(true), true, true);
}
// Timers - Timer Level <
void trigger45() {
	aquaWiFi.SendCacheResponse(Display.SetTimerLevel(false), true, true);
}
// Timers - Timer Level >
void trigger46() {
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


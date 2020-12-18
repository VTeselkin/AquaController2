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

#include <AquaTimers.h>
#include <AquaTemp.h>
#include <AquaHelper.h>
#include <AquaStop.h>
#include <AquaCanal.h>
#include <AquaEEPROM.h>
#include <AquaAnalog.h>
#include <AquaWiFi.h>
#include <AquaFAN.h>

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

bool isNeedEnableZeroCanal = false;

void setup() {
	Serial.begin(76800);
	if (!Wire.begin(I2C_SDA, I2C_SCL))
		Serial.printf("I2C Fail = %d/n", Wire.lastError());

	Helper.ScanI2C();
	aquaEEPROM.Init();
	aquaTemp.Init(aquaEEPROM);
	aquaCanal.Init();
	aquaAnalog.Init();
	aquaWiFi.Init(ChangeWiFiLog, GetUDPWiFiPOSTRequest, SaveUTCSetting, ChandeDebugLED);
	Helper.ToneForce(2000, 500);

}

void loop() {
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
		aquaWiFi.SendCacheResponse(PH, true);
	}
	if (aquaTemp.AddTempElementToStats()) {
		aquaWiFi.SendCacheResponse(TEMPSTATS, true);
	}
	aquaAnalog.CheckWaterLevel(ChangeWaterLevelStatus);
	aquaWiFi.WaitRequest();
	aquaCanal.DisableLED();
}

void ChangeChanalState(typeResponse type) {
	if (PWMTIMER) {
		aquaWiFi.SendCacheResponse(CANAL, true);
	} else {
		aquaWiFi.SendCacheResponse(type, true);
	}
}

void ChangeTempState(typeResponse type) {
	aquaWiFi.SendCacheResponse(type, true);
}

void ChangeWaterLevelStatus(bool warning, byte canal) {
	if (warning) {
		Helper.Tone();
		//send to UDP
	}
}

void ChandeDebugLED(typeDebugLED led, typeLightLED type) {
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
	Serial.print("[");
	Serial.print(Helper.GetFormatTimeNow());
	Serial.print("]");
	Serial.println(log);
}

void GetUDPWiFiPOSTRequest(typeResponse type, String json) {
	if (Helper.SetPostRequest(json, SetPHSensorConfig)) {
		aquaCanal.SetStateCanal(ChangeChanalState);
		aquaCanal.SetStatePWMCanal(ChangeChanalState);
		switch (type) {
		case CANAL:
			aquaEEPROM.SaveChanalState();
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
			break;
		case SETTINGS:
			aquaEEPROM.SaveWifiSettings();
			type = DEVICE;
			break;
		case FAN:
			aquaEEPROM.SaveFANSettings();
			break;
		}
		aquaWiFi.SendCacheResponse(type, true);
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

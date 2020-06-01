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

AquaTimers aquaTimers;
AquaTemp aquaTemp;
AquaStop aquaStop;
AquaCanal aquaCanal;
AquaEEPROM aquaEEPROM;
AquaAnalog aquaAnalog;
AquaWiFi aquaWiFi;

unsigned int _timerForCheck = 0;
unsigned int _hourTimerForCheck = 0;
unsigned int _secondTimerForCheck = 0;
unsigned int _pwmTimerForCheck = 0;
String LastMessage = "";

bool isNeedEnableZeroCanal = false;

void setup() {
	Serial.begin(115200);
	Wire.begin(I2C_SDA, I2C_SCL, I2C_CLOCK);                       // Wire must be started first
	aquaEEPROM.Init();
	aquaCanal.Init();
	aquaTemp.Init(aquaEEPROM);
	aquaAnalog.Init();
	aquaWiFi.Init(ChangeWiFiLog, GetUDPWiFiPOSTRequest, SaveUTCSetting);


			for (byte address = 10; address <= 127; address++) {
				Wire.beginTransmission(address);
				byte error = Wire.endTransmission();
				if (error == 0) {
					Serial.println(address, HEX);
				}
			}


}

void loop() {
	isNeedEnableZeroCanal = aquaStop.GetTemporaryStopCanal(
			isNeedEnableZeroCanal, ChangeChanalState);
	aquaTimers.CheckStateTimer(_timerForCheck, TIMER_MIN, ChangeChanalState,
			isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_hourTimerForCheck, TIMER_OTHER,
			ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_secondTimerForCheck, TIMER_SEC,
			ChangeChanalState, isNeedEnableZeroCanal);
	aquaTimers.CheckStateTimer(_pwmTimerForCheck, TIMER_PWM, ChangeChanalState,
			isNeedEnableZeroCanal);
	aquaTemp.GetTemperature();
	aquaTemp.CheckStateTempTimer(ChangeChanalState, isNeedEnableZeroCanal);
	aquaCanal.SetStateCanal(ChangeChanalState);
	aquaCanal.SetStatePWMCanal(ChangeChanalState);
	aquaAnalog.Update();
	if (aquaAnalog.AddPhElementToStats()) {
		aquaWiFi.CacheResponse(PH, Helper.GetPhStats());
	}
	if (aquaTemp.AddTempElementToStats()) {
		aquaWiFi.CacheResponse(TEMPSTATS, Helper.GetTempStats());
	}
	aquaAnalog.CheckWaterLevel(ChangeWaterLevelStatus);
	aquaWiFi.WaitRequest();
}

void ChangeChanalState(String str) {
	Serial.println(str);
}

void ChangeWaterLevelStatus(bool warning, byte canal) {
	if (warning) {
		Helper.Tone();
		//send to UDP
	}
}

void ChangeWiFiLog(String log) {
	Serial.println(log);
	LastMessage = log;
}

void GetUDPWiFiPOSTRequest(typeResponse type, String json) {
	if (Helper.SetPostRequest(json, SetPHSensorConfig)) {
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
		}
	}
}

void SetPHSensorConfig(bool ph_686, byte sensor) {
	if (ph_686) {
		Helper.data.PHTimer686[sensor] = aquaAnalog.CheckPhLevel(sensor);
	} else {
		Helper.data.PHTimer401[sensor] = aquaAnalog.CheckPhLevel(sensor);
	}
}
uint16_t SaveUTCSetting(uint16_t utc) {
	return aquaEEPROM.SaveUTCSetting(utc);
}

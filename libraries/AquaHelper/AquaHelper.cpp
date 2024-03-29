/*
 * AquaHelper.cpp
 *
 *  Created on: Oct 2, 2019
 *      Author: doc
 */
#include "AquaHelper.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <locale>
#include <SPIFFS.h>
#include <FS.h>
#include <vector>

dataController AquaHelper::data;
AquaHelper Helper = AquaHelper();
RTClib RTC;
DS3231 ds3231;

const byte tonePin = 4;

/**
 * Method of signaling through the system speaker
 */
void AquaHelper::Tone(const word frequency, const word duration) {
	if (data.isTone == 1) {
		ESP_tone(frequency, duration);
	}
}

void AquaHelper::Tone() {
	if (data.isTone == 1) {
		ESP_tone(3000, 1000);
	}
}

void AquaHelper::ToneForce(const word frequency, const word onDuration) {
	ESP_tone(frequency, onDuration);
}

void AquaHelper::ESP_tone(unsigned int frequency, unsigned int const onDuration) {
	ledcAttachPin(TONE_PIN, BUZZER_CHANNEL);
	ledcWriteTone(BUZZER_CHANNEL, 2000);
	delay(100);
	ledcDetachPin(TONE_PIN);
}

void AquaHelper::ESP_noTone() {

}

String SendStartMess() {
	return "{\"status\":\"success\",\"message\":\"";
}

String SendEndMess() {
	return "}}";
}
//============================================GET===============================================
AquaHelper::AquaHelper() {
	 ledcSetup(BUZZER_CHANNEL, 2000, 8);

}

String AquaHelper::GetDevice(String ip) {
	String result = SendStartMess();
	result += "dev\",\"data\":{\"ver\":\"AQ_V2_ESP32\",\"firm\":\"";
	result += VERTION_FIRMWARE;
	result += "\",\"update\":";
	result += data.auto_update;
	result += ",\"ntp\":";
	result += data.ntp_update;
	result += ",\"sound\":";
	result += data.isTone;
	result += ",\"debug\":";
	result += data.debug;
	result += ",\"ip\":\"";
	result += ip;
	result += "\",\"m_t\":10,\"m_t_se\":4,\"min_t\":";
	result += MIN_TEMP;
	result += ",\"max_t\":";
	result += MAX_TEMP;
	result += ",\"time\":\"";
	result += GetFormatTimeNow(false);
	result += "\"";
	result += SendEndMess();
	return result;
}

String AquaHelper::GetDataTime() {
	String result = "{\"status\":\"success\",\"";
	result += "time\":\"";
	result += GetFormatTimeNow(false);
	result += "\",\"date\":\"";
	result += GetFormatDataNow();
	result += "\"}";
	return result;
}

String AquaHelper::GetFormatTimeNow(bool isShort) {
	tmElements_t tm = GetTimeNow();
	String time_fm = "";
	if (tm.Hour < 10)
		time_fm += "0";
	time_fm += String(tm.Hour) + ":";
	if (tm.Minute < 10)
		time_fm += "0";
	time_fm += String(tm.Minute);
	if (isShort) {
		return time_fm;
	}
	time_fm += ":";
	if (tm.Second < 10)
		time_fm += "0";
	time_fm += String(tm.Second);
	return time_fm;
}

String AquaHelper::GetFormatDataNow() {
	tmElements_t tm = GetTimeNow();
	String data_fm = "";
	if (tm.Day < 10)
		data_fm += "0";
	data_fm += String(tm.Day) + "/";
	if (tm.Month < 10)
		data_fm += "0";
	data_fm += String(tm.Month);
	data_fm += "/";
	data_fm += String(y2kYearToTm(tmYearToCalendar(tm.Year)));
	return data_fm;
}

/**
 {
 "status": "success",
 "message": "c_s",
 "data": {
 "cl": [2, 1, 1, 1, 1, 1, 1, 1],
 "c_t": [2, 3, 3, 3, 3, 3, 3, 3]
 }
 }
 */
String AquaHelper::GetChanalState() {
	String result = SendStartMess();
	result += "c_s\",\"data\":{\"cl\"";
	result += GetJsonValue(data.CurrentStateChanalsByTypeTimer, MAX_CHANALS);
	result += ",\"c_t\"";
	result += GetJsonValue(data.StateChanals, MAX_CHANALS);
	result += SendEndMess();
	return result;
}

/**
 {
 "status": "success",
 "message": "pwm_c_s",
 "data": {
 "pwm_cl": [2, 1, 1, 1, 1, 1, 1, 1],
 "pwm_c_t": [2, 3, 3, 3, 3, 3, 3, 3]
 }
 }
 */
String AquaHelper::GetChanalPWMState() {
	String result = SendStartMess();
	result += "pwm_cs\",\"data\":{\"pwm_cl\"";
	result += GetJsonValue(data.CurrentStatePWMChanalsByTypeTimer, MAX_CHANALS_PWM);
	result += ",\"pwm_ct\"";
	result += GetJsonValue(data.StatePWMChanals, MAX_CHANALS_PWM);
	result += SendEndMess();
	return result;
}

/**
 {
 "status": "success",
 "message": "td_s",
 "data": {
 "dt_h_s": [0, 12, 12, 0, 0, 0, 0, 0, 0, 0],
 "dt_h_end": [0, 20, 21, 0, 0, 0, 0, 0, 0, 0],
 "dt_m_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "dt_m_e": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "dt_s": [0, 1, 1, 0, 0, 0, 0, 0, 0, 0],
 "dt_c": [0, 1, 2, 0, 0, 0, 0, 0, 0, 0]
 }
 }
 */
String AquaHelper::GetDailyTimerState() {
	String result = SendStartMess();
	result += "td_s\",\"data\":{\"dt_h_s\"";
	result += GetJsonValue(data.DailyTimerHourStart, MAX_TIMERS);
	result += ",\"dt_h_end\"";
	result += GetJsonValue(data.DailyTimerHourEnd, MAX_TIMERS);
	result += ",\"dt_m_s\"";
	result += GetJsonValue(data.DailyTimerMinStart, MAX_TIMERS);
	result += ",\"dt_m_e\"";
	result += GetJsonValue(data.DailyTimerMinEnd, MAX_TIMERS);
	result += ",\"dt_s\"";
	result += GetJsonValue(data.DailyTimerState, MAX_TIMERS);
	result += ",\"dt_c\"";
	result += GetJsonValue(data.DailyTimerChanal, MAX_TIMERS);
	result += SendEndMess();
	return result;
}

/**
 {
 "status": "success",
 "message": "pwm_timer",
 "data": {
 "pwm_h_s": [0, 12, 12, 0, 0, 0, 0, 0, 0, 0],
 "pwm_h_end": [0, 20, 21, 0, 0, 0, 0, 0, 0, 0],
 "pwm_m_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "pwm_m_e": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "pwm_s": [0, 1, 1, 0, 0, 0, 0, 0, 0, 0],
 "pwm_c": [0, 1, 2, 0, 0, 0, 0, 0, 0, 0],
 "pwm_delay": [0, 1, 2, 0, 0, 0, 0, 0, 0, 0]
 }
 }
 */
String AquaHelper::GetPWMTimerState() {
	String result = SendStartMess();
	result += "pwm_timer\",\"data\":{\"pwm_h_s\"";
	result += GetJsonValue(data.TimerPWMHourStart, MAX_TIMERS);
	result += ",\"pwm_h_end\"";
	result += GetJsonValue(data.TimerPWMHourEnd, MAX_TIMERS);
	result += ",\"pwm_m_s\"";
	result += GetJsonValue(data.TimerPWMMinStart, MAX_TIMERS);
	result += ",\"pwm_m_e\"";
	result += GetJsonValue(data.TimerPWMMinEnd, MAX_TIMERS);
	result += ",\"pwm_s\"";
	result += GetJsonValue(data.TimerPWMState, MAX_TIMERS);
	result += ",\"pwm_c\"";
	result += GetJsonValue(data.TimerPWMChanal, MAX_TIMERS);
	result += ",\"pwm_delay\"";
	result += GetJsonValue(data.TimerPWMDuration, MAX_TIMERS);
	result += ",\"pwm_level\"";
	result += GetJsonValue(data.TimerPWMLevel, MAX_TIMERS);
	result += SendEndMess();
	return result;
}

/**
 {
 "status": "success",
 "message": "th_s",
 "data": {
 "ht_m_st": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "ht_m_sp": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "ht_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "ht_c": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
 }
 }
 */
String AquaHelper::GetHoursTimerState() {
	String result = SendStartMess();
	result += "th_s\",\"data\":{\"ht_m_st\"";
	result += GetJsonValue(data.HoursTimerMinStart, MAX_TIMERS);
	result += ",\"ht_m_sp\"";
	result += GetJsonValue(data.HoursTimerMinStop, MAX_TIMERS);
	result += ",\"ht_s\"";
	result += GetJsonValue(data.HoursTimerState, MAX_TIMERS);
	result += ",\"ht_c\"";
	result += GetJsonValue(data.HoursTimerCanal, MAX_TIMERS);
	result += SendEndMess();
	return result;
}

/**
 {
 "status": "success",
 "message": "ts_s",
 "data": {
 "st_h_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "st_m_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "st_d": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "st_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
 "st_c": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
 }
 }
 */
String AquaHelper::GetSecondsTimerState() {
	String result = SendStartMess();
	result += "ts_s\",\"data\":{\"st_h_s\"";
	result += GetJsonValue(data.SecondTimerHourStart, MAX_TIMERS);
	result += ",\"st_m_s\"";
	result += GetJsonValue(data.SecondTimerMinStart, MAX_TIMERS);
	result += ",\"st_d\"";
	result += GetJsonValue(data.SecondTimerDuration, MAX_TIMERS);
	result += ",\"st_s\"";
	result += GetJsonValue(data.SecondTimerState, MAX_TIMERS);
	result += ",\"st_c\"";
	result += GetJsonValue(data.SecondTimerCanal, MAX_TIMERS);
	result += SendEndMess();
	return result;
}

/**
 {"status":"success","message":"settings","NTP": 1,"AUTO": 1, "MAX_TIMERS": 8, "MAX_TEMP_SENSOR": 4}
 */
String AquaHelper::GetWiFiSettings() {
	String result = SendStartMess();
	result += "settings\",\"ntp\":";
	result += data.ntp_update;
	result += ",\"update\":";
	result += data.auto_connect;
	result += ",\"TIMERS\":";
	result += MAX_TIMERS;
	result += ",\"SENSOR\":";
	result += MAX_TEMP_SENSOR;
	result += "}";
	return result;
}

/**
 {
 "status": "success",
 "message": "te_s",
 "data": {
 "tt_s": [0, 0, 0, 0],
 "tt_m_s": [0, 0, 0, 0],
 "tt_m_e": [76, 76, 76, 76],
 "tt_c": [1, 0, 0, 0]
 }
 }
 */
String AquaHelper::GetTempState() {
	String result = SendStartMess();
	result += "te_s\",\"data\":{\"tt_s\"";
	result += GetJsonValue(data.TempTimerState, MAX_TEMP_SENSOR);
	result += ",\"tt_m_s\"";
	result += GetJsonValue(data.TempTimerMinStart, MAX_TEMP_SENSOR);
	result += ",\"tt_m_e\"";
	result += GetJsonValue(data.TempTimerMaxEnd, MAX_TEMP_SENSOR);
	result += ",\"tt_c\"";
	result += GetJsonValue(data.TempTimerChanal, MAX_TEMP_SENSOR);
	result += SendEndMess();
	return result;
}

/**
 {
 "status": "success",
 "message": "t_sen",
 "data": {
 "t_se": [4, 3, 0, 0]
 }
 }
 */
String AquaHelper::GetRealTemp() {
	String result = SendStartMess();
	result += "t_sen\",\"data\":{\"t_se\"";
	result += GetJsonValue(data.TempSensor, MAX_TEMP_SENSOR);
	result += SendEndMess();
	return result;
}

String AquaHelper::GetFANTemp() {
	String result = SendStartMess();
	result += "t_fan\",\"data\":{\"t_fan_s\"";
	result += GetJsonValue(data.FANTimerMinStart, MAX_CHANALS_FAN);
	result += ",\"t_fan_e\"";
	result += GetJsonValue(data.FANTimerMaxEnd, MAX_CHANALS_FAN);
	result += ",\"t_fan_se\"";
	result += GetJsonValue(data.FANSensor, MAX_CHANALS_FAN);
	result += SendEndMess();
	return result;
}

/**
 * v0.6
 * {"status":"success","message":"info","data":{"type" : 1024}}
 * @param level
 */
String AquaHelper::GetAlarmWaterLevel(int level) {
	String result = SendStartMess();
	result += "info\",\"data\":{\"type\":";
	result += level;
	result += SendEndMess();
	return result;
}

/**
 * v0.6
 * @param data
 * {"status":"post","message":"ph_timer","data":
 * {"ph_401v2":[844, 844, 844, 844],
 * "ph_686v2":[797, 797, 797, 797],
 * "ph_c":[1,1],
 * "ph_e":[60, 60],
 * "ph_s":[0,0],
 * "ph_st":[1,1]
 * }}
 */
String AquaHelper::GetPhTimerState() {
	String result = "";
	result += "{\"status\":\"success\",\"message\":\"ph_timer\",\"data\":{\"ph_s\"";
	result += GetJsonValue(data.PHTimerStart, MAX_TIMERS_PH);
	result += ",\"ph_e\"";
	result += GetJsonValue(data.PHTimerEnd, MAX_TIMERS_PH);
	result += ",\"ph_st\"";
	result += GetJsonValue(data.PHTimerState, MAX_TIMERS_PH);
	result += ",\"ph_c\"";
	result += GetJsonValue(data.PHTimerCanal, MAX_TIMERS_PH);
	result += ",\"ph_value\"";
	result += GetJsonValue(data.PHCalibrationValue, MAX_TIMERS_PH * 2);
	result += ",\"ph_voltage\"";
	result += GetJsonValue(data.PHCalibrationVoltage, MAX_TIMERS_PH * 2);
	result += "}}";
	return result;
}
/**
 * @return {"status":"success","message":"ph_state","data":
 * {"ph":[113 113],
 * "ph1":[113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113],
 * "ph2":[113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113]
 * }}
 */
String AquaHelper::GetPhStats() {
	String result = "{\"status\":\"success\",\"message\":\"ph_state\",\"data\":{\"ph\":[";
	for (byte i = 0; i < MAX_TIMERS_PH; i++) {
		result += String(Helper.data.PHCurrent[i]);
		if (i < MAX_TIMERS_PH - 1) {
			result += ",";
		}
	}
	result += "],";
	for (byte i = 0; i < MAX_TIMERS_PH; i++) {
		result += "\"ph";
		result += String(i);
		result += "\":[";
		for (byte j = 0; j < MAX_STATS - 1; j++) {
			result += String(Helper.data.PHStats[i][j]);
			result += ",";
		}
		result += String(Helper.data.PHStats[i][MAX_STATS - 1]);

		if (i < MAX_TIMERS_PH - 1) {
			result += "],";
		} else {
			result += "]";
		}
	}
	result += "}}";
	return result;

}

String AquaHelper::GetTempStats() {
	String result = "{\"status\":\"success\",\"message\":\"temp_stats\",\"data\":{";
	for (byte j = 0; j < MAX_TEMP_SENSOR; j++) {
		if (j != 0) {
			result += ",";
		}
		result += "\"sensor" + String(j) + "\":";
		result += "[";
		for (byte i = 0; i < MAX_STATS - 1; i++) {
			result += String(Helper.data.TempStats[j][i]);
			result += ",";
		}
		result += String(Helper.data.TempStats[j][MAX_STATS - 1]);
		result += "]";
	}
	result += "}}";
	return result;
}

//============================================POST==============================================
bool AquaHelper::SetPostRequest(String inString, void (*GetPHLevelConfig)(byte, byte, int)) {
	if (inString.indexOf("post") != -1) {
		DynamicJsonBuffer jsonBuffer(200);
		JsonObject &root = jsonBuffer.parseObject(inString);
		if (!root.success()) {
			Serial.print("ERROR PARSE = ");
			Serial.println(inString);
		}
		JsonObject &request = root["data"];
		if (inString.indexOf("c_s") != -1) {
			SetJsonValue(Helper.data.StateChanals, MAX_CHANALS, "c_t", request);
			return true;
		} else if (inString.indexOf("pwm_cs") != -1) {
			SetJsonValue(Helper.data.StatePWMChanals, MAX_CHANALS_PWM, "pwm_ct", request);
			return true;
		} else if (inString.indexOf("te_s") != -1) {
			SetJsonValue(Helper.data.TempTimerState, MAX_TEMP_SENSOR, "tt_s", request);
			SetJsonValue(Helper.data.TempTimerMinStart, MAX_TEMP_SENSOR, "tt_m_s", request);
			SetJsonValue(Helper.data.TempTimerMaxEnd, MAX_TEMP_SENSOR, "tt_m_e", request);
			SetJsonValue(Helper.data.TempTimerChanal, MAX_TEMP_SENSOR, "tt_c", request);
			return true;
		} else if (inString.indexOf("td_s") != -1) {
			SetJsonValue(Helper.data.DailyTimerHourStart, MAX_TIMERS, "dt_h_s", request);
			SetJsonValue(Helper.data.DailyTimerHourEnd, MAX_TIMERS, "dt_h_end", request);
			SetJsonValue(Helper.data.DailyTimerMinStart, MAX_TIMERS, "dt_m_s", request);
			SetJsonValue(Helper.data.DailyTimerMinEnd, MAX_TIMERS, "dt_m_e", request);
			SetJsonValue(Helper.data.DailyTimerState, MAX_TIMERS, "dt_s", request);
			SetJsonValue(Helper.data.DailyTimerChanal, MAX_TIMERS, "dt_c", request);
			return true;
		} else if (inString.indexOf("th_s") != -1) {
			SetJsonValue(Helper.data.HoursTimerMinStart, MAX_TIMERS, "ht_m_st", request);
			SetJsonValue(Helper.data.HoursTimerMinStop, MAX_TIMERS, "ht_m_sp", request);
			SetJsonValue(Helper.data.HoursTimerState, MAX_TIMERS, "ht_s", request);
			SetJsonValue(Helper.data.HoursTimerCanal, MAX_TIMERS, "ht_c", request);
			return true;
		} else if (inString.indexOf("ts_s") != -1) {
			SetJsonValue(Helper.data.SecondTimerHourStart, MAX_TIMERS, "st_h_s", request);
			SetJsonValue(Helper.data.SecondTimerMinStart, MAX_TIMERS, "st_m_s", request);
			SetJsonValue(Helper.data.SecondTimerDuration, MAX_TIMERS, "st_d", request);
			SetJsonValue(Helper.data.SecondTimerState, MAX_TIMERS, "st_s", request);
			SetJsonValue(Helper.data.SecondTimerCanal, MAX_TIMERS, "st_c", request);
			return true;
		} else if (inString.indexOf("pwm_timer") != -1) {
			SetJsonValue(Helper.data.TimerPWMHourStart, MAX_TIMERS, "pwm_h_s", request);
			SetJsonValue(Helper.data.TimerPWMHourEnd, MAX_TIMERS, "pwm_h_end", request);
			SetJsonValue(Helper.data.TimerPWMMinStart, MAX_TIMERS, "pwm_m_s", request);
			SetJsonValue(Helper.data.TimerPWMMinEnd, MAX_TIMERS, "pwm_m_e", request);
			SetJsonValue(Helper.data.TimerPWMState, MAX_TIMERS, "pwm_s", request);
			SetJsonValue(Helper.data.TimerPWMChanal, MAX_TIMERS, "pwm_c", request);
			SetJsonValue(Helper.data.TimerPWMDuration, MAX_TIMERS, "pwm_delay", request);
			SetJsonValue(Helper.data.TimerPWMLevel, MAX_TIMERS, "pwm_level", request);
			return true;
		} else if (inString.indexOf("ph_timer") != -1) {
			if (inString.indexOf("ph_s") != -1)
				SetJsonValue(Helper.data.PHTimerStart, MAX_TIMERS_PH, "ph_s", request);
			if (inString.indexOf("ph_e") != -1)
				SetJsonValue(Helper.data.PHTimerEnd, MAX_TIMERS_PH, "ph_e", request);
			if (inString.indexOf("ph_st") != -1)
				SetJsonValue(Helper.data.PHTimerState, MAX_TIMERS_PH, "ph_st", request);
			if (inString.indexOf("ph_c") != -1)
				SetJsonValue(Helper.data.PHTimerCanal, MAX_TIMERS_PH, "ph_c", request);

			return true;
			//{"status":"post","message":"settings","data": {"ntp":0,"update":0,"sound":0,"debug":0}}
		} else if (inString.indexOf(SETTINGS_DEV) != -1) {
			Helper.data.ntp_update = request[SETTINGS_NTP].as<bool>();
			Helper.data.auto_update = request[SETTINGS_UPDATE].as<bool>();
			Helper.data.isTone = request[SETTINGS_SOUND].as<bool>();
			Helper.data.debug = request[SETTINGS_DEBUG].as<bool>();
			return true;
		} else if (inString.indexOf(GET_DEVICE_FAN) != -1) {
			if (inString.indexOf("t_fan_s") != -1)
				SetJsonValue(Helper.data.FANTimerMinStart, MAX_CHANALS_FAN, "t_fan_s", request);
			if (inString.indexOf("t_fan_e") != -1)
				SetJsonValue(Helper.data.FANTimerMaxEnd, MAX_CHANALS_FAN, "t_fan_e", request);
			if (inString.indexOf("t_fan_se") != -1)
				SetJsonValue(Helper.data.FANSensor, MAX_CHANALS_FAN, "t_fan_se", request);
			return true;
		} else if (inString.indexOf(GET_DEVICE_PH_SET) != -1) {
			byte index = 0;
			byte point = 0;
			int value = 0.0f;
			if (inString.indexOf("ph_index") != -1) {
				index = request["ph_index"].as<byte>();
				if (inString.indexOf("ph_point") != -1) {
					point = request["ph_point"].as<byte>();
					if (inString.indexOf("ph_value") != -1) {
						value = request["ph_value"].as<int>();
						GetPHLevelConfig(index, point, value);
						return true;
					}
				}
			}

		}
		return false;
	}
	return false;
}
//=========================================GSON HELPER===========================================

bool SetJsonValue(byte arrayData[], const byte count, const String key, const JsonObject &root) {
	if (!root.containsKey(key))
		return false;
	for (byte i = 0; i < count; i++) {
		arrayData[i] = root[key][i].as<byte>();
	}
	return true;
}

String GetJsonValue(const word arrayData[], const byte count) {
	String result = "";
	result = ":[";
	for (byte i = 0; i < count; i++) {
		result += arrayData[i];
		if (i != count - 1) {
			result += ",";
		}
	}
	result += "]";
	return result;
}

String GetJsonValue(const byte arrayData[], const byte count) {
	String result = "";
	result = ":[";
	for (byte i = 0; i < count; i++) {
		result += arrayData[i];
		if (i != count - 1) {
			result += ",";
		}
	}
	result += "]";
	return result;
}

bool SetJsonValue(word arrayData[], const byte count, const String key, const JsonObject &root) {
	if (!root.containsKey(key))
		return false;
	for (byte i = 0; i < count; i++) {
		arrayData[i] = root[key][i].as<word>();
	}
	return true;
}

int AquaHelper::GetLevelPWM(byte timer) {
	return MAX_PWM_POWER_CALCULATE * Helper.data.TimerPWMLevel[timer] / 100;
}
//==========================================PH HELPER==============================================
byte AquaHelper::ConvertPHWordToByte(const word ph) {
	return (ph - MIN_PH) / STEP_PH;
}
//=================================================================================================

tmElements_t AquaHelper::GetTimeNow() {
	tmElements_t tm;
	tm.Day = RTC.now().day();
	tm.Hour = RTC.now().hour();
	tm.Minute = RTC.now().minute();
	tm.Month = RTC.now().month();
	tm.Second = RTC.now().second();
	tm.Year = RTC.now().year();
	return tm;
}

void AquaHelper::SetTimeNow(unsigned long epoch) {
	ds3231.setYear(year(epoch));
	ds3231.setMonth(month(epoch));
	ds3231.setDate(day(epoch));
	ds3231.setHour(hour(epoch));
	ds3231.setMinute(minute(epoch));
	ds3231.setSecond(second(epoch));
	Serial.println(((epoch / 86400)) % 7);
	ds3231.setDoW(((epoch / 86400)) % 7);

}

byte AquaHelper::GetHourNow() {
	return RTC.now().hour();
}

String AquaHelper::GetDayOfWeek() {
	switch (ds3231.getDoW()) {
	case 4:
		return "Monday";
	case 5:
		return "Tuesday";
	case 6:
		return "Wednesday";
	case 0:
		return "Thursday";
	case 1:
		return "Friday";
	case 2:
		return "Saturday";
	case 3:
		return "Sunday";
	}

	return "";
}

bool i2cReady(uint8_t adr) {
	uint32_t timeout = millis();
	bool ready = false;
	while ((millis() - timeout < 100) && (!ready)) {
		Wire.beginTransmission(adr);
		ready = (Wire.endTransmission() == 0);
	}
	return ready;
}

int AquaHelper::ChipSize() {
	return static_cast<int>(spi_flash_get_chip_size()) / 1024 / 1024;
}

int AquaHelper::SPIFFSSize() {
	return SPIFFS.totalBytes() / 1024;
}

String AquaHelper::Split(String data, char separator, int index) {
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

/*
 * AquaHelper.cpp
 *
 *  Created on: Oct 2, 2019
 *      Author: doc
 */
#include "AquaHelper.h"

dataController AquaHelper::data;
AquaHelper Helper = AquaHelper();
RTClib RTC;
DS3231 ds3231;

// Speaker Setup
bool isTone = true;
//Pin for speaker
const byte tonePin = 4;
/**
 * Method of signaling through the system speaker
 */
void AquaHelper::Tone(const word frequency, const word duration) {
	if (isTone) {
		ESP_tone(TONE_PIN, frequency, duration, BUZZER_CHANNEL);
	}
}

void AquaHelper::Tone() {
	if (isTone) {
		ESP_tone(TONE_PIN, 500, 10, BUZZER_CHANNEL);
	}
}

void AquaHelper::ToneForce(const word frequency, const word duration) {
	ESP_tone(TONE_PIN, frequency, duration, BUZZER_CHANNEL);
}

void AquaHelper::ESP_tone(uint8_t pin, unsigned int frequency,
		unsigned long duration, uint8_t channel) {
	if (ledcRead(channel)) {
		log_e("Tone channel %d is already in use", ledcRead(channel));
		return;
	}
	ledcAttachPin(pin, channel);
	ledcWriteTone(channel, frequency);
	if (duration) {
		delay(duration);
		ESP_noTone(pin, channel);
	}
}

void AquaHelper::ESP_noTone(uint8_t pin, uint8_t channel) {
	ledcDetachPin(pin);
	ledcWrite(channel, 0);
}

void AquaHelper::SetToneEnable(bool enable) {
	isTone = enable;
}

String SendStartMess() {
	return "{\"status\":\"success\",\"message\":\"";
}

String SendEndMess() {
	return "}}";
}
//============================================GET===============================================
AquaHelper::AquaHelper() {

}

String AquaHelper::GetDevice(String ip) {
	String result = SendStartMess();
	result += "dev\",\"data\":{\"ver\":\"AQ_V2_ESP32\",\"firm\":\"";
	result += VERTION_FIRMWARE;
	result += "\",\"update\":";
	result += data.auto_update;
	result += ",\"ip\":\"";
	result += ip;
	result += "\",\"m_t\":10,\"m_t_se\":4,\"min_t\":1600,\"max_t\":3500";
    result +=",\"time\":\"";
    result += GetFormatTimeNow();
    result += "\"";
	result += SendEndMess();
	return result;
}

String AquaHelper::GetDataTime() {
    String result = "{\"status\":\"success\",\"";
    result += "time\":\"";
    result += GetFormatTimeNow();
    result += "\",\"date\":\"";
    result += GetFormatDataNow();
    result += "\"}";
    return result;
}

String AquaHelper::GetFormatTimeNow() {
	tmElements_t tm = GetTimeNow();
	String time_fm = "";
	if (tm.Hour < 10)
		time_fm += "0";
	time_fm += String(tm.Hour) + ":";
	if (tm.Minute < 10)
		time_fm += "0";
	time_fm += String(tm.Minute);
	time_fm += ":";
	if (tm.Second < 10)
		time_fm += "0";
	time_fm += String(tm.Second);
	return time_fm;
}


String AquaHelper::GetFormatDataNow(){
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
	result += GetJsonValue(data.CurrentStatePWMChanalsByTypeTimer, MAX_CHANALS);
	result += ",\"pwm_ct\"";
	result += GetJsonValue(data.StatePWMChanals, MAX_CHANALS);
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
	result += "set\",\"NTP\":";
	result += data.ntp_update;
	result += ",\"AUTO\":";
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
 * {"status":"post","message":"ph_timer","data": {"ph_401v2":[844, 844],"ph_686v2":[797, 797],"ph_c":[1,1],"ph_e":[60, 60],"ph_s":[0,0],"ph_st":[1,1]}}
 */
String AquaHelper::GetPhTimerState() {
	String result = "";
	result +=
			"{\"status\":\"success\",\"message\":\"ph_timer\",\"data\":{\"ph_s\"";
	result += GetJsonValue(data.PHTimerStart, MAX_TIMERS_PH);
	result += ",\"ph_e\"";
	result += GetJsonValue(data.PHTimerEnd, MAX_TIMERS_PH);
	result += ",\"ph_st\"";
	result += GetJsonValue(data.PHTimerState, MAX_TIMERS_PH);
	result += ",\"ph_c\"";
	result += GetJsonValue(data.PHTimerCanal, MAX_TIMERS_PH);
	result += ",\"ph_401v2\"";
	result += GetJsonValue(data.PHTimer401, MAX_TIMERS_PH);
	result += ",\"ph_686v2\"";
	result += GetJsonValue(data.PHTimer686, MAX_TIMERS_PH);
	result += "}}";
	return result;
}
/**
 * @return {"status":"success","message":"ph_state","data":
 * {"ph":[113],
 * "ph1":[113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113],
 * "ph2":[113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113]
 * }}
 */
String AquaHelper::GetPhStats() {
	String result =
			"{\"status\":\"success\",\"message\":\"ph_state\",\"data\":{\"ph\":[";
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
	String result =
			"{\"status\":\"success\",\"message\":\"temp_stats\",\"data\":{";
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
bool AquaHelper::SetPostRequest(String inString,
		void (*GetPHLevelConfig)(bool, byte)) {
	if (inString.indexOf("post") != -1) {
		DynamicJsonBuffer jsonBuffer(200);
		JsonObject &root = jsonBuffer.parseObject(inString);
		JsonObject &request = root["data"];
		if (inString.indexOf("c_s") != -1) {
			SetJsonValue(Helper.data.StateChanals, MAX_CHANALS, "c_t", request);
			return true;
		} else if(inString.indexOf("pwm_cs")){
			SetJsonValue(Helper.data.StatePWMChanals, MAX_CHANALS_PWM, "pwm_c_t", request);
			return true;
		}else if (inString.indexOf("te_s") != -1) {
			SetJsonValue(Helper.data.TempTimerState, MAX_TEMP_SENSOR, "tt_s",
					request);
			SetJsonValue(Helper.data.TempTimerMinStart, MAX_TEMP_SENSOR,
					"tt_m_s", request);
			SetJsonValue(Helper.data.TempTimerMaxEnd, MAX_TEMP_SENSOR, "tt_m_e",
					request);
			SetJsonValue(Helper.data.TempTimerChanal, MAX_TEMP_SENSOR, "tt_c",
					request);
			return true;
		} else if (inString.indexOf("td_s") != -1) {
			SetJsonValue(Helper.data.DailyTimerHourStart, MAX_TIMERS, "dt_h_s",
					request);
			SetJsonValue(Helper.data.DailyTimerHourEnd, MAX_TIMERS, "dt_h_end",
					request);
			SetJsonValue(Helper.data.DailyTimerMinStart, MAX_TIMERS, "dt_m_s",
					request);
			SetJsonValue(Helper.data.DailyTimerMinEnd, MAX_TIMERS, "dt_m_e",
					request);
			SetJsonValue(Helper.data.DailyTimerState, MAX_TIMERS, "dt_s",
					request);
			SetJsonValue(Helper.data.DailyTimerChanal, MAX_TIMERS, "dt_c",
					request);
			return true;
		} else if (inString.indexOf("th_s") != -1) {
			SetJsonValue(Helper.data.HoursTimerMinStart, MAX_TIMERS, "ht_m_st",
					request);
			SetJsonValue(Helper.data.HoursTimerMinStop, MAX_TIMERS, "ht_m_sp",
					request);
			SetJsonValue(Helper.data.HoursTimerState, MAX_TIMERS, "ht_s",
					request);
			SetJsonValue(Helper.data.HoursTimerCanal, MAX_TIMERS, "ht_c",
					request);
			return true;
		} else if (inString.indexOf("ts_s") != -1) {
			SetJsonValue(Helper.data.SecondTimerHourStart, MAX_TIMERS, "st_h_s",
					request);
			SetJsonValue(Helper.data.SecondTimerMinStart, MAX_TIMERS, "st_m_s",
					request);
			SetJsonValue(Helper.data.SecondTimerDuration, MAX_TIMERS, "st_d",
					request);
			SetJsonValue(Helper.data.SecondTimerState, MAX_TIMERS, "st_s",
					request);
			SetJsonValue(Helper.data.SecondTimerCanal, MAX_TIMERS, "st_c",
					request);
			return true;
		} else if (inString.indexOf("pwm_timer") != -1) {
			SetJsonValue(Helper.data.TimerPWMHourStart, MAX_TIMERS, "pwm_h_s",
					request);
			SetJsonValue(Helper.data.TimerPWMHourEnd, MAX_TIMERS, "pwm_h_end",
					request);
			SetJsonValue(Helper.data.TimerPWMMinStart, MAX_TIMERS, "pwm_m_s",
					request);
			SetJsonValue(Helper.data.TimerPWMMinEnd, MAX_TIMERS, "pwm_m_e",
					request);
			SetJsonValue(Helper.data.TimerPWMState, MAX_TIMERS, "pwm_s",
					request);
			SetJsonValue(Helper.data.TimerPWMChanal, MAX_TIMERS, "pwm_c",
					request);
			SetJsonValue(Helper.data.TimerPWMDuration, MAX_TIMERS, "pwm_delay",
								request);
			return true;
		}else if (inString.indexOf("ph_timer") != -1) {
			if (inString.indexOf("ph_s") != -1)
				SetJsonValue(Helper.data.PHTimerStart, MAX_TIMERS_PH, "ph_s",
						request);
			if (inString.indexOf("ph_e") != -1)
				SetJsonValue(Helper.data.PHTimerEnd, MAX_TIMERS_PH, "ph_e",
						request);
			if (inString.indexOf("ph_st") != -1)
				SetJsonValue(Helper.data.PHTimerState, MAX_TIMERS_PH, "ph_st",
						request);
			if (inString.indexOf("ph_c") != -1)
				SetJsonValue(Helper.data.PHTimerCanal, MAX_TIMERS_PH, "ph_c",
						request);
			return true;
		} else if (inString.indexOf("ph_timer") != -1) {
			//{"status":"post","message":"ph_config","data": {"ph_401":0,"ph_686":0}}
			if (inString.indexOf("ph_401") != -1) {
				byte canal = request["ph_401"].as<byte>();
				if (canal > 0 && canal <= 2) {
					GetPHLevelConfig(LOW, canal);
				}
			}
			if (inString.indexOf("ph_686") != -1) {
				byte canal = request["ph_686"].as<byte>();
				if (canal > 0 && canal <= 2) {
					GetPHLevelConfig(HIGH, canal);
				}
			}

		}
		return false;
	}
	return false;
}
//=========================================GSON HELPER===========================================
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

String GetJsonValue(const uint16_t arrayData[], const byte count) {
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

bool SetJsonValue(byte arrayData[], const byte count, const String key,
		const JsonObject &root) {
	if (!root.containsKey(key))
		return false;
	for (byte i = 0; i < count; i++) {
		arrayData[i] = root[key][i].as<byte>();
	}
	return true;
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
}




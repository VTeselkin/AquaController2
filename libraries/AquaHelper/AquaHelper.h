/*
 * AquaHelper.h
 *
 *  Created on: Oct 2, 2019
 *      Author: doc
 */
#pragma once

#ifndef AquaHelper_h
#define AquaHelper_h

#include "Arduino.h"
#include <map>
#include <DS3231.h>
#include <time.h>
#include <TimeLib.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

//Maximum number of canals
#define MAX_CHANALS 8
#define MAX_CHANALS_TIMER_PWM 10
//Maximum number of timers
#define MAX_TIMERS 10
#define MAX_PWM_LEVEL 100

#define MAX_EEPROM 1024

#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_CLOCK 400000

//CHANAL HANDS CONTROLL
#define OFF_CHANAL 1
#define ON_CHANAL 2
#define AUTO_CHANAL 3

//Current state canals timer
#define TIMER_OFF 1
#define TIMER_ON 2
#define TIMER_MIN 3
#define TIMER_OTHER 4
#define TIMER_SEC 5
#define TIMER_TEMP 6
#define TIMER_PWM 7
#define TIMER_TEMPFAN 8

#define ENABLE_TIMER 1
#define DISABLE_TIMER 0

#define HOUR 23
#define MINUTE 59
#define DAY 31
#define MOUNTH 12
#define SECONDS 255
#define SEC_IN_MIN 60
#define SECOND_BY_DAY 86399
#define MIN_BY_DAY 1440
//CHANAL HANDS CONTROLL
#define OFF_CHANAL 1
#define ON_CHANAL 2
#define AUTO_CHANAL 3

//Canal for temporary disable
#define CHANAL_BTN_DISABLE 0

#define FREQURENCY_SEND_TEMP 10
//Accuracy of the temperature sensor
#define TEMPERATURE_PRECISION 11
//Maximum number of temperature sensors
#define MAX_TEMP_SENSOR 4
//Used Arduino terminals for temperature sensors
#define TEMP_SENS 32
//State temperature sensors
#define DISCONNECT_SENSOR 0
//State temperature sensors
#define CONNECT_SENSOR 2
// Minimal step of temperature change
#define STEP 25
// Minimum temperature index: MIN_TEMP + STEP* MIN_INDEX_TEMP
#define MIN_INDEX_TEMP 0
// The maximum possible temperature index: MIN_TEMP + STEP* MAX_INDEX_TEMP
#define MAX_INDEX_TEMP 76
// The lowest possible setting is the temperature
#define MIN_TEMP 1600
// Maximum possible maximum temperature
#define MAX_TEMP 3500
//Maximum number of PWM canals
#define MAX_CHANALS_PWM 16
//Maximum number of FAN canals
#define MAX_CHANALS_FAN 2
//Maximum number of Power PWM canals
#define  MAX_PWM_POWER_CALCULATE 4096
//Maximum number of analog canals
#define MAX_ADC_CANAL 4
//Buffer size for UDP
#define MAX_BUFFER 4000
#define DELAY_UPDATE_DEVICE 1000
#define DELAY_MESSAGE_UPDATE 60000
#define DELAY_TEMP_UPDATE 55000
#define DELAY_TEMP_UPDATE_STATE 3600000
#define DELAY_PH_UPDATE 60000
#define DELAY_TIME_UPDATE 10000
#define DELAY_PH_UPDATE_STATE 3600000
#define DELAY_DEVICE_INFO_UPDATE 12000
#define DELAY_DEVICE_TIME_UPDATE 1000
// 60 minutes
#define DELAY_NTP_UPDATE 21600000
#define MAX_STATS 24

#define MAX_TIMERS_PH 2
#define MAX_LEVEL_PH 17600 //https://microcontrollerslab.com/ads1115-external-adc-with-esp32/

#define TONE_PIN 15

#define BUZZER_CHANNEL 0
/**
 * Addressing memory to store device states
 */
const byte ADDR_FIRST_LAUNCH = 1;

const byte ChanalsStateAddr = 20;

const word AUTO_CONNECT_ADDR = 1000;
const word NTP_UPDATE_ADDR = 998;
const word LCD_LED_ON_ADDR = 996;
const word LCD_BACK_ADDR = 994;
const word LCD_BUTTON_ADDR = 992;
const word LCD_SOUND_ADDR = 990;
const word LCD_I2C_ADDR = 988;
const word addrTempSensor = 980;
const word AUTO_UPDATE_ADDR = 978;

const word FANSensorAddr = 454;
const word FANTimerMinStartAddr = 452;
const word FANTimerMaxEndAddr = 450;

const word PWMTimerHourLevelAddr = 447;
const word PWMTimerHourDurationAddr = 437;
const word PWMTimerHourStartAddr = 427;
const word PWMTimerHourEndAddr = 417;
const word PWMTimerMinStartAddr = 407;
const word PWMTimerMinEndAddr = 397;
const word PWMTimerStateAddr = 387;
const word PWMTimerChanalAddr = 377;

const word ChanalsPWMStateAddr = 365;

const word PHTimerStartAddr = 347;
const word PHTimerEndAddr = 345;
const word PHTimerStateAddr = 343;
const word PHTimerCanalAddr = 341;
const word PHTimer401Addr = 349;
const word PHTimer686Addr = 337;
const word Ph_6_86_levelAddr = 335;
const word Ph_4_01_levelAddr = 330;

const word TimerPWMHourStartAddr = 325;
const word TimerPWMHourEndAddr = 315;
const word TimerPWMMinStartAddr = 305;
const word TimerPWMMinEndAddr = 295;
const word TimerPWMStateAddr = 285;
const word TimerPWMChanalAddr = 275;
const word TimerPWMDurationAddr = 265;

const byte DailyTimerHourStartAddr = 255;
const byte DailyTimerHourEndAddr = 245;
const byte DailyTimerMinStartAddr = 235;
const byte DailyTimerMinEndAddr = 225;
const byte DailyTimerStateAddr = 215;
const byte DailyTimerChanalAddr = 205;

const byte SecondTimerHourStartAddr = 195;
const byte SecondTimerMinStartAddr = 185;
const byte SecondTimerDurationAddr = 175;
const byte SecondTimerStateAddr = 165;
const byte SecondTimerCanalAddr = 155;

const byte DailyHoursTimerMinStartAddr = 145;
const byte DailyHoursTimerMinDurationAddr = 135;
const byte DailyHoursTimerStateAddr = 125;
const byte DailyHoursTimerCanalAddr = 115;

const byte TempTimerMinStartAddr = 105;
const byte TempTimerMaxEndAddr = 101;
const byte TempTimerStateAddr = 97;
const byte TempTimerChanalAddr = 93;

const String responseNull = "{\"status\":\"error\",\"message\":\"Not Initialized\",\"data\":{}}";

//------------------type of message-----------------------
const String GET_COMMAND = "get";
const String POST_COMMAND = "post";
const String INFO_COMMAND = "info";
//----------------------------------------------------------
//type of error
const String RESPONSE_NOT_RESPONSE = "Not response";
const String RQUEST_NOT_VALID = "Request not valid";
const String RQUEST_JSON_CORUPTED = "Request is corrupted";
const String RQUEST_DATA_CORUPTED = "Data is corrupted";

//type of response
const String RQUEST_COMPLETE = "Request complete";
//--------------------post request--------------------------
const String CANAL_STATE = "c_s";
const String CANAL_TIMER = "c_t";

const String TIMER_DAILY_STATE = "td_s";

const String DAILY_TIMER_HOUR_START = "dt_h_s";
const String DAILY_TIMER_HOUR_END = "dt_h_end";
const String DAILY_TIMER_MIN_START = "dt_m_s";
const String DAILY_TIMER_MIN_END = "dt_m_e";
const String DAILY_TIMER_STATE = "dt_s";
const String DAILY_TIMER_CANAL = "dt_c";

const String TEMP_STATE = "te_s";

const String TEMP_TIMER_STATE = "tt_s";
const String TEMP_TIMER_MIN_START = "tt_m_s";
const String TEMP_TIMER_MAX_END = "tt_m_e";
const String TEMP_TIMER_CHANAL = "tt_c";

const String TIMER_HOURS_STATE = "th_s";
const String HOURS_TIMER_MIN_START = "ht_m_st";
const String HOURS_TIMER_MIN_STOP = "ht_m_sp";
const String HOURS_TIMER_STATE = "ht_s";
const String HOURS_TIMER_CANAL = "ht_c";

const String TIMER_SECONDS_STATE = "ts_s";

const String SECOND_TIMER_HOUR_START = "st_h_s";
const String SECOND_TIMER_MIN_START = "st_m_s";
const String SECOND_TIMER_DURATIONT = "st_d";
const String SECOND_TIMER_STATE = "st_s";
const String SECOND_TIMER_CANAL = "st_c";

const String TIMER_DAILY_PWM_STATE = "pwm_timer";
const String CANAL_STATE_PWM = "pwm_cs";
const String SETTINGS_DEV = "settings";
//--------------------------JSONs--------------------------------

const String GET_PARAM_REQUEST = "{\"status\":\"get\",\"message\":\"set\"}";
const String GET_DEVICE_INFO = "dev";
const String GET_DEVICE_CHANAL_SETTINGS = "c_s";
const String GET_DEVICE_TEMP_SENSOR = "t_sen";
const String GET_DEVICE_TEMP_STATE = "te_s";
const String GET_DEVICE_DAILY_TIMER_SATE = "td_s";
const String GET_DEVICE_HOURS_TIMER_SATE = "th_s";
const String GET_DEVICE_SECOND_TIMER_SATE = "ts_s";
const String GET_DEVICE_TEMP_STATS = "temp_stats";
const String GET_DEVICE_PH_TIMER = "ph_timer";
const String GET_DEVICE_PH = "ph_state";
const String GET_DEVICE_FAN = "t_fan";

//---------------------------------------------------------------

//settings param

const String SETTINGS_SSID = "SSID";
const String SETTINGS_PASS = "PASS";
const String SETTINGS_NTP = "NTP";
const String SETTINGS_AUTO = "AUTO";
const String SETTINGS_UPDATE = "update";
const String SETTINGS_MAX_CHANALS = "TIMERS";
const String SETTINGS_MAX_TEMP_SENSOR = "SENSOR";
const String SETTINGS_UTC = "utc";
const String SETTINGS_EPOCH = "epoch";
const String VERTION_PROTOCOL = "/4/";
const String UPDATE_URL = "http://update.aquacontroller.ru/v2";
const String PATH_FIRMWARE = "/bin/";
const String PATH_SPIFFS = "/spiffs/";

//VERTION_FIRMWARE должно указывать на след версию прошивки.
const String VERTION_FIRMWARE = "202";

// The lowest possible setting is the PH
const word MIN_PH = 400;
// Maximum possible maximum PH
const word MAX_PH = 1600;
// Minimum PH index: MIN_TEMP + STEP* MIN_INDEX_TEMP
const byte MIN_INDEX_PH = 0;
// The maximum possible PH index: MIN_TEMP + STEP* MAX_INDEX_TEMP
const byte MAX_INDEX_PH = 120;
// Minimal step of PH change
const byte STEP_PH = 10;

const float Ph6_86 = 6.86f;
const float Ph4_01 = 4.01f;

typedef enum {
	RXLED,
	TXLED,
	WIFILED,
	ERRLED
} typeDebugLED;

typedef enum {
	NONE,
	SHORT,
	LONG,
	LIGHT,
	PULSE
} typeLightLED;

typedef enum {
	DEVICE,
	CANAL,
	TIMERDAY,
	TIMERHOUR,
	TIMERSEC,
	TIMERTEMP,
	TEMPSENSOR,
	PH,
	PHTIMER,
	TEMPSTATS,
	PWMCANAL,
	PWMTIMER,
	SETTINGS,
	FAN
} typeResponse;
using Dictionary = std::map<typeResponse, String>;

String GetJsonValue(const uint8_t arrayData[], const byte count);
String GetJsonValue(const word arrayData[], const byte count);

bool SetJsonValue(byte arrayData[], const byte count, const String key, const JsonObject &root);
bool SetJsonValue(word arrayData[], const byte count, const String key, const JsonObject &root);

typedef struct {
	/**
	 * Daily timer program settings
	 */
	byte DailyTimerHourStart[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte DailyTimerHourEnd[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte DailyTimerMinStart[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte DailyTimerMinEnd[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte DailyTimerState[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte DailyTimerChanal[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/**
	 * Second timer program settings
	 */
	byte SecondTimerHourStart[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte SecondTimerMinStart[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte SecondTimerDuration[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte SecondTimerState[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte SecondTimerCanal[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/**
	 * Status of hourly timer programs
	 */
	byte HoursTimerMinStart[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte HoursTimerMinStop[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte HoursTimerState[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte HoursTimerCanal[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/**
	 * Status of programs for temperature sensors
	 */
	byte TempTimerState[MAX_TEMP_SENSOR] = { 0, 0, 0, 0 };
	byte TempSensorState[MAX_TEMP_SENSOR] = { 0, 0, 0, 0 };
	byte TempTimerMinStart[MAX_TEMP_SENSOR] = { MIN_INDEX_TEMP, MIN_INDEX_TEMP, MIN_INDEX_TEMP, MIN_INDEX_TEMP };
	byte TempTimerMaxEnd[MAX_TEMP_SENSOR] = { MAX_INDEX_TEMP, MAX_INDEX_TEMP, MAX_INDEX_TEMP, MAX_INDEX_TEMP };
	byte TempSensor[MAX_TEMP_SENSOR] = { 0, 0, 0, 0 };
	byte TempTimerChanal[MAX_TEMP_SENSOR] = { 0, 0, 0, 0 };
	uint16_t TempStats[MAX_TEMP_SENSOR][MAX_STATS] = { };

	// Enabled channels for relays
	const byte nRelayDrive[MAX_CHANALS] = { 18, 19, 4, 13, 23, 25, 26, 33 };

	const byte nPinsESP32[40] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 0, 5, 6, 7, 0, 8, 9, 10,
			0, 0, 0, 0, 11, 12, 0, 0, 0, 0, 0, 0 };
	/**
	 * The array of the current PWM canals status
	 * OFF_CHANAL 1
	 * ON_CHANAL 2
	 * AUTO_CHANAL 3
	 */
	byte StateChanals[MAX_CHANALS] = { 3, 3, 3, 3, 3, 3, 3, 3 };

	/**
	 * Array of types of timers that enabled the canals
	 * TIMER_OFF 1
	 * TIMER_ON 2
	 * TIMER_MIN 3
	 * TIMER_OTHER 4
	 * TIMER_SEC 5
	 * TIMER_TEMP 6
	 * TIMER_PWM 7
	 */
	byte CurrentStateChanalsByTypeTimer[MAX_CHANALS] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	DeviceAddress addrThermometer[MAX_TEMP_SENSOR];
	DeviceAddress addrNewThermometer[MAX_TEMP_SENSOR];

	// Indexes of current device settings
	byte indexDelayLCDLedOn = 0;
	byte indexDelayLCDBackInmainScreen = 0;
	byte indexDelayLCDButton = 2;

	// Speaker Setup
	byte isTone = 1;
	bool ntp_update = 1;
	bool auto_connect = 1;
	bool auto_update = 1;
	bool internet_avalible = 0;
	/** ----------------------------------------PWM---------------------------------- */

	//Enabled canals for PWM
	const byte nPWMDrive[MAX_CHANALS_PWM] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 14, 15, 10, 11, 12, 13 };

	byte TimerPWMHourStart[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte TimerPWMHourEnd[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte TimerPWMMinStart[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte TimerPWMMinEnd[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte TimerPWMState[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte TimerPWMChanal[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	byte TimerPWMDuration[MAX_TIMERS] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	byte TimerPWMLevel[MAX_TIMERS] = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };

	/**
	 * The array of the current PWM canals status
	 * OFF_CHANAL 1
	 * ON_CHANAL 2
	 * AUTO_CHANAL 3
	 */
	byte StatePWMChanals[MAX_CHANALS_PWM] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

	/**
	 * The array of the current PWM canals power level
	 */
	int PowerPWMChanals[MAX_CHANALS_PWM] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/**
	 * Array of types of timers that enabled the canals
	 * TIMER_OFF 1
	 * TIMER_ON 2
	 * TIMER_MIN 3
	 * TIMER_OTHER 4
	 * TIMER_SEC 5
	 * TIMER_TEMP 6
	 * TIMER_PWM 7
	 */
	byte CurrentStatePWMChanalsByTypeTimer[MAX_CHANALS_PWM] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	unsigned long TimetoCheckPWMstate[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	bool TimetoCheckPWMLastState[MAX_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/** ------------------------------------FAN-------------------------------------- */
	const byte nFANDrive[MAX_CHANALS_FAN] = { 10, 11 };
	//byte FANTimerState[MAX_CHANALS_FAN] = { 0, 0 };
	//byte FANSensorState[MAX_CHANALS_FAN] = { 0, 0 };
	byte FANTimerMinStart[MAX_CHANALS_FAN] = { MIN_INDEX_TEMP, MIN_INDEX_TEMP };
	byte FANTimerMaxEnd[MAX_CHANALS_FAN] = { MAX_INDEX_TEMP, MAX_INDEX_TEMP };
	byte FANSensor[MAX_CHANALS_FAN] = { 0, 0};
	//byte FANTimerChanal[MAX_CHANALS_FAN] = { 0, 0 };


	/** ------------------------------------PH-------------------------------------- */
	byte PHTimerStart[MAX_TIMERS_PH] = { 0, 0 };
	byte PHTimerEnd[MAX_TIMERS_PH] = { 0, 0 };
	byte PHTimerState[MAX_TIMERS_PH] = { 0, 0 };
	byte PHTimerCanal[MAX_TIMERS_PH] = { 0, 0 };
	word PHTimer401[MAX_TIMERS_PH] = { 1, 1 };
	word PHTimer686[MAX_TIMERS_PH] = { 1, 1 };
	uint16_t PHStats[MAX_TIMERS_PH][MAX_STATS] = { };
	uint16_t PHCurrent[MAX_TIMERS_PH] = { 0, 0 };

	/** ------------------------------------ADC-------------------------------------- */
	//Enabled canals for ADC
	const byte nADCPins[MAX_ADC_CANAL] = { 34, 35, 36, 39 };
	/** ----------------------------------------------------------------------------- */
} dataController;

class AquaHelper {
public:
	AquaHelper();
	static dataController data;
	static void Tone();
	static void Tone(const word frequency, const word duration);
	static void ToneForce(const word frequency, const word duration);
	static void SetToneEnable(bool enable);
	static bool SetPostRequest(String inString, void (*GetPHLevelConfig)());
	static String GetDevice(String ip);
	static String GetDataTime();
	static String GetChanalState();
	static String GetChanalPWMState();
	static String GetDailyTimerState();
	static String GetHoursTimerState();
	static String GetSecondsTimerState();
	static String GetPWMTimerState();
	static String GetWiFiSettings();
	static String GetTempState();
	static String GetRealTemp();
	static String GetFANTemp();
	static String GetAlarmWaterLevel(int level);
	static String GetPhTimerState();
	static String GetPhStats();
	static String GetTempStats();
	static byte GetHourNow();
	static String GetDayOfWeek();
	static tmElements_t GetTimeNow();
	static String GetFormatTimeNow(bool isShort);
	static String GetFormatDataNow();
	static void SetTimeNow(unsigned long epoch);
	static byte ConvertPHWordToByte(const word ph);
	static void ESP_tone(uint8_t pin, unsigned int frequency, unsigned long duration, uint8_t channel);
	static void ESP_noTone(uint8_t pin, uint8_t channel);
	static int GetLevelPWM(byte timer);

private:
};

#ifdef Helper
#undef Helper // workaround for Arduino Due, which defines "RTC"...
#endif

extern AquaHelper Helper;

#endif

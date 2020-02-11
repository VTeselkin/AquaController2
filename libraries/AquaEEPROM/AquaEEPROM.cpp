/*
 * AquaEEPROM.cpp
 *
 *  Created on: Oct 10, 2019
 *      Author: doc
 */
#include "AquaEEPROM.h"

void AquaEEPROM::Init() {
	LoadChanelState();
	LoadDailyTimersReadFromERROM();
	LoadHoursTimersReadFromERROM();
	LoadSecondsTimersReadFromERROM();
	LoadLcdSetings();
	LoadWiFiSettings();
}

void AquaEEPROM::SaveChanalState() {
	for (byte i = 0; i < MAX_CHANALS; i++) {
		EEPROM.write(ChanalsStateAddr - i, Helper.data.StateChanals[i]);
	}
}

void AquaEEPROM::LoadChanelState() {
	for (byte i = 0; i < MAX_CHANALS; i++) {
		Helper.data.StateChanals[i] = EEPROM.read(ChanalsStateAddr - i);
	}
}

void AquaEEPROM::LoadDailyTimersReadFromERROM() {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		Helper.data.DailyTimerHourStart[i] = EEPROM.read(DailyTimerHourStartAddr - i);
		Helper.data.DailyTimerHourEnd[i] = EEPROM.read(DailyTimerHourEndAddr - i);
		Helper.data.DailyTimerMinStart[i] = EEPROM.read(DailyTimerMinStartAddr - i);
		Helper.data.DailyTimerMinEnd[i] = EEPROM.read(DailyTimerMinEndAddr - i);
		Helper.data.DailyTimerState[i] = EEPROM.read(DailyTimerStateAddr - i);
		Helper.data.DailyTimerChanal[i] = EEPROM.read(DailyTimerChanalAddr - i);
	}
}

void AquaEEPROM::SaveDailyTimerToERROM() {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		EEPROM.write(DailyTimerHourStartAddr - i, Helper.data.DailyTimerHourStart[i]);
		EEPROM.write(DailyTimerHourEndAddr - i, Helper.data.DailyTimerHourEnd[i]);
		EEPROM.write(DailyTimerMinStartAddr - i, Helper.data.DailyTimerMinStart[i]);
		EEPROM.write(DailyTimerMinEndAddr - i, Helper.data.DailyTimerMinEnd[i]);
		EEPROM.write(DailyTimerStateAddr - i, Helper.data.DailyTimerState[i]);
		EEPROM.write(DailyTimerChanalAddr - i, Helper.data.DailyTimerChanal[i]);
	}
}
void AquaEEPROM::LoadSecondsTimersReadFromERROM() {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		Helper.data.SecondTimerHourStart[i] = EEPROM.read(SecondTimerHourStartAddr - i);
		Helper.data.SecondTimerMinStart[i] = EEPROM.read(SecondTimerMinStartAddr - i);
		if (EEPROM.read(SecondTimerDurationAddr - i) > 0)
			Helper.data.SecondTimerDuration[i] = EEPROM.read(SecondTimerDurationAddr - i);
		Helper.data.SecondTimerState[i] = EEPROM.read(SecondTimerStateAddr - i);
		Helper.data.SecondTimerCanal[i] = EEPROM.read(SecondTimerCanalAddr - i);
	}
}
void AquaEEPROM::SaveSecondsTimerToERROM() {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		EEPROM.write(SecondTimerHourStartAddr - i, Helper.data.SecondTimerHourStart[i]);
		EEPROM.write(SecondTimerMinStartAddr - i, Helper.data.SecondTimerMinStart[i]);
		EEPROM.write(SecondTimerDurationAddr - i, Helper.data.SecondTimerDuration[i]);
		EEPROM.write(SecondTimerStateAddr - i, Helper.data.SecondTimerState[i]);
		EEPROM.write(SecondTimerCanalAddr - i, Helper.data.SecondTimerCanal[i]);
	}
}
void AquaEEPROM::LoadHoursTimersReadFromERROM() {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		Helper.data.HoursTimerMinStart[i] = EEPROM.read(DailyHoursTimerMinStartAddr - i);
		Helper.data.HoursTimerMinStop[i] = EEPROM.read(DailyHoursTimerMinDurationAddr - i);
		Helper.data.HoursTimerState[i] = EEPROM.read(DailyHoursTimerStateAddr - i);
		Helper.data.HoursTimerCanal[i] = EEPROM.read(DailyHoursTimerCanalAddr - i);
	}
}
void AquaEEPROM::SaveHoursTimerToERROM() {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		EEPROM.write(DailyHoursTimerMinStartAddr - i, Helper.data.HoursTimerMinStart[i]);
		EEPROM.write(DailyHoursTimerMinDurationAddr - i, Helper.data.HoursTimerMinStop[i]);
		EEPROM.write(DailyHoursTimerStateAddr - i, Helper.data.HoursTimerState[i]);
		EEPROM.write(DailyHoursTimerCanalAddr - i, Helper.data.HoursTimerCanal[i]);
	}
}

void AquaEEPROM::LoadTempTimerFromERROM(DallasTemperature ds) {
	for (byte j = 0; j < MAX_TEMP_SENSOR; j++) {
		for (byte i = 0; i < 8; i++) {
			Helper.data.addrThermometer[j][i] = EEPROM.read(addrTempSensor - j * 8 + i);
		}
		ds.setResolution(Helper.data.addrThermometer[j], TEMPERATURE_PRECISION);
	}
	for (byte i = 0; i < MAX_TEMP_SENSOR; i++) {
		Helper.data.TempTimerState[i] = EEPROM.read(TempTimerStateAddr - i);
		Helper.data.TempTimerMinStart[i] = EEPROM.read(TempTimerMinStartAddr - i);
		Helper.data.TempTimerMaxEnd[i] = EEPROM.read(TempTimerMaxEndAddr - i);
		Helper.data.TempTimerChanal[i] = EEPROM.read(TempTimerChanalAddr - i);

	}

}

void AquaEEPROM::SaveTempSensorAdress(byte i, byte j) {
	EEPROM.write(addrTempSensor - i * 8 + j, Helper.data.addrThermometer[i][j]);
}

void AquaEEPROM::SaveTempTimerToERROM() {
	for (byte i = 0; i < MAX_TEMP_SENSOR; i++) {
		EEPROM.write(TempTimerStateAddr - i, Helper.data.TempTimerState[i]);
		EEPROM.write(TempTimerMinStartAddr - i, Helper.data.TempTimerMinStart[i]);
		EEPROM.write(TempTimerMaxEndAddr - i, Helper.data.TempTimerMaxEnd[i]);
		EEPROM.write(TempTimerChanalAddr - i, Helper.data.TempTimerChanal[i]);
	}
}

void AquaEEPROM::SavePHTimerToERROM() {
	for (byte i = 0; i < MAX_TIMERS_PH; i++) {
		EEPROM.write(PHTimerStartAddr - i, Helper.data.PHTimerStart[i]);
		EEPROM.write(PHTimerEndAddr - i, Helper.data.PHTimerEnd[i]);
		EEPROM.write(PHTimerStateAddr - i, Helper.data.PHTimerState[i]);
		EEPROM.write(PHTimerCanalAddr - i, Helper.data.PHTimerCanal[i]);
		EEPROM.write(PHTimer401Addr - i * 2, Helper.data.PHTimer401[i]);
		EEPROM.write(PHTimer686Addr - i * 2, Helper.data.PHTimer686[i]);
	}
}

void AquaEEPROM::LoadPHTimerToERROM() {
	for (byte i = 0; i < MAX_TIMERS_PH; i++) {
		Helper.data.PHTimerStart[i] = EEPROM.read(PHTimerStartAddr - i);
		Helper.data.PHTimerEnd[i] = EEPROM.read(PHTimerEndAddr - i);
		Helper.data.PHTimerState[i] = EEPROM.read(PHTimerStateAddr - i);
		Helper.data.PHTimerCanal[i] = EEPROM.read(PHTimerCanalAddr - i);
		Helper.data.PHTimer401[i] = EEPROM.read(PHTimer401Addr - i * 2);
		Helper.data.PHTimer686[i] = EEPROM.read(PHTimer686Addr - i * 2);
	}
}

void AquaEEPROM::SaveLcdSetings() {
	EEPROM.write(LCD_LED_ON_ADDR, Helper.data.indexDelayLCDLedOn);
	EEPROM.write(LCD_BACK_ADDR, Helper.data.indexDelayLCDBackInmainScreen);
	EEPROM.write(LCD_BUTTON_ADDR, Helper.data.indexDelayLCDButton);
	EEPROM.write(LCD_SOUND_ADDR, Helper.data.isTone);

}
void AquaEEPROM::LoadLcdSetings() {
	Helper.data.indexDelayLCDLedOn = EEPROM.read(LCD_LED_ON_ADDR);
	Helper.data.indexDelayLCDBackInmainScreen = EEPROM.read(LCD_BACK_ADDR);
	Helper.data.indexDelayLCDButton = EEPROM.read(LCD_BUTTON_ADDR);
	Helper.data.isTone = EEPROM.read(LCD_SOUND_ADDR);
}

void AquaEEPROM::LoadWiFiSettings() {
	Helper.data.ntp_update = (bool) EEPROM.read(NTP_UPDATE_ADDR);
	Helper.data.auto_connect = (bool) EEPROM.read(AUTO_CONNECT_ADDR);
}

void AquaEEPROM::SaveWifiSettings() {
	EEPROM.write(NTP_UPDATE_ADDR, (byte) Helper.data.ntp_update);
	EEPROM.write(AUTO_CONNECT_ADDR, (byte) Helper.data.auto_connect);
}

uint16_t AquaEEPROM::SaveUTCSetting(uint16_t utc) {
	EEPROM.write(UTC_ADDR, utc);
	EEPROM.commit();
	return LoadUTCSetting();
}

uint16_t AquaEEPROM::LoadUTCSetting() {
	return EEPROM.read(UTC_ADDR);
}


/*
 * AquaEEPROM.cpp
 *
 *  Created on: Oct 10, 2019
 *      Author: doc
 */
#include "AquaEEPROM.h"

void AquaEEPROM::Init() {
	EEPROM.begin(MAX_EEPROM);
	OnFirstLunch();
	LoadChanelState();
	LoadPWMChanelState();
	LoadPWMTimersReadFromERROM();
	LoadDailyTimersReadFromERROM();
	LoadHoursTimersReadFromERROM();
	LoadSecondsTimersReadFromERROM();
	LoadTempTimerFromERROM();
	LoadLcdSetings();
	LoadPHTimerToERROM();
	LoadWiFiSettings();
	LoadFANSettings();
	LoadTempStats();
	LoadPhStats();
}

void AquaEEPROM::SaveChanalState() {
	for (byte i = 0; i < MAX_CHANALS; i++) {
		EEPROM.write(ChanalsStateAddr - i, Helper.data.StateChanals[i]);
	}
	EEPROM.commit();
}

void AquaEEPROM::LoadChanelState() {
	for (byte i = 0; i < MAX_CHANALS; i++) {
		Helper.data.StateChanals[i] = EEPROM.read(ChanalsStateAddr - i);
	}
}

void AquaEEPROM::SavePWMChanalState() {
	for (byte i = 0; i < MAX_CHANALS_PWM; i++) {
		EEPROM.write(ChanalsPWMStateAddr - i, Helper.data.StatePWMChanals[i]);
	}
	EEPROM.commit();
}
void AquaEEPROM::LoadPWMChanelState() {
	for (byte i = 0; i < MAX_CHANALS_PWM; i++) {
		Helper.data.StatePWMChanals[i] = EEPROM.read(ChanalsPWMStateAddr - i);
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
	EEPROM.commit();
}

void AquaEEPROM::LoadPWMTimersReadFromERROM() {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		Helper.data.TimerPWMHourStart[i] = EEPROM.read(PWMTimerHourStartAddr - i);
		Helper.data.TimerPWMHourEnd[i] = EEPROM.read(PWMTimerHourEndAddr - i);
		Helper.data.TimerPWMMinStart[i] = EEPROM.read(PWMTimerMinStartAddr - i);
		Helper.data.TimerPWMMinEnd[i] = EEPROM.read(PWMTimerMinEndAddr - i);
		Helper.data.TimerPWMState[i] = EEPROM.read(PWMTimerStateAddr - i);
		Helper.data.TimerPWMChanal[i] = EEPROM.read(PWMTimerChanalAddr - i);
		Helper.data.TimerPWMDuration[i] = EEPROM.read(PWMTimerHourDurationAddr - i);
		Helper.data.TimerPWMLevel[i] = EEPROM.read(PWMTimerHourLevelAddr - i);
	}
}

void AquaEEPROM::SavePWMTimerToERROM() {
	for (byte i = 0; i < MAX_TIMERS; i++) {
		EEPROM.write(PWMTimerHourStartAddr - i, Helper.data.TimerPWMHourStart[i]);
		EEPROM.write(PWMTimerHourEndAddr - i, Helper.data.TimerPWMHourEnd[i]);
		EEPROM.write(PWMTimerMinStartAddr - i, Helper.data.TimerPWMMinStart[i]);
		EEPROM.write(PWMTimerMinEndAddr - i, Helper.data.TimerPWMMinEnd[i]);
		EEPROM.write(PWMTimerStateAddr - i, Helper.data.TimerPWMState[i]);
		EEPROM.write(PWMTimerChanalAddr - i, Helper.data.TimerPWMChanal[i]);
		EEPROM.write(PWMTimerHourDurationAddr - i, Helper.data.TimerPWMDuration[i]);
		EEPROM.write(PWMTimerHourLevelAddr - i, Helper.data.TimerPWMLevel[i]);
	}
	EEPROM.commit();
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
	EEPROM.commit();
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
	EEPROM.commit();
}

void AquaEEPROM::LoadTempTimerFromERROM() {
	for (byte j = 0; j < MAX_TEMP_SENSOR; j++) {
		for (byte i = 0; i < 8; i++) {
			Helper.data.addrThermometer[j][i] = EEPROM.read(addrTempSensor - j * 8 + i);
		}
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
	EEPROM.commit();
}

void AquaEEPROM::SaveTempTimerToERROM() {
	for (byte i = 0; i < MAX_TEMP_SENSOR; i++) {
		EEPROM.write(TempTimerStateAddr - i, Helper.data.TempTimerState[i]);
		EEPROM.write(TempTimerMinStartAddr - i, Helper.data.TempTimerMinStart[i]);
		EEPROM.write(TempTimerMaxEndAddr - i, Helper.data.TempTimerMaxEnd[i]);
		EEPROM.write(TempTimerChanalAddr - i, Helper.data.TempTimerChanal[i]);
	}
	EEPROM.commit();
}

void AquaEEPROM::SavePHTimerToERROM() {
	Serial.println("SavePHTimerToERROM");
	for (byte i = 0; i < MAX_TIMERS_PH; i++) {
		EEPROM.write(PHTimerStartAddr - i, Helper.data.PHTimerStart[i]);
		EEPROM.write(PHTimerEndAddr - i, Helper.data.PHTimerEnd[i]);
		EEPROM.write(PHTimerStateAddr - i, Helper.data.PHTimerState[i]);
		EEPROM.write(PHTimerCanalAddr - i, Helper.data.PHTimerCanal[i]);
	}
	for (byte i = 0; i < MAX_TIMERS_PH * 2; i++) {
		EEPROM_writeint(PHTimer401Addr - i * 2, Helper.data.PHCalibrationValue[i]);
		EEPROM_writeint(PHTimer686Addr - i * 2, Helper.data.PHCalibrationVoltage[i]);
	}

	EEPROM.commit();
}

void AquaEEPROM::LoadPHTimerToERROM() {
	for (byte i = 0; i < MAX_TIMERS_PH; i++) {
		Helper.data.PHTimerStart[i] = EEPROM.read(PHTimerStartAddr - i);
		Helper.data.PHTimerEnd[i] = EEPROM.read(PHTimerEndAddr - i);
		Helper.data.PHTimerState[i] = EEPROM.read(PHTimerStateAddr - i);
		Helper.data.PHTimerCanal[i] = EEPROM.read(PHTimerCanalAddr - i);
	}
	for (byte i = 0; i < MAX_TIMERS_PH * 2; i++) {
		Helper.data.PHCalibrationValue[i] = EEPROM_readint(PHTimer401Addr - i * 2);
		Helper.data.PHCalibrationVoltage[i] = EEPROM_readint(PHTimer686Addr - i * 2);
	}
}

void AquaEEPROM::SaveLcdSetings() {
	EEPROM.write(LCD_LED_ON_ADDR, Helper.data.indexDelayLCDLedOn);
	EEPROM.write(LCD_BACK_ADDR, Helper.data.indexDelayLCDBackInmainScreen);
	EEPROM.write(LCD_BUTTON_ADDR, Helper.data.indexDelayLCDButton);
	EEPROM.write(LCD_SOUND_ADDR, Helper.data.isTone);
	EEPROM.commit();
}
void AquaEEPROM::LoadLcdSetings() {
	Helper.data.indexDelayLCDLedOn = EEPROM.read(LCD_LED_ON_ADDR);
	Helper.data.indexDelayLCDBackInmainScreen = EEPROM.read(LCD_BACK_ADDR);
	Helper.data.indexDelayLCDButton = EEPROM.read(LCD_BUTTON_ADDR);
	Helper.data.isTone = EEPROM.read(LCD_SOUND_ADDR);
}
void AquaEEPROM::LoadFANSettings() {
	for (byte i = 0; i < MAX_CHANALS_FAN; i++) {
		Helper.data.FANTimerMinStart[i] = EEPROM.read(FANTimerMinStartAddr - i);
		Helper.data.FANTimerMaxEnd[i] = EEPROM.read(FANTimerMaxEndAddr - i);
		Helper.data.FANSensor[i] = EEPROM.read(FANSensorAddr - i);
	}
}
void AquaEEPROM::SaveFANSettings() {
	for (byte i = 0; i < MAX_CHANALS_FAN; i++) {
		EEPROM.write(FANTimerMinStartAddr - i, Helper.data.FANTimerMinStart[i]);
		EEPROM.write(FANTimerMinStartAddr - i, Helper.data.FANTimerMaxEnd[i]);
		EEPROM.write(FANSensorAddr - i, Helper.data.FANSensor[i]);
	}
}
void AquaEEPROM::LoadWiFiSettings() {
	Helper.data.ntp_update = (bool) EEPROM.read(NTP_UPDATE_ADDR);
	Helper.data.auto_connect = (bool) EEPROM.read(AUTO_CONNECT_ADDR);
	Helper.data.auto_update = (bool) EEPROM.read(AUTO_UPDATE_ADDR);
	Helper.data.debug = (bool) EEPROM.read(AUTO_DEBUG_ADDR);

}

void AquaEEPROM::SaveWifiSettings() {
	EEPROM.write(NTP_UPDATE_ADDR, (byte) Helper.data.ntp_update);
	EEPROM.write(AUTO_CONNECT_ADDR, (byte) Helper.data.auto_connect);
	EEPROM.write(AUTO_UPDATE_ADDR, (byte) Helper.data.auto_update);
	EEPROM.write(AUTO_DEBUG_ADDR, (byte) Helper.data.debug);
	EEPROM.commit();
}

void AquaEEPROM::SaveTempStats(byte sensor, byte hour) {
	EEPROM.write(TempStatsAddr + 24 * sensor + hour, (byte) Helper.data.TempStats[sensor][hour]);
	EEPROM.commit();
}
void AquaEEPROM::LoadTempStats() {
	for (byte sensor = 0; sensor < MAX_TEMP_SENSOR; sensor++) {
		for (byte hour = 0; hour < MAX_STATS; hour++) {
			Helper.data.TempStats[sensor][hour] = EEPROM.read(TempStatsAddr + 24 * sensor + hour);
		}
	}
}

void AquaEEPROM::SavePhStats(byte sensor, byte hour) {
	EEPROM_writeint(PHStatsAddr + 48 * sensor + 2 * hour, Helper.data.PHStats[sensor][hour]);
	EEPROM.commit();
}
void AquaEEPROM::LoadPhStats() {
	for (byte sensor = 0; sensor < MAX_TIMERS_PH; sensor++) {
		for (byte hour = 0; hour < MAX_STATS; hour++) {
			Helper.data.PHStats[sensor][hour] = EEPROM_readint(PHStatsAddr + 48 * sensor + 2 * hour);
		}
	}
}
uint16_t AquaEEPROM::SaveUTCSetting(uint16_t utc) {
	EEPROM.write(UTC_ADDR, utc);
	EEPROM.commit();
	return LoadUTCSetting();
}

uint16_t AquaEEPROM::LoadUTCSetting() {
	return EEPROM.read(UTC_ADDR);
}

void AquaEEPROM::OnFirstLunch() {
	if (EEPROM.read(ADDR_FIRST_LAUNCH) != PARAM_FIRST_LAUNCH) {
		for (int i = 0; i < MAX_EEPROM; i++) {
			EEPROM.write(i, 0);
		}
		EEPROM.commit();
		Display.SendLog("Clear EEPROM On First Launch");
		SaveChanalState();
		SavePWMChanalState();
		SavePWMTimerToERROM();
		SaveDailyTimerToERROM();
		SaveSecondsTimerToERROM();
		SaveHoursTimerToERROM();
		SaveTempTimerToERROM();
		SavePHTimerToERROM();
		SaveUTCSetting(3);
		SaveFANSettings();
		EEPROM.write(ADDR_FIRST_LAUNCH, PARAM_FIRST_LAUNCH);
		EEPROM.commit();
		SaveWifiSettings();
		SaveTempTimerToERROM();
	}
}
// read double word from EEPROM, give starting address
unsigned long AquaEEPROM::EEPROM_readlong(int address) {
//use word read function for reading upper part
	unsigned long dword = EEPROM_readint(address);
//shift read word up
	dword = dword << 16;
// read lower word from EEPROM and OR it into double word
	dword = dword | EEPROM_readint(address + 2);
	return dword;
}

//write word to EEPROM
void AquaEEPROM::EEPROM_writeint(int address, int value) {
	EEPROM.write(address, highByte(value));
	EEPROM.write(address + 1, lowByte(value));
}

//write long integer into EEPROM
void AquaEEPROM::EEPROM_writelong(int address, unsigned long value) {
//truncate upper part and write lower part into EEPROM
	EEPROM_writeint(address + 2, word(value));
//shift upper part down
	value = value >> 16;
//truncate and write
	EEPROM_writeint(address, word(value));
}

unsigned int AquaEEPROM::EEPROM_readint(int address) {
	unsigned int word = word(EEPROM.read(address), EEPROM.read(address + 1));
	return word;
}

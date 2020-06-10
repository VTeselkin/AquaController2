/*
 * AquaTemp.c
 *
 *  Created on: Oct 2, 2019
 *      Author: doc
 */
#include "AquaTemp.h"

OneWire oneWire(TEMP_SENS);
DallasTemperature ds(&oneWire);
bool isTempCanalWarning = false;
bool isWaitTemp = false;
bool isUpdateTemp = false;
byte sensorIndexWarning = 0;
unsigned long lastTempStatetime = 0;

/**
 * Bus initialization for temperature sensors
 */
void AquaTemp::Init(AquaEEPROM aquaEEPROM) {
	ds.begin();
	ds.setWaitForConversion(false);
	aquaEEPROM.LoadTempTimerFromERROM(ds);
}

/**
 * Obtaining temperature with DS18B20
 */
void AquaTemp::GetTemperature() {

	if (Helper.GetTimeNow().Second % FREQURENCY_SEND_TEMP != 0) {
		isUpdateTemp = false;
		return;
	}

	if (isUpdateTemp)
		return;
	isUpdateTemp = true;
	ds.requestTemperatures();
	for (byte var = 0; var < MAX_TEMP_SENSOR; var++) {
		//We obtain the temperature index
		word temp = (word) (ds.getTempC(Helper.data.addrThermometer[var]) * 100);

		if (temp > MAX_TEMP || temp < MIN_TEMP) {
			Helper.data.TempSensorState[var] = DISCONNECT_SENSOR;
			Helper.data.TempSensor[var] = 0;
		} else {
			Helper.data.TempSensorState[var] = CONNECT_SENSOR;
			Helper.data.TempSensor[var] = ConvertTempWordToByte(temp);
		}
	}

	return;
}

byte ConvertTempWordToByte(unsigned short temp) {
	return (temp - MIN_TEMP) / STEP;
}

word ConvertTempByteToWord(unsigned short temp) {
	return temp * STEP + MIN_TEMP;
}

/**
 * Checking the status of programs for temperature sensors
 */
void AquaTemp::CheckStateTempTimer(void (*GetChanalState)(typeResponse type), bool isNeedEnableZeroCanal) {
//	if (MenuIndex >= 31 && MenuIndex <= 35)
//		return;
	if (Helper.GetTimeNow().Second % FREQURENCY_SEND_TEMP == 0) {
		if (!isWaitTemp) {
			isTempCanalWarning = false;
			isWaitTemp = true;
			for (byte canalIndex = 0; canalIndex < MAX_CHANALS; canalIndex++) {
				bool result = false;
				for (byte indexSensor = 0; indexSensor < MAX_TEMP_SENSOR; indexSensor++) {
					if (Helper.data.TempTimerChanal[indexSensor] == canalIndex&& Helper.data.TempTimerState[indexSensor] == ENABLE_TIMER
					&& Helper.data.TempSensorState[indexSensor] != DISCONNECT_SENSOR) {
						if (Helper.data.StateChanals[Helper.data.TempTimerChanal[indexSensor]] == AUTO_CHANAL) {
							if (CheckStateTemp(indexSensor, canalIndex))
								result = true;
						}
					}
				}
				CheckCollisionsTemp(canalIndex, result, TIMER_TEMP, GetChanalState, isNeedEnableZeroCanal);
			}
		}
	} else {
		isWaitTemp = false;
	}
}

/**
 * Check channel status before making any changes.
 * If we have a conflict timer switches the channel then we change the type
 * of timer is on this channel in the property stateChanalsTimer
 */
bool CheckCollisionsTemp( byte chanal, bool isEnable, byte timerType, void (*GetChanalState)(typeResponse type), bool isNeedEnableZeroCanal) {
	if (chanal == CHANAL_BTN_DISABLE && isNeedEnableZeroCanal)
		return false;
	if (isEnable) {
		if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] == TIMER_OFF) {
			Helper.data.CurrentStateChanalsByTypeTimer[chanal] = timerType;
			GetChanalState(CANAL);
			return true;
		} else {
			if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] != TIMER_TEMP && timerType == TIMER_TEMP) {
				Helper.data.CurrentStateChanalsByTypeTimer[chanal] = timerType;
				GetChanalState(CANAL);
			}
			return false;
		}
	} else if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] != TIMER_OFF) {
		if (Helper.data.CurrentStateChanalsByTypeTimer[chanal] == timerType) {
			Helper.data.CurrentStateChanalsByTypeTimer[chanal] = TIMER_OFF;
			GetChanalState(CANAL);
			return true;
		}
	}
	return false;
}

bool AquaTemp::GetTemperatureIsWarning() {
	return isTempCanalWarning;
}

byte AquaTemp::GetTemperatereSensorIndexWarning() {
	return sensorIndexWarning;
}

/**
 * Checking whether the program can be run for temperature sensors
 */
bool AquaTemp::CheckStateTemp(byte sensorIndex, byte canalIndex) {

//Cooling
//-----Tmin=25------Tmax=20------/
	if (Helper.data.TempTimerMinStart[sensorIndex] > Helper.data.TempTimerMaxEnd[sensorIndex]) {
//-----Tmin=25------Tmax=20----Tcur=26--/
		if (Helper.data.TempSensor[sensorIndex] >= Helper.data.TempTimerMinStart[sensorIndex]) {
			CheckStateWarningTemp(sensorIndex, false);
			return true;
		}
//-----Tmin=25------Tmax=22----Tcur=24--/
		if (Helper.data.TempSensor[sensorIndex] < Helper.data.TempTimerMinStart[sensorIndex] && Helper.data.TempSensor[sensorIndex] > Helper.data.TempTimerMaxEnd[sensorIndex]) {
			if (Helper.data.CurrentStateChanalsByTypeTimer[sensorIndex] == TIMER_TEMP) {
				CheckStateWarningTemp(sensorIndex, false);
				return true;
			}
		}
		CheckStateWarningTemp(sensorIndex, false);
		return false;
//Warming
//-----Tmin=20------Tmax=25------/
	} else if (Helper.data.TempTimerMinStart[sensorIndex] < Helper.data.TempTimerMaxEnd[sensorIndex]) {
//--Tcur=19---Tmin=20------Tmax=25------/
		if (Helper.data.TempSensor[sensorIndex] < Helper.data.TempTimerMinStart[sensorIndex]) {
			CheckStateWarningTemp(sensorIndex, true);
			return true;
		}
//-----Tmin=20---Tcur=22---Tmax=25------/
		if (Helper.data.TempSensor[sensorIndex] >= Helper.data.TempTimerMinStart[sensorIndex] && Helper.data.TempSensor[sensorIndex] < Helper.data.TempTimerMaxEnd[sensorIndex]) {
			if (Helper.data.CurrentStateChanalsByTypeTimer[canalIndex] == TIMER_TEMP) {
				CheckStateWarningTemp(sensorIndex, true);
				return true;
			}
		}
		CheckStateWarningTemp(sensorIndex, true);
		return false;

	}
	return false;

}

void AquaTemp::CheckStateWarningTemp(byte sensorIndex, bool isWarming) {

	if (isWarming) {
		if (Helper.data.TempSensor[sensorIndex] < Helper.data.TempTimerMinStart[sensorIndex]) {
			if (Helper.data.TempTimerMinStart[sensorIndex] - Helper.data.TempSensor[sensorIndex] > 4) {
				isTempCanalWarning = true;
			}
		}
		if (Helper.data.TempSensor[sensorIndex] > Helper.data.TempTimerMaxEnd[sensorIndex]) {
			if (Helper.data.TempSensor[sensorIndex] - Helper.data.TempTimerMaxEnd[sensorIndex] > 4) {
				isTempCanalWarning = true;
			}
		}
	} else {
		if (Helper.data.TempSensor[sensorIndex] > Helper.data.TempTimerMinStart[sensorIndex]) {
			if (Helper.data.TempSensor[sensorIndex] - Helper.data.TempTimerMinStart[sensorIndex] > 4) {
				isTempCanalWarning = true;
			}
		}
	}
	if (isTempCanalWarning) {
		sensorIndexWarning = sensorIndex;
	}

}


/**
 * Sensor initialization method DS18B20
 */
void AquaTemp::SetDalasSensor(AquaEEPROM eeprom) {
	DeviceAddress device;
	byte newIndex = 0;
//Search all temperatures device
	for (byte var = 0; var < ds.getDeviceCount(); var++) {
//If we found the address of the sensor
		if (ds.getAddress(device, var)) {
			bool isNew = true;
//Looking for the address of the found device in the current address list
			for (byte i = 0; i < sizeof(Helper.data.addrThermometer); i++) {
				if (CompareDeviceAddress(Helper.data.addrThermometer[i], device)) {
					isNew = false;
				}
			}
//If the address is new, we add it to the list of new addresses
			if (isNew) {
				for (byte i = 0; i < 8; i++) {
					Helper.data.addrNewThermometer[newIndex][i] = device[i];
				}
				newIndex++;
			}
		}
	}

//Replace the disabled sensors with new ones
//If the new sensors a few they replace the old in accordance with the order of their addresses
	byte newIndex2 = 0;
	for (byte i = 0; i < MAX_TEMP_SENSOR; i++) {

		if (Helper.data.TempSensorState[i] == DISCONNECT_SENSOR && newIndex > newIndex2) {
			for (byte j = 0; j < 8; j++) {
				Helper.data.addrThermometer[i][j] = Helper.data.addrNewThermometer[newIndex2][j];
				eeprom.SaveTempSensorAdress(i,j);
			}
			newIndex2++;
		}
		ds.setResolution(Helper.data.addrThermometer[i], TEMPERATURE_PRECISION);
	}

}
/**
 * Method for comparing addresses of DS18B20 devices
 */
bool CompareDeviceAddress(DeviceAddress &device1, DeviceAddress &device2) {
	for (byte j = 0; j < 8; j++) {
		if (device1[j] != device2[j])
			return false;
	}
	return true;
}

bool AquaTemp::AddTempElementToStats(){
	if (millis() > lastTempStatetime + DELAY_TEMP_UPDATE_STATE) {
		lastTempStatetime = millis();
		for (byte j = 0; j < MAX_TEMP_SENSOR; j++) {
			for (byte i = 1; i < MAX_STATS; i++) {
				Helper.data.TempStats[j][i - 1] = Helper.data.TempStats[j][i];
			}
		}
		for (byte j = 0; j < MAX_TEMP_SENSOR; j++) {
			Helper.data.TempStats[j][MAX_STATS - 1] = Helper.data.TempSensor[j];
		}
		return true;
	}
	return false;
}

/*
 * AquaCanal.h
 *
 *  Created on: Oct 9, 2019
 *      Author: doc
 */
#pragma once
#include "Arduino.h"
#include <AquaHelper.h>
#include <PCA9685.h>

class AquaCanal{
	unsigned long _timeDebugLED[4] = { 0, 0, 0, 0 };
	byte _stateDebugLED[4] = { 0, 0, 0, 0 };
	typeLightLED _typeDebugLED[4] = { NONE, NONE, NONE, NONE };

public:

	void Init();
	void SetCanal(byte canal, byte state);
	void SetPWMCanal(byte canal, word level);


	byte GetCanal(byte canal);
	word GetPWMCanalLevel(byte canal);
	byte GetPWMCanalState(byte canal);
	void SetStateCanal(void (*GetChanalState)(typeResponse type));
	void SetStatePWMCanal(void (*GetChanalState)(typeResponse type));
	void SetPWMCanalOn(byte canal);
	void SetPWMCanalOff(byte canal);
	void SetLEDError(typeLightLED type);
	void SetLEDConnect(typeLightLED type);
	void SetLEDRx(typeLightLED type);
	void SetLEDTx(typeLightLED type);
	void DisableLED();

	private:
	void SetPWMOnCanal(bool isOn, byte timers);
	void SetDebugLED(typeLightLED type, byte canal, byte index);
};

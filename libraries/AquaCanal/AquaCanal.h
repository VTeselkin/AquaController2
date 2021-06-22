/*
 * AquaCanal.h
 *
 *  Created on: Oct 9, 2019
 *      Author: doc
 */
#pragma once
#pragma once
#ifndef AquaCanal_h
#define AquaCanal_h
#include "Arduino.h"
#include <AquaHelper.h>
#include <PCA9685.h>

class AquaCanal{
	static unsigned long _timeDebugLED[4];
		static byte _stateDebugLED[4];
		static typeLightLED _typeDebugLED[4];

public:

	void Init();
	void SetCanal(byte canal, byte state);
	static void SetPWMCanal(byte canal, word level);


	byte GetCanal(byte canal);
	word GetPWMCanalLevel(byte canal);
	byte GetPWMCanalState(byte canal);
	void SetStateCanal(void (*GetChanalState)(typeResponse type));
	void SetStatePWMCanal(void (*GetChanalState)(typeResponse type));
	void SetPWMCanalOn(byte canal);
	static void SetPWMCanalOff(byte canal);
	static void SetLEDError(typeLightLED type);
	static void SetLEDConnect(typeLightLED type);
	static void SetLEDRx(typeLightLED type);
	static void SetLEDTx(typeLightLED type);
	void DisableLED();

	private:
	void SetPWMOnCanal(bool isOn, byte timers);
	static void SetDebugLED(typeLightLED type, byte canal, byte index);
};
#ifdef AquaCanal
#undef AquaCanal
#endif

extern AquaCanal Canal;

#endif

/*
 * AquaCanal.h
 *
 *  Created on: Oct 9, 2019
 *      Author: doc
 */
#pragma once
#include "Arduino.h"
#include <AquaHelper.h>
#include <Adafruit_PWMServoDriver.h>

class AquaCanal{
public:
	void Init();
	void SetCanal(byte canal, byte state);
	void SetPWMCanal(byte canal, word level);


	byte GetCanal(byte canal);
	word GetPWMCanalLevel(byte canal);
	byte GetPWMCanalState(byte canal);
	void SetStateCanal(void (*GetChanalState)(String));
	void SetStatePWMCanal(void (*GetChanalState)(String));

	private:
	void SetPWMOnCanal(bool isOn, byte timers);
};

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

	private:
	void SetPWMOnCanal(bool isOn, byte timers);
};

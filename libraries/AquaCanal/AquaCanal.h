/*
 * AquaCanal.h
 *
 *  Created on: Oct 9, 2019
 *      Author: doc
 */
#pragma once
#include "Arduino.h"
#include <PCA9685.h>
#include <AquaHelper.h>

class AquaCanal{
public:
	void Init();
	void SetCanal(byte canal, byte state);
	void SetPWMCanal(byte canal, word level);


	byte GetCanal(byte canal);
	word GetPWMCanalLevel(byte canal);
	byte GetPWMCanalState(byte canal);
	void SetStateCanal(void (*GetChanalState)());
	void SetStatePWMCanal(void (*GetChanalState)());

	private:
	void SetPWMOnCanal(bool isOn, byte timers);
};

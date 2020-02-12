/*
 * AquaUpdate.h
 *
 *  Created on: Nov 18, 2019
 *      Author: doc
 */
#pragma once
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <AquaHelper.h>
#include <HttpUpdate.h>



#ifndef LIBRARIES_AQUAUPDATE_AQUAUPDATE_H_
#define LIBRARIES_AQUAUPDATE_AQUAUPDATE_H_

class AquaUpdate {
public:
	void CheckOTAUpdate(bool isForce, void (*funcChangeLog)(String), DynamicJsonBuffer &jsonBuffe);
};
String OTAUpdate(String host, DynamicJsonBuffer &jsonBuffer);
void SendResultOTAUpdate(t_httpUpdate_return ret, void (*funcChangeLog)(String)) ;

#endif /* LIBRARIES_AQUAUPDATE_AQUAUPDATE_H_ */

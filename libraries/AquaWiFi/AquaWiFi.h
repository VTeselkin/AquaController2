/*
 * AquaWiFi.h
 *
 *  Created on: Oct 31, 2019
 *      Author: doc
 */

#pragma once
#include "Arduino.h"
#include <WiFiManager.h>
#include <WiFi.h>
#include <AquaNTP.h>
#include <AquaHTTP.h>
#include <AquaHelper.h>
#include <AquaUpdate.h>
#include <SPIFFS.h>
#include <FS.h>

class AquaWiFi {
public:
	void Init(void (*ChangeLog)(String), void (*GetUDPRequest)(typeResponse, String), uint16_t (*NTPUpdate)(uint16_t), void(*ChandeDebugLED)(typeDebugLED led, typeLightLED type));
	void WaitRequest();
	void CacheResponse(typeResponse type, String json);
	void SendCacheResponse(typeResponse type, bool sendCache);
	void Connection();
	void StartCaching();

private:


};
void UDPSendMessage(String message, bool isBroadcast);
void UDPSendError(String error);
void saveConfigCallback();
void configModeCallback(WiFiManager *myWiFiManager);
bool SendWifiIp();
bool Connection();
void SendFromUDPToController(String inString);
void SendPOSTSuccess(typeResponse type, String inString);


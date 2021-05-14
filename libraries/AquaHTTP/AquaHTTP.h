/*
 * AquaHTTP.h
 *
 *  Created on: Oct 31, 2019
 *      Author: doc
 */

#pragma once

#include <Arduino.h>
#include <AquaHTTP.h>
#include <AquaHelper.h>
#include <HTTPUpdate.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <FS.h>


class AquaHTTP{
public:
	void Init(Dictionary &responseCache, DynamicJsonBuffer &jsonBuffer);
	void HandleClient();
private:
};

void HttpSendJson(typeResponse type, String data, String param);
String getContentType(String filename);
void FS_init();


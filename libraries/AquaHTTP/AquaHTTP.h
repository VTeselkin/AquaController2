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
#include <WebSocketsServer.h>

class AquaHTTP{
public:
	void Init(Dictionary &responseCache, DynamicJsonBuffer &jsonBuffer);
	void HandleClient();
	void SocketUpdate(String updateJson);
private:
};

void HttpSendJson(typeResponse type, String data, String param);
void cbWebSocketsEvent (uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleFileList();
void handleFileCreate();
void handleFileDelete();
void handleFileUpload();
bool handleFileRead(String path);
String getContentType(String filename);
void FS_init();


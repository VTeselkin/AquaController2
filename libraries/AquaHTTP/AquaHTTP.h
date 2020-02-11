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

class AquaHTTP{
public:
	void Init(Dictionary &responseCache);
	void HandleClient();
private:
};

void HttpSendDeviceJson();
void HttpSendCanalJson();
void HttpSendDayJson();
void HttpSendHourJson();
void HttpSendSecJson();
void HttpSendTempJson();
void HttpSendSensorJson();
void HttpSendPhJson();
void HttpSendStatsJson();


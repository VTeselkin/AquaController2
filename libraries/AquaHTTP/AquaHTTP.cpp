/*
 * AquaHTTP.cpp
 *
 *  Created on: Oct 31, 2019
 *      Author: doc
 */

#include <AquaHTTP.h>
HTTPClient httpClient;
WebServer http(80);


Dictionary response = { { DEVICE, responseNull }, { CANAL, responseNull }, { TIMERDAY, responseNull }, { TIMERHOUR, responseNull }, { TIMERSEC,
		responseNull }, { TIMERTEMP, responseNull }, { TEMPSENSOR, responseNull }, { PH, responseNull }, { TEMPSTATS, responseNull } };

void AquaHTTP::Init(Dictionary &responseCache) {
	response = responseCache;
	//http.on("/", handleRoot);
	http.on("/device.json", HttpSendDeviceJson);
	http.on("/canal.json", HttpSendCanalJson);
	http.on("/timerday.json", HttpSendDayJson);
	http.on("/timerhour.json", HttpSendHourJson);
	http.on("/timersec.json", HttpSendSecJson);
	http.on("/timertemp.json", HttpSendTempJson);
	http.on("/tempsensor.json", HttpSendSensorJson);
	http.on("/ph.json", HttpSendPhJson);
	http.on("/tempstats.json", HttpSendStatsJson);
//	http.serveStatic("/js", SPIFFS, "/js");
//	http.serveStatic("/css", SPIFFS, "/css");
//	http.serveStatic("/img", SPIFFS, "/img");
//	http.serveStatic("/", SPIFFS, "/index.html");
	http.begin();
}

void AquaHTTP::HandleClient(){
	http.handleClient();
}

void HttpSendDeviceJson() {
	http.send(200, "application/json", response[DEVICE]);
}
void HttpSendCanalJson() {
	http.send(200, "application/json", response[CANAL]);
}
void HttpSendDayJson() {
	http.send(200, "application/json", response[TIMERDAY]);
}
void HttpSendHourJson() {
	http.send(200, "application/json", response[TIMERHOUR]);
}
void HttpSendSecJson() {
	http.send(200, "application/json", response[TIMERSEC]);
}
void HttpSendTempJson() {
	http.send(200, "application/json", response[TIMERTEMP]);
}
void HttpSendSensorJson() {
	http.send(200, "application/json", response[TEMPSENSOR]);
}
void HttpSendPhJson() {
	http.send(200, "application/json", response[PH]);
}
void HttpSendStatsJson() {
	http.send(200, "application/json", response[TEMPSTATS]);
}

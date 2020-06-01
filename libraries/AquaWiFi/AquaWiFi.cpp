/*
 * AquaWiFi.cpp
 *
 *  Created on: Oct 31, 2019
 *      Author: doc
 */

#include <AquaWiFi.h>
#include <ping.h>
#include <ArduinoJson.h>

WiFiUDP Udp; // @suppress("Abstract class cannot be instantiated")
IPAddress broadcastAddress;
AquaNTP ntp;
AquaHTTP web;
AquaUpdate update;
const IPAddress remote_ip(8, 8, 8, 8);
const size_t bufferSize = 6 * JSON_ARRAY_SIZE(10) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 300;
DynamicJsonBuffer jsonBuffer(bufferSize);

bool _isWiFiEnable = false;
bool _isError = false;
bool _isConnected = false;
bool isInterenetAvalible = false;

unsigned long timeIP, lastDeviceInfoTime, lastNTPtime, lastTemptime, lastPHTime;
unsigned int localUdpPort = 8888;

char incomingPacket[MAX_BUFFER];
void (*funcChangeLog)(String);
void (*funcGetUDPRequest)(typeResponse, String);
uint16_t (*funcNTPUpdate)(uint16_t);

word UTC3 = 3; //UTC+3

Dictionary responseCache = { { DEVICE, responseNull }, { CANAL, responseNull }, { PWMCANAL, responseNull }, { TIMERDAY,
		responseNull }, { TIMERHOUR, responseNull }, { TIMERSEC, responseNull }, { TIMERTEMP, responseNull }, {
		TEMPSENSOR, responseNull }, { PH, responseNull }, { PHTIMER, responseNull }, { TEMPSTATS, responseNull }, {
		PWMTIMER, responseNull } };

void AquaWiFi::Init(void (*ChangeLog)(String), void (*GetUDPRequest)(typeResponse, String),
		uint16_t (*NTPUpdate)(uint16_t)) {
	pinMode(2, OUTPUT);
	funcChangeLog = ChangeLog;
	funcGetUDPRequest = GetUDPRequest;
	funcNTPUpdate = NTPUpdate;
	timeIP = millis();
	lastNTPtime = millis();
	lastTemptime = millis();
	_isWiFiEnable = Helper.data.auto_connect;
	update.Init();
	if (Connection()) {
		_isConnected = true;
		_isError = false;
		SendWiFiLog("HTTP:Service started...");
		update.CheckOTAUpdate(true, funcChangeLog, jsonBuffer);
		StartCaching();
		web.Init(responseCache, jsonBuffer);
		if (isInterenetAvalible) {
			ntp.SetNTPTimeToController(ChangeLog);
		} else {
			SendWiFiLog("WAN:Not connection..");
		}
	}
}

bool Connection() {

	if (_isWiFiEnable) {
		SendWiFiLog("WiFi:Try connect...");
		WiFiManager wifiManager;

		//Disable debug log connection
		wifiManager.setDebugOutput(false);
		wifiManager.setAPCallback(configModeCallback);
		wifiManager.setSaveConfigCallback(saveConfigCallback);
		wifiManager.setTimeout(300);
		//set custom ip for portal
		wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1),
				IPAddress(255, 255, 255, 0));
		/**
		 * if we couldn't connected to save WiFi access point
		 * we will start our own access point with ip address 192.168.1.4
		 */
		if (!wifiManager.autoConnect("AP: AquaController")) {
			ESP.restart();
			return false;
		}
		Udp.begin(localUdpPort);
		broadcastAddress = (uint32_t) WiFi.localIP() | ~((uint32_t) WiFi.subnetMask());
		SendWifiIp(true);
		return true;
	} else {
		SendWiFiLog("WiFi:Disable...");
		return false;
	}
}

/**
 * callback for config mode
 */
void configModeCallback(WiFiManager *myWiFiManager) {
	if (_isWiFiEnable && !_isError) {
		SendWiFiLog("WiFi:Failed connect");
		SendWiFiLog("WiFi:Config mode...");
		SendWiFiLog("WiFi:Server start...");
		SendWiFiLog("WiFi:192.168.1.4...");
	} else {
		SendWiFiLog("WiFi:Disable...");
	}
}

/**
 * callback for save config
 */
void saveConfigCallback() {

	SendWiFiLog("WiFi:Save config");
	SendWiFiLog("WiFi:Try reconnect..");
}

void AquaWiFi::WaitRequest() {

	if (_isWiFiEnable && !_isError) {
		if (WiFi.status() == WL_CONNECTED) {
			web.HandleClient();
			int packetSize = Udp.parsePacket();
			if (packetSize > 0) {
				int len = Udp.read(incomingPacket, MAX_BUFFER);
				if (len > 0) {
					if (len > MAX_BUFFER) {
						len = MAX_BUFFER;
					}
					incomingPacket[len] = 0;
				}
				digitalWrite(2, HIGH);
				delay(50);
				digitalWrite(2, LOW);
				SendFromUDPToController(incomingPacket);
				memset(incomingPacket, 0, sizeof(incomingPacket));

			}
		}
	}

	if (millis() > timeIP + DELAY_MESSAGE_UPDATE) {
		if (_isWiFiEnable && !_isError) {
			SendWifiIp(false);
		}
		timeIP = millis();
		return;
	}
	//send device info
	if (millis() > lastDeviceInfoTime + DELAY_DEVICE_INFO_UPDATE) {
		lastDeviceInfoTime = millis();
		if (_isWiFiEnable && !_isError && _isConnected) {
			UDPSendMessage(responseCache[DEVICE], true);
			return;
		}
	}
	//update time from NTP
	if (millis() > lastNTPtime + DELAY_NTP_UPDATE) {
		lastNTPtime = millis();
		if (_isWiFiEnable && !_isError && _isConnected) {
			ntp.SetNTPTimeToController(SendWiFiLog);
			return;
		}
	}
	//get temp every DELAY_TEMP_UPDATE time
	if (millis() > lastTemptime + DELAY_TEMP_UPDATE) {
		lastTemptime = millis();
		if (_isWiFiEnable && !_isError && _isConnected) {
			UDPSendMessage(responseCache[TEMPSENSOR], true);
			return;
		}
	}
	//get PH every DELAY_PH_UPDATE time
	if (millis() > lastPHTime + DELAY_PH_UPDATE) {
		lastPHTime = millis();
		if (_isWiFiEnable && !_isError && _isConnected) {
			UDPSendMessage(responseCache[PH], true);
			return;
		}
	}
}

void SendFromUDPToController(String inString) {
	Serial.println(inString);
	jsonBuffer.clear();
	JsonObject &root = jsonBuffer.parseObject(inString);
	if (!root.success()) {
		UDPSendError(RQUEST_JSON_CORUPTED);
		return;
	}

	if (inString.indexOf(GET_COMMAND) != -1) {
		if (inString.indexOf(GET_DEVICE_INFO) != -1) {
			responseCache[DEVICE] = Helper.GetDevice(WiFi.localIP().toString());
			UDPSendMessage(responseCache[DEVICE], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_PH) != -1) {
			UDPSendMessage(responseCache[PH], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_TEMP_STATS) != -1) {
			UDPSendMessage(responseCache[TEMPSTATS], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_TEMP_SENSOR) != -1) {
			UDPSendMessage(responseCache[TEMPSENSOR], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_CHANAL_SETTINGS) != -1) {
			UDPSendMessage(responseCache[CANAL], false);
			return;
		}
		if (inString.indexOf(CANAL_STATE_PWM)) {
			UDPSendMessage(responseCache[PWMCANAL], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_DAILY_TIMER_SATE) != -1) {
			UDPSendMessage(responseCache[TIMERDAY], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_HOURS_TIMER_SATE) != -1) {
			UDPSendMessage(responseCache[TIMERHOUR], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_SECOND_TIMER_SATE) != -1) {
			UDPSendMessage(responseCache[TIMERSEC], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_TEMP_STATE) != -1) {
			UDPSendMessage(responseCache[TIMERTEMP], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_PH_TIMER) != -1) {
			UDPSendMessage(responseCache[PHTIMER], false);
			return;
		}

	} else if (inString.indexOf(POST_COMMAND) != -1) {
		if (inString.indexOf("data") == -1) {
			UDPSendError(RQUEST_DATA_CORUPTED);
			return;
		}
		JsonObject &data = root["data"];
		if (inString.indexOf("time_NTP") != -1) {
			UTC3 = funcNTPUpdate(data[SETTINGS_UTC].as<uint16_t>());
			UDPSendMessage("{\"status\":\"success\",\"message\":\"Time update\",\"data\":{}}", false);
			return;
		}
		if (inString.indexOf(CANAL_STATE) != -1) {
			funcGetUDPRequest(CANAL, inString);
			responseCache[CANAL] = Helper.GetChanalState();
			UDPSendMessage(responseCache[CANAL], false);
			return;
		}
		if (inString.indexOf(TIMER_DAILY_STATE) != -1) {
			funcGetUDPRequest(TIMERDAY, inString);
			responseCache[TIMERDAY] = Helper.GetDailyTimerState();
			UDPSendMessage(responseCache[TIMERDAY], false);
			return;
		}
		if (inString.indexOf(TEMP_STATE) != -1) {
			funcGetUDPRequest(TIMERTEMP, inString);
			responseCache[TIMERTEMP] = Helper.GetTempState();
			UDPSendMessage(responseCache[TIMERTEMP], false);
			return;
		}
		if (inString.indexOf(TIMER_HOURS_STATE) != -1) {
			funcGetUDPRequest(TIMERHOUR, inString);
			responseCache[TIMERHOUR] = Helper.GetHoursTimerState();
			UDPSendMessage(responseCache[TIMERHOUR], false);
			return;
		}
		if (inString.indexOf(TIMER_SECONDS_STATE) != -1) {
			funcGetUDPRequest(TIMERSEC, inString);
			responseCache[TIMERSEC] = Helper.GetSecondsTimerState();
			UDPSendMessage(responseCache[TIMERSEC], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_PH_TIMER) != -1) {
			funcGetUDPRequest(PHTIMER, inString);
			responseCache[PHTIMER] = Helper.GetPhTimerState();
			UDPSendMessage(responseCache[PHTIMER], false);
			return;

		}
		if (inString.indexOf(TIMER_DAILY_PWM_STATE) != -1) {
			funcGetUDPRequest(PWMTIMER, inString);
			responseCache[PWMTIMER] = Helper.GetPWMTimerState();
			UDPSendMessage(responseCache[PWMTIMER], false);
			return;
		}
		if (inString.indexOf(CANAL_STATE_PWM) != -1) {
			funcGetUDPRequest(PWMCANAL, inString);
			responseCache[PWMCANAL] = Helper.GetChanalPWMState();
			UDPSendMessage(responseCache[PWMCANAL], false);
			return;
		}

	} else if (inString.indexOf(INFO_COMMAND) != -1) {
		return;
	}
}

void AquaWiFi::StartCaching() {
	SendWiFiLog("WiFi:Caching data...");

	responseCache[DEVICE] = Helper.GetDevice(WiFi.localIP().toString());
	responseCache[CANAL] = Helper.GetChanalState();
	responseCache[PH] = Helper.GetPhStats();
	responseCache[TEMPSTATS] = Helper.GetTempStats();
	responseCache[TEMPSENSOR] = Helper.GetRealTemp();
	responseCache[TIMERDAY] = Helper.GetDailyTimerState();
	responseCache[TIMERHOUR] = Helper.GetHoursTimerState();
	responseCache[TIMERSEC] = Helper.GetSecondsTimerState();
	responseCache[TIMERTEMP] = Helper.GetTempState();
	responseCache[PHTIMER] = Helper.GetPhTimerState();
	responseCache[PWMTIMER] = Helper.GetPWMTimerState();
	responseCache[PWMCANAL] = Helper.GetChanalPWMState();
}

void AquaWiFi::CacheResponse(typeResponse type, String json) {
	responseCache[type] = json;
}

/**
 *
 * @param isNeedPing
 */
void SendWifiIp(bool isNeedPing) {
	if (isNeedPing) {
		isInterenetAvalible = ping_start(remote_ip, 4, 0, 0, 5);
	}
	if (!isInterenetAvalible) {
		SendWiFiLog("LAN:" + WiFi.localIP().toString());
	} else {
		SendWiFiLog("WAN:" + WiFi.localIP().toString());
	}

}

/**
 *
 * @param message
 * @param isBroadcast
 */
void UDPSendMessage(String message, bool isBroadcast) {
	if (!_isWiFiEnable) {
		SendWiFiLog("WiFi:Disable...");
		return;
	}
	if (_isError) {
		SendWiFiLog("WiFi:Error...");
		return;
	}
	if (isBroadcast) {
		web.SocketUpdate(message);
		Udp.beginPacket(broadcastAddress, localUdpPort);
	} else
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
	Serial.println(message);
	Udp.println(message);
	Udp.endPacket();
}

void UDPSendError(String error) {
	if (!_isWiFiEnable) {
		SendWiFiLog("WiFi:Disable...");
		return;
	}
	if (_isError) {
		SendWiFiLog("WiFi:Error...");
		return;
	}
	String response = "{\"status\":\"error\",\"message\":\"" + error + "\",\"data\":{}}";
	Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
	Serial.println(response);
	Udp.println(response);
	Udp.endPacket();

}

/**
 *
 * @param log
 */
void SendWiFiLog(String log) {
	funcChangeLog(log);

}

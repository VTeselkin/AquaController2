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
bool _isConnected = false;
bool isInterenetAvalible = false;

unsigned long timeIP, lastDeviceInfoUpdate, lastDeviceInfoTime, lastNTPtime, lastTemptime, lastPHTime;
unsigned int localUdpPort = 8888;

char incomingPacket[MAX_BUFFER];
void (*funcChangeLog)(String);
void (*funcGetUDPRequest)(typeResponse, String);
void (*ChandeDebugLED)(typeDebugLED led, typeLightLED type);
uint16_t (*funcNTPUpdate)(uint16_t);

word UTC3 = 3; //UTC+3

Dictionary responseCache = { { DEVICE, responseNull }, { CANAL, responseNull }, { TIMERDAY, responseNull }, { TIMERHOUR,
		responseNull }, { TIMERSEC, responseNull }, { TIMERTEMP, responseNull }, { TEMPSENSOR, responseNull }, { PH,
		responseNull }, { PHTIMER, responseNull }, { TEMPSTATS, responseNull }, { PWMCANAL, responseNull }, { PWMTIMER,
		responseNull }, { SETTINGS, responseNull }, { FAN, responseNull } };

void AquaWiFi::Init(void (*ChangeLog)(String), void (*GetUDPRequest)(typeResponse, String),
		uint16_t (*NTPUpdate)(uint16_t), void (*chandeDebugLED)(typeDebugLED led, typeLightLED type)) {
	pinMode(2, OUTPUT);
	funcChangeLog = ChangeLog;
	funcGetUDPRequest = GetUDPRequest;
	funcNTPUpdate = NTPUpdate;
	ChandeDebugLED = chandeDebugLED;
	timeIP = millis();
	lastNTPtime = millis();
	lastTemptime = millis();
	_isWiFiEnable = Helper.data.auto_connect;
	update.Init();
	Connection();
}

void AquaWiFi::Connection() {

	if (_isWiFiEnable) {
		funcChangeLog("WiFi:Try connect...");
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
		}
		Udp.begin(localUdpPort);
		broadcastAddress = (uint32_t) WiFi.localIP() | ~((uint32_t) WiFi.subnetMask());
		SendWifiIp();
		_isConnected = true;
		funcChangeLog("HTTP:Service started...");
		update.CheckOTAUpdate(true, funcChangeLog, jsonBuffer);
		StartCaching();
		web.Init(responseCache, jsonBuffer);
		if (isInterenetAvalible) {
			ntp.SetNTPTimeToController(funcChangeLog);
		} else {
			funcChangeLog("WAN:Not connection..");
		}

	} else {
		funcChangeLog("WiFi:Disable...");

	}
}

/**
 * callback for config mode
 */
void configModeCallback(WiFiManager *myWiFiManager) {
	if (_isWiFiEnable) {
		funcChangeLog("WiFi:Failed connect");
		funcChangeLog("WiFi:Config mode...");
		funcChangeLog("WiFi:Server start...");
		funcChangeLog("WiFi:192.168.1.4...");
		ChandeDebugLED(RXLED, PULSE);
		ChandeDebugLED(TXLED, PULSE);
		ChandeDebugLED(WIFILED, PULSE);
	} else {
		funcChangeLog("WiFi:Disable...");
		ChandeDebugLED(WIFILED, NONE);
	}
}

/**
 * callback for save config
 */
void saveConfigCallback() {
	ChandeDebugLED(RXLED, NONE);
	ChandeDebugLED(TXLED, NONE);
	ChandeDebugLED(WIFILED, NONE);
	funcChangeLog("WiFi:Save config");
	funcChangeLog("WiFi:Try reconnect..");
}

void AquaWiFi::WaitRequest() {

	if (_isWiFiEnable) {
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

	if (millis() > lastDeviceInfoUpdate + DELAY_UPDATE_DEVICE) {
		SendCacheResponse(DEVICE, false);
		lastDeviceInfoUpdate = millis();
		return;
	}
	if (millis() > timeIP + DELAY_MESSAGE_UPDATE) {
		if (_isWiFiEnable) {
			SendWifiIp();
		}
		timeIP = millis();
		return;
	}
	//send device info
	if (millis() > lastDeviceInfoTime + DELAY_DEVICE_INFO_UPDATE) {
		lastDeviceInfoTime = millis();
		if (_isWiFiEnable && _isConnected) {
			UDPSendMessage(responseCache[DEVICE], true);
			funcChangeLog("[TX]" + responseCache[DEVICE]);
			return;
		}
	}
	//update time from NTP
	if (millis() > lastNTPtime + DELAY_NTP_UPDATE) {
		lastNTPtime = millis();
		if (_isWiFiEnable && _isConnected) {
			ntp.SetNTPTimeToController(funcChangeLog);
			return;
		}
	}
}

void SendFromUDPToController(String inString) {
	inString.trim();
	if (inString.length() == 0)
		return;
	jsonBuffer.clear();
	JsonObject &root = jsonBuffer.parseObject(inString);
	if (!root.success()) {
		ChandeDebugLED(RXLED, SHORT);
		UDPSendError(RQUEST_JSON_CORUPTED);
		return;
	}

	if (inString.indexOf(GET_COMMAND) != -1) {
		funcChangeLog("[RX]" + inString);
		ChandeDebugLED(RXLED, SHORT);
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
		if (inString.indexOf(CANAL_STATE_PWM) != -1) {
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
		if (inString.indexOf(TIMER_DAILY_PWM_STATE) != -1) {
			UDPSendMessage(responseCache[PWMTIMER], false);
			return;
		}
		if (inString.indexOf(GET_DEVICE_FAN) != -1) {
			UDPSendMessage(responseCache[FAN], false);
			return;
		}

	} else if (inString.indexOf(POST_COMMAND) != -1) {
		funcChangeLog("[RX]" + inString);
		ChandeDebugLED(RXLED, SHORT);
		if (inString.indexOf("data") == -1) {
			UDPSendError(RQUEST_DATA_CORUPTED);
			return;
		}
		JsonObject &data = root["data"];
		if (inString.indexOf("time_NTP") != -1) {
			UTC3 = funcNTPUpdate(data[SETTINGS_UTC].as<uint16_t>());
			Helper.SetTimeNow(data[SETTINGS_EPOCH].as<unsigned long>());
			UDPSendMessage("{\"status\":\"success\",\"message\":\"Time update\",\"data\":{}}", false);
			return;
		}
		if (inString.indexOf(CANAL_STATE) != -1) {
			funcGetUDPRequest(CANAL, inString);
			return;
		}
		if (inString.indexOf(TIMER_DAILY_STATE) != -1) {
			funcGetUDPRequest(TIMERDAY, inString);
			return;
		}
		if (inString.indexOf(TEMP_STATE) != -1) {
			funcGetUDPRequest(TIMERTEMP, inString);
			return;
		}
		if (inString.indexOf(TIMER_HOURS_STATE) != -1) {
			funcGetUDPRequest(TIMERHOUR, inString);
			return;
		}
		if (inString.indexOf(TIMER_SECONDS_STATE) != -1) {
			funcGetUDPRequest(TIMERSEC, inString);
			return;
		}
		if (inString.indexOf(GET_DEVICE_PH_TIMER) != -1) {
			funcGetUDPRequest(PHTIMER, inString);
			return;

		}
		if (inString.indexOf(TIMER_DAILY_PWM_STATE) != -1) {
			funcGetUDPRequest(PWMTIMER, inString);
			return;
		}
		if (inString.indexOf(CANAL_STATE_PWM) != -1) {
			funcGetUDPRequest(PWMCANAL, inString);
			return;
		}
		if (inString.indexOf(SETTINGS_DEV) != -1) {
			funcGetUDPRequest(SETTINGS, inString);
			return;
		}
		if (inString.indexOf(GET_DEVICE_FAN) != -1) {
			funcGetUDPRequest(FAN, inString);
			return;
		}

	} else if (inString.indexOf(INFO_COMMAND) != -1) {
		funcChangeLog("[RX]" + inString);
		ChandeDebugLED(RXLED, SHORT);
		return;
	}
}

void AquaWiFi::StartCaching() {
	funcChangeLog("WiFi:Caching data...");
	SendCacheResponse(DEVICE, false);
	SendCacheResponse(CANAL, false);
	SendCacheResponse(PH, false);
	SendCacheResponse(TEMPSTATS, false);
	SendCacheResponse(TEMPSENSOR, false);
	SendCacheResponse(TIMERDAY, false);
	SendCacheResponse(TIMERHOUR, false);
	SendCacheResponse(TIMERSEC, false);
	SendCacheResponse(TIMERTEMP, false);
	SendCacheResponse(PHTIMER, false);
	SendCacheResponse(PWMTIMER, false);
	SendCacheResponse(PWMCANAL, false);
	SendCacheResponse(FAN, false);

}

void AquaWiFi::SendCacheResponse(typeResponse type, bool sendCache) {
	switch (type) {
	case DEVICE:
		responseCache[DEVICE] = Helper.GetDevice(WiFi.localIP().toString());
		break;
	case CANAL:
		responseCache[CANAL] = Helper.GetChanalState();
		break;
	case PH:
		responseCache[PH] = Helper.GetPhStats();
		break;
	case TEMPSTATS:
		responseCache[TEMPSTATS] = Helper.GetTempStats();
		break;
	case TEMPSENSOR:
		responseCache[TEMPSENSOR] = Helper.GetRealTemp();
		break;
	case TIMERDAY:
		responseCache[TIMERDAY] = Helper.GetDailyTimerState();
		break;
	case TIMERHOUR:
		responseCache[TIMERHOUR] = Helper.GetHoursTimerState();
		break;
	case TIMERSEC:
		responseCache[TIMERSEC] = Helper.GetSecondsTimerState();
		break;
	case TIMERTEMP:
		responseCache[TIMERTEMP] = Helper.GetTempState();
		break;
	case PHTIMER:
		responseCache[PHTIMER] = Helper.GetPhTimerState();
		break;
	case PWMTIMER:
		responseCache[PWMTIMER] = Helper.GetPWMTimerState();
		break;
	case PWMCANAL:
		responseCache[PWMCANAL] = Helper.GetChanalPWMState();
		break;
	case SETTINGS:
		responseCache[SETTINGS] = Helper.GetWiFiSettings();
		break;
	case FAN:
		responseCache[FAN] = Helper.GetFANTemp();
		break;

	}
	if (sendCache)
		UDPSendMessage(responseCache[type], false);
}

void AquaWiFi::CacheResponse(typeResponse type, String json) {
	responseCache[type] = json;
}

bool SendWifiIp() {
	auto res = ping_start(remote_ip, 4, 0, 0, 5);
	Helper.data.internet_avalible = res;
	if (res) {
		ChandeDebugLED(WIFILED, LIGHT);
	} else {
		ChandeDebugLED(WIFILED, PULSE);
	}
	return res;

}

/**
 *
 * @param message
 * @param isBroadcast
 */
void UDPSendMessage(String message, bool isBroadcast) {
	if (!_isWiFiEnable) {
		funcChangeLog("WiFi:Disable...");
		ChandeDebugLED(WIFILED, NONE);
		return;
	}
	if (!_isConnected) {
		funcChangeLog("WiFi:Disconnected...");
		ChandeDebugLED(WIFILED, NONE);
		return;
	}
	if (isBroadcast) {
		web.SocketUpdate(message);
		Udp.beginPacket(broadcastAddress, localUdpPort);
	} else
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
	Serial.print("[");
	Serial.print(Helper.GetFormatTimeNow());
	Serial.print("]");
	Serial.println("[TX]" + message);

	Udp.println(message);
	Udp.endPacket();
	ChandeDebugLED(TXLED, SHORT);
}

void UDPSendError(String error) {
	if (!_isWiFiEnable) {
		funcChangeLog("WiFi:Disable...");
		ChandeDebugLED(WIFILED, NONE);
		return;
	}
	String response = "{\"status\":\"error\",\"message\":\"" + error + "\",\"data\":{}}";
	Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
	Serial.print("[");
	Serial.print(Helper.GetFormatTimeNow());
	Serial.print("]");
	Serial.println("[TX]" + response);

	Udp.println(response);
	Udp.endPacket();
	ChandeDebugLED(TXLED, SHORT);
	ChandeDebugLED(ERRLED, LONG);
}


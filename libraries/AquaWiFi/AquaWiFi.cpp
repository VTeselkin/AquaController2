/*
 * AquaWiFi.cpp
 *
 *  Created on: Oct 31, 2019
 *      Author: doc
 */

#include <AquaWiFi.h>
#include <ESP32Ping.h>
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
bool _isConfigMode = false;

unsigned long timeIP, lastDeviceInfoUpdate, lastDeviceInfoTime, lastNTPtime, lastTemptime, lastPHTime, lastConfigMode;
unsigned int localUdpPort = 8888;

char incomingPacket[MAX_BUFFER];
void (*funcChangeLog)(String);
void (*funcGetUDPRequest)(typeResponse, String);
void (*ChandeDebugLED)(typeDebugLED led, typeLightLED type);
uint16_t (*funcNTPUpdate)(uint16_t);
byte _minForReconect = 5;
word UTC3 = 3; //UTC+3

Dictionary responseCache = { { DEVICE, responseNull }, { CANAL, responseNull }, { TIMERDAY, responseNull }, { TIMERHOUR,
		responseNull }, { TIMERSEC, responseNull }, { TIMERTEMP, responseNull }, { TEMPSENSOR, responseNull }, { PH,
		responseNull }, { PHTIMER, responseNull }, { TEMPSTATS, responseNull }, { PWMCANAL, responseNull }, { PWMTIMER,
		responseNull }, { SETTINGS, responseNull }, { FAN, responseNull }, { PH_SETTINGS, responseNull } };

void AquaWiFi::Init(void (*ChangeLog)(String), void (*GetUDPRequest)(typeResponse, String),
		uint16_t (*NTPUpdate)(uint16_t), void (*chandeDebugLED)(typeDebugLED led, typeLightLED type)) {
	funcChangeLog = ChangeLog;
	funcGetUDPRequest = GetUDPRequest;
	funcNTPUpdate = NTPUpdate;
	ChandeDebugLED = chandeDebugLED;
	timeIP = millis();
	lastNTPtime = millis();
	lastTemptime = millis();
	_isWiFiEnable = Helper.data.auto_connect;
	if (wifiManager.getWiFiIsSaved()) {
		_minForReconect = 1;
	}
	update.Init();

	Connection();
}

void AquaWiFi::Connection() {

	if (_isWiFiEnable) {
		//Disable debug log connection
		wifiManager.setDebugOutput(false);
		funcChangeLog("WiFi:Try connect...");
		wifiManager.setAPCallback(configModeCallback);
		wifiManager.setSaveConfigCallback(saveConfigCallback);
		//set custom ip for portal
		wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1),
				IPAddress(255, 255, 255, 0));
		/**
		 * if we couldn't connected to save WiFi access point
		 * we will start our own access point with ip address 192.168.1.4
		 */
		wifiManager.setConfigPortalBlocking(false);
		if (wifiManager.autoConnect("AP: AquaController")) {
			_isConfigMode = false;
			ChandeDebugLED(WIFILED, LIGHT);
			ChandeDebugLED(ERRLED, NONE);

		} else {
			if (wifiManager.getWiFiIsSaved()) {
				ChandeDebugLED(ERRLED, LIGHT);
			}
		}

		Display.SetLANConnection(WiFi.status() == WL_CONNECTED);
		Udp.begin(localUdpPort);
		broadcastAddress = (uint32_t) WiFi.localIP() | ~((uint32_t) WiFi.subnetMask());
		SendWifiIp();
		_isConnected = true;
		funcChangeLog("HTTP:Service started...");
		StartCaching();
		web.Init(responseCache, jsonBuffer);
		if (Helper.data.internet_avalible) {
			if (Helper.ChipSize() < 16) {
				funcChangeLog("OTA: Not possible to update. Chip size is not supported!!!");
			} else {
				update.CheckOTAUpdate(true, jsonBuffer, funcChangeLog);
			}
			ntp.SetNTPTimeToController(funcChangeLog);
		} else {
			funcChangeLog("WiFi:Not Internet connection..");
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
		_isConfigMode = true;
		lastConfigMode = millis();
		funcChangeLog("WiFi:Failed connect");
		funcChangeLog("WiFi:Config mode...");
		funcChangeLog("WiFi:Server start...");
		funcChangeLog("WiFi:192.168.4.1...");
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
				delay(50);
				SendFromUDPToController(incomingPacket);
				memset(incomingPacket, 0, sizeof(incomingPacket));

			}
		} else {
			if (WiFi.status() != WL_CONNECTED && !_isConfigMode) {
				ChandeDebugLED(WIFILED, NONE);
				ChandeDebugLED(ERRLED, NONE);
				Display.SetLANConnection(false);
				Display.SetWANConnection(false);
				funcChangeLog("WiFi:Lost connection. Reconnect...");
				Connection();
				return;

			}
			if (millis() > lastConfigMode + _minForReconect * 60 * 1000 && _isConfigMode) {
				lastConfigMode = millis();
				_isConfigMode = false;
				Connection();
			}
		}
	}

	//TODO CHECK
	if (millis() > lastDeviceInfoUpdate + DELAY_UPDATE_DEVICE) {
		lastDeviceInfoUpdate = millis();
		SendCacheResponse(DEVICE, false, true);
		Display.SetLANConnection(WiFi.status() == WL_CONNECTED);
		return;
	}
	if (millis() > timeIP + DELAY_MESSAGE_UPDATE) {
		if (_isWiFiEnable) {
			SendWifiIp();
		}
		timeIP = millis();
		return;
	}
	//TODO CHECK
	//send device info
	if (millis() > lastDeviceInfoTime + DELAY_DEVICE_INFO_UPDATE) {
		lastDeviceInfoTime = millis();
		if (_isWiFiEnable && _isConnected) {
			UDPSendMessage(responseCache[DEVICE], true);
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

void AquaWiFi::SendFromUDPToController(String inString) {
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
			funcGetUDPRequest(NTP, inString);
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
		if (inString.indexOf(GET_DEVICE_PH_SET) != -1) {
			funcGetUDPRequest(PH_SETTINGS, inString);
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
	SendCacheResponse(DEVICE, false, false);
	SendCacheResponse(CANAL, false, false);
	SendCacheResponse(PH, false, false);
	SendCacheResponse(TEMPSTATS, false, false);
	SendCacheResponse(TEMPSENSOR, false, false);
	SendCacheResponse(TIMERDAY, false, false);
	SendCacheResponse(TIMERHOUR, false, false);
	SendCacheResponse(TIMERSEC, false, false);
	SendCacheResponse(TIMERTEMP, false, false);
	SendCacheResponse(PHTIMER, false, false);
	SendCacheResponse(PWMTIMER, false, false);
	SendCacheResponse(PWMCANAL, false, false);
	SendCacheResponse(FAN, false, false);

}

void AquaWiFi::SendCacheResponse(typeResponse type, bool sendCache, bool isBroadcast) {
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
	case NTP:
		break;
	case PH_SETTINGS:
		type = PHTIMER;
		responseCache[PHTIMER] = Helper.GetPhTimerState();
		break;

	}
	if (sendCache)
		UDPSendMessage(responseCache[type], isBroadcast);
}

void AquaWiFi::CacheResponse(typeResponse type, String json) {
	responseCache[type] = json;
}

bool SendWifiIp() {
	if (WiFi.status() == WL_CONNECTED) {
		auto res = Ping.ping(remote_ip);
		Helper.data.internet_avalible = res;
		if (res) {
			ChandeDebugLED(WIFILED, LIGHT);
		} else {
			ChandeDebugLED(WIFILED, PULSE);
		}
		Display.SetWANConnection(Helper.data.internet_avalible);
		return res;
	}
	return false;

}

/**
 *
 * @param message
 * @param isBroadcast
 */
void AquaWiFi::UDPSendMessage(String message, bool isBroadcast) {
	if (!_isWiFiEnable) {
		//funcChangeLog("WiFi:Disable...");
		ChandeDebugLED(WIFILED, NONE);
		return;
	}
	if (!_isConnected) {
		//funcChangeLog("WiFi:Disconnected...");
		ChandeDebugLED(WIFILED, NONE);
		return;
	}
	if (isBroadcast) {
		Udp.beginPacket(broadcastAddress, localUdpPort);
	} else
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());

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
	String log = "[TX]" + error;
	funcChangeLog(log);
	Udp.println(response);
	Udp.endPacket();
	ChandeDebugLED(TXLED, SHORT);
	ChandeDebugLED(ERRLED, LONG);
}


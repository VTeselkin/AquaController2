/*
 * AquaHTTP.cpp
 *
 *  Created on: Oct 31, 2019
 *      Author: doc
 */

#include <AquaHTTP.h>
#include <SPIFFS.h>
#include <FS.h>

HTTPClient httpClient;
WebServer http(80);
DynamicJsonBuffer _jsonBuffer;
WebSocketsServer sockets = WebSocketsServer(81);
File fsUploadFile;

Dictionary response = { { DEVICE, responseNull }, { CANAL, responseNull }, {
		TIMERDAY, responseNull }, { TIMERHOUR, responseNull }, { TIMERSEC,
		responseNull }, { TIMERTEMP, responseNull },
		{ TEMPSENSOR, responseNull }, { PH, responseNull }, { TEMPSTATS,
				responseNull } };

void AquaHTTP::Init(Dictionary &responseCache, DynamicJsonBuffer &jsonBuffer) {

	response = responseCache;
	http.on("/", HTTP_GET, []() {
		if (!handleFileRead("/"))
			http.send(404, "text/plain", "FileNotFound");
	});

	http.on("/list", HTTP_GET, handleFileList);

	http.on("/edit", HTTP_GET, []() {
		if (!handleFileRead("/edit.htm"))
			http.send(404, "text/plain", "FileNotFound");
	});

	http.on("/edit", HTTP_PUT, handleFileCreate);

	http.on("/edit", HTTP_DELETE, handleFileDelete);

	http.on("/edit", HTTP_POST, []() {
		http.send(200, "text/plain", "");
	}, handleFileUpload);

	http.onNotFound([]() {
		if (!handleFileRead(http.uri()))
			http.send(404, "text/plain", "FileNotFound");
	});

	http.on("/time", HTTP_GET, []() {
		http.send(200, "application/json", Helper.GetDataTime());
	});

	http.on("/device", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(DEVICE, data, param);
	});

	http.on("/canal", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(CANAL, data, param);
	});

	http.on("/timerday", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(TIMERDAY, data, param);
	});
	http.on("/timerhour", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(TIMERHOUR, data, param);
	});

	http.on("/timersec", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(TIMERSEC, data, param);
	});

	http.on("/timertemp", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(TIMERTEMP, data, param);
	});

	http.on("/tempsensor", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(TEMPSENSOR, data, param);
	});
	http.on("/ph", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(PH, data, param);
	});

	http.on("/tempstats", HTTP_GET, []() {
		String data, param = "";
		if (http.hasArg("data"))
			data = http.arg("data");
		if (http.hasArg("param"))
			param = http.arg("param");
		HttpSendJson(TEMPSTATS, data, param);
	});

	http.begin();
	sockets.begin();
	sockets.onEvent(cbWebSocketsEvent);
}

void cbWebSocketsEvent(uint8_t num, WStype_t type, uint8_t *payload,
		size_t length) {
	switch (type) {
	case WStype_ERROR:
	case WStype_DISCONNECTED: {
		Serial.println("Socket Disconnected!");
	}
		break;
	case WStype_CONNECTED: {
		Serial.println("Socket Connected!");
		auto response = Helper.GetDataTime();
		sockets.sendTXT(num, response);
	}
		break;
	case WStype_TEXT:
	case WStype_BIN:
	case WStype_FRAGMENT_TEXT_START:
	case WStype_FRAGMENT_BIN_START:
	case WStype_FRAGMENT:
	case WStype_FRAGMENT_FIN:
	case WStype_PING:
	case WStype_PONG:
		break;
	}
}
String getContentType(String filename) {
	if (http.hasArg("download"))
		return "application/octet-stream";
	else if (filename.endsWith(".htm"))
		return "text/html";
	else if (filename.endsWith(".html"))
		return "text/html";
	else if (filename.endsWith(".json"))
		return "application/json";
	else if (filename.endsWith(".css"))
		return "text/css";
	else if (filename.endsWith(".js"))
		return "application/javascript";
	else if (filename.endsWith(".png"))
		return "image/png";
	else if (filename.endsWith(".gif"))
		return "image/gif";
	else if (filename.endsWith(".jpg"))
		return "image/jpeg";
	else if (filename.endsWith(".ico"))
		return "image/x-icon";
	else if (filename.endsWith(".xml"))
		return "text/xml";
	else if (filename.endsWith(".pdf"))
		return "application/x-pdf";
	else if (filename.endsWith(".zip"))
		return "application/x-zip";
	else if (filename.endsWith(".gz"))
		return "application/x-gzip";
	return "text/plain";
}

bool handleFileRead(String path) {
	if (path.endsWith("/"))
		path += "index.htm";
	String contentType = getContentType(path);
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz))
			path += ".gz";
		File file = SPIFFS.open(path, "r");
		size_t sent = http.streamFile(file, contentType);
		Serial.println(sent);
		file.close();
		return true;
	}
	return false;
}

void handleFileUpload() {
	if (http.uri() != "/edit")
		return;
	HTTPUpload &upload = http.upload();
	if (upload.status == UPLOAD_FILE_START) {
		String filename = upload.filename;
		if (!filename.startsWith("/"))
			filename = "/" + filename;
		fsUploadFile = SPIFFS.open(filename, "w");
		filename = String();
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		//DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
		if (fsUploadFile)
			fsUploadFile.write(upload.buf, upload.currentSize);
	} else if (upload.status == UPLOAD_FILE_END) {
		if (fsUploadFile)
			fsUploadFile.close();
	}
}

void handleFileDelete() {
	if (http.args() == 0)
		return http.send(500, "text/plain", "BAD ARGS");
	String path = http.arg(0);
	if (path == "/")
		return http.send(500, "text/plain", "BAD PATH");
	if (!SPIFFS.exists(path))
		return http.send(404, "text/plain", "FileNotFound");
	SPIFFS.remove(path);
	http.send(200, "text/plain", "");
	path = String();
}

void handleFileCreate() {
	if (http.args() == 0)
		return http.send(500, "text/plain", "BAD ARGS");
	String path = http.arg(0);
	if (path == "/")
		return http.send(500, "text/plain", "BAD PATH");
	if (SPIFFS.exists(path))
		return http.send(500, "text/plain", "FILE EXISTS");
	File file = SPIFFS.open(path, "w");
	if (file)
		file.close();
	else
		return http.send(500, "text/plain", "CREATE FAILED");
	http.send(200, "text/plain", "");
	path = String();

}

void handleFileList() {
	if (!http.hasArg("dir")) {
		http.send(500, "text/plain", "BAD ARGS");
		return;
	}
	String path = http.arg("dir");
	auto root = SPIFFS.open(path);

	path = String();
	String output = "[";
	if (root.isDirectory()) {
		File file = root.openNextFile();
		while (file) {
			if (output != "[")
				output += ',';
			output += "{\"type\":\"";
			output += (file.isDirectory()) ? "dir" : "file";
			output += "\",\"name\":\"";
			output += String(file.name()).substring(1);
			output += "\"}";
			file = root.openNextFile();
		}
	}
	output += "]";
	http.send(200, "text/json", output);
}

long lastDeviceTime = 0;
void AquaHTTP::HandleClient() {
	http.handleClient();
	sockets.loop();

	//send time device to web sockets
	if (millis() > lastDeviceTime + DELAY_DEVICE_TIME_UPDATE) {
		lastDeviceTime = millis();
		SocketUpdate(Helper.GetDataTime());
	}
}

void AquaHTTP::SocketUpdate(String updateJson) {
	sockets.broadcastTXT(updateJson);
}

void HttpSendJson(typeResponse type, String data, String param) {
	if (data.length() > 0 || param.length() > 0) {
		_jsonBuffer.clear();
		JsonObject &root = _jsonBuffer.parseObject(response[type]);
		if (!root.success()) {
			http.send(404, "text/plain", "Request error!");
		}

		if (data == "short") {
			JsonObject &data = root["data"];
			if (data.containsKey(param)) {
				http.send(200, "application/json", data[param]);
			}
			http.send(200, "application/json", root["data"]);
		}
	}
	http.send(200, "application/json", response[type]);
}


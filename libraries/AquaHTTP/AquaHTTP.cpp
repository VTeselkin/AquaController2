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

File fsUploadFile;

Dictionary response = { { DEVICE, responseNull }, { CANAL, responseNull }, { TIMERDAY, responseNull }, { TIMERHOUR, responseNull }, { TIMERSEC,
		responseNull }, { TIMERTEMP, responseNull }, { TEMPSENSOR, responseNull }, { PH, responseNull }, { TEMPSTATS, responseNull } };

void AquaHTTP::Init(Dictionary &responseCache) {
	FS_init();
	response = responseCache;
	http.on("/device.json", HttpSendDeviceJson);
	http.on("/canal.json", HttpSendCanalJson);
	http.on("/timerday.json", HttpSendDayJson);
	http.on("/timerhour.json", HttpSendHourJson);
	http.on("/timersec.json", HttpSendSecJson);
	http.on("/timertemp.json", HttpSendTempJson);
	http.on("/tempsensor.json", HttpSendSensorJson);
	http.on("/ph.json", HttpSendPhJson);
	http.on("/tempstats.json", HttpSendStatsJson);
	http.begin();
}


// Инициализация FFS
void FS_init() {
  SPIFFS.begin(true);
  Serial.println(SPIFFS.totalBytes());
  Serial.println(SPIFFS.usedBytes());
  {
	  	auto root = SPIFFS.open("/");
	  	auto file = root.openNextFile();
	  	while (file) {
	  		String fileName = file.name();
	  		size_t fileSize = file.size();
	  		Serial.println(fileName);
	  		file = root.openNextFile();
	  	}

  }
  http.on("/", HTTP_GET, []() {
	    if (!handleFileRead("/")) http.send(404, "text/plain", "FileNotFound");
	  });
  //http страницы для работы с FFS
  //list directory
  http.on("/list", HTTP_GET, handleFileList);
  //загрузка редактора editor
  http.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) http.send(404, "text/plain", "FileNotFound");
  });
  //Создание файла
  http.on("/edit", HTTP_PUT, handleFileCreate);
  //Удаление файла
  http.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  http.on("/edit", HTTP_POST, []() {
    http.send(200, "text/plain", "");
  }, handleFileUpload);
  //called when the url is not defined here
  //use it to load content from SPIFFS
  http.onNotFound([]() {
    if (!handleFileRead(http.uri()))
      http.send(404, "text/plain", "FileNotFound");
  });
}
// Здесь функции для работы с файловой системой
String getContentType(String filename) {
  if (http.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.htm";
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
  if (http.uri() != "/edit") return;
  HTTPUpload& upload = http.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
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
  if (http.args() == 0) return http.send(500, "text/plain", "BAD ARGS");
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
  if(root.isDirectory()){
	  File file = root.openNextFile();
  while (file) {
    if (output != "[") output += ',';
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

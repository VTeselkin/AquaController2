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
File fsUploadFile;

Dictionary response = { { DEVICE, responseNull }, { CANAL, responseNull }, {
		TIMERDAY, responseNull }, { TIMERHOUR, responseNull }, { TIMERSEC,
		responseNull }, { TIMERTEMP, responseNull },
		{ TEMPSENSOR, responseNull }, { PH, responseNull }, { TEMPSTATS,
				responseNull } };
static const char serverIndex[] PROGMEM =
  R"(<html><body><form method='POST' action='' enctype='multipart/form-data'>
                  <input type='file' name='upload'>
                  <input type='submit' value='Upload'>
               </form>
         </body></html>)";


void AquaHTTP::Init(Dictionary &responseCache, DynamicJsonBuffer &jsonBuffer) {

	response = responseCache;

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

	http.on("/update", HTTP_GET, []() {
		http.sendHeader("Connection", "close");
		http.send(200, "text/html", serverIndex);
	  });

	 /*handling uploading firmware file */
	http.on("/upload", HTTP_POST, []() {
		http.sendHeader("Connection", "close");
		http.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
	    ESP.restart();
	  }, []() {
	    HTTPUpload& upload = http.upload();
	    if (upload.status == UPLOAD_FILE_START) {
	      Display.SendLogLn("Update: "+ upload.filename);
	      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
	        Update.printError(Serial);
	        Display.SendLogLn("Update failure!");
	      }
	    } else if (upload.status == UPLOAD_FILE_WRITE) {
	      /* flashing firmware to ESP*/
	      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
	        Update.printError(Serial);
	        Display.SendLogLn("Update failure!");
	      }
	    } else if (upload.status == UPLOAD_FILE_END) {
	      if (Update.end(true)) { //true to set the size to the current progress
	    	  Display.SendLogLn("Update Success: " + upload.totalSize);
	    	  Display.SendLogLn("Rebooting...");
	      } else {
	        Update.printError(Serial);
	        Display.SendLogLn("Update failure!");
	      }
	    }
	  });

	http.begin();

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




void AquaHTTP::HandleClient() {
	http.handleClient();
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


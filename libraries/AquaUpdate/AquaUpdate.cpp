/*
 * AquaUpdate.cpp
 *
 *  Created on: Nov 18, 2019
 *      Author: doc
 */

#include "AquaUpdate.h"
#include "NexUpload.h"
#include <HttpClient.h>
#include <SPIFFS.h>
#include <FS.h>

HTTPClient httpUpdateClient;
WiFiClient client;

AquaUpdate::AquaUpdate() {

}
void AquaUpdate::Init() {
	if (!SPIFFS.begin(true)) {
		Display.SendLogLn("An Error has occurred while mounting SPIFFS");
		return;
	}
	Display.SendLogLn("[SPIFFS] mounting success!");
	Display.SendLog("[SPIFFS] free space: " + String((SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024));
	Display.SendLogLn("Kb");

}
void AquaUpdate::CheckOTAUpdate(bool isForce, DynamicJsonBuffer &jsonBuffer, void (*ChangeLog)(String)) {

	if (!Helper.data.auto_update) {
		Display.SendLogLn("[OTA]: DISABLE");
		return;
	}

	String url = UPDATE_URL + PATH_SPIFFS + "index.php";
	Display.SendLogLn("[OTA]: Web update = " + url);
	if (url.length() > 0) {
		if (isForce) {
			httpUpdate.rebootOnUpdate(false);
			auto res = httpUpdate.updateSpiffs(client, url, VERTION_FIRMWARE);
			SendResultOTAUpdate(res);
		}
	}

	url = UPDATE_URL + PATH_FIRMWARE + "index.php";
	Display.SendLogLn("[OTA]: Firmware update" + url);
	if (url.length() > 0) {
		if (isForce) {
			httpUpdate.rebootOnUpdate(true);
			auto res = httpUpdate.update(client, url, VERTION_FIRMWARE);
			SendResultOTAUpdate(res);
		}
	}
	if (!Display.GetVersion().equals(VERTION_FIRMWARE)) {
		String fileName = "/" + VERTION_FIRMWARE + ".tft";
		url = "http://update.aquacontroller.ru/v2/tft" + fileName;
		if (DownloadAndSaveFile(fileName, url)) {
			Display.SendLogLn("[OTA]: Start update firmware screen!");
			NexUpload nexUpload(fileName, 115200, ChangeLog);
			nexUpload.upload();
		}
	}else{
		Display.SendLogLn("[OTA]: Display no need update!");
	}
	auto root = SPIFFS.open("/");
	auto file = root.openNextFile();
	while (file) {
		Display.SendLogLn(file.name());
		file = root.openNextFile();
	}

}

void SendResultOTAUpdate(t_httpUpdate_return ret) {
	switch (ret) {
	case HTTP_UPDATE_FAILED: {
		Display.SendLogLn("[OTA]: Update Error!");
		String error = "[OTA]: HTTP_UPDATE_FAILED Error (";
		error += httpUpdate.getLastError();
		error += "): ";
		error += httpUpdate.getLastErrorString().c_str();
		Display.SendLogLn(error);
		break;
	}
	case HTTP_UPDATE_NO_UPDATES: {
		Display.SendLogLn("[OTA]: No Update!");
		break;
	}
	case HTTP_UPDATE_OK: {
		Display.SendLogLn("[OTA]: Complete update!");
		break;
	}
	}
}

//downloadAndSaveFile("/200.tft","http://update.aquacontroller.ru/v2/bin/tft/200.tft");
bool AquaUpdate::DownloadAndSaveFile(String fileName, String url) {

	HTTPClient http;
	Display.SendLogLn("[OTA]: Begin download file...");
	Display.SendLogLn(fileName);
	Display.SendLogLn(url);
	http.begin(url);

	Display.SendLogLn("[OTA]: GET... " + url);
	// start connection and send HTTP header
	int httpCode = http.GET();
	if (httpCode > 0) {
		// HTTP header has been send and Server response header has been handled
		Display.SendLogLn("[OTA]: GET... code: " + httpCode);
		auto root = SPIFFS.open("/");
		File file = root.openNextFile();
		while (file) {
			if (String(file.name()).indexOf("tft") != -1) {
				SPIFFS.remove(file.name());
				Display.SendLogLn("[OTA]: File exists! Delete file: " + String(file.name()));
			}
			file = root.openNextFile();
		}
		Display.SendLogLn("[OTA]: Open file for writing " + fileName);

		file = SPIFFS.open(fileName, FILE_WRITE);

		// file found at server
		if (httpCode == HTTP_CODE_OK) {

			// get lenght of document (is -1 when Server sends no Content-Length header)
			int len = http.getSize();

			// create buffer for read
			uint8_t buff[128] = { 0 };

			// get tcp stream
			WiFiClient *stream = http.getStreamPtr();

			// read all data from server
			while (http.connected() && (len > 0 || len == -1)) {
				// get available data size
				size_t size = stream->available();
				if (size) {
					// read up to 128 byte
					int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
					// write it to Serial
					//Serial.write(buff, c);
					file.write(buff, c);
					if (len > 0) {
						len -= c;
					}
				}
				delay(1);
			}

			Display.SendLogLn("[OTA]: Connection closed or file end.");
			Display.SendLogLn("[OTA]: Closing file.");
			file.close();
			http.end();
			return true;
		}

	}
	http.end();
	return false;
}

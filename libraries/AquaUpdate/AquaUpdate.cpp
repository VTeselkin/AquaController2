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
	if (!SPIFFS.begin(true, "/spiffs", 10)) {
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

	String ver = String(VERTION_FIRMWARE + 1);
	String url = UPDATE_URL + PATH_SPIFFS + "index.php";
	Display.SendLogLn("[OTA]: Web update = " + url);
	if (url.length() > 0) {
		if (isForce) {
			httpUpdate.rebootOnUpdate(false);
			auto res = httpUpdate.updateSpiffs(client, url, ver);
			SendResultOTAUpdate(res);
			delay(1000);
		}
	}

	url = UPDATE_URL + PATH_FIRMWARE + "index.php";
	Display.SendLogLn("[OTA]: Firmware update" + url);
	if (url.length() > 0) {
		if (isForce) {
			httpUpdate.rebootOnUpdate(true);
			auto res = httpUpdate.update(client, url, ver);
			SendResultOTAUpdate(res);
		}
	}
	if (!Display.GetVersion().equals(ver)) {
		String fileName = "/" + ver + ".tft";
		url = "http://update.aquacontroller.ru/v2/tft" + fileName;
		if (DownloadAndSaveFile(fileName, url)) {
//			Display.SendLogLn("[OTA]: Start update firmware screen!");
//			File _myFile = SPIFFS.open(fileName, FILE_READ);
//			Display.SendLog("[NEX]: TFT file size is:");
//			Display.SendLog(String(_myFile.size()));
//			Display.SendLogLn(" byte");
//			_myFile.close();
			NexUpload nexUpload(fileName, 115200, ChangeLog);
			nexUpload.upload();
		}
	} else {
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
	Display.SendLog("[NEX]: Begin download file = ");
	Display.SendLogLn(fileName);
	Display.SendLogLn("[NEX]: " + url);
	http.begin(url);
	// start connection and send HTTP header
	int httpCode = http.GET();
	if (httpCode > 0) {
		// HTTP header has been send and Server response header has been handled
		//Display.SendLogLn("[NEX]: GET... code: " + httpCode);
		auto root = SPIFFS.open("/");
		File file = root.openNextFile();
		while (file) {
			if (String(file.name()).indexOf(fileName) != -1) {
				file.close();
				http.end();
				return true;
//				SPIFFS.remove(file.name());
//				Display.SendLogLn("[NEX]: File exists! Delete file: " + String(file.name()));
			}
			file = root.openNextFile();
		}
		Display.ClearLog();
		Display.SendLogLn("[NEX]: Download file from stream...");
		file.close();
		file = SPIFFS.open(fileName, FILE_APPEND);
		byte lastPercent = 100;
		// file found at server
		if (httpCode == HTTP_CODE_OK) {

			// get lenght of document (is -1 when Server sends no Content-Length header)
			int len = http.getSize();
			double total = len;
			// create buffer for read
			uint8_t buff[2048] = { 0 };

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

					byte percent = static_cast<byte>((len / total) * 100);
					if ((percent != lastPercent)) {
						lastPercent = percent;
						Display.SendLog(String(100 - percent) +"%");
					} else {
						Display.SendLog(".");
					}
				}

			}
			file.close();
			file = SPIFFS.open(fileName, FILE_APPEND);
			if (file.size() != http.getSize()) {
				Display.SendLogLn(
						"[NEX]: Bag size file. Local = " + String(file.size()) + " Remote = " + String(http.getSize()));
				file.close();
				http.end();
				return false;
			} else {
				Display.SendLogLn("[NEX]: Connection closed or file end.");
				Display.SendLogLn("[NEX]: Closing file.");
			}
			file.close();
			http.end();
			return true;
		}

	}
	Display.SendLogLn("[NEX]: HTTP Client error.");
	http.end();
	return false;
}

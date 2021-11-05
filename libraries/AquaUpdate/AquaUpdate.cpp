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
	Display.SendLogLnTime("[SPIFFS] mounting success! Size: " + String(Helper.SPIFFSSize()));
	Display.SendLog("Kb");
	Display.SendLogLnTime("[SPIFFS] free space: " + String((SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024));
	Display.SendLog("Kb");

}
void AquaUpdate::CheckOTAUpdate(bool isForce, DynamicJsonBuffer &jsonBuffer, void (*ChangeLog)(String)) {

	if (!Helper.data.auto_update) {
		Display.SendLogLnTime("[OTA]: DISABLE");
		return;
	}
	t_httpUpdate_return resspiffs = HTTP_UPDATE_FAILED;
	t_httpUpdate_return resfirmware = HTTP_UPDATE_FAILED;
	auto root = SPIFFS.open("/");
	auto file = root.openNextFile();
	while (file) {
		Display.SendLogLn(file.name());
		Display.SendLogLn(String(file.size()));
		file = root.openNextFile();
	}
	String fileName = "/Main.tft";
	String ver = String(VERTION_FIRMWARE + 1);
	String url = UPDATE_URL + PATH_SPIFFS + "index.php";
////////////////////////////////////////OTA SPIFFS /////////////////////////////////////////////////////
	Display.SendLogLnTime("[OTA]:SPIFFS update = " + url);
	if (url.length() > 0) {
		if (isForce) {
			httpUpdate.rebootOnUpdate(false);
			resspiffs = httpUpdate.updateSpiffs(client, url, String(Display.GetVersion() + 1));
			SendResultOTAUpdate(resspiffs);
		}
	}

////////////////////////////////////////OTA FIRMWARE /////////////////////////////////////////////////////
	url = UPDATE_URL + PATH_FIRMWARE + "index.php";
	Display.SendLogLnTime("[OTA]:Firmware update" + url);
	if (url.length() > 0) {

		if (isForce) {
			httpUpdate.rebootOnUpdate(true);
			resfirmware = httpUpdate.update(client, url, ver);
			SendResultOTAUpdate(resfirmware);
		}
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	Display.SendLogLnTime("[NEX]:Start update firmware screen!");
	NexUpload nexUpload(fileName, 115200, ChangeLog);
	if (nexUpload._checkFile()) {
		if (nexUpload.upload()) {
			Display.SendLogLnTime("[NEX]:Update screen complete!");
			root = SPIFFS.open("/");
			file = root.openNextFile();
			while (file) {
				if (String(file.name()).indexOf(fileName) != -1) {
					SPIFFS.remove(file.name());
					Display.SendLogLn("[NEX]: File exists! Delete file: " + String(file.name()));
					file = root.openNextFile();
					continue;
				}
				Display.SendLogLn(file.name());
				file = root.openNextFile();
			}
			Display.SendLogLnTime("[NEX]:Restart controller...");
			delay(1000);
			ESP.restart();
		}
	} else {
		Display.SendLogLnTime("[NEX]: Display no need update!");
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void SendResultOTAUpdate(t_httpUpdate_return ret) {
	switch (ret) {
	case HTTP_UPDATE_FAILED: {
		if (httpUpdate.getLastError() == -104) {
			Display.SendLogLnTime("[OTA]: No Update!");
		} else {
			Display.SendLogLnTime("[OTA]: Update Error!");
			String error = "[OTA]: HTTP_UPDATE_FAILED Error (";
			error += httpUpdate.getLastError();
			error += "): ";
			error += httpUpdate.getLastErrorString().c_str();
			Display.SendLogLnTime(error);
		}
		break;
	}
	case HTTP_UPDATE_NO_UPDATES: {
		Display.SendLogLnTime("[OTA]: No Update!");
		break;
	}
	case HTTP_UPDATE_OK: {
		Display.SendLogLnTime("[OTA]: Complete update!");
		break;
	}
	}
}


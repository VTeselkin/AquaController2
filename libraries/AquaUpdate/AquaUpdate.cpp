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
	Display.SendLogLnTime("[SPIFFS]:Mounting success! Size: " + String(Helper.SPIFFSSize()));
	Display.SendLog("Kb");
	Display.SendLogLnTime("[SPIFFS]:Free space: " + String((SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024));
	Display.SendLog("Kb");

}
void AquaUpdate::CheckOTAUpdate(bool isForce, DynamicJsonBuffer &jsonBuffer, void (*ChangeLog)(String)) {

	if (!Helper.data.auto_update) {
		Display.SendLogLnTime("[OTA]:DISABLE");
		return;
	}
	t_httpUpdate_return resspiffs = HTTP_UPDATE_FAILED;
	t_httpUpdate_return resfirmware = HTTP_UPDATE_FAILED;
	auto root = SPIFFS.open("/");
	auto file = root.openNextFile();
	bool isDisplayConnect = false;
	String verDisplay = String(Display.GetVersion() + 1);
	NexUpload nexUpload(115200, ChangeLog);
	String ver = String(VERTION_FIRMWARE + 1);
	if (Display.GetVersion() != 0) {
		nexUpload.PathForUpdate = Display.GetType(); //"NX4832T035_011R"; "NX4832K035_011R";
		nexUpload.FileForUpdate = "/" + nexUpload.PathForUpdate + ".tft";
		Display.SendLogLnTime("[NEX]:Display ID:" + nexUpload.FileForUpdate);
		isDisplayConnect = true;
	}

	////////////////////////////////////////OTA FIRMWARE /////////////////////////////////////////////////////

	String short_url = PATH_FIRMWARE + "index.php";
	String url = UPDATE_URL + short_url;
	Display.SendLogLnTime("[OTA]:Firmware update " + short_url);
	if (url.length() > 0) {

		if (isForce) {
			httpUpdate.rebootOnUpdate(true);
			esp_task_wdt_delete(NULL);
			esp_task_wdt_deinit();
			resfirmware = httpUpdate.update(client, url, ver);
			esp_task_wdt_init(WDT_TIMEOUT, true);
			esp_task_wdt_add(NULL);
			esp_task_wdt_reset();

			SendResultOTAUpdate(resfirmware);
		}
	}
////////////////////////////////////////OTA SPIFFS /////////////////////////////////////////////////////

	if (isDisplayConnect) { //TODO check other chips
		short_url = PATH_SPIFFS + nexUpload.PathForUpdate + "/index.php";
		url = UPDATE_URL + short_url;
		UPDATE_URL + short_url;
		Display.SendLogLnTime("[OTA]:SPIFFS update = " + short_url);
		if (url.length() > 0) {
			if (isForce) {
				httpUpdate.rebootOnUpdate(false);
				esp_task_wdt_delete(NULL);
				esp_task_wdt_deinit();
				resspiffs = httpUpdate.updateSpiffs(client, url, verDisplay);
				esp_task_wdt_init(WDT_TIMEOUT, true);
				esp_task_wdt_add(NULL);
				esp_task_wdt_reset();
				SendResultOTAUpdate(resspiffs);
			}
		}
	} else {
		Display.SendLogLnTime("[OTA]: Display not fount. Try check to connection!");
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (isDisplayConnect && resspiffs == HTTP_UPDATE_OK) {
		Display.SendLogLnTime("[NEX]:Start update firmware screen!");
		esp_task_wdt_reset();
		if (nexUpload.upload()) {
			Display.SendLogLnTime("[NEX]:Update screen complete!");
			root = SPIFFS.open("/");
			file = root.openNextFile();
			while (file) {
				if (String(file.name()).indexOf(nexUpload.PathForUpdate) != -1) {
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
	}
	while (file) {
		String info = "[SPIFFS]:File '" + String(file.name()) + "' size = " + String(file.size() / 1024) + "Kb";
		Display.SendLogLnTime(info);
		file = root.openNextFile();
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

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


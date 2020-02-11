/*
 * AquaUpdate.cpp
 *
 *  Created on: Nov 18, 2019
 *      Author: doc
 */

#include "AquaUpdate.h"
#include <ESP32httpUpdate.h>

void AquaUpdate::CheckOTAUpdate(bool isForce, void (*funcChangeLog)(String), DynamicJsonBuffer &jsonBuffe) {
	String url = "";

	funcChangeLog("OTA: Web update");
	url = OTAUpdate(UPDATE_URL + PATH_SPIFFS + "index.php", jsonBuffe);
	if (url.length() > 0) {
		if (isForce) {
			SPIFFS.format();
			ESPhttpUpdate.rebootOnUpdate(false);
			   ESPhttpUpdate.update(url);
			SendResultOTAUpdate(ESPhttpUpdate.updateSpiffs(url), funcChangeLog);
		} else {
			funcChangeLog("OTA: YOU NEED UPDATE");
		}
	} else {
		funcChangeLog("OTA: No Update!");
	}

	funcChangeLog("OTA: Firmware update");
	url = OTAUpdate(UPDATE_URL + PATH_FIRMWARE + "index.php", jsonBuffe);
	if (url.length() > 0) {
		if (isForce) {
			ESPhttpUpdate.rebootOnUpdate(true);
			SendResultOTAUpdate(ESPhttpUpdate.update(url), funcChangeLog);
		} else {
			funcChangeLog("OTA: YOU NEED UPDATE");
		}

	} else {
		funcChangeLog("OTA: No Update!");
	}

	if (!SPIFFS.begin()) {
		return;
	}
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		funcChangeLog(dir.fileName());
	}

}

String OTAUpdate(String host, DynamicJsonBuffer &jsonBuffer) {

	httpUpdateClient.begin(host);
	int httpCode = httpUpdateClient.GET();
	if (httpCode > 0) {
		jsonBuffer.clear();
		JsonObject& root = jsonBuffer.parseObject(httpUpdateClient.getString());
		JsonArray& array = root["downloads"].asArray();
		String url = array[array.size() - 1]["url"];
		String id = array[array.size() - 1]["file"];

		if (id.length() > 0 && !id.equals(VERTION_FIRMWARE)) {
			return url;
		}
	}
	return "";
}

void SendResultOTAUpdate(t_httpUpdate_return ret, void (*funcChangeLog)(String)) {
	switch (ret) {
	case HTTP_UPDATE_FAILED:
		funcChangeLog("OTA Update Error!");
		funcChangeLog(httpUpdateClient.errorToString(ESPhttpUpdate.getLastError()));
		break;
	case HTTP_UPDATE_NO_UPDATES:
		funcChangeLog("OTA: No Update!");
		break;
	case HTTP_UPDATE_OK:
		funcChangeLog("OTA: Complete update!");
		break;
	}
}



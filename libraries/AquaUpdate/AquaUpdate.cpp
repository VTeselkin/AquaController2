/*
 * AquaUpdate.cpp
 *
 *  Created on: Nov 18, 2019
 *      Author: doc
 */

#include "AquaUpdate.h"
#include <HttpClient.h>
#include <SPIFFS.h>
#include <FS.h>

HTTPClient httpUpdateClient;
WiFiClient client;

AquaUpdate::AquaUpdate(){

}
void AquaUpdate::Init(){
	if (!SPIFFS.begin(true)) {
	    Serial.println("An Error has occurred while mounting SPIFFS");
	    return;
	  }
	Serial.println("SPIFFS mounting success!");
	Serial.print("SPIFFS free space: ");
	Serial.print((SPIFFS.totalBytes()-SPIFFS.usedBytes())/1024);
	Serial.println(" Kb ");


}
void AquaUpdate::CheckOTAUpdate(bool isForce, void (*funcChangeLog)(String), DynamicJsonBuffer &jsonBuffer) {

	if(!Helper.data.auto_update){
		funcChangeLog("OTA: DISABLE");
		return;
	}

	String url = UPDATE_URL + PATH_SPIFFS + "index.php";
	funcChangeLog("OTA: Web update = " + url);
	if (url.length() > 0) {
		if (isForce) {
			httpUpdate.rebootOnUpdate(false);
			auto res = httpUpdate.updateSpiffs(client, url, VERTION_FIRMWARE);
			SendResultOTAUpdate(res, funcChangeLog);
		} else {
			funcChangeLog("OTA: YOU NEED UPDATE");
		}
	} else {
		funcChangeLog("OTA: No Update!");
	}

	url = UPDATE_URL + PATH_FIRMWARE + "index.php";
	funcChangeLog("OTA: Firmware update" + url);
	if (url.length() > 0) {
		if (isForce) {
			httpUpdate.rebootOnUpdate(true);
			auto res = httpUpdate.update(client, url, VERTION_FIRMWARE);
			SendResultOTAUpdate(res, funcChangeLog);
		} else {
			funcChangeLog("OTA: YOU NEED UPDATE");
		}

	} else {
		funcChangeLog("OTA: No Update!");
	}

	auto root = SPIFFS.open("/");
	auto file = root.openNextFile();
	while (file) {
		funcChangeLog(file.name());
		file = root.openNextFile();
	}

}

void SendResultOTAUpdate(t_httpUpdate_return ret, void (*funcChangeLog)(String)) {
	switch (ret) {
	case HTTP_UPDATE_FAILED:
	{
		funcChangeLog("OTA Update Error!");
		String error = "HTTP_UPDATE_FAILED Error (";
		error += httpUpdate.getLastError();
		error +="): ";
		error+= httpUpdate.getLastErrorString().c_str();
		error+="\n";
		funcChangeLog(error);
		break;
	}
	case HTTP_UPDATE_NO_UPDATES:
	{
		funcChangeLog("OTA: No Update!");
		break;
	}
	case HTTP_UPDATE_OK:
	{
		funcChangeLog("OTA: Complete update!");
		break;
	}
	}
}



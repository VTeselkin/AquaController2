/*
 * AquaNTP.cpp
 *
 *  Created on: Oct 31, 2019
 *      Author: doc
 */

#include <AquaNTP.h>

const char* ntpServerName[] = { "time2.google.com", "time3.google.com", "time4.google.com" };
IPAddress timeServerIP;
const word UTC3 = 3; //UTC+3
byte indexNTPServer = 0;

bool AquaNTP::SetNTPTimeToController(void (*ChangeLog)(String)) {

	if (!Helper.data.ntp_update) {
		ChangeLog("NTP:Update disable..");
		indexNTPServer = 0;
		return false;
	}
	ChangeLog("NTP:Sending packet:");

	ChangeLog(ntpServerName[indexNTPServer]);
	if (WiFi.hostByName(ntpServerName[indexNTPServer], timeServerIP) != 1) {
		ChangeLog("NTP:Lost connection");
		indexNTPServer++;
		if (indexNTPServer < NTPServerCount) {
			SetNTPTimeToController(ChangeLog);
		} else {
			indexNTPServer = 0;
			return false;
		}
	}
	WiFiUDP UdpNTP;
	NTPClient timeClient(UdpNTP, ntpServerName[indexNTPServer]);
	timeClient.setTimeOffset(UTC3 * 3600);
	timeClient.begin();
	timeClient.update();
	Helper.SetTimeNow(timeClient.getEpochTime());
	indexNTPServer = 0;
	ChangeLog("NTP:Time update");
	timeClient.end();
	return true;

}

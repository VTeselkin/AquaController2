/**
 * @file NexUpload.cpp
 *
 * The implementation of download tft file for nextion.
 *
 * @author  Chen Zengpeng (email:<zengpeng.chen@itead.cc>)
 * @date    2016/3/29
 * @copyright
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include "NexUpload.h"
void (*funcChangeLogUpload)(String);

NexUpload::NexUpload(String file_name, uint32_t download_baudrate, void (*ChangeLog)(String)) {
	_file_name = file_name;
	_download_baudrate = download_baudrate;
	funcChangeLogUpload = ChangeLog;
	if (_checkFile()) {
		_myFile = SPIFFS.open(_file_name, "r");
		_undownloadByte = _myFile.size();
	}
}

bool NexUpload::upload(void) {
	if (!_checkFile()) {
		Display.SendLogLnTime("[NEX]: File is not exit");
		return false;
	}

	if (_getBaudrate() == 0) {
		Display.SendLogLn("[NEX]: Error");
		return false;
	}
	if (!_echoTest("mystop_yesABC")) {
		Display.SendLogLn("[NEX]: Echo test failed");
		return false;
	}
	if (!_handlingSleepAndDim()) {
		Display.SendLogLn("[NEX]: Handling sleep and dim settings failed");
		return false;
	}

	if (!_setDownloadBaudrate(_download_baudrate)) {
		Display.SendLogLn("[NEX]: Error");
		return false;
	}

	if (!upload(_myFile)) {
		Display.SendLogLn("[NEX]: Error update screen firmware");
		return false;
	}
	return true;
}

uint16_t NexUpload::_getBaudrate(void) {
	uint32_t baudrate_array[7] = { 115200, 19200, 9600, 57600, 38400, 4800, 2400 };
	for (uint8_t i = 0; i < 7; i++) {
		if (_searchBaudrate(baudrate_array[i])) {
			_baudrate = baudrate_array[i];
			break;
		}
	}
	return _baudrate;
}

bool NexUpload::_checkFile(void) {
	if (!SPIFFS.exists(_file_name)) {
		return false;
	}

	return true;
}

bool NexUpload::_searchBaudrate(uint32_t baudrate) {

	String response = String("");
	Serial.begin(baudrate);
	const char _nextion_FF_FF[3] = { 0xFF, 0xFF, 0x00 };
	this->sendCommand("DRAKJHSUYDGBNCJHGJKSHBDN");
	this->sendCommand("", true, true); // 0x00 0xFF 0xFF 0xFF
	this->recvRetString(response);
	if (response[0] != 0x1A) {
		Display.SendLogLnTime("[NEX]: First indication that baudrate is wrong");
		return 0;
	}

	this->sendCommand("connect"); // first connect attempt

	this->recvRetString(response);
	if (response.indexOf(F("comok")) == -1) {
		Display.SendLogLnTime("[NEX]: display doesn't accept the first connect request");
		return 0;
	}

	response = String("");
	delay(110); // based on serial analyser from Nextion editor V0.58 to Nextion display NX4024T032_011R
	this->sendCommand(_nextion_FF_FF, false, false);

	this->sendCommand("connect"); // second attempt
	this->recvRetString(response);
	if (response.indexOf(F("comok")) == -1 && response[0] != 0x1A) {
		Display.SendLogLnTime(F("display doesn't accept the second connect request"));
		Display.SendLogLnTime(F("conclusion, wrong baudrate"));

		return 0;
	}

	return 1;
}

bool NexUpload::_setDownloadBaudrate(uint32_t upload_baudrate) {

	String response = String("");
	String cmd = String("");

	cmd = F("00");
	this->sendCommand(cmd.c_str());
	delay(0.1);

	this->recvRetString(response, 800, true); // normal response time is 400ms

	String filesize_str = String(_undownloadByte, 10);
	String baudrate_str = String(upload_baudrate);
	cmd = "whmi-wri " + filesize_str + "," + baudrate_str + ",0";

	this->sendCommand(cmd.c_str());

	// Without flush, the whmi command will NOT transmitted by the ESP in the current baudrate
	// because switching to another baudrate (nexSerialBegin command) has an higher prio.
	// The ESP will first jump to the new 'upload_baudrate' and than process the serial 'transmit buffer'
	// The flush command forced the ESP to wait until the 'transmit buffer' is empty
	Serial.flush();

	Serial.begin(upload_baudrate);
	this->recvRetString(response, 800, true); // normal response time is 400ms

	// The Nextion display will, if it's ready to accept data, send a 0x05 byte.
	if (response.indexOf(0x05) != -1) {
		return 1;
	} else {
		return 0;
	}
}

//bool NexUpload::_downloadTftFile(void) {
//	uint8_t c;
//	uint16_t send_timer = 0;
//	uint16_t last_send_num = 0;
//	String string = String("");
//	send_timer = _undownloadByte / 4096 + 1;
//	last_send_num = _undownloadByte % 4096;
//
//	while (send_timer) {
//
//		if (send_timer == 1) {
//			for (uint16_t j = 1; j <= 4096; j++) {
//				if (j <= last_send_num) {
//					c = _myFile.read();
//					Serial.write(c);
//				} else {
//					break;
//				}
//			}
//		}
//
//		else {
//			for (uint16_t i = 1; i <= 4096; i++) {
//				c = _myFile.read();
//				Serial.write(c);
//			}
//		}
//		this->recvRetString(string, 600, true);
//		funcChangeLogUpload(string);
//		if (string.indexOf(0x05) != -1) {
//			string = "";
//		} else {
//			Helper.ToneForce(2000, 500);
//			return 0;
//		}
//		--send_timer;
//	}
//	Helper.ToneForce(1000, 100);
//	delay(100);
//	Helper.ToneForce(1000, 100);
//	return 1;
//}
bool NexUpload::upload(Stream &myFile) {

	// create buffer for read
	uint8_t buff[2048] = { 0 };

	// read all data from server
	while (_undownloadByte > 0 || _undownloadByte == -1) {

		// get available data size
		size_t size = myFile.available();

		if (size) {
			// read up to 2048 byte into the buffer
			int c = myFile.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

			// Write the buffered bytes to the nextion. If this fails, return false.
			if (!this->upload(buff, c)) {
				return false;
			} else {
				if (_updateProgressCallback) {
					_updateProgressCallback();
				}
			}

			if (_undownloadByte > 0) {
				_undownloadByte -= c;
			}
		}
		delay(1);
	}

	return true;
}

bool NexUpload::upload(const uint8_t *file_buf, size_t buf_size) {

	uint8_t c;
	uint8_t timeout = 0;
	String string = String("");

	for (uint16_t i = 0; i < buf_size; i++) {

		// Users must split the .tft file contents into 4096 byte sized packets with the final partial packet size equal to the last remaining bytes (<4096 bytes).
		if (_sent_packets == 4096) {

			// wait for the Nextion to return its 0x05 byte confirming reception and readiness to receive the next packets
			this->recvRetString(string, 500, true);
			if (string.indexOf(0x05) != -1) {

				// reset sent packets counter
				_sent_packets = 0;

				// reset receive String
				string = "";
			} else {
				if (timeout >= 8) {
					return false;
				}

				timeout++;
			}

			// delay current byte
			i--;

		} else {

			// read buffer
			c = file_buf[i];
			// write byte to nextion over serial
			Serial.write(c);

			// update sent packets counter
			_sent_packets++;
		}
	}

	return true;
}

bool NexUpload::_echoTest(String input) {
	String cmd = String("");
	String response = String("");

	cmd = "print \"" + input + "\"";
	this->sendCommand(cmd.c_str());

	uint32_t duration_ms = calculateTransmissionTimeMs(cmd) * 2 + 10; // times 2  (send + receive) and 10 ms extra
	this->recvRetString(response, duration_ms);

	return (response.indexOf(input) != -1);
}

uint32_t NexUpload::calculateTransmissionTimeMs(String message) {
	// In general, 1 second (s) = 1000 (10^-3) millisecond (ms) or
	//             1 second (s) = 1000 000 (10^-6) microsecond (us).
	// To calculate how much microsecond one BIT of data takes with a certain baudrate you have to divide
	// the baudrate by one second.
	// For example 9600 baud = 1000 000 us / 9600 ≈ 104 us
	// The time to transmit one DATA byte (if we use default UART modulation) takes 10 bits.
	// 8 DATA bits and one START and one STOP bit makes 10 bits.
	// In this example (9600 baud) a byte will take 1041 us to send or receive.
	// Multiply this value by the length of the message (number of bytes) and the total transmit/ receive time
	// is calculated.

	uint32_t duration_one_byte_us = 10000000 / _baudrate; // 1000 000 * 10 bits / baudrate
	uint16_t nr_of_bytes = message.length() + 3;          // 3 times 0xFF byte
	uint32_t duration_message_us = nr_of_bytes * duration_one_byte_us;
	uint32_t return_value_ms = duration_message_us / 1000;
	return return_value_ms;
}

bool NexUpload::_handlingSleepAndDim() {
	String cmd = String("");
	String response = String("");
	bool set_sleep = false;
	bool set_dim = false;

	cmd = F("get sleep");
	this->sendCommand(cmd.c_str());

	this->recvRetString(response);

	if (response[0] != 0x71) {
		Display.SendLogLn("[NEX]: Unknown response from 'get sleep' request");
		return false;
	}

	response = String("");
	cmd = F("get dim");
	this->sendCommand(cmd.c_str());

	this->recvRetString(response);

	if (response[0] != 0x71) {
		Display.SendLogLn("[NEX]: Unknown response from 'get dim' request");
		return false;
	}

	if (response[1] == 0x00) {

		set_dim = true;
	}

	if (!_echoTest("ABC")) {
		Display.SendLogLn("[NEX]: Echo test in 'handling sleep and dim' failed");
		return false;
	}

	if (set_sleep) {
		cmd = F("sleep=0");
		this->sendCommand(cmd.c_str());
		// Unfortunately the display doesn't send any respone on the wake up request (sleep=0)
		// Let the ESP wait for one second, this is based on serial analyser from Nextion editor V0.58 to Nextion display NX4024T032_011R
		// This gives the Nextion display some time to wake up
		delay(1000);
	}

	if (set_dim) {
		cmd = F("dim=100");
		this->sendCommand(cmd.c_str());
		delay(15);
	}

	return true;
}
void NexUpload::sendCommand(const char *cmd) {

	while (Serial.available()) {
		Serial.read();
	}

	Serial.print(cmd);
	Serial.write(0xFF);
	Serial.write(0xFF);
	Serial.write(0xFF);
}

void NexUpload::sendCommand(const char *cmd, bool tail, bool null_head) {

	if (null_head) {
		Serial.write(0x00);
	}

	while (Serial.available()) {
		Serial.read();
	}

	Serial.print(cmd);
	if (tail) {
		Serial.write(0xFF);
		Serial.write(0xFF);
		Serial.write(0xFF);
	}

}

uint16_t NexUpload::recvRetString(String &string, uint32_t timeout, bool recv_flag) {
	uint16_t ret = 0;
	uint8_t c = 0;
	long start;
	bool exit_flag = false;
	start = millis();
	while (millis() - start <= timeout) {
		while (Serial.available()) {
			c = Serial.read();
			if (c == 0) {
				continue;
			}
			string += (char) c;

			if (recv_flag) {

				if (string.indexOf(0x05) != -1) {
					exit_flag = true;
				}
			}
		}
		if (exit_flag) {
			break;
		}
	}

	ret = string.length();
	return ret;
}

void NexUpload::setUpdateProgressCallback(THandlerFunction value) {
	_updateProgressCallback = value;
}

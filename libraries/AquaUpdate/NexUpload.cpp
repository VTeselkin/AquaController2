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
NexUpload::NexUpload(String file_name,uint32_t download_baudrate, void (*ChangeLog)(String))
{
    _file_name = file_name;
    _download_baudrate = download_baudrate;
    funcChangeLogUpload = ChangeLog;

}

void NexUpload::upload(void)
{

    if(!_checkFile())
    {
    	Display.SendLogLn("[NEX]: The file is error");
        return;
    }
    delay(500);
    if(_getBaudrate() == 0)
    {
    	Display.SendLogLn("[NEX]: Get baudrate error");
        return;
    }
    delay(500);
    if(!_setDownloadBaudrate(_download_baudrate))
    {
    	Display.SendLogLn("[NEX]: Modify baudrate error");
        return;
    }
    delay(500);
    if(!_downloadTftFile())
    {
    	Display.SendLogLn("[NEX]: Download file error");
        return;
    }
    Display.SendLogLn("[NEX]: Download ok");
}

uint16_t NexUpload::_getBaudrate(void)
{
    uint32_t baudrate_array[7] = {115200,19200,9600,57600,38400,4800,2400};
    for(uint8_t i = 0; i < 7; i++)
    {
        if(_searchBaudrate(baudrate_array[i]))
        {
            _baudrate = baudrate_array[i];
            Display.SendLogLn("[NEX]: Get baudrate");
            break;
        }
    }
    return _baudrate;
}

bool NexUpload::_checkFile(void)
{
	Display.SendLogLn("[NEX]: Start Check file");

    if(!SPIFFS.exists(_file_name))
    {
    	Display.SendLogLn("[NEX]: File is not exit");
    	return false;
    }
    _myFile = SPIFFS.open(_file_name, FILE_READ);

    _undownloadByte = _myFile.size();

    Display.SendLog("[NEX]: TFT file size is:");
    Display.SendLog(String(_undownloadByte));
    Display.SendLogLn("Kb");
    Display.SendLogLn("[NEX]: Check file OK!");

    return true;
}

bool NexUpload::_searchBaudrate(uint32_t baudrate)
{
    String string = String("");
    Serial.begin(baudrate);
    this->sendCommand("");
    this->sendCommand("connect");
    this->recvRetString(string);
    if(string.indexOf("comok") != -1)
    {
    	funcChangeLogUpload(string);
        return true;
    }
    return false;
}

void NexUpload::sendCommand(const char* cmd)
{

    while (Serial.available())
    {
    	Serial.read();
    }

    Serial.print(cmd);
    Serial.write(0xFF);
    Serial.write(0xFF);
    Serial.write(0xFF);
}

uint16_t NexUpload::recvRetString(String &string, uint32_t timeout,bool recv_flag)
{
    uint16_t ret = 0;
    uint8_t c = 0;
    long start;
    bool exit_flag = false;
    start = millis();
    while (millis() - start <= timeout)
    {
        while (Serial.available())
        {
            c = Serial.read();
            if(c == 0)
            {
                continue;
            }
            string += (char)c;

            if(recv_flag)
            {

                if(string.indexOf(0x05) != -1)
                {
                    exit_flag = true;
                }
            }
        }
        if(exit_flag)
        {
            break;
        }
    }

    ret = string.length();
    return ret;
}

bool NexUpload::_setDownloadBaudrate(uint32_t baudrate)
{
    String string = String("");
    String cmd = String("");
    String filesize_str = String(_undownloadByte,10);
    String baudrate_str = String(baudrate,10);
    cmd = "whmi-wri " + filesize_str + "," + baudrate_str + ",0";
    this->sendCommand("");
    this->sendCommand(cmd.c_str());
    delay(50);
    Serial.begin(baudrate);
    this->recvRetString(string,500);
    if(string.indexOf(0x05) != -1)
    {
    	funcChangeLogUpload(string);
        return 1;
    }
    return 0;
}

bool NexUpload::_downloadTftFile(void)
{
    uint8_t c;
    uint16_t send_timer = 0;
    uint16_t last_send_num = 0;
    String string = String("");
    send_timer = _undownloadByte / 4096 + 1;
    last_send_num = _undownloadByte % 4096;

    while(send_timer)
    {

        if(send_timer == 1)
        {
            for(uint16_t j = 1; j <= 4096; j++)
            {
                if(j <= last_send_num)
                {
                    c = _myFile.read();
                    Serial.write(c);
                }
                else
                {
                    break;
                }
            }
        }

        else
        {
            for(uint16_t i = 1; i <= 4096; i++)
            {
                c = _myFile.read();
                Serial.write(c);
            }
        }
        this->recvRetString(string,600,true);
        funcChangeLogUpload(string);
        if(string.indexOf(0x05) != -1)
        {
            string = "";
        }
        else
        {
        	Helper.ToneForce(2000, 500);
            return 0;
        }
         --send_timer;
    }
    Helper.ToneForce(1000, 100);
    delay(100);
    Helper.ToneForce(1000, 100);
    return 1;
}

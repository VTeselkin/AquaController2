#ifndef __NEXUPLOAD_H__
#define __NEXUPLOAD_H__

#include <esp_task_wdt.h>
#include <Arduino.h>
#include <AquaDisplay.h>
#include <SPI.h>
#include <SPIFFS.h>
typedef std::function<void(void)> THandlerFunction;
class NexUpload {
public:
	typedef std::function<void(void)> THandlerFunction;
	NexUpload(uint32_t download_baudrate, void (*ChangeLog)(String));
	~NexUpload() {
	}
	void setUpdateProgressCallback(THandlerFunction value);
	bool upload();
	bool _checkFile(void);
	String FileForUpdate;
	String PathForUpdate;
private:
	uint16_t _getBaudrate(void);
	bool _searchBaudrate(uint32_t baudrate);
	bool _setDownloadBaudrate(uint32_t baudrate);
	bool _downloadTftFile(void);
	void sendCommand(const char *cmd);
	void sendCommand(const char *cmd, bool tail, bool null_head);
	uint16_t recvRetString(String &string, uint32_t timeout = 100, bool recv_flag = false);
	bool _echoTest(String input);
	uint32_t calculateTransmissionTimeMs(String message);
	bool _handlingSleepAndDim();
	bool upload(const uint8_t *file_buf, size_t buf_size);
	bool upload(Stream &myFile);
	void softReset();
private:
	uint32_t _baudrate;
	File _myFile;
	uint32_t _undownloadByte;
	uint32_t _download_baudrate;
	uint16_t _sent_packets = 0;
	THandlerFunction _updateProgressCallback;
};

#endif /* #ifndef __NEXDOWNLOAD_H__ */

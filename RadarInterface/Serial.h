#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>

class CSerial
{
public:
	CSerial(void);
	~CSerial(void);
	UINT OpenSerialPort(char * lpctPort, char * lpctBaud);
	UINT CloseSerialPort(void);
	UINT send_char(UINT8 s);
	UINT send_buf(UINT8 *s, UINT count);
	UINT get_char(timeval * Timeout);
	UINT get_buf(UINT8 *s, UINT count, timeval * Timeout);
	UINT flush_com(void);
	bool BytesAvailable(void);
	void ErrorBox(UINT error, char * message);
private:
	int hComm;
	struct termios tio;
	struct termios old_tio;
	fd_set readfds;
};

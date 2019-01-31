#include "RadarTypes.h"
#include "RadarInterface/Serial.h"
#include <stdio.h>

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif

CSerial::CSerial(void)
{
	hComm = INVALID_HANDLE_VALUE;
	FD_ZERO(&readfds);
}

CSerial::~CSerial(void)
{
	if(hComm != INVALID_HANDLE_VALUE)
	{
		flush_com();
	}
	CloseSerialPort();
}
UINT CSerial::OpenSerialPort(char * lpctPort, char * lpctBaud)
{
	UINT dw;

#ifdef IS_TI_ARM
	hComm=open(lpctPort, O_RDWR | O_NONBLOCK);
// Set up file descriptors for select()
	FD_SET(hComm, &readfds);
	if(hComm != INVALID_HANDLE_VALUE)
	{
		tcgetattr(hComm, &old_tio);
		memset(&tio, 0, sizeof(tio));
		tio.c_iflag=0;
		tio.c_oflag=0;
		tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
		tio.c_lflag=0;
		tio.c_cc[VMIN]=1;
		tio.c_cc[VTIME]=5;
		if(strcmp(lpctBaud, "115200") == 0)
		{
			cfsetospeed(&tio,B115200);            // 115200 baud
			cfsetispeed(&tio,B115200);            // 115200 baud
			dw = 0;
 		}
		else if(strcmp(lpctBaud, "230400") == 0)
		{
			cfsetospeed(&tio,B230400);            // 230400 baud
			cfsetispeed(&tio,B230400);            // 230400 baud
			dw = 0;
 		}
        else if(strcmp(lpctBaud, "460800") == 0)
        {
            cfsetospeed(&tio,B460800);            // 460800 baud
            cfsetispeed(&tio,B460800);            // 460800 baud
            dw = 0;
        }
        else if(strcmp(lpctBaud, "921600") == 0)
        {
            cfsetospeed(&tio,B921600);            // 921600 baud
            cfsetispeed(&tio,B921600);            // 921600 baud
            dw = 0;
        }
        else
		{
			close(hComm);
			dw = 0x8000;
		}
		if(dw == 0)
		{
			tcsetattr(hComm,TCSANOW,&tio);
			tcflush(hComm, TCIOFLUSH);
		}
	}
#else
    hComm=open("/dev/null", O_RDWR | O_NONBLOCK);
    if(hComm != INVALID_HANDLE_VALUE)
    {
        dw = 0;
    }
#endif
    else
	{
		dw = 0x8000;
	}
	return dw;
}


UINT CSerial::CloseSerialPort(void)
{
	if(hComm != INVALID_HANDLE_VALUE)
	{
		tcsetattr(hComm, TCSANOW, &old_tio);
		close(hComm);
		FD_CLR(hComm, &readfds);
	}
	hComm = INVALID_HANDLE_VALUE;
	return 0;
}

UINT CSerial::send_char(UINT8 s)
{
	UINT NumberOfBytesWritten;
	UINT bret;
	NumberOfBytesWritten = write(hComm,	// handle to file
				     &s,	// data buffer
				     1);	// number of bytes to write
	if(NumberOfBytesWritten != 1)
	{
		bret = 0x8000;
	}
	else bret=0;
	return bret;
}

UINT CSerial::send_buf(UINT8 *s, UINT count)
{
	UINT NumberOfBytesWritten;
	UINT bret;
	NumberOfBytesWritten = write(hComm,	// handle to file
				     s,	        // data buffer
				     count);	// number of bytes to write
	if(NumberOfBytesWritten != count)
	{
		bret = 0x8000;
	}
	else bret=0;
	return bret;
}

UINT CSerial::get_char(timeval * Timeout)
{
	UINT8 ucChar;
	UINT bret;
	INT32 selRet;
	UINT dwNumberOfBytesToRead = 1;
	UINT dwNumberOfBytesRead = 0;
// Use select() to implement blocking read with timeout
	selRet = select(hComm+1, &readfds, NULL, NULL, Timeout);
	if(selRet > 0)
	{
		dwNumberOfBytesRead = read(hComm,               	// handle to file
					   &ucChar,            	        // data buffer
					   dwNumberOfBytesToRead);	// number of bytes to read 
		if (dwNumberOfBytesRead == 0)
		{
			bret = 0x8000;
		}
		else
			bret = ucChar;

		return(bret);
	}
	else if(selRet == 0) return 0xFFFF;  // timeout
	else return 0x8000;  //Error
}

UINT CSerial::get_buf(UINT8 *s, UINT count, timeval * Timeout)
{
	INT32 selRet;
	UINT dwNumberOfBytesRead = 0;
	UINT totalBytesRead = 0;
// Use select() to implement blocking read with timeout
	while(totalBytesRead < count)
	{
		selRet = select(hComm+1, &readfds, NULL, NULL, Timeout);
		if(selRet > 0)
		{
			dwNumberOfBytesRead = read(hComm,               
						   s,            
						   count); 
			if (dwNumberOfBytesRead == 0) return 0x8000;
		}
		else if(selRet == 0) return 0xFFFF;  // timeout
		else if(selRet < 0) return 0x8000;  //Error
		totalBytesRead += dwNumberOfBytesRead;
	}
	return totalBytesRead;
}

bool CSerial::BytesAvailable(void)
{
	UINT bytes;
	ioctl(hComm, FIONREAD, &bytes);
	if(bytes > 0)
	{
		return true;
	}
	return false;

}

UINT CSerial::flush_com(void)
{
	UINT bret = 0;
	if (! tcflush(hComm, TCIOFLUSH))
	{
		bret = 0x8000;
	}
	return bret;
}
void CSerial::ErrorBox(UINT syserror, char * message)
{
	printf("Error %d:%s'n", syserror, message);
}

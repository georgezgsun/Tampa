#include "RadarTypes.h"
#include "RadarInterface/Serial.h"
#include "RadarInterface/RadarData.h"
#include "RadarInterface/Crc.h"
#include <math.h>
#include <string.h>
#include <sys/select.h>

bool captureFileIsOpen = false;
bool playbackFileIsOpen = false;
unsigned int filePos = 0;
FILE * fileHandle;
int playbackBufferBytesAvailable = 0;
int playbackBufferIndex = 0;
bool stopPlayback = false;
bool pausePlayback = false;
bool stepPlayback = false;
pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t responseMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waitCondition = PTHREAD_COND_INITIALIZER;

void printbytes(void * data, int count)
{
    unsigned char* bytedata = (unsigned char *)data;
    for(int i = 0; i < count; i++)
    {
        printf("%02hhx ", bytedata[i]);
    }
    printf("\n");
}

// This class implements the Oculii radar protocol

CRadarData::CRadarData(void)
{
    messageState = init;
	radarTargetsState = getTgtIdle;
    portIsOpen = false;
}


CRadarData::~CRadarData(void)
{
}

// RadarComm methods:

bool CRadarData::SetDevice(interface_type port)
{
    if(portIsOpen)
    {
        printf("Skipping set of current radar interface to %d because port is open.\n", port);
        return true;	// Error - can't change interface while port is open
    }
    switch(port)
    {
        case serial:
        case playback:
        {
//            printf("Setting current radar interface to %d.\n", port);
            currentInterface = port;
            return false; // Success
        }
        default:
        {
            printf("Setting current radar interface to invalidInterface.\n");
            currentInterface = invalidInterface;
            return true;  // Error - invalid port argument
        }
    }
}

UINT CRadarData::OpenPort(char * lpctPort, char * lpctBaud)
{
    UINT retval;
    if(! portIsOpen)
    {
        strcpy(Port, lpctPort);
        strcpy(Baud, lpctBaud);
        switch(currentInterface)
        {
            case serial:
            {
                retval = SerialPort.OpenSerialPort(lpctPort, lpctBaud);
                if(retval == 0) portIsOpen = true;
                return retval;
            }
            case playback:
            {
                return 0;  // We don't need to open a "port".  Report success
            }
            default:
            {
                return 0x8000;
            }
        }
    }
    return 0x8000;  // Error - port is already open
}

UINT CRadarData::ClosePort(void)
{
    UINT retval;
    if(portIsOpen)
    {
        switch(currentInterface)
        {
            case serial:
            {
                retval = SerialPort.CloseSerialPort();
                portIsOpen = false;
                return retval;
            }
            case playback:
            {
                return 0;  // We don't have a "port" open.  Report success
            }
            default:
            {
                return 0x8000;
            }
        }
    }
    return 0x8000;  // Error - port is not open
}

UINT CRadarData::SendChar(UINT8 s)
{
    if(portIsOpen)
    {
        switch(currentInterface)
        {
            case serial:
            {
                return SerialPort.send_char(s);
            }
            case playback:
            {
                return 0x8000;
            }
            default:
            {
                return 0x8000;
            }
        }
    }
    return 0x8000;  // Error - port is not open
}

UINT CRadarData::SendBuf(UINT8 *s,  UINT count)
{
    if(portIsOpen)
    {
        switch(currentInterface)
        {
            case serial:
            {
                return SerialPort.send_buf(s, count);
            }
            case playback:
            {
                return 0x8000;
            }
            default:
            {
                return 0x8000;
            }
        }
    }
    return 0x8000;  // Error - port is not open
}

UINT CRadarData::GetChar(void)
{
    timeval Timeout = {1,0};
    if(portIsOpen)
    {
        switch(currentInterface)
        {
            case serial:
            {
                return SerialPort.get_char(&Timeout);
            }
            case playback:
            {
                return 0x8000;
            }
            default:
            {
                return 0x8000;
            }
        }
    }
    return 0x8000;  // Error - port is not open
}

UINT CRadarData::GetBuf(UINT8 *s,  UINT count)
{
    timeval Timeout = {1,0};
    if(portIsOpen)
    {
        switch(currentInterface)
        {
            case serial:
            {
                return SerialPort.get_buf(s, count, &Timeout);
            }
            case playback:
            {
                return 0x8000;
            }
            default:
            {
                return 0x8000;
            }
        }
    }
    return 0x8000;  // Error - port is not open
}

UINT CRadarData::FlushCom(void)
{
    if(portIsOpen)
    {
        switch(currentInterface)
        {
            case serial:
            {
                return SerialPort.flush_com();
            }
            case playback:
            {
                return 0x8000;
            }
            default:
            {
                return 0x8000;
            }
        }
    }
    return 0x8000;  // Error - port is not open
}

bool CRadarData::BytesAvailable(void)
{
    if(portIsOpen)
    {
        switch(currentInterface)
        {
            case serial:
            {
                return SerialPort.BytesAvailable();
            }
            case playback:
            {
                return false;
            }
            default:
            {
                return false;
            }
        }
    }
    return false;  // Error - port is not open
}

UINT32 CRadarData::RunRadarDataStateMachine(config_type * pConfigArg)
{
    pConfig = pConfigArg;  // Save Config structure pointer
    SetDevice(pConfig->port);
    if(!portIsOpen)
    {
        UINT status=0;
        if(pConfig->port == serial)
            status = OpenPort(pConfig->serialPortDev, pConfig->serialBaudRate);
        if(status)
        {
            printf("Failed to open RadarComm device %s.\n"
                   "Please check configuration settings.\n",
                                pConfig->serialPortDev);
            return 1;
        }
#ifdef CAPTURE_BIN
        capture_fd = creat(BIN_CAPTURE_FILE, 0x644);
#endif
        stopPlayback = false;
        pthread_mutex_lock(&waitMutex);  	//Lock the wait mutex
        pthread_mutex_lock(&stateMutex);  	//Get control of the message state
        messageState = idle;
        lastState = invalidState;
        pthread_cond_init(&waitCondition, NULL);
//        printf("Serial port init complete - starting state machine\n");
        while(messageState != shutDown)
        {
            RunRadarDataStates();
        }
    }
    else
    {
        printf("Serial port is already open.  Aborting RunRadarDataStateMachine.\n");
    }
    ClosePort();
    return 0;
}

void CRadarData::RunRadarDataStates(void)
{
//    if(messageState != lastState)
//    {
//        printf("RadarData state machine is entering state %d\n", messageState);
//        lastState = messageState;
//    }
    switch(messageState)
    {
        case idle:
        {
            pthread_mutex_unlock(&stateMutex);  	//Unlock the messageState
//            pthread_mutex_lock(&waitMutex);  	    //Lock the wait condition
            int retval = pthread_cond_wait(&waitCondition, &waitMutex);  //Wait for wakeup signal
    // Before wakeup, the messageState will be changed to beginPoll
            if(retval)
              printf("RunRadarDataStates() is continuing in idle state from pthread_cond_wait with return val = %d\n", retval);
            break;
        }
        case beginPoll:
        {
            pthread_mutex_lock(&stateMutex);  	//lock the messageState
            responseStatus = 0;			// Set waiting status
//            unsigned char * pChar = (unsigned char *)(pPollMsg);
//            printf("Payload pointer = %x\n", (unsigned int)(pPollMsg->payload));
//            for(int i = 0; i < 7; i++)
//            {
//                printf("%02hhx", pChar[i]);
//            }
            SendBuf((unsigned char *)pPollMsg,  7);		//Send 7 bytes of header
            if(pPollMsg->payloadLength > 0)
            {
//                for(int i = 0; i < pPollMsg->payloadLength; i++)
//                {
//                    printf("%02hhx", pPollMsg->payload[i]);
//                }
                SendBuf((unsigned char *)(pPollMsg->payload),
                     pPollMsg->payloadLength);
            }
//            pChar = (unsigned char *)(&(pPollMsg->crc));
//            for(int i = 0; i < 2; i++)
//            {
//                printf("%02hhx", pChar[i]);
//            }
//            printf("\n");
            SendBuf((unsigned char *)(&(pPollMsg->crc)), 2);	//Send the message CRC
            messageState = receiveMsgType;		//Next state
            break;
        }
        case receiveMsgType:
        {
            UINT theChar;
            theChar = GetChar();
#ifdef CAPTURE_BIN
            write(capture_fd, &theChar, 1);
#endif
            rcvBuffIndex = 0;
            if(theChar < 0x100)
            {
                theChar = theChar & 0Xff;
//                printf("Good character %x\n", theChar);
                pResponseMsg->msgType = theChar;
            }
            else
            {
                if(theChar == 0xffff) responseStatus = -1;	// Timeout status
                else responseStatus = -2;			// Error status
//                printf("Bad character %x\n", theChar);
                ClosePort();
                OpenPort(Port, Baud);
                messageState = idle;
                break;
            }
            if(pResponseMsg->msgType == 0xe9)
            {
                messageState = receiveHeader;		//Next state
                rcvBuffIndex = 1;
            }
            break;
        }
        case receiveHeader:
        {
            UINT theChar;
            theChar = GetChar();
#ifdef CAPTURE_BIN
            write(capture_fd, &theChar, 1);
#endif
            if(theChar < 0x100)
            {
                theChar = theChar & 0Xff;
                char * pResponseChar =
                    (char *)((void *)(pResponseMsg));
                pResponseChar[rcvBuffIndex] = theChar;
                rcvBuffIndex++;
            }
            else
            {
                if(theChar == 0xffff) responseStatus = -1;	// Timeout status
                else responseStatus = -2;			// Error status
                ClosePort();
                OpenPort(Port, Baud);
                messageState = idle;
                break;
            }
            if(rcvBuffIndex >= 7)
            {
                rcvBuffIndex = 0;
                if(pResponseMsg->payloadLength > 0)
                    messageState = receivePayload;	//Next state
                else messageState = receiveCRC;
            }
            break;
        }
        case receivePayload:
        {
            UINT theChar;
            theChar = GetChar();
#ifdef CAPTURE_BIN
            write(capture_fd, &theChar, 1);
#endif
            if(theChar < 0x100)
            {
                theChar = theChar & 0Xff;
                pResponseMsg->payload[rcvBuffIndex] = theChar;
                rcvBuffIndex++;
            }
            else
            {
                if(theChar == 0xffff) responseStatus = -1;	// Timeout status
                else responseStatus = -2;			// Error status
                ClosePort();
                OpenPort(Port, Baud);
                messageState = idle;
                break;
            }
            if(rcvBuffIndex >= pResponseMsg->payloadLength)
            {
                rcvBuffIndex = 0;
                messageState = receiveCRC;			//Next state
            }
            break;
        }
        case receiveCRC:
        {
            UINT theChar;
            theChar = GetChar();
#ifdef CAPTURE_BIN
            write(capture_fd, &theChar, 1);
#endif
            if(theChar < 0x100)
            {
                theChar = theChar & 0Xff;
                if(rcvBuffIndex == 0)
                    pResponseMsg->crc = theChar << 8;
                else
                    pResponseMsg->crc += theChar;
                rcvBuffIndex++;
            }
            else
            {
                if(theChar == 0xffff) responseStatus = -1;	// Timeout status
                else responseStatus = -2;			// Error status
                ClosePort();
                OpenPort(Port, Baud);
                messageState = idle;
                break;
            }
            if(rcvBuffIndex >= 2)
            {
                if((pResponseMsg->command & 0xC000) == 0x4000)
                {
//      			printf("Received ACK response");
//      			if(pResponseMsg->payloadLength == 0) printf(" - No payload\n");
//      			else printf(" - Payload size = %hu\n", pResponseMsg->payloadLength);
                    Parser(pResponseMsg->command & 0x3fff);
                    responseStatus = 1;		// Success status!
                }
                else if((ResponseMsg.command & 0xC000) == 0x8000)
                {
                    printf("Received NAK response = %hhu to command %hu\n", *pResponseMsg->payload, pResponseMsg->command & 0x3fff);
                    responseStatus = -3 - *pResponseMsg->payload;
                }
                messageState = idle;		// Next state
                break;
            }
        }
        case shutDown:
        {
            pthread_mutex_unlock(&stateMutex);  	//Unlock the messageState
            pthread_mutex_unlock(&waitMutex);   	//Unlock the wait condition
            break;
        }
        case invalidState:
        default:
        {
            break;
        }
    }
}

void CRadarData::ShutdownRadar(void)
{
    switch(currentInterface)
    {
        case serial:
        {
            pthread_mutex_lock(&waitMutex);		// Get control of wakeup signal
            pthread_mutex_lock(&stateMutex);  	// and the message state
            messageState = shutDown;		// Request shutDown
            pthread_mutex_unlock(&stateMutex);  	// Unlock the message state
            pthread_cond_signal(&waitCondition);	// Wake up the radar state machine
            pthread_mutex_unlock(&waitMutex);       // Release the wakeup signal
            break;
        }
        case playback:
        {
//			PlaybackMessageLoop();
            break;
        }
        case invalidInterface:
        default:
        {
            break;
        }
    }
}

bool CRadarData::PollRadar(poll_msg_t * pPollData, response_msg_t * pResponseData)
{
    switch(currentInterface)
    {
        case serial:
        {
//            printf("Running PollRadar() for message %hd\n", pPollData->command);
            pthread_mutex_lock(&waitMutex);		// Get control of wakeup signal
//            printf("Wait mutex locked in PollRadar()\n");
            pthread_mutex_lock(&stateMutex);  	// and the message state
//            printf("State mutex locked in PollRadar()\n");
            messageState = beginPoll;		// Start a poll
            pPollMsg = pPollData;			// Capture the poll and response pointers
            pResponseMsg = pResponseData;
            responseStatus = 0;			// Set waiting status
            pthread_mutex_unlock(&stateMutex);  	// Unlock the message state
//            printf("Waking up the radar comm thread for message %hd\n", pPollMsg->command);
            pthread_cond_signal(&waitCondition);	// Wake up the radar state machine
            pthread_mutex_unlock(&waitMutex);       // Release the wakeup signal
            break;
        }
        case playback:
        {
//			PlaybackMessageLoop();
            break;
        }
        case invalidInterface:
        default:
        {
            break;
        }
    }
    return false;
}

void CRadarData::setCallbacks(CaptureFunction pCaptureFunction, PlaybackFunction pPlaybackFunction, TrashBinFunction pTrashBinFunction)
{
    pCapture = pCaptureFunction;
    pPlayback = pPlaybackFunction;
    pTrashbin = pTrashBinFunction;
    messageState = idle;
}

// RadarData methods:

int CRadarData::CheckResponse()
{
    int response = responseStatus;
	if(response == -1) printf("Timeout receiving poll response - transaction aborted.\n");
	else if(response == -2) printf("Error receiving poll response - transaction aborted.\n");
	return response;
}

void CRadarData::Parser(unsigned short cmd)
{
	switch(cmd)
	{
		default:
		case 0:
		case 2:
		case 4:
		case 6:
		case 7:
		case 8:
		case 0xC:
		{
			break;
		}
		case 1:
		{
			unsigned char* pPayload = ResponseMsg.payload;
			CalibrationParameters_t * CalibrationParametersResponse = (CalibrationParameters_t *)ResponseStruct;
			memcpy(CalibrationParametersResponse->calibrationDate, pPayload + 1, *pPayload);
			CalibrationParametersResponse->calibrationDate[*pPayload] = 0;
			pPayload += *pPayload + 1;
			memcpy(CalibrationParametersResponse->calibrationExpirationDate, pPayload + 1, *pPayload);
			CalibrationParametersResponse->calibrationExpirationDate[*pPayload] = 0;
			pPayload += *pPayload + 1;
			memcpy(CalibrationParametersResponse->calibrationAuthority, pPayload + 1, *pPayload);
			CalibrationParametersResponse->calibrationAuthority[*pPayload] = 0;
			pPayload += *pPayload + 1;
			memcpy(CalibrationParametersResponse->calibrationCertification, pPayload + 1, *pPayload);
			CalibrationParametersResponse->calibrationCertification[*pPayload] = 0;
			pPayload += *pPayload + 1;
			memcpy(CalibrationParametersResponse->calibrationLocation, pPayload + 1, *pPayload);
			CalibrationParametersResponse->calibrationLocation[*pPayload] = 0;
			pPayload += *pPayload + 1;
			CalibrationParametersResponse->calibrationValid = *pPayload;
			pPayload += 1;
			CalibrationParametersResponse->radarCRC = *pPayload;
			pPayload += 1;
			CalibrationParametersResponse->radarCRC += *pPayload << 8;
			pPayload += 1;
			memcpy(CalibrationParametersResponse->radarApplicationVersion, pPayload + 1, *pPayload);
			CalibrationParametersResponse->radarApplicationVersion[*pPayload] = 0;
			pPayload += *pPayload + 1;
			memcpy(CalibrationParametersResponse->radarSystemVersion, pPayload + 1, *pPayload);
			CalibrationParametersResponse->radarSystemVersion[*pPayload] = 0;
			pPayload += *pPayload + 1;
			break;
		}
		case 3:
		{
			memcpy(ResponseStruct, ResponseMsg.payload, ResponseMsg.payloadLength);
			break;
		}
		case 5:
		{
			memcpy(ResponseStruct, ResponseMsg.payload, ResponseMsg.payloadLength);
			break;
		}
		case 9:
		{
			memcpy(ResponseStruct, ResponseMsg.payload, ResponseMsg.payloadLength);
			break;
		}
		case 0xA:
		{
            Targets_t * Targets = (Targets_t *)ResponseStruct;
            UINT16 intCRC;
            intCRC = GetCrc((unsigned char *)(&ResponseMsg), 0, 7);
            if(CheckCrc(ResponseMsg.payload, intCRC, 4))  // Check intermediate CRC
            {
                Targets->maxTargets = ResponseMsg.payload[0];
                Targets->numTargets = ResponseMsg.payload[1];
                for(int i = 0; i < Targets->numTargets; i++)
                {
                    int j = 4 + i * sizeof(Targets->RadarTargets[i]);
//                    for(int k = j; k < j + (int)(sizeof(Targets->RadarTargets[i])); k++)
//                    {
//                        printf("%2hx ", ResponseMsg.payload[k]);
//                    }
//                    printf("\n");
                    memcpy(&Targets->RadarTargets[i], &ResponseMsg.payload[j], sizeof(Targets->RadarTargets[i]));
//                    float speed = sqrtf(Targets->RadarTargets[i].xVelocity * Targets->RadarTargets[i].xVelocity +
//                                        Targets->RadarTargets[i].yVelocity * Targets->RadarTargets[i].yVelocity +
//                                        Targets->RadarTargets[i].zVelocity * Targets->RadarTargets[i].zVelocity);
//                    float distance = sqrtf( Targets->RadarTargets[i].xCoord * Targets->RadarTargets[i].xCoord +
//                                            Targets->RadarTargets[i].yCoord * Targets->RadarTargets[i].yCoord +
//                                            Targets->RadarTargets[i].zCoord * Targets->RadarTargets[i].zCoord);
//                    printf("Target %d j = %d speed = %8.1f distance = %8.1f\n", i, j, speed, distance);
                }
//                printf("\n");
            }
            else
            {
                printf("Bad intermediate CRC %04hx in message A response, numTargets = %hhd\n", intCRC, ResponseMsg.payload[1]);
                printf("ResponseMsg = ");
                for(int i = 0; i < 13; i++)
                {
                    unsigned char * myPtr = (unsigned char *)(&ResponseMsg);
                    printf("%02hhx ", myPtr[i]);
                }
                printf("\nResponseMsg.payload = ");
                for(int i = 0; i < 4; i++)
                {
                    unsigned char * myPtr = ResponseMsg.payload;
                    printf("%02hhx ", myPtr[i]);
                }
                printf("\n");
            }
            break;
		}
        case 0xB:
        {
			memcpy(ResponseStruct, ResponseMsg.payload, ResponseMsg.payloadLength);
			break;
		}
	}
}

bool CRadarData::Message0(CalibrationParameters_t * CalibrationParameters)
{
	UINT8 *pPayload = pollPayload;

	*pPayload = (UINT8)strlen(CalibrationParameters->calibrationDate);
	memcpy(pPayload+1, &CalibrationParameters->calibrationDate, *pPayload);
	pPayload += *(pPayload) + 1;
	*pPayload = (UINT8)strlen(CalibrationParameters->calibrationExpirationDate);
	memcpy(pPayload+1, &CalibrationParameters->calibrationExpirationDate, *pPayload);
	pPayload += *(pPayload) + 1;
	*pPayload = (UINT8)strlen(CalibrationParameters->calibrationAuthority);
	memcpy(pPayload+1, &CalibrationParameters->calibrationAuthority, *pPayload);
	pPayload += *(pPayload) + 1;
	*pPayload = (UINT8)strlen(CalibrationParameters->calibrationCertification);
	memcpy(pPayload+1, &CalibrationParameters->calibrationCertification, *pPayload);
	pPayload += *(pPayload) + 1;
	*pPayload = (UINT8)strlen(CalibrationParameters->calibrationLocation);
	memcpy(pPayload+1, &CalibrationParameters->calibrationLocation, *pPayload);
	pPayload += *(pPayload) + 1;

	PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
	PollMsg.command = 0;
	PollMsg.payloadLength = (UINT16)(pPayload - pollPayload);
	PollMsg.payload = (unsigned char *)&pollPayload;
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);
	PollMsg.crc = GetCrc((unsigned char *)(&pollPayload), PollMsg.crc, PollMsg.payloadLength);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
	ResponseStruct = CalibrationParameters;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

	return ret;
}

bool CRadarData::Message1(CalibrationParameters_t * CalibrationParametersResponse)
{
	PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
	PollMsg.command = 0x01;
    PollMsg.payloadLength = (UINT16)(0);
	PollMsg.payload = NULL;
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
	ResponseStruct = CalibrationParametersResponse;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

	return ret;
}

bool CRadarData::Message2(RadarParameters_t * RadarParameters)
{
	PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
	PollMsg.command = 0x02;
	PollMsg.payloadLength = (UINT16)(sizeof(*RadarParameters));
	PollMsg.payload = (unsigned char *)RadarParameters;
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);
	PollMsg.crc = GetCrc((unsigned char *)RadarParameters, PollMsg.crc, PollMsg.payloadLength);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
	ResponseStruct = RadarParameters;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

	return ret;
}

bool CRadarData::Message3(RadarParameters_t * RadarParametersResponse)
{
	PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
	PollMsg.command = 0x03;
	PollMsg.payloadLength = (UINT16)(0);
	PollMsg.payload = NULL;
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
	ResponseStruct = RadarParametersResponse;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

	return ret;
}

bool CRadarData::Message4(RadarMode_t * RadarMode)
{
	PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
	PollMsg.command = 0x04;
	PollMsg.payloadLength = (UINT16)(sizeof(*RadarMode));
	PollMsg.payload = (unsigned char *)RadarMode;
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);
	PollMsg.crc = GetCrc((unsigned char *)RadarMode, PollMsg.crc, PollMsg.payloadLength);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
	ResponseStruct = RadarMode;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

	return ret;
}

bool CRadarData::Message5(RadarMode_t * RadarModeResponse)
{
	PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
	PollMsg.command = 0x05;
	PollMsg.payloadLength = (UINT16)(0);
	PollMsg.payload = NULL;
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
	ResponseStruct = RadarModeResponse;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

	return ret;
}
//
// Note:  Messages 6, 7 & 8 (firmware update) skipped for now.
//

bool CRadarData::Message9(RadarOrientation_t * RadarOrientation)
{
	PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
	PollMsg.command = 0x09;
	PollMsg.payloadLength = (UINT16)(sizeof(*RadarOrientation));
	PollMsg.payload = (unsigned char *)RadarOrientation;
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);
	PollMsg.crc = GetCrc((unsigned char *)RadarOrientation, PollMsg.crc, PollMsg.payloadLength);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
	ResponseStruct = RadarOrientation;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

	return ret;
}

bool CRadarData::MessageA(Targets_t * Targets)
{
    static RadarTargetListPoll_t RadarTargetListPoll;
    RadarTargetListPoll.timeout = 1000;
    PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
	PollMsg.command = 0x0A;
    PollMsg.payloadLength = (UINT16)(sizeof(RadarTargetListPoll));
    PollMsg.payload = (unsigned char *)(&RadarTargetListPoll);
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);
    PollMsg.crc = GetCrc((unsigned char *)(&RadarTargetListPoll), PollMsg.crc, PollMsg.payloadLength);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
    ResponseStruct = Targets;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

    return ret;
}

bool CRadarData::MessageB(SelfTestResponse_t * SelfTestResponse)
{
	PollMsg.msgType = 0xE9;
	PollMsg.destAddr = 1;
	PollMsg.sourceAddr = 0;
    PollMsg.command = 0x0B;
	PollMsg.payloadLength = (UINT16)(0);
	PollMsg.payload = NULL;
	PollMsg.crc = GetCrc((unsigned char *)(&PollMsg), 0, 7);

	ResponseMsg.payload = (unsigned char *)&responsePayload;
	ResponseStruct = SelfTestResponse;

    bool ret = PollRadar(&PollMsg, &ResponseMsg);

	return ret;
}

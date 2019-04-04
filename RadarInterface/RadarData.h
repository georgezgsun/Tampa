#pragma once

#include <pthread.h>

//#define BIN_CAPTURE_FILE "/mnt/mmc/ipnc/capture_data.bin"
//#define CAPTURE_BIN

// This class encapsulates all communications with the Oculii radar sensor and
// implements the Oculii radar protocol

typedef enum
{
    init,       // 0
    idle,		// 1
    beginPoll,	// 2
    receiveMsgType,	// 3
    receiveHeader,	// 4
    receivePayload,	// 5
    receiveCRC,	// 6
    shutDown,	// 7
    invalidState	// 8
} state_type;

typedef enum
{
	getTgtIdle,
	listWait,
	getTargets,
	unlockWait
}
radarTargetsState_t;

class CRadarData
{
public:
	CRadarData(void);
	~CRadarData(void);
// RadarComm members:
    bool SetDevice(interface_type port);
    UINT OpenPort(char * lpctPort, char * lpctBaud);
    UINT ClosePort(void);
    UINT SendChar(UINT8 s);
    UINT SendBuf(UINT8 *s,  UINT count);
    UINT GetChar(void);
    UINT GetBuf(UINT8 *s,  UINT count);
    UINT FlushCom(void);
    bool BytesAvailable(void);
    UINT32 RunRadarDataStateMachine(config_type * pConfig);
    void RunRadarDataStates(void);
    bool PollRadar(poll_msg_t * pPollData, response_msg_t * pResponseMsg);
    void ShutdownRadar(void);
    void setCallbacks(CaptureFunction pCaptureFunction, PlaybackFunction pPlaybackFunction, TrashBinFunction pTrashBinFunction);
    CaptureFunction pCapture;
    PlaybackFunction pPlayback;
    TrashBinFunction pTrashbin;
    volatile bool portIsOpen;
    int responseStatus;
    CSerial SerialPort;
    volatile state_type messageState;

// RadarData members:
	int CheckResponse();
	void Parser(unsigned short cmd);
	void * ResponseStruct;
	bool Message0(CalibrationParameters_t * CalibrationParameters);
	bool Message1(CalibrationParameters_t * CalibrationParametersResponse);
	bool Message2(RadarParameters_t * RadarParameters);
	bool Message3(RadarParameters_t * RadarParametersResponse);
	bool Message4(RadarMode_t * RadarMode);
	bool Message5(RadarMode_t * RadarModeResponse);
	bool Message9(RadarOrientation_t * RadarOrientation);
    bool MessageA(Targets_t * Targets);
    bool MessageB(SelfTestResponse_t * SelfTestResponse);
	radarTargetsState_t radarTargetsState;
	UINT8 targetNum;
private:
#ifdef CAPTURE
    int capture_fd;
#endif
// RadarComm members:
    interface_type currentInterface;
    char Port[128];
    char Baud[128];
    state_type lastState;
    UINT rcvBuffIndex;
    config_type * pConfig;
    poll_msg_t * pPollMsg;
    response_msg_t * pResponseMsg;

// RadarData members:
    poll_msg_t PollMsg;
	response_msg_t ResponseMsg;
// Allocate space for longest possible message payloads.
// (Make sure additions to protocol don't create a longer message!)

    UINT8 responsePayload[sizeof(CalibrationParameters_t::calibrationDate) + 1 +
			      sizeof(CalibrationParameters_t::calibrationExpirationDate) + 1 +
			      sizeof(CalibrationParameters_t::calibrationAuthority) + 1 +
			      sizeof(CalibrationParameters_t::calibrationCertification) + 1 +
			      sizeof(CalibrationParameters_t::calibrationLocation) + 1 +
			      sizeof(CalibrationParameters_t::calibrationValid) +
			      sizeof(CalibrationParameters_t::radarCRC) +
			      sizeof(CalibrationParameters_t::radarApplicationVersion) + 1 +
			      sizeof(CalibrationParameters_t::radarSystemVersion) + 1];

//    UINT8 responsePayload[sizeof(Targets_t) + 2];
	UINT8     pollPayload[sizeof(CalibrationParameters_t::calibrationDate) + 1 +
			      sizeof(CalibrationParameters_t::calibrationExpirationDate) + 1 +
			      sizeof(CalibrationParameters_t::calibrationAuthority) + 1 +
			      sizeof(CalibrationParameters_t::calibrationCertification) + 1 +
			      sizeof(CalibrationParameters_t::calibrationLocation) + 1 +
			      sizeof(CalibrationParameters_t::calibrationValid) +
			      sizeof(CalibrationParameters_t::radarCRC) +
			      sizeof(CalibrationParameters_t::radarApplicationVersion) + 1 +
			      sizeof(CalibrationParameters_t::radarSystemVersion) + 1];
};


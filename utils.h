#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include "global.h"
#include "user_db.h"
#include "widgetKeyBoard.h"
#include "vkiline.h"
#include "session.h"
#include "lidarMsg.h"

#include <stdio.h>
#include <iostream>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "Fuel_Gauge_Buff.h"
#ifdef LIDARCAM
#include "Lidar_Buff.h"
#endif
#include "GPS_Buff.h"
#ifdef HH1
#include "RadarMemory.h"
#endif

#ifdef IS_TI_ARM
#include <linux/ti81xxfb.h>
#define TRANSPARENT  0x00000000
#endif

#define IN
#define OUT

typedef struct _APPRO_MSG_BUF
{
   long       des;
   int        src;
   int        cmd;
   char       data[APPRO_DATA_LEN];
   float      speed;
   float      range;
   char       filename[80];
} APPRO_MSG_BUF;

// Use for Camer tuning
typedef struct _ILLUMINATOR_
{
   int mIrcutIndex;
   int mIrisIndex;
   int m2aModeIndex;
   int mShutterIndex;
   int mGainIndex;
   int mBoxIndex;
}ILLUMINATOR;

class Utils
{
public:
    static Utils& get() {
        static Utils instance;
        return instance;
    }

    ~Utils();

    int initialization(int flag);

    int setVideoTransparency(bool b);

    int openUserDB();
    void closeUserDB();
    userDB *db() { return m_userDB; }
	void creatVKB();
    void deleteVKB();
    widgetKeyBoard *vkb() { return m_vkb; }

    void displaySession(struct Users *u);
    int createSession(struct Users *u);

    //handle transitive data
    int setTransitData(int type, IN DBStruct *);
    int getTransitData(int type, OUT DBStruct *);
    void clearTransitData();
    int location() { return m_locationIndex; }
    void setLocationIndex (int l) { m_locationIndex = l; }

    int connectGPSMEM();
    struct GPS_Buff *GPSBuf() {return m_GPSData; }
    
    int connectTILTMEM();
    struct Tilt_Buff *TILTBuf() { return tiltData; }
    
    int connectMAGMEM();
    struct Mag_Buff *MAGBuf() { return magData; }

	int connectRADARMEM();
    struct RadarMemory *RADARBuf() { return radarData; }

    int connectFG();
    struct Fuel_Gauge_Buff *FGBuf() {return m_FGData; }
    void updateFG( void );
#ifdef LIDARCAM
    struct Lidar_Buff *lidarDataBuf() {return m_lidarData; }
    int connectLidar();
    float lidarRange();
    float lidarSpeed();
    int sendCmdToCamera(int cmd, int value, int para1 = 1, int para2 = 5);
   int getConnected() { return m_lidarConnected; }
#else
    int sendCmdToCamera(int cmd, int value, int para1 = 1, int para2 = 5);
#endif
   int SndApproMsg(int cmd, char *value, char *filename);

   bool tiltSeen();
   Session *session() {return m_session;}
   
   int getLidarMemory( void );
   int setLidarMemory( lidarElements type, int *data, struct Message_Queue_Buff *recvBuff );
   int sendMbPacket( unsigned char cmd1, unsigned short length, unsigned char *data1, struct Message_Queue_Buff *recvBuff  );
   bool getPacket(  struct Message_Queue_Buff *, int size, int timeout = 20);
   bool getAlertPacket(unsigned char alertId, char *pData, unsigned short *pLen, int timeout = 20);

#ifdef HH1
    bool getRadarConfig(config_type* config);
    void setVolume(int vol);
#endif
   // Database related functions
    // Location info
    struct Location & getCurrentLoc(void) {return mCurrLoc;}
    void setCurrentLoc(struct Location const& newLocation) {mCurrLoc = newLocation;}

    // Evidence info
    int setEvidenceNum(unsigned int baseNum);
    bool getEvidenceNum(struct Evidence &);
    unsigned int getNewEvidenceNum(int index1 = 0);
    unsigned int getNewImageNum(int index1 = 0);
    unsigned int getNewTicketNum(int index1 = 0);
    // Some Configuration
    int setConfiguration(struct SysConfig & newConf);
    struct SysConfig & getConfiguration(int index1 = 0);
    // Administration menu
    int setAdmin(struct Administration & newAdmin);
    struct Administration & getAdmin(int index1 = 0);
    // Network menu
    int setNetwork(struct Network & newNet);
    struct Network & getNetwork(int index1 = 0);
    // Sensor data in DB
    int setSensorInDB(struct Sensor & newSensor);
    struct Sensor & getSensorFromDB(int index1 = 0);

    // Mode info
    int getMode(void) {return mMode;}
    void setMode(int newMode) {mMode = newMode;}

    // Maximum detected speed for this recording
    float getTopSpeed(void) {return mTopSpeed;}
    void setTopSpeed(float topSpeed) {mTopSpeed = topSpeed;}

    // Menuview Pushbutton info
    void setExitButton(QPushButton *pButton) {mpPbExit = pButton;}
    QPushButton *getExitButton(void) {return mpPbExit;}

	void enableWaterMark( void );

    bool mPassword;
    bool passwordEntered(void) {return mPassword;}
    void setPasswordStatus(bool newStatus) {mPassword = newStatus;}
#ifdef LIDARCAM
    void Send_Msg_To_PIC( int cmd );
#endif
    int getDisplayUnits( void );

   // Illuminator
   ILLUMINATOR mIlluminator;
   ILLUMINATOR & getIlluminator(void) {return mIlluminator;}
   void setIlluminator(ILLUMINATOR *illu);

   // Recording file
   QString & getRecordingFileName(void) {return mRecordingFileName;}
   int getEvidenceId(void) {return mEvidenceId;}
   void takePhoto(QString & fileName, int distance = 100);
   int getDistance(void) {return mDistance;}

private:

#ifdef IS_TI_ARM
    struct ti81xxfb_region_params regp;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    unsigned int* fb;
    int buffersize;
#endif
    //static Utils* m_instance;
    Utils();
    Utils(Utils const&);
    void operator= (Utils const&);

    int m_initFlags;
    QString m_fbDevice;
    //bool m_highResolution;
    int m_fullViewWidth;
    int m_fullViewHeight;
    int m_camViewWidth;
    int m_camViewHeight;
    int m_menuViewWidth;

    userDB *m_userDB;
    widgetKeyBoard *m_vkb;
    Session *m_session;

	// Make Message_buffer connection available
    int ID;

    //transit data and type
    int m_transitType;          //menu command define in global.h
    DBStruct *m_transitData;    //clear on read
    int m_locationIndex;          //used to save Location, CameraSetting

    // Location info
    Location mCurrLoc;  //current location in use
    // Some configuration info
    SysConfig mConfiguration;
    // Adminstration related configuration
    Administration mAdmin;
    // Network related configuration
    Network mNet;
    // Sensor data in DB
    Sensor mSensor;

    QPushButton *mpPbExit;

    // Mode info
    int mMode;
    float mTopSpeed;

    GPS_Buff *m_GPSData;
    Tilt_Buff *tiltData;
    Mag_Buff *magData;

    // Playback and Record Interface
    enum {MSG_TYPE_MSG1 = 1, APPRO_MSG_KEY = 0x54321};
    int mApproId;
    APPRO_MSG_BUF mMsgbuf;
    int MsgInit(int msgkey);

    RadarMemory *radarData;   

    QString mRecordingFileName;
    int mDistance;   // Vehicle distance when the photo was taken
    int mEvidenceId;
    Fuel_Gauge_Buff *m_FGData;
#ifdef LIDARCAM
    int m_lidarConnected;
    Lidar_Buff *m_lidarData;   
#endif
    unsigned short UART_CALCULATE_CHECKSUM(unsigned char * buffer, int size);
    bool responseF1Packet( struct Message_Queue_Buff *, int *, int *);
    bool responseF2Packet( struct Message_Queue_Buff *, int *, int *);
};
#endif // UTILS_H

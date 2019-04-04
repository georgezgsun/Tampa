#ifndef DB_TYPES_H
#define DB_TYPES_H

#include <QString>

enum DB_TABLES {
    TBL_FIRST,      //enum start mark
    TBL_QUICK_CODE,
    TBL_USERS,
    TBL_USER_LEVEL,
    TBL_USER_PERM,
    TBL_LOCATION,
    TBL_CAMERA_SETTING,
    TBL_CONFIGURATION,
    TBL_CALIB_DATA,
    TBL_EVIDENCE,
    TBL_ADMIN,
    TBL_NETWORK,
    TBL_SENSOR,
    //....
    TBL_LAST       //enum limit mark
};

enum DB_QUERY_FLAG {
    QRY_BY_MULTI_FIELDS,
    QRY_BY_KEY,
    QRY_ALL_ENTRIES,

    // the next two flags for getNextEntry call
   // QRY_GET_FIRST_ENTRY,
    //QRY_GET_NEXT_ENTRY
};

enum DB_TBL_ENTRIES {
    //TBL_USERS
    ETR_FIRST_NAME,
    ETR_LAST_NAME,
    ETR_LOGIN_NAME,      //may be used in multiple tables, as key or foreign key
    ETR_BADGE_NUMBER,
    ETR_USER_LEVEL,
    ETR_PASSWORD,

    //TBL_QUICK_CODE
    ETR_QUICK_CODE,
    ETR_LOGIN,          // = Login_Name

    //TBL_USER_LEVEL
    ETR_LEVEL_1,
    ETR_LEVEL_2,
    ETR_LEVEL_3,
    ETR_LEVEL_4,

    //TBL_USER_PERM
    ETR_PERM_LEVEL,
    ETR_PERM_ADMIN,
    ETR_PERM_UPLOAD,
    ETR_PERM_DELETE,

    //TBL_LOCATION
    ETR_LOC_INDEX,
    ETR_LOC_DECRIPTION,
    ETR_LOC_SPEED_LIMIT,
    ETR_LOC_CAPTURE_SPEED,
    ETR_LOC_CAPTURE_DISTANCE,
    ETR_LOC_ROAD_CONDITION,
    ETR_LOC_NUM_LANES,

    //TBL_CAMERA_SETTING (CAMS)
    ETR_CAMS_INDEX,
    ETR_CAMS_MODE,
    ETR_CAMS_SHUTTER,
    ETR_CAMS_EV,
    ETR_CAMS_GAIN,

    //....


};

// each table has a data structure to represent its data
struct Users {
    QString firstName;
    QString lastName;
    QString userLevel;      //TODO: integer?
    QString loginName;      //key
    QString password;
    QString bagdeNumber;    //TODO: integer?

    ~Users() {
        firstName.clear();
        lastName.clear();
        userLevel.clear();
        loginName.clear();
        password.clear();
        bagdeNumber.clear();
    }
};

struct QuickCode {
    QString Code;
    QString LoginName;

    ~QuickCode() {
        Code.clear();
        LoginName.clear();
    }
};

struct UserLevel {
    //QString LoginName;   // key, also a foreign key, not shown on GUI table, used internally
    QString Level_1;
    //...
};

struct UserPerm {
    //QString LoginName;      // foreign key
    QString PermLevel;      // key, value = [1,4]
    bool PermAdmin;          // yes = 1, No = 0
    //...
};

struct Location {
    int index;              //key
    QString description;    //the content
    QString speedLimit;
    QString captureSpeed;
    QString captureDistance;
    QString roadCondition;
    int     numberOfLanes;

    Location() {index = -1;}
    ~Location() {
        description.clear();
        speedLimit.clear();
        captureSpeed.clear();
        captureDistance.clear();
        roadCondition.clear();
    }
};

struct Evidence {
    int index;              //key
    unsigned int evidNum;
    unsigned int imageNum;
    unsigned int ticketNum;

    Evidence() {index = -1;}
    ~Evidence() {}
};

struct SysConfig {
    int index;              //key
#ifdef HH1
    unsigned int speedTenths;
    unsigned int rangeTenths;
    unsigned int autoTrigger;
    unsigned int audioAlert;
    unsigned int targetSort;
    unsigned int units;
    unsigned int direction;
    float        frequency;
    float        bandwidth;
    unsigned int radarPower;
    unsigned int sensitivity;
    unsigned int imageSpacing;
    unsigned int volume;
    unsigned int brightness;
    QString serialNumber;
#endif
    unsigned int backlightOff;
    unsigned int powerOff;
    QString pictureDist;
    unsigned int resolution;
    unsigned int imagesPerFile;
    unsigned int preBuf;
    unsigned int postBuf;
    float radarH;
    float distFrR;
    float targetH;
    unsigned int lensFocal;

    SysConfig() {index = -1;}
    ~SysConfig() {}
};

struct Administration {
    int index;              //key
    bool    userLogin;
    bool    compression;
    bool    autoDelete;
    bool    encryption;
    QString password;
    int     dateFormat;
    int     language;
    QString expiration;
    QString authority;
    QString certification;
    QString serviceDate;
    QString metaData1;  // Line 1
    QString metaData2;  // Line 2
    int     metaPosition;
    int     usrAccAdmin;
    int     usrAccTransfer;
    int     usrAccDelete;

    Administration() {index = -1;}
    ~Administration() {
        password.clear();
    }
};

struct Network {
    int index;              //key
    unsigned int discovery;
    unsigned int ipAddr;
    unsigned int subnetMask;
    unsigned int gateway;
    unsigned int dns1;
    unsigned int dns2;

    Network() {index = -1;}
    ~Network() {}
};

struct CameraSetting {
    int index;          //key, correlate to Location.index
    QString mode;
    QString shutter;
    QString gain;
    QString ev;

    CameraSetting() { index = -1; }
    ~CameraSetting() {
        mode.clear();
        shutter.clear();
        gain.clear();
        ev.clear();
    }
};

#ifdef HH1
struct Sensor {
    int index;              //key
    QString i2cDevice;
    float   magXmax;
    float   magXmin;
    float   magYmax;
    float   magYmin;
    float   magZmax;
    float   magZmin;
    float   magThetaX;
    float   magThetaY;
    float   magThetaZ;
    float   accXmax;
    float   accXmin;
    float   accYmax;
    float   accYmin;
    float   accZmax;
    float   accZmin;
    float   accThetaX;
    float   accThetaY;
    float   accThetaZ;
    Sensor() {index = -1;}
    ~Sensor() {}
};
#endif

union DBStruct {
    struct Users;
    struct QuickCode;
    struct UserLevel;
    struct UserPerm;
    struct Location;
    struct CameraSetting;
    struct Sensor;
};


#endif // DB_TYPES_H

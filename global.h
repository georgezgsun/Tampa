#ifndef GLOBAL_H
#define GLOBAL_H

#define UNUSED(x) (void)(x)

#define SLEEPCOLDFIREWHENTILTED 60 // Sleep the Coldfire when tilted after 60 seconds
#define SLEEPCOLDFIRE 150 // Sleep the Coldfire after 150 seconds

#define ZOOM_MIN    1
#define ZOOM_MAX    6

//LCD view size
#define FULL_WITH       480
#define FULL_HEIGHT     272

// hex fields for CMD definitions
/*
 * f f f f f f f f
 * f                    global flags (sub-menu, save, cancel)
 *   f                 Main Menu
 *     f f f f          sub-menus, 4 levels, start at MSB, next bits for its sub-menus
 *             f        un-defined
 *               f      toggle fields in leaf menus
 */

//command defs of MAIN MENU
#define CMD_MAIN_LOC_SETUP      0x01000000
#define CMD_MAIN_LIDAR_SETUP    0x02000000
#define CMD_MAIN_FILE_MGR       0x03000000
#define CMD_MAIN_USER_MGR       0x04000000
#define CMD_MAIN_MODE_SEL       0x05000000
#define CMD_MAIN_SYS_OPT        0x06000000
#define CMD_MAIN_DEV_INFO       0x07000000
#define CMD_MAIN_PRT_TICKET     0x08000000
#define CMD_MAIN_SELF_TEST      0x09000000
#define CMD_MAIN_LOGOUT         0x0A000000
//global flags
#define CMD_SUB                 0x10000000  // sub cmd bit
#define CMD_SAVE                0x30000000  // for all "save" on menus
#define CMD_CANCEL              0x50000000  // for all "cancel" on menus
#define CMD_NONE                0x00000000

// sub-commands of LOC_SETUP   (0x01000000)
#define CMD_LOC_LOAD            0x11100000
#define CMD_LOC_CAMERA          0x11200000
#define CMD_LOC_SAVE            0x11300000
//#define CMD_LOC_KEY             0x11000001  // location key
#define CMD_LOC_DESC            0x11000002  // location description
#define CMD_SPD_LMT             0x11000003
#define CMD_LOC_ENV             0x11000004
#define CMD_NUM_LANES           0x11000005
#define CMD_CAPT_SPD            0x11000006
#define CMD_CAPT_DIST           0x11000007

//sub-command LOC_LOAD/SAVE or LOAD
//      CMD_LOC_LOAD            0x11100000
#define CMD_LOC_LIST            0x11100001

// sub-commands of LIDAR_SETUP (0x02000000)
#define CMD_SPEED_TENTH         0x12000001
#define CMD_RANGE_TENTH         0x12000002
#define CMD_ANTI_JAM            0x12000003
#define CMD_UNIT                0x12000004
#define CMD_TILT                0x12000005
#define CMD_BACKLIGHT_OFF       0x12000006
#define CMD_POWER_OFF           0x12000007
#define CMD_AUTO_TRIGGER        0x12000008
#define CMD_SORT_TARGETS        0x12000009

// sub-commands of MODE_SEL    (0x05000000)
#define CMD_MODE_RANGE          0x15000001
#define CMD_MODE_AUTO           0x15100000
#define CMD_MODE_ZONE           0x15200000
#define CMD_MODE_INCL           0x15000002
#define CMD_MODE_SS             0x15000003
#define CMD_MODE_FTC            0x15300000
#define CMD_MODE_LOG_CHASE      0x15000004
#define CMD_MODE_LOG_STATS      0x15000005
#define CMD_RADAR_H             0x15000006
#define CMD_DIST_FRR            0x15000007
#define CMD_TARGET_H            0x15000008
#define CMD_LENSFOCAL           0x15000009

//sub-cmds of USER_MGR         (0x04000000)
#define CMD_ADD_USER            0x14100000
#define CMD_EDIT_USER           0x14200000
#define CMD_DEL_USER            0x14300000
#define CMD_LIST_USERS          0x14400000

//sub-commands of USR_MGR / USER_ADD/EDIT
//      CMD_ADD_USER           (0x14100000)
#define CMD_FIRST_NAME          0x14100001
#define CMD_LAST_NAME           0x14100002
#define CMD_BADGE_NUM           0x14100003
#define CMD_USER_LEVEL          0x14100004
#define CMD_LOGINNAME           0x14100005
#define CMD_PASSWORD            0x14100006

//sub-cmds of FILE_MGR
//      CMD_MAIN_FILE_MGR       0x03000000
#define CMD_PLAYBACK            0x13100000
#define CMD_UPLOAD              0x13200000
#define CMD_PRINT               0x13300000
#define CMD_DEL_FILE            0x13000002
#define CMD_SEL_NEW             0x13000003
#define CMD_SEL_ALL             0x13000004
#define CMD_LIST_FILES          0x13000005

// sub-cmds of printTicket
#define CMD_TICKET_PLAYBACK     0x13400000
#define CMD_TICKET_UPLOAD       0x13500000
// sub-cmd of ticket view
#define CMD_TARGET1             0x13400001
#define CMD_TARGET2             0x13500002
#define CMD_TARGET3             0x13400003
#define CMD_TARGET4             0x13500004

//sub-cmd FILE_MGR/UPLOAD_MGR
//      CMD_UPLOAD              0x13200000
#define CMD_UPLOAD_WIFI         0x13210000
#define CMD_UPLOAD_USB          0x13220000
#define CMD_UPLOAD_ENET         0x13230000
#define CMD_UPLOAD_FLIST        0x13240000

// sub-commands of SYS_OPT     (0x06000000)
#define CMD_PWR_OFF             0x16000001
#define CMD_BCKLT_OFF           0x16000002
#define CMD_CAMERA              0x16100000
#define CMD_VIDEO               0x16200000
#define CMD_WIFI                0x16300000
#define CMD_ADMIN               0x16400000
#define CMD_BLUETOOTH           0x16500000
#define CMD_ETHERNET            0x16600000

// sub-commands of SYS_OPT/CAMERA_SETTINGS
//      CMD_CAMERA              0x16100000
#define CMD_ZOOM                0x16100001
#define CMD_FOCUS               0x16100002
#define CMD_FOCUS_MANUAL        0x16100003
#define CMD_SHUTTER             0x16100004
#define CMD_COLOR               0x16100005
#define CMD_IRIS                0x16100006
#define CMD_GAIN                0x16100007
#define CMD_RECORD              0x16100008
#define CMD_STOPRECORD          0x16100009
#define CMD_MODE                0x1610000A
#define CMD_EV                  0x1610000B


// Playback and photo related
#define CMD_STOPPLAY            0x16110000
#define CMD_PAUSEPLAY           0x16120000
#define CMD_RESUMEPLAY          0x16130000
#define CMD_FFPLAY              0x16140000
#define CMD_FRPLAY              0x16150000
#define CMD_SNAPSHOT            0x16160000
#define CMD_DISPLAY_PHOTO       0x16170000
#define CMD_REMOVE_PHOTO        0x16180000

// sub-commands of SYS_OPT/VIDEO_SETTINGS
//      CMD_VIDEO               0x16200000
#define CMD_CAMERACONFIG        0x16210000

#define CMD_IMAGES              0x16200002
#define CMD_PICTURDISTANCE      0x16200003
#define CMD_PREBUFFER           0x16200004
#define CMD_POSTBUFFER          0x16200005
//#define CMD_ILLIMINATOR         0x16200006
#define CMD_ILLIMINATOR         0x16200010

// sub-commands of SYS_OPT/WIFI_SETTINGS
//      CMD_WIFI                0x16300000
#define CMD_WIFI_SCAN           0x16300001
#define CMD_WIFI_LIST           0x16300002
#define CMD_WIFI_USERNAME       0x16300003
#define CMD_WIFI_PASSWD         0x16300004

// sub-commands of SYS_OPT/BT_SETTINGS
//      CMD_BLUETOOTH           0x16500000
#define CMD_BT_SCANNING         0x16500001
#define CMD_BT_LIST             0x16500002
#define CMD_BT_PASSWD           0x16500003

// sub-command of SYS_OPT/ETHERNET_
//      CMD_ETHERNET            0x16600000
#define CMD_ENET_SETTINGS       0x16600001
#define CMD_ENET_ADDRESS        0x16600002
#define CMD_ENET_SUBNET         0x16600003
#define CMD_ENET_GATEWAY        0x16600004
#define CMD_ENET_DNS1           0x16600005
#define CMD_ENET_DNS2           0x16600006

// sub-command of SYS_OPT/ADMIN
//      CMD_ADMIN               0x16400000
#define CMD_SERVICE             0x16410000
#define CMD_FACTORY             0x16420000
#ifndef HH1
#define CMD_METADATA            0x16430000
#endif
#define CMD_UPGRADE_SW          0x16440000
#define CMD_SECURITY_OPTIONS    0x16450000
#define CMD_USER_ACCESS         0x16460000
#define CMD_SAVE_SETTINGS       0x16470000
#define CMD_LOAD_SETTINGS       0x16480000
#define CMD_LOG_FILE            0x16490000

// sub-command of SYS_OPT/ADMIN/SERVICE
//      CMD_SERVICE             0x16410000
#define CMD_SRV_REF_CLK         0x16414000
#define CMD_SRV_SERV_OPTS       0x16411000
#define CMD_SRV_LIDAR_SETUP     0x16412000
#define CMD_SRV_CALIB_DATA      0x16413000

// sub-command of SYS_OPT/ADMIN/SERVICE/SERV_OPTS
//      CMD_SRV_SERV_OPTS       0x16411000
#define CMD_SRV_SPD_TRIG_ON     0x16411001
#define CMD_SRV_SPD_TRIG_YES    0x16411002
#define CMD_SRV_ZONE_MODE_ON    0x16411003
#define CMD_SRV_ZONE_MODE_YES   0x16411004
#define CMD_SRV_TD_MODE_ON      0x16411005  //TD: TIME/DIST MODE
#define CMD_SRV_TD_MODE_YES     0x16411006

// sub-command of SYS_OPT/ADMIN/FACTORY
//      CMD_FACTORY             0x16420000
#define CMD_FAC_REF_CLK         0x16420001
#define CMD_FAC_SERV_OPTS       0x16421000
#define CMD_FAC_DEFAULTS        0x16422000
#define CMD_FAC_CALIB_DATA      0x16423000
#define CMD_FAC_ALIGNMENT       0x16424000
#define CMD_FAC_CACLMAG3110     0x16425000
#define CMD_FAC_RADAR_PARAMS    0x16426000
#define CMD_FAC_TILT_PARAMS     0x16427000
#define CMD_FAC_START           0x16428000
#define CMD_FAC_STOP     		  0x16429000

// sub-command of SYS_OPT/ADMIN/FACTORY/SERV_OPTS
//      CMD_FAC_SERV_OPTS       0x16421000
#define CMD_FAC_SPD_TRIG_ON     0x16421001
#define CMD_FAC_SPD_TRIG_YES    0x16421002
#define CMD_FAC_ZONE_MODE_ON    0x16421003
#define CMD_FAC_ZONE_MODE_YES   0x16421004
#define CMD_FAC_TD_MODE_ON      0x16421005  //TD: TIME/DIST MODE
#define CMD_FAC_TD_MODE_YES     0x16421006

// sub-command of SYS_OPT/ADMIN/FACTORY/CALIB_DATA
//      CMD_FAC_CALIB_DATA      0x16423000
#define CMD_CALIB_EXPIRE        0x16423001
#define CMD_CALIB_AUTHORITY     0x16423002
#define CMD_CALIB_CERT          0x16423003
#define CMD_CALIB_DATE          0x16423004

// sub-command of SYS_OPT/ADMIN/FACTORY/RADAR_PARAMS
//      CMD_FAC_RADAR_PARAMS    0x16426000
#define CMD_FAC_RADAR_FREQ      0x16426001
#define CMD_FAC_RADAR_BANDWIDTH 0x16426002

// sub-command of SYS_OPT/ADMIN/FACTORY/TILT_PARAMS
//      CMD_FAC_TILT_PARAMS     0x16427000
#define CMD_FAC_TILT_MAG_XMAX   0x16427001
#define CMD_FAC_TILT_MAG_XMIN   0x16427002
#define CMD_FAC_TILT_MAG_YMAX   0x16427003
#define CMD_FAC_TILT_MAG_YMIN   0x16427004
#define CMD_FAC_TILT_MAG_ZMAX   0x16427005
#define CMD_FAC_TILT_MAG_ZMIN   0x16427006
#define CMD_FAC_TILT_MAG_THETAX 0x16427007
#define CMD_FAC_TILT_MAG_THETAY 0x16427008
#define CMD_FAC_TILT_MAG_THETAZ 0x16427009
#define CMD_FAC_TILT_ACC_XMAX   0x1642700A
#define CMD_FAC_TILT_ACC_XMIN   0x1642700B
#define CMD_FAC_TILT_ACC_YMAX   0x1642700C
#define CMD_FAC_TILT_ACC_YMIN   0x1642700D
#define CMD_FAC_TILT_ACC_ZMAX   0x1642700E
#define CMD_FAC_TILT_ACC_ZMIN   0x1642700F
#define CMD_FAC_TILT_ACC_THETAX 0x16427010
#define CMD_FAC_TILT_ACC_THETAY 0x16427011
#define CMD_FAC_TILT_ACC_THETAZ 0x16427012

#ifndef HH1
// sub-command of ADMIN/CMD_METADATA
//      CMD_METADATA             0x16430000
#define CMD_METADATA_DEVICE_ID   0x16431001
#define CMD_METADATA_USER_ID     0x16431002
#define CMD_METADATA_RECORD_NUM  0x16431003
#define CMD_METADATA_SPEED_LIMIT 0x16431004
#define CMD_METADATA_SPEED       0x16431005
#define CMD_METADATA_DISTANCE    0x16431006
#define CMD_METADATA_LOCATION    0x16431007
#define CMD_METADATA_METALINE1   0x16431008
#define CMD_METADATA_METALINE2   0x16431009
#define CMD_METADATA_LINE1       0x1643100A
#define CMD_METADATA_TOP_BOTTOM  0x1643100B
#endif

// sub-command of ADMIN/SECURITY
//      CMD_SECURITY_OPTIONS     0x16450000
#define CMD_SECURITY_LOGIN       0x16451001
#define CMD_SECURITY_COMPRESSION 0x16451002
#define CMD_SECURITY_ENCRYPTION  0x16451003
#define CMD_SECURITY_AUTODELETE  0x16451004
#define CMD_SECURITY_COUNTERS    0x16451005
#define CMD_SECURITY_PASSWORD    0x16451006
#define CMD_SECURITY_DATEFORMAT  0x16451007
#define CMD_SECURITY_LANGUAGE    0x16451008

#define CMD_USER_ACCESS_ADMIN    0x16460001
#define CMD_USER_ACCESS_TRANSFER 0x16460002
#define CMD_USER_ACCESS_DELETE   0x16460003

// sub-command of ADMIN/SECURITY
//      CMD_LOG_FILE            0x16490000
#define CMD_LOG_FILE_CLEAR      0x16491000

// Misc. command, not related to a menu
#define CMD_TIMESTAMP            0x20000001
#define CMD_DISPLAY_WATERMARK    0x20000002
#define CMD_DISPLAY_WATERMARK2   0x20000003
#define CMD_REMOVE_WATERMARK     0x20000004
#define CMD_RUN_FOCUS            0x20000005
#define CMD_CAMERA_BOX           0x20000006
#define CMD_CAMERA_IRCUT         0x20000007
#define CMD_CAMERA_2A_MODE       0x20000008

// sub-commands of DEV_INFO
//#define CMD_MAIN_DEV_INFO       0x10400000


enum states {
  STATE_INIT,     //0
  STATE_START,  //1
  STATE_LOGIN,  //2
  STATE_TOP_VIEW, //3
  STATE_MAIN_MENU,  //4
  STATE_LOC_SETUP_MENU,   //5 //sub-menus start
  STATE_LIDAR_SETUP_MENU,
  STATE_FILE_MGR_MENU,
  STATE_USER_MGR_MENU,
  STATE_MODE_SEL_MENU,
  STATE_SYS_OPT_MENU,     //10
  STATE_DEV_INFO_MENU,
  STATE_SELF_TEST_MENU,
  STATE_PRNT_TICKET_MENU,  //sub-menus end
  
  //sysOpt states
  STATE_VIDEO_SETUP,
  STATE_CAMERA_SETUP,	//15
  STATE_ADMIN,
  STATE_WIFI_SETUP,
  STATE_BT_SETUP,
  STATE_ENET_SETUP,
  
  //userMgr states
  STATE_ADD_USER,	//20
  STATE_EDIT_USER,
  
  STATE_SUB_MENU1,        //use sub menus instead of screen name ???
  STATE_SUB_MENU2,
  STATE_SUB_MENU3,
  STATE_SUB_MENU4,	//25
  STATE_LEAF_MENU,  //26      //last menu in menu chain
  STATE_ACC_MENU,    //27      //accessorate menus, not fit in menu view
  STATE_OPERATING, //28
  STATE_OPERATING_MENU, //29
  STATE_OPERATING_SETUP, //30
  STATE_CALIBRATEMAG3110, //31
  STATE_CAMERACONFIG //32
};

// Playback and Record Interface
#define APPRO_RECORD    1
#define APPRO_SNAPSHOT  2
#define APPRO_ZOOMIN    3
#define APPRO_ZOOMOUT   4
#define APPRO_RUNFOCUS  5
#define APPRO_FOCUSMODE 6
#define APPRO_TIMESTAMP 7
#define APPRO_PHOTO     8
#define APPRO_PLAYBACK  9
#define APPRO_AVONOFF   13
#define APPRO_LIDAR     14
#define APPRO_WATERMARK 15
#define APPRO_BOX       16
#define APPRO_IRCUT     17
#define APPRO_IRIS      18
#define APPRO_2A        19
#define APPRO_SHUTTER   20
#define APPRO_GAIN      21
#define APPRO_DTFMT     22

#define WATERMARK_TOPLEFT     0
#define WATERMARK_BOTTOMLEFT  1
#define WATERMARK_TOPRIGHT    2
#define WATERMARK_BOTTOMRIGHT 3

#define FRAMESPERSECOND 15
#define MAX_RECORDING_SECS  25
#define MAX_WATERMARK_LEN   47

#define APPRO_DATA_LEN  8

//
// command line options and project defines
//
#define PROJ_HH1                0x80000000
#define PROJ_LIDAR              0x40000000
#define RESOLUTION_HDMI         0x00000001

#ifdef HH1
#define APP_NAME                "HH1 "
#define APP_VER                 "0.1"
#else
#define APP_NAME                "LidarCam II "
#define APP_VER                 "0.4.7"
#endif
enum CMLParseResult //command line parser
{
    CMLOk,
    CMLError,
    CMLVersionRequested,
    CMLHelpRequested
};


//
// global color set for style sheet
//
#define NEAR_BLACK          0x010101

#define NEAR_BLACK_RGB  rgb(10, 10, 10)
#define NEAR_BLACK_CN   "#0a0a0a"    // color name



#define NEAR_BLACK_TEXT_SS  (QStringLiteral("QMenu, QWidget, QLabel, QLineEdit, QCheckBox, QPushButton, vkILine, QMessageBox, QToolButton " \
                                            ":enabled { color: rgb(10,10,10);}" \
                                            ":disabled { color: rgb(128, 128,128); }" \
                                            ":focus{ background-color: #00ccff;}" \
                                            ":focus{ outline: none; }" \
                                            " QWidget{ font: Bold 11pt DejaVu Sans; } " \
									))

extern bool triggerPulled;
extern bool coldFireSleep;
extern bool m_recording;
extern QString currentPlaybackFileName;
extern int activity;
extern bool inactive;
extern bool backLightOn;
extern qint64 inactiveStart;
extern qint64 currentSeconds;

#define MAG_XMAX "Mag_Xmax"
#define MAG_XMIN "Mag_Xmin"
#define MAG_YMAX "Mag_Ymax"
#define MAG_YMIN "Mag_Ymin"
#define MAG_ZMAX "Mag_Zmax"
#define MAG_ZMIN "Mag_Zmin"
#define MAG_THETA_X "Mag_Theta_X"
#define MAG_THETA_Y "Mag_Theta_Y"
#define MAG_THETA_Z "Mag_Theta_Z"
#define ACC_XMAX "Acc_Xmax"
#define ACC_XMIN "Acc_Xmin"
#define ACC_YMAX "Acc_Ymax"
#define ACC_YMIN "Acc_Ymin"
#define ACC_ZMAX "Acc_Zmax"
#define ACC_ZMIN "Acc_Zmin"
#define ACC_THETA_X "Acc_Theta_X"
#define ACC_THETA_Y "Acc_Theta_Y"
#define ACC_THETA_Z "Acc_Theta_Z"
#define SERIALNUMBER "serialNumber"

#define JPGSIZE_X 4000
#define JPGSIZE_Y 3000
  
#define DISPLAYSCREENSIZE_X 384 //378
#define DISPLAYSCREENSIZE_Y 272 //270

#endif // GLOBAL_H

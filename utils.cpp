#include "utils.h" 
#include <iostream>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <clocale>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "Message_Queue_Struct.h"
#include "Message_Queue_Keys.h"
#include "Msg_Def.h"
#include "Shared_Memory_Keys.h"
#include "Lidar_Buff.h"
#include "Tilt_Buff.h"
#include "Mag_Buff.h"
#ifdef HH1
#include "RadarMemory.h"
#endif
#include <QApplication>
#include <QCryptographicHash>
#include "PicMsg.h"
#include "ColdFireMsg.h"
#include "ColdFireCommands.h"
#include "lidarMsg.h"
#include "debug.h"
#include "back_ground.h"
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QThread>
#include <QSoundEffect>

using namespace std;

Utils::Utils()
{
    m_initFlags = 0;
    m_fbDevice = QString("/dev/fb1");
//    Set both displays to fb0 for now (fb1 doesn't yet exist)
//    m_fbDevice = QString("/dev/fb0");
    m_fullViewWidth = 480;
    m_fullViewHeight = 272;
    m_camViewWidth = 380;
    m_camViewHeight = 272;
    m_menuViewWidth = 380;

    m_userDB = NULL;
    m_vkb = NULL;

    m_transitType = CMD_NONE;
    m_transitData = NULL;
    m_locationIndex = 0;

   // Playback and Record Interface
   mApproId = MsgInit(APPRO_MSG_KEY);

   // ILLUMINATOR not initialized yet
   mIlluminator.mBoxIndex = -1;
}

Utils::~Utils()
{
    qDebug() << "destruct Utils";
    if (m_vkb)
        delete m_vkb;
    if (m_session)
        delete m_session;
}

int Utils::initialization(int flag)
{
    if (flag & RESOLUTION_HDMI) {
        m_initFlags = flag;
        m_fbDevice = QString("/dev/fb1");

        //TODO: detect the screen size here to make sure it holds the HDMI resolution
        //otherwise return and use default setting

        m_fullViewWidth = 1920;
        m_fullViewHeight = 1080;
        m_camViewWidth = 1440;
        m_camViewHeight = 1080;
        m_menuViewWidth = 1520;
    }

#ifdef LIDARCAM
    m_lidarData = NULL;
    m_lidarConnected = 0;
#endif
    tiltData = NULL;
#ifdef HH1
    radarData = NULL;
#endif
	
    return 0;
}

int Utils::setVideoTransparency(bool b )
{
  UNUSED(b);
  DEBUG();
  
#ifdef HH1
#ifdef IS_TI_ARM
  // Real target build
  
  int fd = open (m_fbDevice.toLatin1().data(), O_RDWR);
  if (fd == -1) {
    DEBUG() << "File name " << m_fbDevice.toLatin1().data();
    perror("failed to open display device ");
    QApplication::exit(-1);
  }
  
  if (ioctl(fd, TIFB_GET_PARAMS, &regp) < 0) {
    perror("TIFB_GET_PARAMS\n");
    close(fd);
    QApplication::exit(-2);
  }
  
  /* Set up the framebuffer mode but wait till framebuffer is filled to activate */
  /* Enable the Transparency Keying */
  regp.transen = TI81XXFB_FEATURE_ENABLE;
  /* Set the Transparency Keying Mask type */
  regp.transtype = TI81XXFB_TRANSP_LSBMASK_NO;
  /* Set the Transparency Color */
  regp.transcolor = 0x000000;
  /* Disable / Enable Alpha Blending */
  if (b == false)
    regp.blendtype = TI81XXFB_BLENDING_NO;
  else
    regp.blendtype = TI81XXFB_BLENDING_PIXEL;
  
  int ret = ioctl(fd, FBIOGET_FSCREENINFO, &fix);
  if (ret < 0) {
    printf("FBIOGET_FSCREENINFO\n");
    close(fd);
    QApplication::exit(-3);
  }
  
  //    printf("Line length = %d\n", fix.line_length);
  //    printf("Physical Address = %lx\n", fix.smem_start);
  //    printf("Buffer Length = %d\n", fix.smem_len);
  
  ret = ioctl(fd, FBIOGET_VSCREENINFO, &var);
  if (ret < 0) {
    printf("FBIOGET_VSCREENINFO\n");
    close(fd);
    QApplication::exit(-4);
  }
  
  var.xres = m_fullViewWidth;
  var.yres = m_fullViewHeight;
  
  ret = ioctl(fd, FBIOPUT_VSCREENINFO, &var);
  if (ret < 0) {
    printf("FBIOPUT_VSCREENINFO\n");
    close(fd);
    QApplication::exit(-5);
  }
  
  //    printf("Resolution = %dx%d\n", var.xres, var.yres);
  //    printf("bits per pixel = %d\n", var.bits_per_pixel);
  
  buffersize = fix.line_length * var.yres;
  //    printf("Buffersize = %d\n", buffersize);
  fb = (unsigned int*)(mmap(NULL, buffersize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  close(fd); // Close to prevent showing the following buffer fill until it is done

  unsigned int i, j, k;

  for (j=0; j<var.yres; j++) {
    unsigned int *fbl = fb + j * fix.line_length/4;
    for (i=0; i<var.xres; i++) {
      k = j * var.xres + i;
      if(i < (3 * var.xres) / 4) fbl[i] = TRANSPARENT;
      else fbl[i] = NEAR_BLACK;
    }
  }
  fd = open (m_fbDevice.toLatin1().data(), O_RDWR);
  //    fd = open ("/dev/fb0", O_RDWR);
  if (ioctl(fd, TIFB_SET_PARAMS, &regp) < 0) {
    perror ("TIFB_SET_PARAMS.\n");
    close(fd);
    QApplication::exit(-6);
  }
  close(fd);
#endif  // IS_TI_ARM
#endif // HH1
    return 0;
}

int Utils::openUserDB()
{
  int flag = 0;
  m_userDB = new userDB(&flag);
  if (flag) {
	m_userDB = NULL;
	return -1;
  }
  //  printf("%s(%d) m_userDB 0x%x(%d)\n\r", __FILE__, __LINE__, m_userDB, m_userDB);
  return 0;
}

void Utils::closeUserDB()
{
    if (m_userDB) {
        delete m_userDB;
        m_userDB = NULL;
    }

}

void Utils::creatVKB()
{
    this->m_vkb = new widgetKeyBoard(true);
    this->m_vkb->setZoomFacility(true);
    this->m_vkb->enableSwitchingEcho(false);
    this->m_vkb->setFixedHeight(FULL_HEIGHT);
    this->m_vkb->createKeyboard(); // only create keyboard
}

void Utils::deleteVKB()
{
    Q_ASSERT(m_vkb);
    if (m_vkb->isVisible()) {
        m_vkb->hide(true);
    }
    delete m_vkb;
    m_vkb = NULL;
}

void Utils::displaySession(Users *u)
{
  qDebug() << "firstName " << u->firstName;
  qDebug() << "lastName " << u->lastName;
  qDebug() << "userLevel " << u->userLevel;  
  qDebug() << "loginName " << u->loginName;
  qDebug() << "password " << u->password;
  qDebug() << "bagdeNumber " << u->bagdeNumber;
}
int Utils::createSession(Users *u)
{
    m_session = new Session(u);
    if (!m_session) {
        return -1;
    }
	//	displaySession( u );
    return 0;
}

int Utils::setTransitData(int type, DBStruct *data)
{
    if (m_transitType != CMD_NONE) {
        printf("m_transitState, %X, is not CMD_NONE.\n",  m_transitType);
    }
    if (m_transitData) {
        qWarning() << "m_transData is not NULL";
        clearTransitData();
    }

    switch (type) {
    case CMD_EDIT_USER:
    {
        struct Users *origU = (struct Users *) data;
        struct Users *u = new struct Users;
        u->loginName = origU->loginName;
        u->firstName = origU->firstName;
        u->lastName = origU->lastName;
        u->bagdeNumber = origU->bagdeNumber;
        u->password = origU->password;
        u->userLevel = origU->userLevel;
        m_transitData = (DBStruct *)u;
        m_transitType = type;
    }
        break;

    case CMD_LOC_LOAD:
    case CMD_LOC_SAVE:
    {
        struct Location *origL = (struct Location *) data;
        struct Location *l = new struct Location;
        l->index = origL->index;
        l->description = origL->description;
        l->speedLimit = origL->speedLimit;
        l->captureSpeed = origL->captureSpeed;
        l->roadCondition = origL->roadCondition;
        l->numberLanes = origL->numberLanes;
        m_transitData = (DBStruct *)l;
        m_transitType = type;
    }
        break;

    default:
        qCritical() << "Invalid transitive state " << type;
        return -1;
    }

    return 0;
}

int Utils::getTransitData(int type, DBStruct *data)
{
    int retv = 0;
    if (type != m_transitType) {
        if (m_transitType == CMD_NONE)
            return 1;
        printf("Transitive state mismatch!. %X (%X)\n", type, m_transitType);
        return -1;
    }
    Q_ASSERT(m_transitData);

    switch (type) {
    case CMD_EDIT_USER:
    {
        struct Users * u = (struct Users *)data;
        u->bagdeNumber = ((struct Users *)(m_transitData))->bagdeNumber;
        u->firstName = ((struct Users *)(m_transitData))->firstName;
        u->lastName = ((struct Users *)(m_transitData))->lastName;
        u->loginName = ((struct Users *)(m_transitData))->loginName;
        u->password = ((struct Users *)(m_transitData))->password;
        u->userLevel = ((struct Users *)(m_transitData))->userLevel;
        break;
    }
    case CMD_LOC_LOAD:
    case CMD_LOC_SAVE:
    {
        struct Location *l = (struct Location *)data;
        l->index = ((struct Location *)(m_transitData))->index;
        l->description = ((struct Location *)(m_transitData))->description;
        l->speedLimit = ((struct Location *)(m_transitData))->speedLimit;
        l->captureSpeed = ((struct Location *)(m_transitData))->captureSpeed;
        l->roadCondition = ((struct Location *)(m_transitData))->roadCondition;
        l->numberLanes = ((struct Location *)(m_transitData))->numberLanes;
        break;
    }
    default:
        retv = -1;
        break;
    }

    clearTransitData();

    return retv;
}


void Utils::clearTransitData()
{
    if (!m_transitData) {
        m_transitType = CMD_NONE;
        return;
    }

    switch (m_transitType) {
    case CMD_EDIT_USER:
    {
        Users *u = (Users *)m_transitData;
        delete u;
        break;
    }
    case CMD_LOC_LOAD:
    case CMD_LOC_SAVE:
    {
        Location *l = (Location *)m_transitData;
        delete l;
        break;
    }
    default:
        break;
    }

    m_transitType = CMD_NONE;
    m_transitData = NULL;
}

int Utils::connectGPSMEM()
{
  int GPSID;
  void *mem;
  
  if ((GPSID = shmget(GPS_KEY, sizeof( struct GPS_Buff), 0777)) < 0) {
	printf("%s(%d): shmget GOOD not created 0x%x errno %d \n\r", __FILE__, __LINE__, GPSID, errno);
	if ((GPSID = shmget(GPS_KEY, sizeof( struct GPS_Buff), IPC_CREAT | 0777)) < 0) {
	  printf("%s(%d): shmget failed\n\r", __FILE__, __LINE__);
	  return -1;
	}
  }
  //	printf("%s(%d): shmget GOOD ID 0x%x Key 0x%x size %d\n\r", __FILE__, __LINE__, GPSID, GPS_KEY, sizeof(struct GPS_Buff));
  
  // reuse the mem variable
  mem = shmat(GPSID, NULL, 0);
  if (mem == NULL ) {
	printf("%s(%d): shmat failed\n\r", __FILE__, __LINE__);
	return -1;
  }	
  
  printf("%s(%d): shmat GOOD %p \n\r", __FILE__, __LINE__,mem);
  m_GPSData = (GPS_Buff *)mem;
  return 0;
}

int Utils::connectTILTMEM()
{
  int TILTID;
  void *mem;
  
  DEBUG() << "TILT_KEY " << TILT_KEY << " size " << sizeof( struct Tilt_Buff);
  if ((TILTID = shmget(TILT_KEY, sizeof( struct Tilt_Buff), 0777)) < 0) {
    DEBUG() << "shmget GOOD not created errno " <<  errno << " ID " << TILTID;
    if ((TILTID = shmget(TILT_KEY, sizeof( struct Tilt_Buff), IPC_CREAT | 0777)) < 0) {
      DEBUG() << "shmget failed";
      return -1;
    }
  }
  
  // reuse the mem variable
  mem = shmat(TILTID, NULL, 0);
  if (mem == NULL ) {
    DEBUG() << "shmat failed";
    return -1;
  }	
  
  DEBUG() << "shmat GOOD mem " << mem;
  tiltData = (Tilt_Buff *)mem;
  return 0;
}	

int Utils::connectMAGMEM()
{
  int MAGID;
  void *mem;
  
  DEBUG() << "MAG_KEY " << MAG_KEY << " size " << sizeof( struct Mag_Buff);
  if ((MAGID = shmget(MAG_KEY, sizeof( struct Mag_Buff), 0777)) < 0) {
    DEBUG() << "shmget GOOD not created errno " <<  errno << " ID " << MAGID;
    if ((MAGID = shmget(MAG_KEY, sizeof( struct Mag_Buff), IPC_CREAT | 0777)) < 0) {
      DEBUG() << "shmget failed";
      return -1;
    }
  }
  
  // reuse the mem variable
  mem = shmat(MAGID, NULL, 0);
  if (mem == NULL ) {
    DEBUG() << "shmat failed";
    return -1;
  }	
  
  DEBUG() << "shmat GOOD mem " << mem;
  magData = (Mag_Buff *)mem;
  return 0;
}	
  
#ifdef LIDARCAM
int Utils::connectLidar()
{
    // Attach to the message queue
    key_t Key;
    int smID;
    int FGID;

    if((Key = ftok(MESSAGE_QUEUE_FILE, MESSAGE_QUEUE_CHAR)) == -1)
    {
        cout << Key << endl;
        cout << "Error getting message queue key" << MESSAGE_QUEUE_FILE << MESSAGE_QUEUE_CHAR << endl;
        return -1;
    }

    if((ID = msgget(Key, 0666)) == -1)
    {
      if((ID = msgget(Key, 0666| IPC_CREAT)) == -1)
      {
		cout << "Error attaching to message queue " << strerror(errno) << endl;
        return -1;
      }
    }

	//    cout << "message queue key " << Key << " ID " << ID << endl;

  // PICCMD, size, xx, Enable_Disable_Streaming_Command,PICENABLE
#pragma pack(1)
	typedef struct PicPacket {
	  struct PicHdr hdr;
	  struct PicStream data;
	} PicPacket;
#pragma pack(0)
	
	PicPacket command;
	
	command.hdr.picMsg = PICCMD;
	command.hdr.size = sizeof( PicPacket);
	command.hdr.piccmd = Enable_Disable_Streaming_Command;
	command.data.cntrl = PICENABLE;
	
	Message_Queue_Buff Message_Buffer;
    memset((void *)&Message_Buffer, 0, sizeof(Message_Queue_Buff));
    
    Message_Buffer.mtype = PicMTypeSend;
    Message_Buffer.Msg_Info.From_Address = 3;
    Message_Buffer.Msg_Info.Command = CommandPassthru;
    memcpy((void *)&Message_Buffer.Msg_Info.data[0], (void *)&command, sizeof(PicPacket));
    
    int result = msgsnd(ID, &Message_Buffer, sizeof(Message_Queue_Buff), 0);
	//    printf("%s(%d) sending result 0x%x(%d) size %d\r\n", __FILE__, __LINE__, result, result, sizeof(Message_Queue_Buff));
    if(result < 0) {
      printf("%s(%d) Write failed ret %d errno %d %s\r\n", __FILE__, __LINE__, result, errno, strerror(errno));
      return  -1;
    }
  
	// poll the lidar/coldfire for the current lidar memory
	// Will cause the lidar Shared Memory to update.
	int ret = getLidarMemory( );
	if( ret != 0 ) {
      printf("%s(%d) getLidarMemeory ret %d\r\n", __FILE__, __LINE__, ret);
	}	  
	
    if ((smID = shmget(LIDAR_KEY, sizeof( struct Lidar_Buff), 0777)) < 0) {
      printf("%s(%d): shmget GOOD not created 0x%x errno %d \n\r", __FILE__, __LINE__, smID, errno);
      if ((smID = shmget(LIDAR_KEY, sizeof( struct Lidar_Buff), IPC_CREAT | 0777)) < 0) {
		printf("%s(%d): shmget failed\n\r", __FILE__, __LINE__);
		return -1;
      }
    }
    //	printf("%s(%d): shmget GOOD ID 0x%x Key 0x%x size %d\n\r", __FILE__, __LINE__, smID, LIDAR_KEY, sizeof(struct Lidar_Buff));
    
    void *mem = shmat(smID, NULL, 0);
    if (mem == NULL ) {
      printf("%s(%d): shmat failed\n\r", __FILE__, __LINE__);
      return -1;
    }	
    
    printf("%s(%d): shmat GOOD %p \n\r", __FILE__, __LINE__,mem);
    m_lidarData = (Lidar_Buff *)mem;
    printf("%s(%d): shmat GOOD %p RG 0x%x SP 0x%x %f %f \n\r", __FILE__, __LINE__,
	   m_lidarData,
	   *(unsigned int *)&m_lidarData->Range.Range_c[0],
	   *(unsigned int *)&m_lidarData->Speed.Speed_c[0],
	   m_lidarData->Range.Range_f,
	   m_lidarData->Speed.Speed_f);
    printf("Speed:%f Range:%f 0x%x 0x%x \r\n",m_lidarData->Speed.Speed_f,m_lidarData->Range.Range_f,
	   *(unsigned int *)&m_lidarData->Speed.Speed_c[0],
	   *(unsigned int *)&m_lidarData->Range.Range_c[0]);
    
    this->m_lidarConnected = 1;

    if ((FGID = shmget(FUEL_GAUGE_KEY, sizeof( struct Fuel_Gauge_Buff), 0777)) < 0) {
      printf("%s(%d): shmget GOOD not created 0x%x errno %d \n\r", __FILE__, __LINE__, FGID, errno);
      if ((FGID = shmget(FUEL_GAUGE_KEY, sizeof( struct Fuel_Gauge_Buff), IPC_CREAT | 0777)) < 0) {
		printf("%s(%d): shmget failed\n\r", __FILE__, __LINE__);
		return -1;
      }
    }
    //	printf("%s(%d): shmget GOOD ID 0x%x Key 0x%x size %d\n\r", __FILE__, __LINE__, FGID, FG_KEY, sizeof(struct FG_Buff));
    
	// reuse the mem variable
    mem = shmat(FGID, NULL, 0);
    if (mem == NULL ) {
      printf("%s(%d): shmat failed\n\r", __FILE__, __LINE__);
      return -1;
    }	
    
    printf("%s(%d): shmat GOOD %p \n\r", __FILE__, __LINE__,mem);
    m_FGData = (Fuel_Gauge_Buff *)mem;

    return 0;
}

float Utils::lidarRange()
{
    if (m_lidarData) {
      return m_lidarData->Range.Range_f;
    }
    return 0.0;
}
float Utils::lidarSpeed()
{
    if (m_lidarData) {
      return m_lidarData->Speed.Speed_f;
    }
    return 0.0;
}
#endif

#ifdef LIDARCAM
//int zoomValue[] = {1000, 2000, 5000, 10000, 13000, 18000};
extern int zoomValue[];
#endif

int Utils::sendCmdToCamera( int cmd, int value, int para1, int para2)
{
  //   DEBUG() << " cmd " << cmd << " value " << value;
   char data1[APPRO_DATA_LEN];

   memset((void *)data1, 0, APPRO_DATA_LEN);
   
   switch (cmd)
   {
#ifdef LIDARCAM
   case CMD_ZOOM:
      {
         value = (value > ZOOM_MAX) ? ZOOM_MAX : value;
         value = (value < ZOOM_MIN) ? ZOOM_MIN : value;
         memcpy((void *)data1, &(zoomValue[value - 1]), 4);
         data1[4] = 0;  // Absolute position
//         data1[4] = 1;  // Zoom in relatively
         SndApproMsg(APPRO_ZOOMIN, data1, NULL);
         break;
      }

      case CMD_FOCUS:
      {
         char *focus = (char *)value;
         if (!strncmp(focus, "AUTO", 4))
            data1[0] = 4;
         else
            data1[0] = 0;  // Manual
         SndApproMsg(APPRO_FOCUSMODE, data1, NULL);
         break;
      }

      case CMD_RUN_FOCUS:  // Force to run auto-focus once
      {
         data1[0] = 0;
         SndApproMsg(APPRO_RUNFOCUS, data1, NULL);
         break;
      }
#endif

      case CMD_RECORD:
      {
         QDateTime dateTime = QDateTime::currentDateTime();
         QString fileName( "/mnt/mmc/ipnc/" );
         fileName.append( m_session->user()->loginName);
         //	  qDebug() << fileName;
         if ( !QDir( fileName ).exists() )
         {
            // if not add it.
            QDir().mkdir( fileName );
         }
         fileName.append( "/" );

         // Add the Serial Number at the beginning of the fileName, vs the complete fileName
         // Create a QString from the shared Memory Serial Number
         std::string str;
         str.resize(8);
	 QString serialNumber;
#ifdef LIDARCAM
         memcpy((char *)str.data(), (char *)&m_lidarData->lidarStruct.SERIAL_NUMBER[0], 8);
	 serialNumber = QString::fromStdString(str);
#else
	 // Need HH1 serial number
	 SysConfig & cfg = getConfiguration();
	 serialNumber = cfg.serialNumber;
#endif
         //	  DEBUG() << "serialNumber " << serialNumber << "len " << serialNumber.length();
         //	  DEBUG() << "DataTime " << dateTime.toString("yyyyMMddhhmmss");

         Administration admin = Utils::get().getAdmin();

         if (admin.dateFormat)
         {  // dd-MM-yyyy format
            fileName.append( dateTime.toString( "ddMMyy_hhmmss_" ));
         }
         else
         {  // yyyy-MM-dd format
            fileName.append( dateTime.toString( "yyMMdd_hhmmss_" ));
         }

         fileName.append ( serialNumber );

         fileName.append ( "_" );

         mEvidenceId = getNewEvidenceNum();
         char num[256];
         memset(num, 0, 256);
         sprintf(&num[0], "%05d", mEvidenceId);
         QString evidenceNumber = QString::fromStdString( num );

         fileName.append ( evidenceNumber );

         // create a copy of the complete new file name
         mRecordingFileName = fileName;

         fileName.append( ".avi" );

         DEBUG() << fileName;
	 
	 SysConfig& mConf = getConfiguration( 0 );
	 DEBUG() << "postBuf " << mConf.postBuf << "preBuf " << mConf.preBuf;
	 
         data1[0] = mConf.preBuf + mConf.postBuf;    // recording seconds (< 256)
         data1[1] = 0;     			     // recording minutes
         data1[2] = mConf.preBuf;                    // prebuffer seconds (< 10)
         data1[3] = 1;                               // audio on/off
         data1[4] = 1;				     // for snap shot at the start of recording
	 data1[5] = 0;data1[6] = 0;data1[7] = 0;
         SndApproMsg(APPRO_RECORD, data1, fileName.toLatin1().data());
         break;
      }

      case CMD_STOPRECORD:
      {
         SndApproMsg(APPRO_RECORD, data1, NULL);
         backGround::get().createJson(&mRecordingFileName);
         break;
      }

      case CMD_PLAYBACK:
      {
         char *filename = (char *)value;
         data1[0] = 2;  // Start
         data1[1] = 1;  // Audio on
         SndApproMsg(APPRO_PLAYBACK, data1, filename);
         break;
      }

      case CMD_STOPPLAY:
         data1[0] = 1;  // Stop
         data1[1] = 0;  // Audio off
         SndApproMsg(APPRO_PLAYBACK, data1, NULL);
         break;

      case CMD_PAUSEPLAY:
         data1[0] = 3;  // Pause
         data1[1] = 0;  // Audio off
         SndApproMsg(APPRO_PLAYBACK, data1, NULL);
         break;

      case CMD_RESUMEPLAY:
         data1[0] = 4;  // Resume
         data1[1] = 1;  // Audio on
         SndApproMsg(APPRO_PLAYBACK, data1, NULL);
         break;

      case CMD_SNAPSHOT:
      {
         char *filename = (char *)value;
         data1[0] = 0;
         SndApproMsg(APPRO_SNAPSHOT, data1, filename);
         break;
      }

      case CMD_DISPLAY_PHOTO:
      {
         char *filename = (char *)value;
         data1[0] = 2;  // Display
         SndApproMsg(APPRO_PHOTO, data1, filename);
         break;
      }

      case CMD_REMOVE_PHOTO:
      {
         data1[0] = 1;  // Remove the picture from displaying
         SndApproMsg(APPRO_PHOTO, data1, NULL);
         break;
      }

      case CMD_DISPLAY_WATERMARK:
      {
         char *str1 = (char *)value;
         if (strlen(str1) > MAX_WATERMARK_LEN)
            str1[MAX_WATERMARK_LEN]  = '\0';
         data1[0] = 1;  // Line 1 only
         data1[1] = para1 & 0x0FF;
         data1[2] = para2 & 0x0FF;
//         SndApproMsg(APPRO_WATERMARK, data1, str1);
         SndApproMsg(APPRO_LIDAR, data1, str1);
         break;
      }

      case CMD_DISPLAY_WATERMARK2:
      {
         char *str1 = (char *)value;
         if (strlen(str1) > MAX_WATERMARK_LEN)
            str1[MAX_WATERMARK_LEN]  = '\0';
//         data1[0] = 2;  // Both lines
         data1[0] = 3;  // Line 2 only
         data1[1] = para1 & 0x0FF;
         data1[2] = para2 & 0x0FF;
//         SndApproMsg(APPRO_WATERMARK, data1, str1);
         SndApproMsg(APPRO_LIDAR, data1, str1);
         break;
      }

      case CMD_REMOVE_WATERMARK:
      {
         data1[0] = 0;  // Remove
//         SndApproMsg(APPRO_WATERMARK, data1, NULL);
         SndApproMsg(APPRO_LIDAR, data1, NULL);
         break;
      }

      case CMD_TIMESTAMP:
      {
	data1[1] = 5;  // font size
	if (value)
	  data1[0] = 1;  // Timestamp on
	else
	  data1[0] = 0;  // Timestamp off
	SndApproMsg(APPRO_TIMESTAMP, data1, NULL);
	break;
      }

      case CMD_CAMERA_BOX:
      {
         if (value)
            data1[0] = 1;  // BOX on
         else
            data1[0] = 0;  // BOX off
         data1[1] = para1 & 0x0FF;  // Box size
         SndApproMsg(APPRO_BOX, data1, NULL);
         break;
      }

      case CMD_CAMERA_IRCUT:
      {
         data1[0] = (unsigned char)value;
         SndApproMsg(APPRO_IRCUT, data1, NULL);
         break;
      }

      case CMD_IRIS:
      {
         data1[0] = (unsigned char)value;
         SndApproMsg(APPRO_IRIS, data1, NULL);
         break;
      }

      case CMD_CAMERA_2A_MODE:
      {
         data1[0] = (unsigned char)value;
         SndApproMsg(APPRO_2A, data1, NULL);
         break;
      }

      case CMD_SHUTTER:
      {
         data1[0] = (unsigned char)value;
         SndApproMsg(APPRO_SHUTTER, data1, NULL);
         break;
      }

      case CMD_GAIN:
      {
         data1[0] = (unsigned char)value;
         SndApproMsg(APPRO_GAIN, data1, NULL);
         break;
      }

      case CMD_SECURITY_DATEFORMAT:
      {
         data1[0] = (unsigned char)value;
         data1[1] = 1;  // 0 -> 12 hours, 1 -> 24 hours
         SndApproMsg(APPRO_DTFMT, data1, NULL);
         break;
      }
         break;

      default:
        qDebug() << "cmd not supported yet " << cmd;
        break;
      }
   return 0;
}

#ifdef LIDARCAM
unsigned short Utils::UART_CALCULATE_CHECKSUM(unsigned char * buffer, int size)
{
  int counter = 0;
  unsigned short checksum = 0;
  
  for(counter=0; counter < size; counter++)
  {
	if(counter & 0x01)
	{
	  checksum += (buffer[counter] << 8);
	}
	else
	{
	  checksum += buffer[counter];
	}
  }
  return checksum;
}

int Utils::getLidarMemory( void )
{
#ifdef IS_TI_ARM
  //build the cfMessage
  // get some memory for the packet
  Message_Queue_Buff Message_Buffer;
  memset((void *)&Message_Buffer, 0, sizeof(Message_Queue_Buff));
  
#define tmpBufferSize 64	
  char buf[tmpBufferSize];
  memset((void *)&buf[0], 0, tmpBufferSize);
  
#pragma pack(1)
  typedef struct CmdSendParms {
	struct PicHdr hdr;
	CF_F2_COMMAND data;
  } CmdSendParms;
#pragma pack(0)
  
  CmdSendParms *msg = (CmdSendParms *)&buf[0];
  struct PicHdr *top = (struct PicHdr *)&msg->hdr;
  
  top->picMsg = PICCMD;
  top->size = 0; // will be filled in before this message is sent
  top->piccmd = PicLidarPassThru;
  
  CF_F2_COMMAND *ptr = (CF_F2_COMMAND *) &msg->data;
  
  ptr->COMMAND = ColdFireF2;
  ptr->addr.ADDRESS_CHAR.FROM_ADDRESS = 1;
  ptr->addr.ADDRESS_CHAR.TO_ADDRESS = 1;
  ptr->len.BIT16_LENGTH_VALUE = (unsigned char)(sizeof(CF_F2_COMMAND) - sizeof(unsigned char) + sizeof(CHECKSUM));
  ptr->COMMAND_TO_SEND = CMD_SEND_PARAMS; 
  ptr->BODY_LENGTH = 0;
  
  // get checksum
  // copy checksum into the ptr packet
  unsigned short chksum = UART_CALCULATE_CHECKSUM( (unsigned char *)ptr, (sizeof(CF_F2_COMMAND) - sizeof(unsigned char)));
  
  //  printf("%s(%d) ptr %p sz %d len %d cs 0x%x\r\n", __FILE__, __LINE__, ptr, sizeof(CF_F2_COMMAND), ptr->len.BIT16_LENGTH_VALUE, chksum);
  
  Message_Buffer.mtype = PicMTypeSend;
  Message_Buffer.Msg_Info.From_Address = 3;
  Message_Buffer.Msg_Info.Command = CommandPassthru;
  memcpy((void *)&ptr->BODY[0], &chksum, sizeof(unsigned short));
  
  top->size = sizeof(struct PicHdr) + ptr->len.BIT16_LENGTH_VALUE;
  
  // copy it to the Message_Buffer
  memcpy((void *)&Message_Buffer.Msg_Info.data[0], (void *)top, top->size );
  
  // hexDump("pkt", &Message_Buffer, sizeof(struct Message_Queue_Buff));
  
  int result = 0;
  
  //  cout << "message queue ID " << ID << endl;

  result = msgsnd(ID, &Message_Buffer, sizeof(struct Message_Queue_Buff), 0);
  //  printf("%s(%d) sending result 0x%x(%d) size %d\r\n", __FILE__, __LINE__, result, result, sizeof(struct Message_Queue_Buff));
  if(result < 0) {
	printf("%s(%d) Write failed ret %d errno %d %s\r\n", __FILE__, __LINE__, result, errno, strerror(errno));
	return  -1;
  }
  bool ackRecved = false;
  while( 1 ) {
	result = msgrcv(ID, &Message_Buffer, sizeof(Message_Queue_Buff), PicMTypeRecv, MSG_NOERROR);
	// get the response packet for the command
	if (result != -1 ) {
	  int ack = 0;
	  int cmd = ptr->COMMAND_TO_SEND;
	  ackRecved = responseF2Packet( &Message_Buffer, &cmd, &ack); 

	  if( ackRecved == false ){
		continue;
	  }
	  break;
	}
  }
  //  hexDump((char *)"sndReturn", &Message_Buffer, 32);
#endif
  
  return 0;
}
int Utils::setLidarMemory( lidarElements type, int *data, struct Message_Queue_Buff *recvBuff )
{
#ifdef IS_TI_ARM
  int paramater_data_size = 0;
  switch(type)
	{
	case SPEAKER_VOLUME_ID:
	case BACKLIGHT_ID:
	case HUD_BRIGHTNESS_ID:
	case UNITS_ID:
	case SPEED_RANGE_TENTHS_ID:
	case HIGH_ACCURACY_ID:
	case HUD_RANGE_ID:
	case HUD_SPEED_ID:
	case DISTANCE_CALIBRATION_ID:
	case TILT_ANGLE_ID:
	case ANTI_JAMMING_ID:
	case SERIAL_FORMATS_ID:
	case TEST_CLOCK_ID:
	case POWER_DOWN_TIMEOUT_ID:
	case RESTING_TIMEOUT_ID:
	case FACTORY_MODE_ID:
	case LIDAR_TYPE_ID:
	case LIDAR_PROFILE_ID:
	case DELTA_DIFF_ID:
	case SSC_MODE_ID:
	case SPEED_RANGE_DISPLAY_ID:
	case TRIGGER_MODE_ID:
	case TD_MODE_ID:
	case LATCHED_TRIGGER_ID:
	case EU_HARDWARE_VERSION_ID:
	case INHIBIT_MODES_ID:
	case SPECIAL_MODES_ID:
	case MORE_MODES_ID:
	case MODES_1_ID:
	case MODES_2_ID:
	case MODES_3_ID:
	case FEATURE_SET_1_ID:
	case FEATURE_SET_2_ID:
	case FEATURE_SET_3_ID:
	  paramater_data_size = 1;
	  break;
	case BAUD_RATE_ID:
	case RFI_TRIP_ID:
	case MINIMUM_DISTANCE_ID:
	case MAXIMUM_DISTANCE_ID:
	case TD_RANGE_MIN_ID:
	case TD_RANGE_MAX_ID:
	case THRESHOLD_SPEED_ID:
	case ALARM_SPEED_ID:
	  paramater_data_size = 2;
      break;
	case BLUETOOTH_HOST_PIN_ID:
	case BLUETOOTH_CLIENT_PIN_ID:
	case TIME_VALUE_ID:
	case CAL_EXP_TIME_ID:
	case MAIN_BOARD_FW_VER:
	  paramater_data_size = 4;
	  break;
	case SERIAL_NUMBER_ID:
	  paramater_data_size = 8;
	  break;
	default:
	  break;
	}
  //  printf("%s(%d) type %d data 0x%x size %d\n", __FILE__, __LINE__, type, *data, paramater_data_size );

  unsigned char buf[1024];
  struct Message_Queue_Buff Message_Buffer;
  
  #pragma pack(1)
  typedef struct F1Cmd {
	struct PicHdr hdr;
	CF_F1_COMMAND data;
  } F1Cmd;
#pragma pack(0)

  memset((void *)&Message_Buffer, 0, sizeof(struct Message_Queue_Buff));
  
  Message_Buffer.mtype = PicMTypeSend;
  Message_Buffer.Msg_Info.From_Address = 3;
  Message_Buffer.Msg_Info.Command = CommandPassthru;

  F1Cmd *msg = (F1Cmd *)&buf[0];
  struct PicHdr *top = (struct PicHdr *)&msg->hdr;
  
  top->picMsg = PICCMD;
  top->size = PICCMD; // TBD
  top->piccmd = PicLidarPassThru;
  
  CF_F1_COMMAND *ptr = (CF_F1_COMMAND *) &msg->data;

  ptr->COMMAND = ColdFireF1;
  ptr->addr.ADDRESS_CHAR.FROM_ADDRESS = 1;
  ptr->addr.ADDRESS_CHAR.TO_ADDRESS = 1;
  ptr->len.BIT16_LENGTH_VALUE = (unsigned char)(sizeof(CF_F1_COMMAND) + paramater_data_size + sizeof(CHECKSUM));
  ptr->BODY_LENGTH.BIT16_LENGTH_VALUE = 1;

  //  printf("%s(%d) ptr %p sz %d len %d\r\n", __FILE__, __LINE__, ptr, sizeof(CF_F1_COMMAND), ptr->len.BIT16_LENGTH_VALUE);


  BIT16_LENGTH tmp16;
  tmp16.BIT16_LENGTH_VALUE = (unsigned short)type;
  memcpy( &ptr->Body[0], &tmp16, sizeof(BIT16_LENGTH));

  tmp16.BIT16_LENGTH_VALUE = (unsigned short)paramater_data_size;
  memcpy( &ptr->Body[2], &tmp16, sizeof(BIT16_LENGTH));
  
  memcpy(&ptr->Body[4], (unsigned char*)data, paramater_data_size);

  // get checksum
  // copy checksum into the ptr packet
  unsigned short chksum = UART_CALCULATE_CHECKSUM( (unsigned char *)ptr, (sizeof(CF_F1_COMMAND) + paramater_data_size));
  
  //  printf("%s(%d) ptr %p sz %d len %d cs 0x%x\r\n", __FILE__, __LINE__, ptr, sizeof(CF_F1_COMMAND), ptr->len.BIT16_LENGTH_VALUE, chksum);

  memcpy((void *)&ptr->Body[paramater_data_size + 4], &chksum, sizeof(unsigned short));
  
  top->size = sizeof(struct PicHdr) + ptr->len.BIT16_LENGTH_VALUE;


  // copy it to the Message_Buffer
  memcpy((void *)&Message_Buffer.Msg_Info.data[0], (void *)top, top->size );
  
  //  hexDump("pkt", &Message_Buffer, sizeof(struct Message_Queue_Buff) - 256  + top->size);
  
  int result = 0;
  
  result = msgsnd(ID, &Message_Buffer, sizeof(struct Message_Queue_Buff), 0);
  //  printf("%s(%d) sending result 0x%x(%d) size %d\r\n", __FILE__, __LINE__, result, result, sizeof(struct Message_Queue_Buff));
  if(result < 0) {
	printf("%s(%d) Write failed ret %d errno %d %s\r\n", __FILE__, __LINE__, result, errno, strerror(errno));
	return  -1;
  }
  bool ackRecved = false;
  while( 1 ) {
	result = msgrcv(ID, &Message_Buffer, sizeof(Message_Queue_Buff), PicMTypeRecv, MSG_NOERROR);
	// get the response packet for the command
	if (result != -1 ) {
	  int ack = 0;
	  int cmd = type;
	  ackRecved = responseF1Packet( &Message_Buffer, &cmd, &ack); 

	  if( ackRecved == false ){
		continue;
	  }
	  break;
	}
  }

  //  hexDump("sndReturn", &Message_Buffer, 32);

  if( recvBuff != NULL ) {
	memcpy( recvBuff, &Message_Buffer, sizeof(struct Message_Queue_Buff ));
  }
  int ret = getLidarMemory( );
  if( ret != 0 ) {
	printf("%s(%d) getLidarMemeory ret %d\r\n", __FILE__, __LINE__, ret);
  }	  
  
#endif
  return 0;
}

#endif   // LIDARCAM

int Utils::sendMbPacket( unsigned char cmd1, unsigned short length, unsigned char *data1, struct Message_Queue_Buff *recvBuff )
{
  UNUSED(length);
  UNUSED(data1);
  UNUSED(recvBuff);

  if ( cmd1 == CMD_KEYBOARD_BEEP ) {
#ifndef LIDARCAM
    QSound *beep = new QSound(":/logo/tampaBeep");
    beep->play();
#endif
    activity++;
  }
  
#ifdef LIDARCAM
  //build the cfMessage
  // get some memory for the packet
  Message_Queue_Buff Message_Buffer;
  memset((void *)&Message_Buffer, 0, sizeof(Message_Queue_Buff));

#define tmpBufferSize 64
  char buf[tmpBufferSize];
  memset((void *)&buf[0], 0, tmpBufferSize);

#pragma pack(1)
  typedef struct CmdSendParms {
    struct PicHdr hdr;
    CF_F2_COMMAND data;
  } CmdSendParms;
#pragma pack(0)

  CmdSendParms *msg = (CmdSendParms *)&buf[0];
  struct PicHdr *top = (struct PicHdr *)&msg->hdr;

  top->picMsg = PICCMD;
  top->size = 0; // will be filled in before this message is sent
  top->piccmd = PicLidarPassThru;

  CF_F2_COMMAND *ptr = (CF_F2_COMMAND *) &msg->data;

  ptr->COMMAND = ColdFireF2;
  ptr->addr.ADDRESS_CHAR.FROM_ADDRESS = 2;
  ptr->addr.ADDRESS_CHAR.TO_ADDRESS = 1;
  ptr->len.BIT16_LENGTH_VALUE = (unsigned char)(sizeof(CF_F2_COMMAND) - sizeof(unsigned char) + sizeof(CHECKSUM)) + length;
  ptr->COMMAND_TO_SEND = cmd1;
  ptr->BODY_LENGTH = length;
  if (length)
    memcpy(&ptr->BODY[0], (unsigned char*)data1, length);

  // get checksum
  // copy checksum into the ptr packet
  unsigned short chksum = UART_CALCULATE_CHECKSUM( (unsigned char *)ptr, (sizeof(CF_F2_COMMAND) - sizeof(unsigned char) + length ));

//    printf("%s(%d) ptr %p sz %d len %d cs 0x%x\r\n", __FILE__, __LINE__, ptr, sizeof(CF_F2_COMMAND), ptr->len.BIT16_LENGTH_VALUE, chksum);

  Message_Buffer.mtype = PicMTypeSend;
  Message_Buffer.Msg_Info.From_Address = 3;
  Message_Buffer.Msg_Info.Command = CommandPassthru;
  memcpy((void *)&ptr->BODY[length], &chksum, sizeof(unsigned short));

  top->size = sizeof(struct PicHdr) + ptr->len.BIT16_LENGTH_VALUE;

  // copy it to the Message_Buffer
  memcpy((void *)&Message_Buffer.Msg_Info.data[0], (void *)top, top->size );

//  hexDump("pkt", &Message_Buffer, sizeof(struct Message_Queue_Buff));

  int result = 0;

  //  cout << "message queue ID " << ID << endl;
  result = msgsnd(ID, &Message_Buffer, sizeof(struct Message_Queue_Buff), 0);
  //    printf("%s(%d) sending result 0x%x(%d) size %d\r\n", __FILE__, __LINE__, result, result, sizeof(struct Message_Queue_Buff));
  if(result < 0) {
    printf("%s(%d) Write failed ret %d errno %d %s\r\n", __FILE__, __LINE__, result, errno, strerror(errno));
    return  -1;
  }
  bool ackRecved = false;
  while( 1 ) {
	result = msgrcv(ID, &Message_Buffer, sizeof(Message_Queue_Buff), PicMTypeRecv, MSG_NOERROR);
	// get the response packet for the command
	if (result != -1 ) {
	  int ack = 0;
	  int cmd = ptr->COMMAND_TO_SEND;
	  ackRecved = responseF2Packet( &Message_Buffer, &cmd, &ack); 

	  if( ackRecved == false ){
		continue;
	  }
	  break;
	}
  }

  //  hexDump("sndReturn", &Message_Buffer, 32);

  if( recvBuff != NULL ) {
	memcpy( recvBuff, &Message_Buffer, sizeof(struct Message_Queue_Buff ));
  }
#endif
  return 0;
}

bool Utils::responseF2Packet( struct Message_Queue_Buff *ptr, int *cmd, int *ack)
{
   CF_F2_ACK *pkt;
   pkt = (CF_F2_ACK *)&ptr->Msg_Info.data[0];

   if( pkt->COMMAND != ColdFireF2 )
   {
      printf("F2 command missmatch 0x%x\r\n",pkt->COMMAND);
      return false;
   }
   if( pkt->COMMAND_TO_SEND != *cmd )
   {
      printf("F2 data missmatch 0x%x : 0x%x\r\n",pkt->COMMAND_TO_SEND,*cmd);
      return false;
   }

   // packet is good
   *ack = pkt->ACK;
   return true;
}
bool Utils::responseF1Packet( struct Message_Queue_Buff *ptr, int *cmd, int *ack)
{
   UNUSED(cmd);

   CF_F2_ACK *pkt;
   pkt = (CF_F2_ACK *)&ptr->Msg_Info.data[0];

   if( pkt->COMMAND != ColdFireF1 )
      return false;

   if( pkt->COMMAND_TO_SEND != ColdFireF1 )
      return false;

   // packet is good
   *ack = pkt->ACK;
   return true;
}

bool Utils::getPacket(  Message_Queue_Buff *Message_Buffer, int size, int timeout)
{
#ifdef IS_TI_ARM
   int result = 0;
   while( 1 )
   {
      result = msgrcv(ID, Message_Buffer, size, PicMTypeRecv, MSG_NOERROR|IPC_NOWAIT);
      // get the response packet for the command
      if (result != -1 )
      {
         DEBUG() << "Good packet";
         return true;
      }
      // wait for a while
      if( timeout-- < 0 )
      {
//         DEBUG() << "Timeout";
         break;
      }
      QThread::msleep(100);
   }
#endif
   return false;
}

bool Utils::getAlertPacket(unsigned char alertId, char *pData, unsigned short *pLen, int timeout)
{
#ifdef LIDARCAM
   struct Message_Queue_Buff Message_Buffer;
   memset((void *)&Message_Buffer, 0, sizeof(struct Message_Queue_Buff));
   bool ret;

   ret = getPacket(&Message_Buffer, sizeof(struct Message_Queue_Buff), timeout);
   if ( ret == true )
   {
      CF_MB_ALERT *ptr;
      ptr = (CF_MB_ALERT *)&Message_Buffer.Msg_Info.data[0];
      if( ptr->COMMAND == 4 )
      {
         if( ptr->len.BIT16_LENGTH_VALUE == ((unsigned short)(ptr->BODY_LENGTH) + 7) )
         {
            if ( ptr->ALERT == alertId)
            {
               *pLen = ptr->BODY_LENGTH;
               memcpy((void *)pData, (void *)ptr->data, *pLen);
               return true;
            }
            else
            {
               hexDump("getPacket FAILED", (void *)&Message_Buffer, 32);
            }
         }
      }
   }
#else
   UNUSED(alertId);
   UNUSED(pData);
   UNUSED(pLen);
   UNUSED(timeout);
#endif
  return false;
}

int Utils::setEvidenceNum(unsigned int baseNum)
{
   struct Evidence l;
   int retv;

   l.index = 0;
   retv = get().db()->queryEntry(TBL_EVIDENCE, (DBStruct *)&l, QRY_ALL_ENTRIES);
   l.evidNum = l.imageNum = l.ticketNum = baseNum;
   //update database
   if (retv > 0)   // this entry already created
      retv = get().db()->updateEntry(TBL_EVIDENCE, (DBStruct *)&l);
   else if (!retv)
      retv = get().db()->addEntry(TBL_EVIDENCE, (DBStruct *)&l);
   else
      qCritical() << "Add/Update Evidence entry failed (" << l.evidNum << ")";

   return retv;
}

// Get the new serial number for this video
bool Utils::getEvidenceNum(struct Evidence &l)
{
   int retv;

   l.index = 0;
   retv = get().db()->queryEntry(TBL_EVIDENCE, (DBStruct *)&l, QRY_BY_KEY);
   retv = get().db()->getNextEntry(TBL_EVIDENCE, (DBStruct *)&l);

   if (retv)
      return false;  // Some errors
   else
      return true;   // good
}

// Get the new serial number for this video
unsigned int Utils::getNewEvidenceNum(int index1)
{
   struct Evidence l;
   int retv;
   unsigned int num1 = 0;

   l.index = index1;
   retv = get().db()->queryEntry(TBL_EVIDENCE, (DBStruct *)&l, QRY_BY_KEY);
   retv = get().db()->getNextEntry(TBL_EVIDENCE, (DBStruct *)&l);

   if (!retv)
   {
      num1 = l.evidNum;
      l.evidNum++;    // Automatically post-increment
      if (!l.evidNum)
         l.evidNum = 1;
      retv = get().db()->updateEntry(TBL_EVIDENCE, (DBStruct *)&l);
   }

   return num1;
}

// Get the new serial number for this image
unsigned int Utils::getNewImageNum(int index1)
{
    struct Evidence l;
    int retv;
    unsigned int num1 = 0;

    l.index = index1;
    retv = get().db()->queryEntry(TBL_EVIDENCE, (DBStruct *)&l, QRY_BY_KEY);
    retv = get().db()->getNextEntry(TBL_EVIDENCE, (DBStruct *)&l);

    if (!retv)
    {
        num1 = l.imageNum;
        l.imageNum++;   // Automatically post-increment
        if (!l.imageNum)
            l.imageNum = 1;
        retv = get().db()->updateEntry(TBL_EVIDENCE, (DBStruct *)&l);
    }

    return num1;
}

// Get the new serial number for this ticket
unsigned int Utils::getNewTicketNum(int index1)
{
    struct Evidence l;
    int retv;
    unsigned int num1 = 0;

    l.index = index1;
    retv = get().db()->queryEntry(TBL_EVIDENCE, (DBStruct *)&l, QRY_BY_KEY);
    retv = get().db()->getNextEntry(TBL_EVIDENCE, (DBStruct *)&l);

    if (!retv)
    {
        num1 = l.ticketNum;
        l.ticketNum++;  // Automatically post-increment
        if (!l.ticketNum)
            l.ticketNum = 1;
        retv = get().db()->updateEntry(TBL_EVIDENCE, (DBStruct *)&l);
    }

    return num1;
}

int Utils::setConfiguration(struct SysConfig & l)
{
    int retv;

    l.index = 0;
    retv = get().db()->queryEntry(TBL_CONFIGURATION, (DBStruct *)&l, QRY_ALL_ENTRIES);
    //update database
    if (retv > 0)   // this entry already created
        retv = get().db()->updateEntry(TBL_CONFIGURATION, (DBStruct *)&l);
    else if (!retv)
        retv = get().db()->addEntry(TBL_CONFIGURATION, (DBStruct *)&l);
    else
        qCritical() << "Add/Update Configuration entry failed (" << l.backlightOff << ")";

    if (retv >= 0)
        mConfiguration = l;

    return retv;
}

SysConfig &  Utils::getConfiguration(int index1)
{
  mConfiguration.index = index1;
  (void)get().db()->queryEntry(TBL_CONFIGURATION, (DBStruct *)&mConfiguration, QRY_BY_KEY);
  (void)get().db()->getNextEntry(TBL_CONFIGURATION, (DBStruct *)&mConfiguration);
  
  return mConfiguration;
}

int Utils::setAdmin(Administration & l)
{
    int retv;

    l.index = 0;
    retv = get().db()->queryEntry(TBL_ADMIN, (DBStruct *)&l, QRY_ALL_ENTRIES);
    //update database
    if (retv > 0)   // this entry already created
        retv = get().db()->updateEntry(TBL_ADMIN, (DBStruct *)&l);
    else if (!retv)
        retv = get().db()->addEntry(TBL_ADMIN, (DBStruct *)&l);
    else
        qCritical() << "Add/Update Administration entry failed";

    if (retv >= 0)
        mAdmin = l;

    return retv;
}

Administration &  Utils::getAdmin(int index1)
{
    mAdmin.index = index1;
    (void)get().db()->queryEntry(TBL_ADMIN, (DBStruct *)&mAdmin, QRY_BY_KEY);
    (void)get().db()->getNextEntry(TBL_ADMIN, (DBStruct *)&mAdmin);

    return mAdmin;
}

int Utils::setNetwork(Network & l)
{
    int retv;

    l.index = 0;
    retv = get().db()->queryEntry(TBL_NETWORK, (DBStruct *)&l, QRY_ALL_ENTRIES);
    //update database
    if (retv > 0)   // this entry already created
        retv = get().db()->updateEntry(TBL_NETWORK, (DBStruct *)&l);
    else if (!retv)
        retv = get().db()->addEntry(TBL_NETWORK, (DBStruct *)&l);
    else
        qCritical() << "Add/Update Network entry failed";

    if (retv >= 0)
        mNet = l;

    return retv;
}

Network &  Utils::getNetwork(int index1)
{
    mNet.index = index1;
    (void)get().db()->queryEntry(TBL_NETWORK, (DBStruct *)&mNet, QRY_BY_KEY);
    (void)get().db()->getNextEntry(TBL_NETWORK, (DBStruct *)&mNet);

    return mNet;
}

int Utils::setSensorInDB(struct Sensor & l)
{
    int retv;

    l.index = 0;
    retv = get().db()->queryEntry(TBL_SENSOR, (DBStruct *)&l, QRY_ALL_ENTRIES);
    //update database
    if (retv > 0)   // this entry already created
        retv = get().db()->updateEntry(TBL_SENSOR, (DBStruct *)&l);
    else if (!retv)
        retv = get().db()->addEntry(TBL_SENSOR, (DBStruct *)&l);
    else
        qCritical() << "Add/Update Sensor entry failed";

    if (retv >= 0)
        mSensor = l;

    return retv;
}

Sensor &  Utils::getSensorFromDB(int index1)
{
    mSensor.index = index1;
    (void)get().db()->queryEntry(TBL_SENSOR, (DBStruct *)&mSensor, QRY_BY_KEY);
    (void)get().db()->getNextEntry(TBL_SENSOR, (DBStruct *)&mSensor);

    return mSensor;
}

void Utils::setIlluminator(ILLUMINATOR *illu)
{
   memcpy ((void *)&mIlluminator, (void *)illu, sizeof(ILLUMINATOR));
}

void Utils::enableWaterMark( void )
{
  std::string str;
  str.resize(8);
#ifdef LIDARCAM
  memcpy((char *)str.data(), (char *)&m_lidarData->lidarStruct.SERIAL_NUMBER[0], 8);
#else
  // HH1 serial number
  SysConfig & cfg = getConfiguration();
  memcpy((char *)str.data(), cfg.serialNumber.toStdString().c_str(), 8);
#endif
  QString serialNumber = QString::fromStdString(str);

  DEBUG() << serialNumber;
  
  QProcess dostalkerWaterMark;
  dostalkerWaterMark.start("/usr/local/stalker/bin/stalkerWaterMark", QStringList() << serialNumber );
  dostalkerWaterMark.waitForFinished(1000);
  int ret = dostalkerWaterMark.exitCode();
  
  if( ret != 0 ) {
	printf("%s(%d) ret = %d\n\r", __FILE__, __LINE__, ret);
  }
  return;
}

float cosTable[] = {0.9998,0.9994,0.9986,0.9976,0.9962,0.9945,0.9925,0.9903,0.9877,0.9848,0.9816,0.9781,0.9744,0.9703,0.9659,0.9613,0.9563,0.9511,0.9455,0.9397,0.9336,0.9272,0.9205,0.9135,0.9063,0.8988,0.8910,0.8829,0.8746,0.8660,0.8572,0.8480,0.8387,0.8290,0.8192,0.8090,0.7986,0.7880,0.7771,0.7660,0.7547,0.7431,0.7314,0.7193,0.7071,0.6947,0.6820,0.6691,0.6561,0.6428,0.6293,0.6157,0.6018,0.5878,0.5736,0.5592,0.5446,0.5299,0.5150,0.5000,0.4848,0.4695,0.4540,0.4384,0.4226,0.4067,0.3907,0.3746,0.3584,0.3420,0.3256,0.3090,0.2924,0.2756,0.2588,0.2419,0.2250,0.2079,0.1908,0.1736,0.1564,0.1392,0.1219,0.1045,0.0872,0.0698,0.0523,0.0349,0.0175,0.0000};

bool Utils::tiltSeen()
{

  int tilt = 90;

#ifdef LIDARCAM
  tilt = m_lidarData->lidarStruct.TILT_ANGLE_THRESHOLD;
#endif
  
  if( tilt == 90 ) {
	return false;
  }

#ifdef HH1
  // TODO figure out the correct axis to use for HH1
  float ratio = abs( tiltData->X_Axis ) / cosTable[tilt];
#else
  float ratio = abs( tiltData->X_Axis ) / cosTable[tilt];
#endif
  if( ratio < 1.0 ) {
	return true;
  }
  return false;
}

void Utils::Send_Msg_To_PIC( int cmd )
{
  struct Message_Queue_Buff Message_Buffer;
#pragma pack(1)
  typedef struct PicPacket {
        struct PicHdr hdr;
        struct PicStream data;
  } PicPacket;
#pragma pack(0)

  PicPacket command;

  command.hdr.picMsg = PICCMD;
  command.hdr.size = sizeof(PicPacket);
  command.hdr.piccmd = cmd;

  memset((void *)&Message_Buffer, 0, sizeof(struct Message_Queue_Buff));

  Message_Buffer.mtype = PicMTypeSend;
  Message_Buffer.Msg_Info.From_Address = 3;
  Message_Buffer.Msg_Info.Command = CommandPassthru;
  memcpy((void *)&Message_Buffer.Msg_Info.data[0], (void *)&command, sizeof(PicPacket));

  int result = msgsnd(ID, &Message_Buffer, sizeof(struct Message_Queue_Buff), 0);

  if(result < 0)
    DEBUG() << "Set Power_Led_color FAILED";
}

int Utils::getDisplayUnits( void )
{
#ifdef HH1
  return 0;
#else
#ifdef LIDARCAM
  return m_lidarData->lidarStruct.DISPLAY_UNITS;
#else
   QString qs1 = "evidence1";
   backGround::get().createJson(&qs1);
#endif
#endif
}

#ifdef HH1
int Utils::connectRADARMEM()
{
  int RADARID;
  void *mem;
  
  if ((RADARID = shmget(RADAR_KEY, sizeof( RadarMemory ), 0777)) < 0) {
	if ((RADARID = shmget(RADAR_KEY, sizeof( RadarMemory ), IPC_CREAT | 0777)) < 0) {
	  printf("%s(%d): shmget failed\n\r", __FILE__, __LINE__);
	  return -1;
	}else{
	  printf("%s(%d): shmget GOOD created 0x%x(%d)\n\r", __FILE__, __LINE__, RADAR_KEY, RADAR_KEY);
	  mem = shmat(RADARID, NULL, 0);
	  if (mem == NULL ) {
        printf("%s(%d): shmat failed\n\r", __FILE__, __LINE__);
        return -1;
	  }
	  memset((void *)mem, 0, sizeof(RadarMemory));
	}
  }else{
	mem = shmat(RADARID, NULL, 0);
	if (mem == NULL ) {
	  printf("%s(%d): shmat failed\n\r", __FILE__, __LINE__);
	  return -1;
	}
  }
  
  printf("%s(%d): RadarMemory shmat GOOD %p ID 0x%x(%d)\n\r", __FILE__, __LINE__, mem, RADAR_KEY, RADAR_KEY);

  radarData = (RadarMemory *)mem;

  return 0;
}
#endif

// Playback and Record Interface
int Utils::MsgInit( int msgKey )
{
#if defined(LIDARCAM) || defined(HH1)
   int qid;
   key_t key = msgKey;

   qid = msgget(key,0);
   if(  qid < 0 )
   {
      qid = msgget(key,IPC_CREAT|0666);
   }

   return qid;
#endif
}

int Utils::SndApproMsg(int cmd, char *value, char *filename)
{
#ifdef IS_TI_ARM
   int msg_size;

   mMsgbuf.des = MSG_TYPE_MSG1;
   mMsgbuf.src = 17;	//PROC_MSG_ID;
   mMsgbuf.cmd = cmd;
   memcpy((void *)mMsgbuf.data, value, APPRO_DATA_LEN);
   mMsgbuf.speed = 0;
   mMsgbuf.range = 0;
   if (filename != NULL)
      memcpy((void *)mMsgbuf.filename, filename, sizeof(mMsgbuf.filename));
   else
      mMsgbuf.filename[0] = '\0';
   msg_size = sizeof(mMsgbuf);

   msgsnd(mApproId, &mMsgbuf, msg_size,0);

   msgrcv(mApproId, &mMsgbuf, msg_size, 17, 0);
#endif
   return 0;
}

void Utils::takePhoto(QString & fileName, int distance)
{
  char buf[512];

  memset((void *)buf, 0, sizeof(buf));
  
  mDistance = distance;
  
  long value = (long)fileName.toLatin1().data();
  memcpy( (void *)buf, (void *)((char *)value), strlen((char *)value));
#ifdef IS_TI_ARM
  sendCmdToCamera(CMD_SNAPSHOT, (int)buf);
#endif
}

#ifdef HH1
bool Utils::getRadarConfig(config_type* config)
{
    // Configuration settings should be stored in and read from a database table
    // The following is a temporary workaround until the database is implemented

    // Init the config variables
    config->port = serial;
#ifdef IS_TI_ARM
    strcpy(config->serialPort,     "/dev/ttyO1");
    strcpy(config->serialPortDev,  "/dev/ttyO1");
    strcpy(config->serialBaudRate, "921600");

#else
    //#warning "ttyUSB2 is used"
    //    strcpy(config->serialPort,     "/dev/ttyUSB2");
    //    strcpy(config->serialPortDev,  "/dev/ttyUSB2");
    //    strcpy(config->serialBaudRate, "460800");
    
    strcpy(config->serialPort,     "/dev/ttyS0");
    strcpy(config->serialPortDev,  "/dev/ttyS0");
    strcpy(config->serialBaudRate, "921600");
#endif
    config->radar_data_is_roadway = false;
    config->Xs = -2.0f;
    config->Zs = 1.7f;
    config->Zt = 1.0f;
    config->FocalLength = 25.0f;
    config->SensorWidth = 6.2f;   // IMX 172, pixel size = 1.55 u x 4000 pixels
    config->SensorHeight = 4.65f; // IMX 172, pixel size = 1.55 u x 3000 pixels
    config->FOVh = config->SensorWidth/config->FocalLength * 180/PI;  //(10.15 degrees)
    config->FOVv = config->SensorHeight/config->FocalLength * 180/PI; //(7.61 degrees)
    config->port = serial;
    config->num_to_show = 4;

    return true;
}
#endif

void Utils::setVolume( int vol )
{
  QString amixerCmd = QString("/usr/bin/amixer sset PCM ");
  amixerCmd.append( " ");
  QString percent = QString::number( vol );
  //  DEBUG() << "percent " << percent;
  amixerCmd.append( percent );
  amixerCmd.append( "%");
  
  //  DEBUG() << amixerCmd;
    
  QProcess doamixer;
  doamixer.start( amixerCmd );
  doamixer.waitForFinished(-1);
  int ret = doamixer.exitCode();
  
  if( ret != 0 ) {
    printf("%s(%d) ret = %d\n\r", __FILE__, __LINE__, ret);
  }

  return;
}

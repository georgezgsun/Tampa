#include <QApplication>
#include <QTimer>
#include <QObject>

#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "Message_Queue_Struct.h"
#include "Message_Queue_Keys.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include "PicMsg.h"
#include "state.h"
#include "hardButtons.h"
#include "global.h"
#include "top_view.h"
#include "debug.h"
#include "utils.h"
#include "ColdFireCommands.h"
#include "vkiline.h"
#include <QMessageBox>

#ifdef LIDARCAM
extern "C" {
#include "dm355_gio_util.h"
}
#endif
#define LidarKeyPad        "/dev/input/event1"
#define SWV1    32		// Qt programs function
#define SWV2    23		// Qt programs function
#define SWV3    33		// Qt programs function
#define SWV4    19		// Qt programs function

#define SWH1    37
#define SWH2    106
#define SWH3    18
#define SWH4    105

#define TRIG    34		// touches hardward to operate lazer


hardButtons::hardButtons( void )
{
    // Attach to the message queue
    key_t Key;
    Key = ftok(MESSAGE_QUEUE_FILE, MESSAGE_QUEUE_CHAR);
    MSG_ID = msgget(Key,0666);
    mKeyBoard = false;
    mEnabled = true;    // default is on
}

hardButtons::~hardButtons( void )
{
    mReadThreadFlag = 0;
    pthread_join(mReadThreadId, NULL);

    if (mDevFd != -1)
        close(mDevFd);
}

int hardButtons::initialization(void)
{
    int ret1;

    DEBUG() << "HardButton Initialization\n";
    mDevFd = open(LidarKeyPad, O_RDONLY);
    if (mDevFd == -1)
    {
       fprintf(stderr, "Open Hard Button failed. Errno: %s\n", strerror(errno));
       return -1;
    }

    mReadThreadFlag = 1;
    ret1 = 0;
    ret1 = pthread_create(&mReadThreadId, NULL, static_entry, (void *)this);
    if (ret1)
    {
        mReadThreadFlag = 0;
        fprintf(stderr, "Error - pthread_create() failure. Code: %d\n", ret1);
        close(mDevFd);
        return -1;
    }
    return 0;
}

void hardButtons::ReadThread(void)
{
    int ret;
    struct input_event ev;

    DEBUG() << "ReadThread Start!";
    while (mReadThreadFlag)
    {
        ret = read(mDevFd, &ev, sizeof(struct input_event));
        if (ret < (int)sizeof(struct input_event))
        {
            fprintf(stderr, "%s(%d) Read input device ERROR! %d\n", __FILE__, __LINE__, errno);
        }
        else
        {
            ProcessHardButtons(ev);
        }
    }
    printf("Read Thread Exit\n");
}

void hardButtons::ProcessHardButtons(struct input_event& ev)
{
  //Debugging printf to see what is happening on the keyboards
  //  printf("%s(%d) inputKey 0x%x ev.value 0x%x ev.code 0x%x(%d)\r\n", __FILE__, __LINE__, mInputKey, ev.value, ev.code, ev.code);

  Utils& u = Utils::get();

  // Special case so far TRIG is only key we need to know when it is released
  if( mInputKey != 0 )
  {
    
    mInputKey = 0;
    if ( ev.code == (unsigned char)TRIG)
    {
#ifdef LIDARCAM
      // at this point if in autotrigger ignore this
      Utils& u = Utils::get();
      struct Lidar_Buff *ptr = u.lidarDataBuf();
      LIDAR *mpLidar = &(ptr->lidarStruct);
      if( mpLidar->TRIGGER_MODE ) {
         return;
      }
#endif
      
      DEBUG() << "trigger clear ";
#ifdef LIDARCAM
      dm355_gio_write(61, 0);
#endif
      triggerPulled = false;
    }
    return;
  }

  // Ignore all button release event1, ev.value is zero when releaseed.
  // ev.value is 1 when pressed, and 2 when in autorepeat mode
  if( ev.value == 0 ) {
	return;
  }

  // Save it for the possible key release
  mInputKey = ev.code;
  
  // get the name from the list
  // nothing to do if NULL
  if( mInputKey == (unsigned char)SWV1)
  {
    mInputKey = 0;
    if( mEnabled == true && map[0] != NULL )
      map[0]->clicked();
    return;
  }
  if ( mInputKey == (unsigned char)SWV2)
  {
    mInputKey = 0;
    if( mEnabled == true && map[1] != NULL )
      map[1]->clicked();
    return;
  }
  if ( mInputKey == (unsigned char )SWV3)
  {
    mInputKey = 0;
    if( mEnabled == true && map[2] != NULL )
    {
        if (mKeyBoard == false)
            map[2]->clicked();  // regular push-button
        else
        {   // related to key board
            vkILine *pKeyBoard = (vkILine *)map[2];
            pKeyBoard->linePressed(pKeyBoard);
        }
    }
	return;
  }

  if ( mInputKey == (unsigned char)SWV4)
  {
    mInputKey = 0;
    if( mEnabled == true && map[3] != NULL )
      map[3]->clicked();
    return;
  }

  if(mInputKey == (unsigned char)SWH1)
  {
    mInputKey = 0;
    DEBUG() << "SWH1 Pressed\r\n";

#ifdef LIDARCAM
    // Portland this key is the brightness key.
    int Brightness = 0;
    Brightness = 20 * (u.lidarDataBuf()->lidarStruct.HUD_BRIGHTNESS - 1 );
    
    // Decrease the display brightness
    if(Brightness > 0)
      {
	Brightness -= 20;
	int data = Brightness / 20;
	if( Brightness < 20 ) {
	  data = 1;
	}else{
	  data++;
	}
	//	  DEBUG() << data << Brightness;
	u.setLidarMemory( HUD_BRIGHTNESS_ID, &data, NULL); 
	Send_Display_Brightness( Brightness );
      }
#endif
#ifdef HH1
    // tampa this is the power down key
    emit tv->powerDown();
#endif
    Send_Audio_Beep();
    return;
  }

  if(mInputKey == (unsigned char)SWH2)
  {
    mInputKey = 0;
    DEBUG() << "SWH2 Pressed\r\n";
#ifdef LIDARCAM
    int Brightness = 0;
    Brightness = 20 * (u.lidarDataBuf()->lidarStruct.HUD_BRIGHTNESS - 1);
    
    // Increase the display brightness
    if(Brightness < 100) {
      Brightness += 20;
      int data = Brightness / 20;
      if( Brightness < 20 ) {
	data = 1;
      }else{
	data++;
      }
      //	  DEBUG() << data << Brightness;
      u.setLidarMemory( HUD_BRIGHTNESS_ID, &data, NULL); 
      Send_Display_Brightness( Brightness );
    }
#else
    // hh1 volume handled here
    int vol = 0;
    
    // Get volume from the database
    SysConfig & cfg = u.getConfiguration();
    vol = cfg.volume;

    // Increment it, if greater than 4 set to 0
    vol++;

    if ( vol > 4 ) {
      vol = 0;
    }

    // amixer sset PCM 100%
    vol = vol * 25;

    u.setVolume( vol );
    
    // Save it to database
    cfg.volume = vol / 25;
    u.setConfiguration( cfg );
    
#endif
    Send_Audio_Beep();
    return;
  }

  if(mInputKey == (unsigned char)SWH3)
  {
    mInputKey = 0;
    DEBUG() << "SWH3 Pressed\r\n";

#ifdef LIDARCAM
    // Increase the audio volume
    struct Lidar_Buff * data = u.lidarDataBuf();

    if(data->lidarStruct.SPEAKER_VOLUME < 4)
    {
        Set_Audio_Volume(data->lidarStruct.SPEAKER_VOLUME + 1);
    }
#endif
    Send_Audio_Beep();
    return;
  }

  if(mInputKey == (unsigned char)SWH4)
  {
    mInputKey = 0;
    DEBUG() << "SWH4 Pressed\r\n";
#ifdef LIDARCAM
    // Decrease the audio volume
    struct Lidar_Buff * data = u.lidarDataBuf();

    if(data->lidarStruct.SPEAKER_VOLUME > 0)
    {
        Set_Audio_Volume(data->lidarStruct.SPEAKER_VOLUME - 1);
    }
#endif
    Send_Audio_Beep();
    return;
  }


  // Special need to wait for the release b4 clearing mInputKey
  if ( mInputKey == (unsigned char)TRIG) {
    Send_Audio_Beep();
    // ignore trigger if not logged in
    if( state::get().getState() < STATE_TOP_VIEW ) {
      mInputKey = 0;
      return;
    }
    // trigger does not work if tilted 
    if( u.tiltSeen() == true ) {
      mInputKey = 0;
      return;
    }
    
#ifdef LIDARCAM
    // At this point autotrigger might clear trigger pull
    // otherwise do the normal trigger
    Utils& u = Utils::get();
    struct Lidar_Buff *ptr = u.lidarDataBuf();
    LIDAR *mpLidar = &(ptr->lidarStruct);
    if( mpLidar->TRIGGER_MODE ) {
      if ( triggerPulled == true ) {
	dm355_gio_write(61, 0);
	triggerPulled = false;
	// DEBUG() << "trigger clear ";
	return;
      }
    }
#endif
    
    DEBUG() << "trigger set ";
    // DO NOT clear mInputKey, because it is used to see the button release
#ifdef LIDARCAM
    dm355_gio_write(61, 1);
#endif
    triggerPulled = true;
    return;
  }

  // keys above this only need to know clicked
  // below will wait for button release
  mInputKey = 0;

  return;
}

void hardButtons::setHardButtonMap( int i, QPushButton * s )
{
  //DEBUG() << "setHardButtonMap " << i << s;
  map[i] = s;
}

void hardButtons::Send_Display_Brightness( int Brightness )
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
  command.hdr.piccmd = Set_Display_Brightness;
  command.data.cntrl = Brightness;

  memset((void *)&Message_Buffer, 0, sizeof(struct Message_Queue_Buff));

  Message_Buffer.mtype = PicMTypeSend;
  Message_Buffer.Msg_Info.From_Address = 3;
  Message_Buffer.Msg_Info.Command = CommandPassthru;
  memcpy((void *)&Message_Buffer.Msg_Info.data[0], (void *)&command, sizeof(PicPacket));

  int result = msgsnd(MSG_ID, &Message_Buffer, sizeof(struct Message_Queue_Buff), 0);

  if(result < 0)
    printf("Set Display Brightness FAILED\r\n");
}

void hardButtons::Send_Audio_Beep()
{
#ifdef LIDARCAM
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
#else
  QProcess process;
  QString cmd(" /bin/sh -c \"/usr/bin/aplay /usr/local/stalker/TampaBeep.wav \"");
  //  DEBUG() << "BEEP" << "cmd " << cmd;
  QString stdout;
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  // stdout = process.readAllStandardOutput();
  // DEBUG() << stdout;
#endif
}

void hardButtons::Set_Audio_Volume(int volume)
{
  //  printf("Setting volume to a value of %d\r\n",volume);
#ifdef LIDARCAM
  Utils& u = Utils::get();
  u.setLidarMemory(SPEAKER_VOLUME_ID, &volume, NULL);
#else
  UNUSED( volume);
#endif
}
void hardButtons::settopView( topView *ptr )
{
  //DEBUG() << "setHardButtonMap " << i << s;
  tv = ptr;
}

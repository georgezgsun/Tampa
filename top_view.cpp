#include "top_view.h"
#include "ui_top_view.h"
#include <QDialog>
#include <QLineEdit>
#include <QGraphicsProxyWidget>
#include <QPen>
#include <QDesktopWidget>
#include <QtGlobal>
#include "menu_view.h"
#include "hardButtons.h"
#include "state.h"
#include <QApplication>
#include "debug.h"
#include "utils.h"
#include "Lidar_Buff.h"
#include "hardButtons.h"
#include "ColdFireCommands.h"
#include "PicMsg.h"
#include <QMessageBox>
#include <linux/i2c-dev.h>
#include <fstream>

#ifdef HH1
#include "widgets/icon_frame/icon_frame.h"
#endif
#include "db_types.h"
//#include "play_back.h"
//#include "ticket_view.h"
#include "printTicket.h"
#include "back_ground.h"
#include "hh1MetaData.h"

extern QElapsedTimer sysTimer;
extern CRadarData RadarData;

float topSpeed;
int backLightTime = 0;
int powerOffTime = 0;

#ifdef LIDARCAM
//int zoomFt[] = { 18,35,53,71,88,106,123,141,159,176,194,212,229,247,264,282,300,317 };
int focusFt[] = { 100, 175, 250, 300, 500, 650 };
//float zoomM[] = { 4.7,9.4,14.1,18.8,23.5,28.2,32.9,37.6,42.3,47,51.7,56.4,61.1,65.8,70.5,75.2,79.9,84.6};
float focusM[] = { 30, 50, 75, 100, 150, 200};
//int zoomValue[] = {1000, 2000, 5000, 10000, 13000, 18000};
int zoomValue[] = {3000, 5000, 7000, 9000, 14000, 18000};
#endif

bool iconFrameShow = false;
bool printButtonActive = false;

QString displayFileName;

void* StartRadarPoll(void* radarDataArgs);

void* PlaybackThread(void* radarDataArgs);

QString dNumber( QString s, float f)
{
  QString tmp = QString("    \"stalker-");
  tmp.append(s);
  tmp.append("\": \"");
  tmp.append( QString::number(f));
  tmp.append("\",\n");;
  return tmp;
}

pthread_t radarPollId, playbackThreadId;

FILE * capture_fd;

//#define SPEED_INROOM_DEBUG

#ifdef JUNK
static void hexDump(char *desc, void *addr, int len)
{
  int i;
  unsigned char buff[17];
  unsigned char *pc = (unsigned char*)addr;
  
  // Output description if given.
  if (desc != NULL)
    printf ("%s:  len 0x%x\n", desc, len);
  
  // Process every byte in the data.
  int cnt = 0;
  for (i = 0; i < len; i++) {
    // Multiple of 16 means new line (with line offset).
    
    if( cnt > 0 ) {
      cnt--;
      continue;
    }
    if ((i % 16) == 0) {
      // Just don't print ASCII for the zeroth line.
      if (i != 0) {
	if( buff[0] != 0 ) {
	  printf("  %s\n", buff);
	  memset(buff,0,17);
	}
      }
      
      // Check to see if line of zero's
      int k;
      cnt = 0;
      for(k=0;k<16;k++){
	if(pc[i+k] == 0 ) {
	  cnt++;
	}
      }
      if( cnt == 16 ) {
	cnt--;
	continue;
      }else{
	cnt = 0;
      }

      // Output the offset.
      printf("  %04x ", i);
    }
    
    // Now the hex code for the specific character.
    printf(" %02x", pc[i]);
    
    // And store a printable ASCII character for later.
    if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
      buff[i % 16] = '.';
    } else {
      buff[i % 16] = pc[i];
    }
    
    buff[(i % 16) + 1] = '\0';
  }
  
  // Pad out last line if not exactly 16 characters.
  while ((i % 16) != 0) {
    printf("   ");
    i++;
  }
  
  // And print the final ASCII bit.
  printf("  %s\n", buff);
}
#endif

topView::topView() :
    ui(new Ui::topView)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_AcceptTouchEvents);
    DEBUG() << "Completed setAttribute() in topView constructor";

    Utils& u = Utils::get();

#ifdef HH1
    // get RadarMemory
    mpRadarData = (RadarMemory *)u.RADARBuf();
    //    DEBUG() << mpRadarData;
    radarPollId = 0;

    HH1MetaData& md = HH1MetaData::get();
    md.reset();
#endif

    this->initVariables();

    //add left frame
    if( m_iconFrame ) {
      delete m_iconFrame;
      m_iconFrame = NULL;
    }
    
    m_iconFrame = new iconFrame(40, 272, 5);

    if ( state::get().getState() == STATE_START) {
      this->openLoginScreen();
    }else{
      this->openTopScreen();
    }

    this->setWindowTitle(QString(APP_NAME));
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    ui->pb_zoomin->setText("");
    ui->pb_focus->setText("");
    
    struct Location loc;
    loc = u.getCurrentLoc();
    
    // Display zoom value on top screen
    
    int percent = u.FGBuf()->State_Of_Charge;
    
    DEBUG() << "Battery Percent " << percent;

#ifdef LIDARCAM
    if( percent > 10 ) {
      u.Send_Msg_To_PIC( Set_Power_Led_Green );
    }else{
      u.Send_Msg_To_PIC( Set_Power_Led_Red );
    }
#endif
    
    this->m_view->installEventFilter(this);

    ui->pb_displayjpg->setStyleSheet("background-color: rgba(0, 0, 0, 0); border: rgba(0, 0, 0, 0) ;");
    ui->lb_autoTitle->setStyleSheet("color: yellow; font: BOLD 8pt;");
    ui->lb_limitTitle->setStyleSheet(ui->lb_autoTitle->styleSheet());
    ui->lb_zoomTitle->setStyleSheet(ui->lb_autoTitle->styleSheet());
    ui->lb_auto->setStyleSheet("color: yellow; font: BOLD 10pt;");
    ui->lb_limit->setStyleSheet(ui->lb_auto->styleSheet());
    ui->lb_zoom->setStyleSheet(ui->lb_auto->styleSheet());

    //    QTimer::singleShot(1000, this, SLOT(initProcesses()));
    initProcesses();

    connect( this, SIGNAL(powerDown()), this, SLOT(powerDownSystem()) );
    connect( this, SIGNAL(hh1Trigger()), this, SLOT(hh1TriggerPulled()) );
    connect( this, SIGNAL(hh1Home()), this, SLOT(reopenTopScreen()) );
    connect( this, SIGNAL(lowBatterySignal()), this, SLOT(lowBattery()) );

    hardButtons& uu = hardButtons::get();
    uu.settopView( this );
    
    int vol = 0;
    // Get volume from the database
    SysConfig & cfg = u.getConfiguration();
    vol = cfg.volume;

    // amixer sset PCM 100%
    vol = vol * 25;

    u.setVolume( vol );

    unsigned int response;

    // Make sure radar transmit is off
    mpRadarData->Data.RadarMode.transmit = 0;
    CRadarData &RadarData = backGround::get().getRadarData();	// This object provides the Oculii radar interface
    DEBUG() << "Turn off radar transmit";
    RadarData.Message4(&mpRadarData->Data.RadarMode);  // Set the Radar Mode
    while((response = RadarData.CheckResponse()) == 0);   
}

topView::~topView()
{
  unsigned int response;

  //    DEBUG("Running topView destructor for object %08lx\n", (long unsigned int)this);
  if (m_TopViewTimer) {
    m_TopViewTimer->stop();
    delete m_TopViewTimer;
  }

  mpRadarData->Data.RadarMode.transmit = 0;
  CRadarData &RadarData = backGround::get().getRadarData();	// This object provides the Oculii radar interface
  DEBUG() << "Turn off radar transmit";
  RadarData.Message4(&mpRadarData->Data.RadarMode);  // Set the Radar Mode
  while((response = RadarData.CheckResponse()) == 0);

#ifdef HH1
  pthread_t radarPollIdSave = radarPollId;
  radarPollId = 0;
  if( radarPollIdSave != 0 ) {
    pthread_join( radarPollIdSave, NULL );
  }
#endif

  delete ui;

  //logFile->close();
  logDebug.close();
}

//
//public functions
//

#ifdef HH1
void topView::openHomeScreen(int i)
{
   DEBUG() << "open Home Screen " << i;

   if (i != 1 && i != REOPEN_HOME_SCREEN)
      return;

   reopenTopScreen();
	
   return;
}

// Related to 'operat_frames.cpp'
void topView::switchScreen(int flag)
{
  UNUSED( flag );
  
  //  DEBUG() << "switch opr screen flag " << flag << "State " << state::get().getState();
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
  emit hh1Home();
  return;  
}

void topView::openOperateScreen(void)
{
  unsigned int response;
  
  // DEBUG() << "openOperateScreen " << "State " << state::get().getState();
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  // This code sets up the theta_hs_ref, theta_rs_ref and theta_vs_ref
  pSensor->ReadSensor(&theta_vs_ref, &theta_rs_ref, &theta_hs_ref);
  
  if(theta_hs_ref > PI) theta_hs_ref = PI - theta_hs_ref;
  else if(theta_hs_ref < -PI) theta_hs_ref = PI + theta_hs_ref;
  
  if(theta_rs_ref > PI) theta_rs_ref = PI - theta_rs_ref;
  else if(theta_rs_ref < -PI) theta_rs_ref = PI + theta_rs_ref;
  
  if(theta_vs_ref > PI) theta_vs_ref = PI - theta_vs_ref;
  else if(theta_vs_ref < -PI) theta_vs_ref = PI + theta_vs_ref;
  
  //  printf("Sensor Relative Ref: ");
  //  printf("hs %f rs %f vs %f\n", theta_hs_ref, theta_rs_ref, theta_vs_ref);
  
  mpRadarData->Data.RadarMode.trackingDirection = topView_mConf.direction;
  mpRadarData->Data.RadarMode.transmit = 1;
  CRadarData &RadarData = backGround::get().getRadarData();	// This object provides the Oculii radar interface
  DEBUG() << "Turn on radar transmit";
  RadarData.Message4(&mpRadarData->Data.RadarMode);  // Set the Radar Mode
  while((response = RadarData.CheckResponse()) == 0);

  if (!m_oprScene) {
    m_oprScene = new QGraphicsScene;
  }
  m_view->setScene(m_oprScene);
  if( m_iconFrame) {
    m_iconFrame->hide();
  }
  if ( m_oprFrames ) {
    m_oprFrames->setCurrentIndex(0);
  }
  state& v = state::get();
  v.setState(STATE_OPERATING);
  
  m_currScene = m_oprScene;
  
  if (!m_oprScene) {
    qCritical("memory request failed");
    Q_ASSERT_X(0, "openOperatScene", "memory request failed");
  }
  if (!m_view) {
    qCritical("memory request failed");
    Q_ASSERT_X(0, "openOperatScene", "No m_view memory request failed");
  }
  
  //set background color
  QBrush b (QColor(0, 0, 0, 255));
  m_oprScene->setBackgroundBrush(b);
  m_oprScene->setSceneRect(0, 0, FULL_WIDTH, FULL_HEIGHT);
  
  //add right frame
  if ( m_oprFrames ) {
    delete m_oprFrames;
    m_oprFrames = NULL;
  }
  
  m_oprFrames = new OperatFrames;
  
  connect( m_oprFrames, SIGNAL(sig_oprScreenReq(int)), this, SLOT(switchScreen(int)) );
  connect( this, SIGNAL(sig_topViewScreenReq(int)), this, SLOT(switchScreen(int)) );
  m_oprFramesProxy = m_oprScene->addWidget(m_oprFrames);
  m_oprFramesProxy->setPos(360, 0);
  

  if( m_iconFrame ) {
    delete m_iconFrame;
    m_iconFrame = NULL;
  }
  
  m_iconFrame = new iconFrame(40, 272, 5);
  
  QGraphicsProxyWidget *proxyw = m_oprScene->addWidget(m_iconFrame);
  proxyw->setPos(0, 10);
  m_iconFrame->hide();
  
  //update view
  this->showFullView();
  m_view->setScene(m_oprScene);
  state::get().setState(STATE_OPERATING );
  //delete m_currScene and its widgets
  m_currScene = m_oprScene;
  m_view->show();
  
  //poll radar data
  connect(this, SIGNAL(handleTargets()), this, SLOT(processTargets()));
  
  radarDataArgs_t *pRadarDataArgs = backGround::get().getRadarDataArgs();
  pRadarDataArgs->menuClass = (void *)(this);
  err = pthread_create(&(radarPollId), NULL, &StartRadarPoll, (void *)pRadarDataArgs);
  //  DEBUG() << "StartRadarPoll " << err;
  if (err != 0)
  {
    DEBUG() << "Can't create StartRadarPoll thread " <<  strerror(err);
    Q_ASSERT(0);
  }
  //  DEBUG() << "Ok";
  return;
}

#endif

#ifdef LIDARCAM
void topView::drawZoomSquare(int zoomRatio)
{
    static int ratio;
    if (m_zoomSquare && ratio == zoomRatio)
        return;
    ratio = zoomRatio;
    int rWidth, startX, startY, retv = -1;

    Q_ASSERT(m_topScene);

    FILE *fp1 = fopen("/usr/local/stalker/laser.cfg", "rb");
    if (fp1 != NULL)
    {
        char str1[32];
        fgets(str1, sizeof(str1), fp1);
        if (sscanf(str1, "%d %d %d", &startX, &startY, &rWidth) == 3)
        {
            int maxWidth = m_topScene->width();
            int maxHeight = m_topScene->height();
            maxWidth -= rWidth;
//            maxHeight -= rWidth;
            if (startX >= 0 && startX <= maxWidth &&
                startY >= 0 && startY <= maxHeight && rWidth <= 60)
            {
                rWidth *= zoomRatio;
//                printf("New Laser Box: X->%d, Y->%d, W->%d\n", startX, startY, rWidth);
                retv = 0;
            }
        }
    }

    if (retv)
    {   // Use default
        rWidth = 40 * zoomRatio;  //40 pixel, 1cm?
        startX = m_topScene->width() / 2;
        startY = m_topScene->height() / 2;
    }

    if (m_zoomSquare)
    {
        m_topScene->removeItem(m_zoomSquare);
        delete m_zoomSquare;
        m_zoomSquare = NULL;
    }

    const QRect r (startX - (rWidth / 2), startY - (rWidth / 2), rWidth, rWidth);
    QPen p(Qt::red);
    m_zoomSquare = m_topScene->addRect(r, p);
    m_topScene->update();
}
#endif

void topView::updateRecordView(bool endRecord)
{
  UNUSED( endRecord );
  
  Q_ASSERT(m_topScene);
  
  if (!m_recordMark)
  {
    QRect r(10, 14, 12, 12);
    m_recordMark = m_oprScene->addEllipse(r);
    m_recordMark->setBrush(Qt::red);
    m_recordMark->setPos(6, 6);
    m_recordMark->stackBefore(m_oprFramesProxy);
  }else{
    m_oprScene->removeItem(m_recordMark);
    delete m_recordMark;
    this->m_recordMark = NULL;
  }
}

void topView::displayRange( float distance )
{
    UNUSED(distance);
#ifdef LIDARCAM
    Q_ASSERT(m_topScene);
    QString d;
    
	// TODO rework so DISPLAY_RESOLUTION sand DISPLAY_UNITS to database
	// TODO rework PicTask to put speed and range into shared memory so both lidarCam and Appro can
	// display speed and range in video and
	// TODO determine if display range and speed needed anymore, just displayed in video
	Utils& u = Utils::get();
	int percision = 0;
	if( u.lidarDataBuf()->lidarStruct.DISPLAY_RESOLUTION.bits.RANGE ) {
	  // tenth will be displayed
	  percision = 1;
	}else{
	  // tenths will not be displayed
	  percision = 0;
	}
	switch( u.lidarDataBuf()->lidarStruct.DISPLAY_UNITS ) {
	case 0: // MPH
	case 2: // KNOTS
	  d = QString("<font color=red><font size=12><font weight=75>%1</font>").arg(distance,0,'f', percision,'0');
	  d.append( QString("<font color=red><font size=\"5\"><font weight=75> FT</font>"));
	  break;
	case 1: // km/h
	  d = QString("<font color=red><font size=12><font weight=75>%1</font>").arg(distance,0,'f', percision,'0');
	  d.append( QString("<font color=red><font size=\"5\"><font weight=75> M</font>"));
	  break;
	default:
	  break;
	}
	//	DEBUG() << "percision " << percision << " " << d;
    if (!m_distance) {
        m_distance = new QGraphicsTextItem;
        m_distance->setHtml(d);
        m_distance->setPos(250, 4);
        m_topScene->addItem(m_distance);
    } else {
        m_distance->setHtml(d);
	}
#endif
}
void topView::displaySpeed( float speed)
{
    UNUSED(speed);
#ifdef LIDARCAM
    Q_ASSERT(m_topScene);
    QString s;
    
	Utils& u = Utils::get();
	int percision = 0;
	if( u.lidarDataBuf()->lidarStruct.DISPLAY_RESOLUTION.bits.SPEED ) {
	  // tenth will be displayed
	  percision = 1;
	}else{
	  // tenths will not be displayed
	  percision = 0;
	}
	switch( u.lidarDataBuf()->lidarStruct.DISPLAY_UNITS ) {
    case 0: // MPH
	  s = QString("<font color=red><font size=12><font weight=75>%1</font>"  ).arg(speed,0,'f',percision,'0');
	  s.append( QString("<font color=red><font size=\"5\"><font weight=75> MPH</font>"  ));
	  break;
	case 1: // km/h
	  s = QString("<font color=red><font size=12><font weight=75>%1</font>" ).arg(speed,0,'f',percision,'0');
	  s.append( QString("<font color=red><font size=\"5\"><font weight=75> km/h</font>" ));
	  break;
	case 2:
	  s = QString("<font color=red><font size=12>%1<font weight=75></font>").arg(speed,0,'f',percision,'0');
	  s.append( QString("<font color=red><font size=\"5\">%1<font weight=75> KNOTS</font>"));
	  break;
	default:
	  break;
	}
	//	DEBUG() << "percision " << percision << " " << s;

    if (!m_speed) {
        m_speed = new QGraphicsTextItem;
        m_speed->setHtml(s);
        m_speed->setPos(50, 4);
        m_topScene->addItem(m_speed);
    } else{
	  m_speed->setHtml(s);
	}
#endif
}

void topView::closeEvent(QCloseEvent *e)
{
    if (m_vkb)
        m_vkb->hide(true);

    qDebug() << "close event";

    this->m_vkb = NULL;

    QWidget::closeEvent(e);
}

int topView::initVariables()
{
    Utils& u = Utils::get();

    this->m_startScene = NULL;
    this->m_loginScene = NULL;
    this->m_topScene = NULL;
    this->m_view = this->ui->gv_cam;
    this->m_loginSuccess = 0;
    this->m_loginName = QString("");
    this->m_passWord = QString("");
    this->m_TopViewTimer = NULL;
    this->m_le_passwd = NULL;
    this->m_le_userName = NULL;
    this->m_fullRect = new QRect(0, 0, this->width(), this->height());
    this->m_camRect = new QRect(0, 0, CAM_VIEW_WIDTH, this->height());
    this->m_homeScene = NULL;

    this->m_oprFramesProxy = NULL;
    this->m_currProxyWidget = NULL;
    this->m_command = CMD_NONE;
    this->m_prevCmd = CMD_NONE;
    this->m_vkb = NULL;
    this->m_zoomSquare = NULL;
    this->m_recordMark = NULL;
    this->m_speed = NULL;
    this->m_distance = NULL;
    memset( (void *)&RadarConfig, 0, sizeof(RadarConfig));
    mSpeed = 0.0;
    m_range = 0.0;
    m_newRange = 0.0;
    mNewSpeed = 0.0;
    mRecordingSecs = 0;
    mAutoRecording = false;
    mPhotoNum = 0;
    
#ifdef LIDARCAM
    struct Lidar_Buff *ptr = u.lidarDataBuf();
    LIDAR *pLidar = &(ptr->lidarStruct);
    if (pLidar->SPECIAL_MODES.bits.FORCE_R_DISPLAY ||
        pLidar->SPEED_RANGE_DISPLAY == 2 ||
        pLidar->SPEED_RANGE_DISPLAY == 5)
        mRangeOnly = true;
    else
        mRangeOnly = false;
#else
    mRangeOnly = false;
#endif

    // Will rotate the menu 180 degrees
    //    this->m_view->rotate(180);
    
    topSpeed = 0.0;
    
    // set backLightTimer and powerOffTimer
    topView_mConf = u.getConfiguration();
    backLightTime = topView_mConf.backlightOff * 60;  // Time is in seconds, config is minutes, 
    powerOffTime = topView_mConf.powerOff * 60; // Time is in seconds, config is minutes, 
    
#ifdef HH1
    m_rollLine = NULL;
    m_oprScene = NULL;
    m_iconFrame = NULL;
    m_oprFrames = NULL;
    m_targetRect1= NULL;
    m_targetRect2= NULL;
    m_targetRect3= NULL;
    m_targetRect4= NULL;

    Utils::get().getRadarConfig(&RadarConfig);
    
    RadarConfig.radar_data_is_roadway = false;
    RadarConfig.port = serial;
    RadarConfig.num_to_show = 4;
	
    //zxm added
    frameRate=15;
    frameCnt = 0;
    json_md_fd = 0;
    m_playbackScreen=false;

    // Set up 4 radar data display box
    QColor color = QColor(255, 128, 128, 255);
    QPen pen(color);
    mP1 = pen;
    mP1.setWidth(1);
    color = QColor(255, 255, 0, 255);
    pen.setColor(color);
    mP2 = pen;
    mP2.setWidth(1);
    color = QColor(204, 102, 255, 255);
    pen.setColor(color);
    mP3 = pen;
    mP3.setWidth(1);
    color = QColor(0, 204, 255, 255);
    pen.setColor(color);
    mP4 = pen;
    mP4.setWidth(1);

    color = QColor(0, 0, 0, 255);
    pen.setColor(color);
    mP0 = pen;
    mP0.setWidth(1);

    mRect.setRect(0,0,0,0);
    mRect1.setRect(0,0,0,0);
    mRect2.setRect(0,0,0,0);
    mRect3.setRect(0,0,0,0);
    mRect4.setRect(0,0,0,0);

    RadarConfig.Xs = -2.0f;
    RadarConfig.Zs = 3.0f;
    RadarConfig.FocalLength = 35.0f;
    RadarConfig.SensorWidth = 6.2f;   // IMX 172, pixel size = 1.55 u x 4000 pixels
    RadarConfig.SensorHeight = 4.65f; // IMX 172, pixel size = 1.55 u x 3000 pixels
    RadarConfig.FOVh = RadarConfig.SensorWidth/RadarConfig.FocalLength * 180.0/PI;  //(10.15 degrees)
    RadarConfig.FOVv = RadarConfig.SensorHeight/RadarConfig.FocalLength * 180.0/PI; //(7.61 degrees)
	
    // Init the coordinate transformations
    Transforms.InitCoordTransforms(RadarConfig.Xs,
                                   RadarConfig.Zs,
                                   RadarConfig.Zt,
                                   RadarConfig.FOVh * PI/180.0f,      // Convert to radians
                                   RadarConfig.FOVv * PI/180.0f);     // Convert to radians


    // Init the Radar Orientation
#ifdef IS_TI_ARM
    if(mpRadarData)
    {
        mpRadarData->Data.RadarOrientation.azimuthAngle = 0.0f;
        mpRadarData->Data.RadarOrientation.elevationAngle = 0.0f;
        mpRadarData->Data.RadarOrientation.rollAngle = 0.0f;
        mpRadarData->Data.RadarOrientation.radarHeight = RadarConfig.Zs;
        mpRadarData->Data.RadarOrientation.targetHeight = RadarConfig.Zt;
    }
    else
    {
        DEBUG() << "Shared memory pointer is null";
    }
#endif
#endif
	
    return 0;
}

void topView::connectSignals()
{
  hardButtons& u = hardButtons::get();

  u.EnableHardButtons(true);
#ifdef LIDARCAM
  connect( ui->pb_zoomin, SIGNAL(clicked()), this, SLOT(zoomin()) );
  u.setHardButtonMap( 0, ui->pb_zoomin);
  connect( ui->pb_focus, SIGNAL(clicked()), this, SLOT(focusMode()) );
  u.setHardButtonMap( 1, ui->pb_focus);
#endif
  connect( ui->pb_menu, SIGNAL(clicked()), this, SLOT(openMenuScreen()) );
  u.setHardButtonMap( 2, ui->pb_menu);

#ifdef HH1
  u.setHardButtonMap( 0, NULL );
  u.setHardButtonMap( 1, NULL );
//  DEBUG() << "connectSignals pb_record to openOperateScreen " << "State " << state::get().getState();
  connect( ui->pb_record, SIGNAL(clicked()), this, SLOT(openOperateScreen()) );
  u.setHardButtonMap( 3, ui->pb_record);
  ui->pb_record->setText("START");
#else
  connect( ui->pb_record, SIGNAL(clicked()), this, SLOT(exeRecord()) );
  u.setHardButtonMap( 3, ui->pb_record);
#endif
  
  if (m_vkb != NULL)
    disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(closeVKB()));  // new keyboard

  connect( ui->pb_displayjpg, SIGNAL(clicked()), this, SLOT(displayJPG()) );
}

void topView::create_vkb()
{
   Utils& u = Utils::get();
   // u.creatVKB();  moved to main.cpp
   m_vkb = u.vkb();
   connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(closeVKB()), Qt::UniqueConnection);
}
 
void topView::emitHandleTargets()
{
  if (state::get().getState() == STATE_OPERATING || state::get().getState() == STATE_OPERATING_MENU) {
    emit handleTargets();
  }
}

int topView::initLoginScene()
{
    if (!this->m_loginScene)
        m_loginScene = new QGraphicsScene;

    if (!m_vkb)
        this->create_vkb();
	
#ifdef HH1
    QPixmap ico(":/logo/Splashpng");
#else
    QPixmap ico(":/logo/Splash");
#endif
    
    QGraphicsPixmapItem *gPItem = this->m_loginScene->addPixmap(ico);
    gPItem->setPos(0, 0);

    this->m_loginScene->setSceneRect(0,0,this->width(),this->height());

    QWidget *dlg = new QWidget();
    QLabel *nameLable = new QLabel("User Name");
    vkILine *nameEdit = new vkILine();
    nameEdit->setObjectName("User Name");
    QLabel *passwdLabel = new QLabel("Password");
    vkILine *passwdEdit = new vkILine();
    passwdEdit->setObjectName("Pass Word");
    QPushButton *login = new QPushButton("Login");

    QGridLayout *vBox = new QGridLayout(dlg);
    vBox->addWidget(nameLable, 0, 0, Qt::AlignLeft);
    vBox->addWidget(nameEdit, 0, 1, Qt::AlignLeft);
    vBox->addWidget(passwdLabel, 1, 0, Qt::AlignLeft);
    vBox->addWidget(passwdEdit, 1, 1, Qt::AlignLeft);
    vBox->addWidget(login, 2, 1, Qt::AlignRight);
	ui->pb_displayjpg->setVisible(false);

#ifdef QUICK_LOGIN_SIM
    int i;
    m_qkcTimeLimit = 0;

    //add items for quick code status
    QLabel *qkcLabel = new QLabel ("Quick Code");
    m_qkcSlider = new QSlider(Qt::Horizontal);
    m_qkcSlider->setMaximum(4);
    m_qkcSlider->setMinimum(0);
    m_qkcSlider->setFixedWidth(144);
    m_qkcSlider->setValue(0);
    vBox->addWidget(qkcLabel, 3, 0, Qt::AlignLeft);
    vBox->addWidget(m_qkcSlider, 3, 1, Qt::AlignLeft | Qt::AlignVCenter);

    m_qkcPbFrame = new QFrame;
    m_qkcPbFrame->setFixedWidth(50);
    QVBoxLayout *v = new QVBoxLayout(m_qkcPbFrame);
	hardButtons& u = hardButtons::get();

	for (i = 0; i< 4; i++) {
        QString s = QString("p%1").arg(i+1);
        m_qkcButton[i] = new QPushButton(s);
        m_qkcButton[i]->setObjectName(s);
        v->addWidget(m_qkcButton[i]);
        connect(m_qkcButton[i], SIGNAL(clicked()), this, SLOT(setQuickCode()));
		u.setHardButtonMap( i, m_qkcButton[i]);
    }

    QGraphicsProxyWidget *p = this->m_loginScene->addWidget(m_qkcPbFrame);
    p->resize(50, 272);
    p->setPos(430, 0);
#endif

	
    m_le_userName = nameEdit;
    m_le_passwd = passwdEdit;

    passwdEdit->setEchoMode(QLineEdit::Password);
    connect( nameEdit, SIGNAL(textChanged(QString)), this, SLOT(setUserName(QString)) );
    connect( passwdEdit, SIGNAL(textChanged(QString)), this, SLOT(setPassWord(QString)) );
    //connect( passwdEdit, SIGNAL(returnPressed()), this, SLOT(exeLogin()) );
    connect(m_le_userName, SIGNAL(linePressed(vkILine*)), this, SLOT(toggleVKB(vkILine*)));
    connect(m_le_userName, SIGNAL(lineFocused(vkILine*)), this, SLOT(focusLine(vkILine*)));
    connect(m_le_passwd, SIGNAL(linePressed(vkILine*)), this, SLOT(toggleVKB(vkILine*)));
    connect(m_le_passwd, SIGNAL(lineFocused(vkILine*)), this, SLOT(focusLine(vkILine*)));

    connect( login, SIGNAL(clicked()), this, SLOT(exeLogin()) );
    connect( this, SIGNAL(clearLogin()), this, SLOT(clearLoginInfo()) );

    QGraphicsProxyWidget *proxy = this->m_loginScene->addWidget(dlg);

#ifdef QUICK_LOGIN_SIM
    proxy->resize(280, 160);
    proxy->setPos(120, 40);
#else
    proxy->resize(280, 100);
    proxy->setPos(120, 80);
#endif
    m_currProxyWidget = proxy;

    return 0;
}

int topView::initTopScene()
{
    if (!m_topScene)
        this->m_topScene = new QGraphicsScene;
    else{
        //top screen should not be deleted
        return 1;
    }

    Q_ASSERT(m_topScene);

    QGraphicsProxyWidget *proxyw = m_topScene->addWidget(m_iconFrame);
    proxyw->setPos(0, 10);
    m_iconFrame->hide();

    //set background color to black
    QColor c(0, 0, 0, 255);
    QBrush bgB(c);
    m_topScene->setBackgroundBrush(bgB);
    m_topScene->setSceneRect(0, 0, CAM_SCENE_WIDTH, this->height());

#ifdef LIDARCAM
    this->drawZoomSquare(1);   // Ticket 21230
#endif

    QPalette palZoom = ui->lb_zoom->palette();
        // foreground color
    palZoom.setColor(palZoom.WindowText, QColor(255, 255, 0));
    //th    palDate.setColor(palDate.WindowText, QColor(255, 255, 64));
        // "light" border
    //th    palTime.setColor(palTime.Light, QColor(255, 255, 0));

        // set the palette
    ui->lb_zoom->setPalette(palZoom);
    ui->lb_zoomTitle->setPalette(palZoom);
    ui->lb_limit->setPalette(palZoom);
    ui->lb_limitTitle->setPalette(palZoom);
    ui->lb_auto->setPalette(palZoom);
    ui->lb_autoTitle->setPalette(palZoom);

    QFont f( "Verdana", 10);
    ui->lb_autoTitle->setFont(f);
    ui->lb_limitTitle->setFont(f);
    ui->lb_zoomTitle->setFont(f);

//    QString arrow;
//    arrow = QChar(0x25b2);
//    ui->lb_arrowu->setText(arrow);
//    ui->lb_arrowu->setPalette(palZoom);
//    arrow = QChar(0x25bc);
//    ui->lb_arrowd->setText(arrow);
//    ui->lb_arrowd->setPalette(palZoom);

    return 0;
}

int topView::hideTopPanel()
{
    ui->pb_menu->parentWidget()->setVisible(false);

    return 0;
}

int topView::showTopPanel()
{
    QWidget *p = ui->pb_menu->parentWidget();
    p->setVisible(true);

    return 0;
}

int topView::showFullView()
{
    this->hideTopPanel();

    this->m_view->setVisible(false);
    this->m_view->setFixedSize(this->width(), this->height());
    this->m_view->setGeometry(*m_fullRect);
    this->m_view->setVisible(true);
    return 0;
}

int topView::showTopView()
{
    this->m_view->setVisible(false);
    this->m_view->setFixedSize(CAM_VIEW_WIDTH, this->height());
    this->m_view->setGeometry(*m_camRect);
    this->m_view->setVisible(true);

    this->showTopPanel();

    return 0;
}

int topView::hideTopView()
{
    this->m_view->setVisible(false);
    this->hideTopPanel();
    return 0;
}

void topView::TopViewInitTimer()
{
    m_TopViewTimer = new QTimer(this);
    //    m_TopViewTimer->setInterval(TIMTER_INTERVAL);
    connect(m_TopViewTimer, SIGNAL(timeout()), this, SLOT(TopViewTimerHit()));
    m_TopViewTimer->start( 67 );
}

void topView::TopViewTimerHit()
{
  state& v = state::get();
  
  if(v.getPlaybackState())
  {
    if(!m_playbackScreen)
    {
      m_playbackScreen = true;
      hardButtons::get().setHardButtonMap( 0, NULL);
      hardButtons::get().setHardButtonMap( 1, NULL);
      hardButtons::get().setHardButtonMap( 2, NULL);
      hardButtons::get().setHardButtonMap( 3, NULL);
      if(m_oprFrames==NULL)
		m_oprFrames = new OperatFrames;
      m_oprFrames->show();
      m_oprFrames->move(360,0);
      
      radarDataArgs_t *pRadarDataArgs = backGround::get().getRadarDataArgs();
      pRadarDataArgs->menuClass = (void *)(this);
      if(pthread_create(&(playbackThreadId), NULL, &PlaybackThread, (void *)pRadarDataArgs))
      {
		DEBUG() << "Can't create PlaybackThread thread ";
		Q_ASSERT(0);
      }
    }
    updateEvidence(m_oprFrames);
  }else{
    if(m_playbackScreen) PlaybackExit();
  }
  
  frameRate++;
  
  if(frameRate>=15) //once every second
  {
    frameRate=0;
    
    Utils& u = Utils::get();
    currentSeconds++;
    if (true == triggerPulled ) {
#ifdef LIDARCAM
      //  DEBUG() << "Elapsed " << sysTimer.elapsed() << "CurrentSeconds " << currentSeconds;
      // monitor speed and record if over the speed limit
      // every thing happens when the trigger is Pulled
      lidarCamTriggerPulled();
#else
      //    hh1TriggerPulled(); // Do not do anything based on timerHit for hh1TriggerPulled, 
#endif // LIDARCAM
    }else{
#ifdef LIDARCAM
      lidarCamTriggerNotPulled();
#else
      //    hh1TriggerNotPulled(); // Do not do anything based on timerHit for hh1TriggerNotPulled
#endif // LIDARCAM
    }
    
    if ( true == m_recording ) {   // recording is going     
#ifdef LIDARCAM
      int zoomDis;
      int zoomNum = mCamSetting.zoom.toInt();
      int displayUnits = u.getDisplayUnits();
      if (mCamSetting.focus == (QString)"AUTO")	{  // Auto focus mode
	if (displayUnits == 1)  // kmH
	  zoomDis = focusM[zoomNum-1];
	else  // MPH or KNOTS
	  zoomDis = focusFt[zoomNum-1];
      }	else {  // Manual focus mode
	zoomDis = mCamSetting.focus1;
      }
      
      if (!mPhotoNum && m_range <= (zoomDis + 10)) {
	QString fileName = u.getRecordingFileName() + "_1.jpg";
	u.takePhoto(fileName, m_range);
	mPhotoNum++;
      }
#endif
      // how long the recording has been going?
      mRecordingSecs++;
      //     DEBUG() << "elapsed time " << sysTimer.elapsed() << "mRecordingSecs " << mRecordingSecs;
      if (mRecordingSecs > topView_mConf.postBuf ) {   // Stop recording since maximum seconds are reached
	exeRecord();  // timerHit kill recording
	mAutoRecording = false;
      }
    }
    
    // Activity moniotoring
    if( activity == 0 ) {
      // No activity
      if( inactive == false ) {
	// changing active to inactive and save the start of inactive
	// DEBUG() << "Start inactive !!!";
	inactive = true;
	inactiveStart = currentSeconds;
      } else {
	// This is where we are inactive and determining how long we are inactive
	qint64 elapsedTime = currentSeconds - inactiveStart;
	
	// Sleep the coldfire if tilt
	// Hardcoded to 60 seconds for a possible tilt SLEEP to coldfire
	if( coldFireSleep == false ) {
	  if( (elapsedTime % SLEEPCOLDFIREWHENTILTED) == 0 ) {
	    DEBUG() << "Inactivity " <<  SLEEPCOLDFIREWHENTILTED  << " seconds seen" << currentSeconds ;
	    // check if tilt active and over tilt to do the sleep
	    if( u.tiltSeen() == true ) {
	      // sleep the coldFire
	      DEBUG() << "coldFireSleep set" << currentSeconds ;
#ifdef LIDARCAM
	      u.sendMbPacket( (unsigned char) CMD_SLEEP, 0, NULL, NULL );
	      u.Send_Msg_To_PIC( Set_Power_Led_Both );
#endif
		coldFireSleep = true;
	    }
	  }
	}
	
	// Sleep the coldfire if tilt not being used
	// Hardcoded to 150 seconds SLEEP to coldfire
	if( coldFireSleep == false ) {
	  if( ( elapsedTime % SLEEPCOLDFIRE) == 0 ) {
	    DEBUG() << "Inactivity " << SLEEPCOLDFIRE << "  seconds seen coldFireSleep " << currentSeconds ;
	    // sleep the coldFire
#ifdef LIDARCAM
	    u.sendMbPacket( (unsigned char) CMD_SLEEP, 0, NULL, NULL );
	    u.Send_Msg_To_PIC( Set_Power_Led_Both );
#endif
	    coldFireSleep = true;
	  }
	}
	
	// possible backLight off
	if( backLightOn == true ) {
	  if( backLightTime > 0 ) {
	    if( ( elapsedTime % backLightTime) == 0 ) {
	      DEBUG() << "Inactivity " << backLightTime << " seconds seen BACKLIGHTON off" << currentSeconds ;
#ifdef LIDARCAM
	      hardButtons& h = hardButtons::get();
	      h.Send_Display_Brightness( 0 );
#else
	      // display control turn off capability
	      // This controls the backlight on the keypad
	      QString cmd = QString("1");
	      const QString qPath("/sys/kernel/timerPWM/PWM");
	      QFile qFile(qPath);
	      if (qFile.open(QIODevice::WriteOnly)) {
		QTextStream out(&qFile); out << cmd;
		qFile.close();
	      }
#endif
	      backLightOn = false;
	    }
	  }
	}
	
	// possible logout
	
	// possible powerdown
	if( powerOffTime > 0 ) {
	  if( ( elapsedTime % powerOffTime) == 0 ) {
	    DEBUG() << "Inactivity " << powerOffTime << " seconds seen POWER OFF" << currentSeconds ;
	    
	    // close the local database
	    u.closeUserDB();
	    
	    // sync the linux
	    system("/bin/sync");
	    
	    // send message to pic and turn off power
#ifdef LIDARCAM
	    u.Send_Msg_To_PIC( Set_Power_Off);
#else
	    system( "/sbin/halt -p");
#endif
	    m_TopViewTimer->stop();
	    QApplication::exit(0);
	    return;
	  }
	}
      }
    } else {  // Active
      // There is activity
      if( inactive == true ) {
	// Changing from inactive to active!!
	// do what is needed
	// look at turn on touch screen
	// unSLEEP the coldfire
	//		DEBUG() << "Reactivate !!!";
	if ( coldFireSleep == true ) {
	  DEBUG() << "coldFireSleep clear" << currentSeconds ;
#ifdef LIDARCAM
	  u.sendMbPacket( (unsigned char) CMD_WAKEUP, 0, NULL, NULL );
	  int percent = u.FGBuf()->State_Of_Charge;
	  DEBUG() << "Percent " << percent;
	 if( percent > 10 ) {
	   u.Send_Msg_To_PIC( Set_Power_Led_Green );
	 }else{
	   u.Send_Msg_To_PIC( Set_Power_Led_Red );
	 }
#endif
	 coldFireSleep = false;
	}
	if ( backLightOn == false ) {
	  DEBUG() << "Turn on Backlight" << currentSeconds ;
#ifdef LIDARCAM
	  hardButtons& h = hardButtons::get();
	  int b = u.lidarDataBuf()->lidarStruct.HUD_BRIGHTNESS;
	  h.Send_Display_Brightness( b * 20 );
#else
	  // this code resets the display and keypad brightness back to last settings
	  int brightness;
	  
	  // Get brightness from the database
	  SysConfig & cfg = u.getConfiguration();
	  brightness = cfg.brightness;
	  
	  QString cmd;
	  
	  switch ( brightness ) {
	  case 0:
	    cmd = QString("9");
	    break;
	  case 1:
	    cmd = QString("6");
	    break;
	  case 2:
	    cmd = QString("3");
	    break;
	  case 3:
	    cmd = QString("0");
	    break;
	  default:
	    break;
	  }
	  
	  const QString qPath("/sys/kernel/timerPWM/PWM");
	  QFile qFile(qPath);
	  if (qFile.open(QIODevice::WriteOnly)) {
	    QTextStream out(&qFile); out << cmd;
	    qFile.close();
	  }
#endif
	  backLightOn = true;
	}
	inactive = false;
      }
      // reset the activity counter, so we can determine if actual activity in the next timerHit
      activity = 0;
    }
    
#ifdef HH1
    //set date and time
    //state& v = state::get();
    static int oldDays = 0;
    if (v.getState() == STATE_TOP_VIEW) 
	{
      QDateTime now = QDateTime::currentDateTime();
      QString time = now.toString("hh:mm:ss");
      ui->lb_zoom->setText(time);
      int days = now.date().dayOfYear();
      if (oldDays != days) 
	  {
		oldDays = days;
		QString date = now.toString("MM-dd-yy");
		//            QByteArray ba = date.toLatin1();
		//            const char *buf1 = ba.data();
		//            printf("%s\n", buf1);
		ui->lb_limit->setText(date);
      }
    }
   else
     oldDays = 0;
#endif
    
#ifdef IS_TI_ARM
    // check battery level
    Utils::get().updateFG();
    int percent = Utils::get().FGBuf()->State_Of_Charge;
    bool isCharging = Utils::get().FGBuf()->Is_Charging;
    
    // skip check if system is charging
    if( isCharging == false ) {
      // Just picked 5 because of ..., added greater than 0 for case where there is no battery in device.
      if ( (percent < 5) && (percent > 0)) {
	DEBUG() << "Battery percent " << percent << " calling lowBatterySignal() " ;
	emit lowBatterySignal();
      }
    }
    
    // update info for the iconFrame
    if ( m_iconFrame ) {
     if( percent > 75 ) {
       m_iconFrame->batterysetPixmap(":/dynamic/battery-4");
     }else{
       if( percent > 50 ) {
	 m_iconFrame->batterysetPixmap(":/dynamic/battery-3");
       }else{
	 if( percent > 25 ) {
	   m_iconFrame->batterysetPixmap(":/dynamic/battery-2");
	 }else{
	   if( percent > 10 ) {
	     m_iconFrame->batterysetPixmap(":/dynamic/battery-1");
	   }else{
#ifdef LIDARCAM
	     v.Send_Msg_To_PIC( Set_Power_Led_Red );
#endif
	     m_iconFrame->batterysetPixmap(":/dynamic/battery-0");
	   }
	 }
       }
     }
     
     if( true == Utils::get().FGBuf()->Is_Charging ) {
       m_iconFrame->batterysetPixmap(":/dynamic/charging");
     }
     
     // toggle the GPS connected icon
     if( Utils::get().GPSBuf()->GPS_Fixed == true ) {
       m_iconFrame->GPSsetPixmap(":/dynamic/GPS-on");
     }else{
       m_iconFrame->GPSsetPixmap(":/dynamic/GPS-off");
     }
     
     // get the amount of storage being used
     QProcess process;
     process.start("df");
     process.waitForFinished(1000); 
     
     QString stdout = process.readAllStandardOutput();
     QString stderr = process.readAllStandardError();
     //  DEBUG() << "stdout " << stdout << "stderr " << stderr;
     
     QStringList l = stdout.split('\n');
     for( QStringList::iterator it = ++l.begin(); it != --l.end(); ) 
	 {
       if( it->contains("mmcblk0p1") ) 
	   {
		 QString current = *it;
		 QString c = current.split(QRegExp("\\s+")).at(4);
		 c.replace(QString("%"), QString(""));
		 int percent = c.toInt();
		 //	 DEBUG() << "Found " << current << "percent used " << percent;
		 if( percent > 95 ) 
		 {
		   m_iconFrame->storagesetPixmap(":/dynamic/disk-0");
		   break;
		 }
		 if( percent > 75 ) 
		 {
		   m_iconFrame->storagesetPixmap(":/dynamic/disk-4");
		   break;
		 }
		 if( percent > 50 ) 
		 {
		   m_iconFrame->storagesetPixmap(":/dynamic/disk-3");
		   break;
		 }
		 if( percent > 25 ) 
		 {
		   m_iconFrame->storagesetPixmap(":/dynamic/disk-2");
		   break;
		 }
		 // determine which icon to display
		 m_iconFrame->storagesetPixmap(":/dynamic/disk-1");
		 break;
       } 
	   else 
	   {
		 ++it;
       }
     }
    }
#endif
   }
  return;
}

void topView::updateEvidence(OperatFrames *m_oprFrames)
{
    if(json_md_fd) //if *.json and *.md exits
    {
        static u_int32_t frame=0;

        if(frame!=frameCnt)
        {
            frame = frameCnt;
            //DEBUG() << "frame" << frame;

            char speedsArray[10], distancesArray[10];
            memset(speedsArray, 0, 10);
            memset(distancesArray, 0,10);

            for(int k = RadarConfig.num_to_show - 1; k >= 0; k--)  // Iterate backwards so fastest targets are drawn first
            {
                //DEBUG() << "target" << k << "spd" << Video_Coords[k].V << "dst" << Video_Coords[k].R;
                //DEBUG() << "RadarConfig.FOVh" << RadarConfig.FOVh; //10.1495
                float FOVh = 2.0f * tanf(RadarConfig.FOVh * PI/360.0f) * Video_Coords[k].R;  // Width of camera horizontal FOV at target distance
                float fract;
                if(FOVh > 3.03f) {
                  fract = 3.0f / FOVh;  //Fraction of screen width for 3 meter wide box
                } else {
                  fract = 0.99f; //but limit to 99% of screen size
                }

                //int camWidth = m_topScene->width() - m_oprFrames->width(); // m_topScene->width()=378, m_oprFrames->width()=120
                int camWidth = m_topScene->width();
                int camHeight = m_topScene->height();

                int rWidth = camWidth * fract;  //Map to screen size
                int rHeight = camHeight * fract;
                if(rWidth < 1) rWidth = 1;
                if(rHeight < 1) rHeight = 1;

                int centerX = (0.5 * Video_Coords[k].X + 0.5) * (camWidth-1);
                int centerY = (-0.5 * Video_Coords[k].Z + 0.5) * (camHeight-1);

                //DEBUG() << "centerX" << centerX << "centerY" << centerY;

                // based on units and speedTenths and rangeTenths      // Speed Need unit and decimal place info here also
                switch( topView_mConf.units )
                {
                    case 0: // MPH
                    case 2: // KNOTS
                    {
                      float speed = Video_Coords[k].V / 1.609344 ;
                      float dst = Video_Coords[k].R  * 3.28084;
                      if( topView_mConf.speedTenths == 0 ) {
                        sprintf(speedsArray,"%3.0f MPH", speed);
                      }else{
                        sprintf(speedsArray,"%4.1f MPH", speed);
                      }
                      if( topView_mConf.rangeTenths == 0 ) {
                        sprintf(distancesArray,"%3.0f FT", dst);
                       }else{
                        sprintf(distancesArray,"%4.1f FT", dst);
                       }
                    }
                    case 1: // km/h
                        if( topView_mConf.speedTenths == 0 ) {
                          sprintf(speedsArray,"%3.0f km/h", Video_Coords[k].V);
                        }else{
                          sprintf(speedsArray,"%4.1f km/h", Video_Coords[k].V);
                        }
                        if( topView_mConf.rangeTenths == 0 ) {
                          sprintf(distancesArray,"%3.0f m", Video_Coords[k].R);
                        }else{
                          sprintf(distancesArray,"%4.1f m", Video_Coords[k].R);
                        }
                    break;
                    default:
                    break;
                }//end of switch

                // Distance
                QString speedsString = QString(speedsArray);
                QString distancesString = QString(distancesArray);

                if(k==0)
                {
                    m_topScene->addRect(mRect1, mP0);
                    mRect1.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
                    m_topScene->addRect(mRect1, mP1);
                    // Show text
                    m_oprFrames->setSpeedDistance(OPR_SDLABEL1, speedsString, distancesString);
                }
                else if(k==1)
                {
                    m_topScene->addRect(mRect2, mP0);
                    mRect2.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
                    m_topScene->addRect(mRect2, mP2);
                    // Show text
                    m_oprFrames->setSpeedDistance(OPR_SDLABEL2, speedsString, distancesString);
                }
                else if(k == 2)
                {
                    m_topScene->addRect(mRect3, mP0);
                    mRect3.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
                    m_topScene->addRect(mRect3, mP3);
                    // Show text
                    m_oprFrames->setSpeedDistance(OPR_SDLABEL3, speedsString, distancesString);
                }
                else if(k == 3)
                {
                    m_topScene->addRect(mRect4, mP0);
                    mRect4.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
                    m_topScene->addRect(mRect4, mP4);
                    // Show text
                    m_oprFrames->setSpeedDistance(OPR_SDLABEL4, speedsString, distancesString);
                }
            }//end of update target data
        }
    }
    else
    {
        #if 1
        static float speed;
        static float range;
        char sbuf[10];

        QString speedsString = QString("speed1");
        QString distancesString = QString("dist1");

        if(range>3.33)
            range -= 3.33;
        else
            range = 1003.23;

        if(speed<120)
            speed += 3.33;
        else
            speed = 12.34;

        snprintf(sbuf, 10, "%5.2fmph", speed);
        sbuf[strlen(sbuf)]='\0';
        speedsString = QString(sbuf);

        snprintf(sbuf, 10, "%5.2fft", range);
        sbuf[strlen(sbuf)]='\0';
        distancesString = QString(sbuf);

        m_oprFrames->setSpeedDistance(OPR_SDLABEL1, speedsString, distancesString);

        if(range>3.33)
            range -= 3.33;
        else
            range = 1003.23;

        if(speed<120)
            speed += 3.33;
        else
            speed = 12.34;

        snprintf(sbuf, 10, "%5.2fmph", speed);
        sbuf[strlen(sbuf)]='\0';
        speedsString = QString(sbuf);

        snprintf(sbuf, 10, "%5.2fft", range);
        sbuf[strlen(sbuf)]='\0';
        distancesString = QString(sbuf);

        m_oprFrames->setSpeedDistance(OPR_SDLABEL2, speedsString, distancesString);

        if(range>3.33)
            range -= 3.33;
        else
            range = 1003.23;

        if(speed<120)
            speed += 3.33;
        else
            speed = 12.34;

        snprintf(sbuf, 10, "%5.2fmph", speed);
        sbuf[strlen(sbuf)]='\0';
        speedsString = QString(sbuf);

        snprintf(sbuf, 10, "%5.2fft", range);
        sbuf[strlen(sbuf)]='\0';
        distancesString = QString(sbuf);

        m_oprFrames->setSpeedDistance(OPR_SDLABEL3, speedsString, distancesString);

        if(range>3.33)
            range -= 3.33;
        else
            range = 1003.23;

        if(speed<120)
            speed += 3.33;
        else
            speed = 12.34;

        snprintf(sbuf, 10, "%5.2fmph", speed);
        sbuf[strlen(sbuf)]='\0';
        speedsString = QString(sbuf);

        snprintf(sbuf, 10, "%5.2fft", range);
        sbuf[strlen(sbuf)]='\0';
        distancesString = QString(sbuf);

        m_oprFrames->setSpeedDistance(OPR_SDLABEL4, speedsString, distancesString);

        // Draw a cross cursor
        //QPen p0(Qt::black);
        QPen p1(Qt::green);
        //QPen p2(Qt::yellow);
        //QPen p3(Qt::red);

        qreal centerX, centerY;
        centerX = (width() - ui->frame->width()) >> 1;
        centerY = height() >> 1;
        //DEBUG() << "centerX=" << centerX;
        //DEBUG() << "centerY=" << centerY;
        qreal halfLen = centerX / 2;
        (void)m_topScene->addLine(halfLen, centerY, centerX + halfLen, centerY, p1);
        (void)m_topScene->addLine(centerX, centerY - 30, centerX, centerY + 30, p1);

        // Draw tracking box
        static qreal offset=10;
        m_topScene->addRect(mRect1,mP0); //erase all previous boxes
        m_topScene->addRect(mRect2,mP0);
        m_topScene->addRect(mRect3,mP0);
        m_topScene->addRect(mRect4,mP0);

        if(offset<100)
            offset += 5;
        else
            offset = 10;

        //mRect.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
        mRect1.setRect(halfLen+offset, offset*2, offset, offset);
        mRect2.setRect(offset, offset, offset, offset);
        mRect3.setRect(halfLen+offset, offset, offset/2, offset/2);
        mRect4.setRect(halfLen+offset*1.5, offset*1.5, offset/2, offset/2);

        m_topScene->addRect(mRect1,mP1); //draw new box1
        m_topScene->addRect(mRect2,mP2); //draw new box2
        m_topScene->addRect(mRect3,mP3); //draw new box3
        m_topScene->addRect(mRect4,mP4); //draw new box4
        //m_topScene->update(); no difference
        #endif
    }

}

void topView::setUserName(QString s)
{
    if (this->m_loginName == s)
        return;

    this->m_loginName = s;
    return;
}

void topView::setPassWord(QString s)
{
    if (this->m_passWord == s)
        return;
    this->m_passWord = s;
    return;
}

void topView::exeLogin()
{
  int retv = 0;

	userDB *m_userDB;
	Utils& Util = Utils::get();
    m_userDB = Util.db();
	Q_ASSERT(m_userDB);

    if (m_vkb && m_vkb->isVisible()) {
        m_vkb->hide(true);
    }

	struct Users u;
	u.loginName = m_loginName;
	u.password = m_passWord;
	//u.loginName = "admin";
	//u.password = "test2";
	retv = m_userDB->queryEntry(TBL_USERS, (DBStruct *)&u, QRY_BY_MULTI_FIELDS);
	DEBUG();
	if (retv == 1) {
	  // switch to top opertaion menu
	  m_loginSuccess = 1;
//	  this->openTopScreen();
	  
	  struct Users m_currUser;    //the current login user
	  //get the login user info
	  retv = m_userDB->getNextEntry(TBL_USERS, (DBStruct *)&m_currUser);

	  Util.createSession( &m_currUser );
	  openTopScreen();
	  
	  //delete login scene
	  if (m_loginScene) {
		//m_currProxyWidget->deleteLater();  //TODO: may not needed
		m_loginScene->deleteLater();
		m_loginScene = NULL;
	  }
	} else {
	  printf("%s(%d)\n\r", __FILE__, __LINE__);
	  qDebug() << "login qry returned " << retv;
	  emit clearLogin();
	}
    return;
}

#ifdef QUICK_LOGIN_SIM
void topView::setQuickCode()
{
    static char code[5] = {'@', '$', '!', '|', '\0'};
    int idx = m_quickCode.size();
    QString name = QObject::sender()->objectName();
    QChar c;

	Utils& u = Utils::get();
	u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

    if (name=="p1")
        c = code[0];
    else if (name == "p2")
        c = code[1];
    else if (name == "p3")
        c = code[2];
    else if (name == "p4")
        c = code[3];
    else {
        qDebug() << "invalide quick code input";
        return;
    }

    m_quickCode.append(c);
    idx = m_quickCode.size();
    m_qkcSlider->setValue(idx);

	// qDebug() << "quick code: " << m_quickCode;

    if (idx == 1) {
        m_qkcTimeLimit = 6;
    }
    else if (idx == 4) {
        m_qkcTimeLimit = 0;
        m_qkcPbFrame->setEnabled(false);
        this->exeQuickLogin();
    }
}

void topView::exeQuickLogin()
{
  if (m_quickCode == "@!!$") {
	m_loginName = "jsmith";
	m_passWord = "user-1";
  } else if (m_quickCode == "$$|!") {
	m_loginName = "rtylor";
  } else {
	emit clearLogin();
	return;
  }

  userDB *m_userDB;
  Utils& Util = Utils::get();
  m_userDB = Util.db();
  Q_ASSERT(m_userDB);
  
  // do the rest of the real login
  struct Users u;
  u.loginName = m_loginName;
  u.password = m_passWord;
  
  int retv = m_userDB->queryEntry(TBL_USERS, (DBStruct *)&u, QRY_BY_MULTI_FIELDS);
  if (retv == 1) {
    // switch to top opertaion menu
    m_loginSuccess = 1;
    //	this->openTopScreen();
    
    struct Users m_currUser;    //the current login user
    //get the login user info
    retv = m_userDB->getNextEntry(TBL_USERS, (DBStruct *)&m_currUser);
    
    Util.createSession( &m_currUser );
    openTopScreen();
    
    //delete
    if (m_loginScene) {
      //m_currProxyWidget->deleteLater();
      m_loginScene->deleteLater();
      m_loginScene = NULL;
    }
    return;
  } 
  emit clearLogin();
}
#endif

void topView::clearLoginInfo()
{
    m_le_userName->setText("");
    m_le_passwd->setText("");

#ifdef QUICK_LOGIN_SIM
    m_qkcSlider->setValue(0);
    m_quickCode.clear();
    m_qkcTimeLimit = 0;
    m_qkcPbFrame->setEnabled(true);
#endif
}

void topView::enableRecord()
{
   ui->pb_record->setEnabled(true);
}

void topView::exeRecord()
{
  DEBUG() << "elapsed time " << sysTimer.elapsed();

  // Special case turn record button into printTicket button.
  if( printButtonActive == true ) {
    DEBUG() << "printButonActive " << printButtonActive;
    DEBUG() << displayFileName;

    // Print ticket
    QString ticketFileName;    

    QFileInfo fi( displayFileName );
    ticketFileName = fi.fileName();

    DEBUG() << ticketFileName;
    
   // printTicket::get().print(ticketFileName);

    return;
  }
  
   Utils& u = Utils::get();

   // TODO different sound for violation recording
   if( topView_mConf.audioAlert == 1 ) {
     u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   }

   if ((m_recording == false) && (ui->pb_record->isEnabled() == true))
   {
     m_recording = true; // as soon as possible grab the fact we are recording
     DEBUG() << "recording start ";
     mPhotoNum = 0; // Can take photo now
     ui->pb_record->setText("STOP\nRECORD");
     
     updateRecordView(false);
     ui->pb_menu->setEnabled(false);
     hardButtons::get().setHardButtonMap( 2, NULL);
     int retv = u.sendCmdToCamera(CMD_RECORD, 0);
     if (retv != 0)
       qWarning() << "set camera record failed";
     mRecordingSecs = 0;
   }
   else
   {
      m_recording = false;    // Not recording anymore
      topSpeed = 0.0;
      DEBUG() << "recording stop ";
#ifdef LIDARCAM
      ui->pb_record->setText("RECORD");
#else
      ui->pb_record->setText("START");
#endif
      ui->pb_record->setEnabled(false);
      QTimer::singleShot(3000, this, SLOT(enableRecord()));
      updateRecordView(true);
      ui->pb_menu->setEnabled(true);
#if defined(LIDARCAM) || defined(HH1)
      hardButtons::get().setHardButtonMap( 2, ui->pb_menu);
      int retv = u.sendCmdToCamera(CMD_STOPRECORD, 0);
      if (retv != 0)
         qWarning() << "set camera stop record failed";
      DEBUG() << "elapsed time " << sysTimer.elapsed();
      // get and dump a 2metaData record
      //  HH1MetaData& md = HH1MetaData::get();
      //  struct metaDataGet *data = md.getMD( 0 );
      //  hexDump((char *)"Data for 0 ", data, sizeof(struct metaDataGet));
      
      // Get the offset for the timeStamp
      HH1MetaData& md = HH1MetaData::get();
      int offset = md.findMDOffset( violationTimeStamp );
      DEBUG() << "violationTimeStamp " << violationTimeStamp << " offset " << offset;
      
      offset = offset - (5 * FRAMESPERSECOND);
      if( offset < 0 ) {
	offset = DATAMAX + offset;
      }

      DEBUG() << "After figure offset violationTimeStamp " << violationTimeStamp << " offset " << offset;

      // TODO need to figure out real offset to the start of the video
      // m_RecordingFile = u.getRecordingFileName(); // store this recording filename for future picture taken
      QString fileName = u.getRecordingFileName() + ".md";
      
      DEBUG() << "Filename " << fileName;
      //DEBUG() << "Basename " << mRecordingFileName;
      
      QFile file( fileName);
      if(!file.open(QIODevice::WriteOnly | QFile::Truncate )) {
	DEBUG() << "Failed to open file " << fileName;
      }
      
      int i;
      int nextOffset = 0;
      struct metaDataGet *data = NULL;
      
      // get the file name and open it
      // number of records is 15 frames/sec and 30 seconds
      for( i=0;i<(FRAMESPERSECOND * MAX_RECORDING_SECS ); i++ ){
	data = md.getMD(offset, &nextOffset);
	if( data != NULL ) {
	  //write the data
	  file.write((char *)data, sizeof(struct metaDataGet));
	  offset = nextOffset;
	}else{
	  DEBUG() << "getMD failed";
	}
      }
      
      // close the file.
      file.close();
#endif
   }
}

void topView::openMenuScreen()
{
  menuView *m_menuView;

  state& v = state::get();
  
  if ( v.getState() == STATE_MAIN_MENU ) {
    return;
  }
    
  //  DEBUG();
  Utils& u = Utils::get();

  // Suspend AV server
#ifdef LIDARCAM
  DEBUG() << "AV Server OFF";
  char data1[APPRO_DATA_LEN];
  data1[0] = 0; // 0 -> suspend
  u.SndApproMsg(APPRO_AVONOFF, data1, NULL);
#endif
  
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  if (!m_vkb)
	create_vkb();
  
  m_menuView = new menuView( m_vkb, u.session()->user(), this);
  
  connect(m_menuView, SIGNAL(closeMenuView()), this, SLOT(reopenTopScreen()));

  m_menuView->show();
}

void topView::reopenTopScreen()
{
  
  // Make sure you get back to reopenTopScree a different way
  state& v = state::get();
  v.setState(STATE_TOP_VIEW);

  m_TopViewTimer->stop();
  QApplication::exit(1);
  return;
}

int topView::openLoginScreen()
{
#ifdef LIDARCAM
  // init the zoom on the camera
  initZoom();
#endif
  
#ifdef TODO
   Utils& u = Utils::get();
   // HH1 first phase never have any data in the video or photo's
   // enabe the watermark information on the video display
   u.enableWaterMark();
   u.sendCmdToCamera(CMD_TIMESTAMP, 1);   // Enable timestamp
#endif
   
   initLoginScene();
   showFullView();
   m_view->setScene(m_loginScene);
	state& v = state::get();
   v.setState(STATE_LOGIN);

   delete m_startScene;
   m_startScene = NULL;
   return 0;
}

void topView::openTopScreen()
{
    // start the timer and logDebug after person has logged in.
    // cut back on cpu usage during bring up/login
    this->TopViewInitTimer();

    //logFile = new QFile("/mnt/mmc/ipnc/jsmith/top_view.log");
    //QString filenameLog("/mnt/mmc/ipnc/jsmith/top_view.log");
    //if (logFile->open( QIODevice::WriteOnly | QIODevice::Text ))
    logDebug.open("/mnt/mmc/ipnc/jsmith/top_view.log", std::ios::app);
    if (logDebug.is_open())
    {
        //logDebug(&logFile);
        //QTextStream logDebug(logFile);
        logDebug << "Log file openned @ " << QDateTime::currentDateTime().toString().toStdString() << std::endl << std::flush;
        DEBUG() << "Opened the log file";
    }
    else
    {
        //QTextStream logDebug(stdout);
        DEBUG() << "Cannot opened the log file";
        //logDebug << "Cannot opened the log file." << std::endl << std::flush;
    }
    

    Utils& u = Utils::get();
   this->initTopScene();
   this->showTopView();
   m_view->setScene(m_topScene);

   //  qDebug() << "openTopScreen " << this->centralWidget() ;
   m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   state& v = state::get();
   v.setState(STATE_TOP_VIEW); // 3
   this->connectSignals();

   // Restart AV server
#ifdef LIDARCAM
   DEBUG() << "AV Server ON";
   char data1[APPRO_DATA_LEN];
   data1[0] = 1; // 1 -> start
   u.SndApproMsg(APPRO_AVONOFF, data1, NULL);
#endif
   
   ui->pb_zoomin->setText("");
   ui->pb_focus->setText("");

   // Setup current location
   struct Location loc;

   loc.index = CAMS_DEFAULT_INDEX;
   loc.speedLimit = loc.captureSpeed = "0";    // in case 'query' failure

   int retv = u.db()->queryEntry(TBL_LOCATION, (DBStruct *)&loc, QRY_BY_KEY);
   if (retv == 1)
     retv = u.db()->getNextEntry(TBL_LOCATION, (DBStruct *)&loc);

   u.setCurrentLoc(loc); // Set the default one

   //   DEBUG() << "loc spdLimit " << loc.speedLimit << "loc capSpd " << loc.captureSpeed;
   
   speedLimit = loc.speedLimit.toFloat();
   mCaptureSpeed = loc.captureSpeed.toFloat();


   //initialize lastDistance and target ID
   for (int i = 0; i < MAX_TARGETS; i++)
   {
       lastDistance[i] = 0;
       maxSpeed[i] = 0;
       targetID[i] = 0;
   }

   // Get picture distance from database mConf.pictureDist
   SysConfig mConf = u.getConfiguration();
   threshold0 = mConf.pictureDist.toFloat();
   // because system is using kph internally, so captureSpeed needs to be converted to kph
   switch( topView_mConf.units ) {
   case 0: // MPH
   case 2: // KNOTS
     {
       mCaptureSpeed *= 1.60934;
       threshold0 = threshold0 / 3.28;
     }
     break;
   case 1: // km/h
	break;
   default:
     break;
   }
   
   // get local copy of autoTrigger
   autoTrigger =  topView_mConf.autoTrigger;

   DEBUG() << "speedLimit " << speedLimit << "captureSpeed " << mCaptureSpeed << "pictureDistance " << threshold0 << " autoTrigger " << autoTrigger;
   DEBUG() << "units " << topView_mConf.units << " spd tenths " << topView_mConf.speedTenths << " range tenths " << topView_mConf.rangeTenths;

#ifdef LIDARCAM
   // Display zoom value on top screen
   int zoomValue = 1;
   // display the zoom value
   queryCamSetting();
   zoomValue = mCamSetting.zoom.toInt();
#endif

#ifdef HH1
   ui->lb_zoom->setText("");
   ui->lb_zoomTitle->setText("");
   ui->lb_limit->setText("");
   ui->lb_limitTitle->setText("");
   ui->lb_auto->setText("");
   ui->lb_autoTitle->setText("");

   // Draw a crosshair
   QPen p(Qt::red);
   qreal centerX, centerY;
   centerX = (width() - ui->frame->width()) >> 1;
   centerY = height() >> 1;
   qreal halfLen = centerX / 2;
   (void)m_topScene->addLine(halfLen, centerY, centerX + halfLen, centerY, p);
   (void)m_topScene->addLine(centerX, centerY - 30, centerX, centerY + 30, p);
   m_topScene->update();
#else

   displayZoom(zoomValue);

   // Display the speed limit and capture value
   QString d;
   QString dd;
   int displayUnits = u.getDisplayUnits();
   switch( displayUnits )
   {
      case 0: // MPH
      case 2: // KNOTS
         d.append(loc.speedLimit);
         d.append(" MPH");
         dd.append(loc.captureSpeed);
         dd.append(" MPH");
         break;
      case 1: // km/h
         d.append(loc.speedLimit);
         d.append(" km/h");
         dd.append(loc.captureSpeed);
         dd.append(" km/h");
         break;
   }
   ui->lb_limit->setText(d);
   ui->lb_auto->setText(dd);
#endif

   // make displayjpg work
   ui->pb_displayjpg->setVisible(true);

#ifdef LIDARCAM
   // Update the watermark once
   Administration admin1 = u.getAdmin();
   if (admin1.metaData1.isEmpty() == true && admin1.metaData2.isEmpty() == true)
      u.sendCmdToCamera(CMD_REMOVE_WATERMARK, 0);  // Clear the watermark
   else
      setRecordingText();
   // Update dateformat
   int value1 = 0;    // YYYY/MM/DD
   if (admin1.dateFormat)
      value1 = 2;     // DD/MM/YYYY, 1 -> MM/DD/YYYY
   u.sendCmdToCamera(CMD_SECURITY_DATEFORMAT, value1);
#endif   

   // do this last
   this->TopViewInitTimer();
   
   logFile = new QFile("/mnt/mmc/ipnc/jsmith/top_view.log");
   //QString filenameLog("/mnt/mmc/ipnc/jsmith/top_view.log");
   if (logFile->open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text )) {
     //logDebug(&logFile);
     QTextStream logDebug(logFile);
     logDebug << "Opened the log file." << endl;
     DEBUG() << "Opened the log file";
   } else {
     QTextStream logDebug(stdout);
     DEBUG() << "Cannot opened the log file";
     logDebug << "Cannot opened the log file." << endl;
   }
}

//
// public slots
//
void topView::toggleVKB(vkILine *l)
{
    Q_ASSERT(m_vkb);

  if (m_vkb) {
      if (m_vkb->isVisible()) {
          this->setFocus();
          m_vkb->hide(true);
      } else {
          m_vkb->show(l, m_currProxyWidget->widget());
          m_vkb->move(this->x(), this->y());
          /*
          m_vkb->move(this->x(), this->y() + (this->height() - m_vkb->height()) + 5);
          */
      }
  } else {
      qDebug() << "virtual keyboard is not created";
  }
}

void topView::focusLine(vkILine *l)
{
    Q_ASSERT(m_vkb);

    if (m_vkb->isVisible() == false)
        return;

    if (m_vkb->currentTextBox() == l)
        return;

    m_vkb->setActiveForms(l, m_currProxyWidget->widget());
}

void topView::closeVKB()
{
    Q_ASSERT(m_vkb);
    if (m_vkb->isVisible()) {
        //this->setFocus();
        m_vkb->hide(true);
    }
}


#ifdef LIDARCAM
void topView::monitorSpeed()
{
  // for lidarcam
  // we are here because trigger is pulled.
  // get current speed limit and capture speed
  // if mNewSpeed is above or equal to capture speed start recording
  // recording is hardcoded to max. 26 seconds.

   Utils& u = Utils::get();
   struct Location loc;
   loc = u.getCurrentLoc();
   float captureSpeed = loc.captureSpeed.toFloat();

   if( loc.captureSpeed == QString("") ) {
    return;
   }

   topSpeed= 0.0;
  
   if ( fabs( mNewSpeed) > captureSpeed  )
   {
      // TODO add speed/range to the recording
      exeRecord();   // Start recording based on speed in LIDARCAM
      mAutoRecording = true;
   }
   return;
}
#endif

void topView::setRecordingText()
{
   if( fabs( mNewSpeed ) > fabs( topSpeed) )
   {
     // maintain the top speed during recording
     topSpeed = mNewSpeed;
     Utils::get().setTopSpeed(topSpeed);
   }

   Utils& u = Utils::get();
   Administration admin1 = u.getAdmin();
   if (admin1.metaData1.isEmpty() == true && admin1.metaData2.isEmpty() == true)
      return;  // Nothing to display

   char input_str[MAX_WATERMARK_LEN + 1]; // +1 for the last ' '
   char units[6], rangeUnit[6];
#ifdef LIDARCAM
   struct Lidar_Buff *ptr = u.lidarDataBuf();
   LIDAR *pLidar = &(ptr->lidarStruct);
   if (pLidar->DISPLAY_UNITS == 1)
   {
      sprintf(units, "km/h");
      sprintf(rangeUnit, "M");
   }
   else if (pLidar->DISPLAY_UNITS == 2)
   {
      sprintf(units, "KNOTS");
      sprintf(rangeUnit, "ft");
   }
   else
   {
      sprintf(units, "mph");
      sprintf(rangeUnit, "ft");
   }
   units[strlen(units)] = '\0';
   rangeUnit[strlen(rangeUnit)] = '\0';
#else
   sprintf(units, "mph");
   sprintf(rangeUnit, "ft");
#endif

   QStringList list1 = admin1.metaData1.split(" ");
   QStringList list2 = admin1.metaData2.split(" ");
   QString current1;

   int loop1 = 0;
   while (loop1 < 2)
   {
      QStringList::iterator it, end;
      if (!loop1)
      {  // Line 1
         it = list1.begin();
         end = list1.end();
      }
      else
      {  // Line 2
         it = list2.begin();
         end = list2.end();
      }
      memset(input_str, 0, MAX_WATERMARK_LEN + 1); // '+ 1' is for ' ' at the end of string
      for (; it != end; it++)
      {
         int totalLen = strlen(input_str);
         Location loc1 =u.getCurrentLoc();

         current1 = *it;
         if (current1 == (QString)"%0")
         {  // Device ID
            if ((totalLen + 8 + 1) <= MAX_WATERMARK_LEN)
            {  // Enough spaces.
               char SN_str[10];
#ifdef LIDARCAM
               memcpy(SN_str, &u.lidarDataBuf()->lidarStruct.SERIAL_NUMBER[0], 8);
#else
               memcpy(SN_str, topView_mConf.serialNumber.toStdString().c_str(), 8);
               
#endif
               SN_str[8] = '\0';
               sprintf(&input_str[totalLen], "%s ", SN_str );
            }
         }
         else if (current1 == (QString)"%1")
         {  // User ID
            Session *currentSession = u.session();
            if (!currentSession)
               continue;
            QString qs1 = currentSession->user()->loginName;
            if (qs1.isEmpty() == false && ((totalLen + qs1.length() + 3) <= MAX_WATERMARK_LEN))
            {  // Enough spaces
               sprintf(&input_str[totalLen], "U:%s ", u.session()->user()->loginName.toLatin1().data());
            }
         }
         else if (current1 == (QString)"%2")
         {  // Record #
            if ((totalLen + 5) <= MAX_WATERMARK_LEN)
               sprintf(&input_str[totalLen], "R:%d ", u.getEvidenceId());
         }
         else if (current1 == (QString)"%3")
         {  // Speed Limit
            int speedLimit = loc1.speedLimit.toInt();
            if (speedLimit > 99 && (totalLen + 8 + strlen(units)) <= MAX_WATERMARK_LEN)
               sprintf(&input_str[totalLen], "SL:%3d %s ", speedLimit, units);
            else if (speedLimit <= 99 && (totalLen + 7 + strlen(units)) <= MAX_WATERMARK_LEN)
               sprintf(&input_str[totalLen], "SL:%2d %s ", speedLimit, units);
         }
         else if (current1 == (QString)"%4")
         {  // Speed
            if (topSpeed >= 100.0 && (totalLen + 9 + strlen(units)) <= MAX_WATERMARK_LEN)
               sprintf(&input_str[totalLen], "S:%3.1f %s ", topSpeed, units);
            else if (topSpeed < 100.0 && (totalLen + 8 + strlen(units)) <= MAX_WATERMARK_LEN)
               sprintf(&input_str[totalLen], "S:%2.1f %s ", topSpeed, units);
         }
         else if (current1 == (QString)"%5")
         {  // Distance
            if (m_range >= 100.0 && (totalLen + 9 + strlen(rangeUnit)) <= MAX_WATERMARK_LEN)
               sprintf(&input_str[totalLen], "D:%3.1f %s ", m_range, rangeUnit);
            else if (m_range < 100.0 && (totalLen + 8 + strlen(rangeUnit)) <= MAX_WATERMARK_LEN)
               sprintf(&input_str[totalLen], "D:%2.1f %s ", m_range, rangeUnit );
         }
         else if (current1 == (QString)"%6")
         {  // Location
            if ((totalLen + loc1.description.length() + 3) <= MAX_WATERMARK_LEN)
            {  // Enough spaces
               // Current APPRO does not support ',', Replace it.
               QString qs1 = loc1.description.replace((QChar)',', (QChar)'.');
               sprintf(&input_str[totalLen], "L:%s ", qs1.toLatin1().data());
            }
         }
         else if (!current1.isEmpty())
         {  // Customized data
            if ((totalLen + current1.length() + 1) <= MAX_WATERMARK_LEN)
               sprintf(&input_str[totalLen], "%s ", current1.toLatin1().data());
         }
      }
//      DEBUG() << input_str << "(" << strlen(input_str) << ")";
      if (strlen(input_str))
         input_str[strlen(input_str) - 1] = '\0';  // Remove the end ' '
#ifdef LIDARCAM
      if (!loop1)
      {  // Line 1
//         if (input_str[0] != '\0')  // Not an empty string
         {
            if (admin1.metaPosition == WATERMARK_TOPLEFT)
               u.sendCmdToCamera(CMD_DISPLAY_WATERMARK, (int)input_str);
            else
               u.sendCmdToCamera(CMD_DISPLAY_WATERMARK, (int)input_str, 1, 245);
         }
      }
      else
      {  // Line 2
         if (input_str[0] != '\0')  // Not an empty string
         {
            if (admin1.metaPosition == WATERMARK_TOPLEFT)
               u.sendCmdToCamera(CMD_DISPLAY_WATERMARK2, (int)input_str);
            else
               u.sendCmdToCamera(CMD_DISPLAY_WATERMARK2, (int)input_str, 1, 245);
         }
      }
#endif
      loop1++;
   }
}

#ifdef LIDARCAM
void topView::zoomin()
{
   Utils& u = Utils::get();

/*
   // Test Camera parameters
   ILLUMINATOR illu = u.getIlluminator();
   if (illu.m2aModeIndex)  // Manual mode
   {
      illu.mShutterIndex++;
      if (illu.mShutterIndex > 7)
         illu.mShutterIndex = 0;
      u.sendCmdToCamera(CMD_SHUTTER, illu.mShutterIndex + 7);
      u.setIlluminator(&illu);
   }
   return;
*/
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   queryCamSetting();
   int zoomValue = mCamSetting.zoom.toInt();
   zoomValue = zoomValue + 1;
   if( zoomValue > ZOOM_MAX )
   {
      zoomValue = ZOOM_MIN;
   }
   int retv = u.sendCmdToCamera(CMD_ZOOM, zoomValue);

   if (retv != 0)
   {
      qWarning() << "set camera zoom failed";
      return;
   }
   // save the camera state
   mCamSetting.zoom = QString::number(zoomValue);
   retv = u.db()->updateEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting);
   if (retv)
   {
      DEBUG() << "update camera setting failed for location " << mCamSetting.index;
      return;
   }

   displayZoom( zoomValue );
}

void topView::focusMode()
{

#ifdef SPEED_INROOM_DEBUG
   // In room debug
   static bool first = true;
   if (first == true)
   {
      m_newRange = 250;
      first = false;
      triggerPulled = true;
      DEBUG() << "Trigger On";
   }
   else
   {
      first = true;
      triggerPulled = false;
      DEBUG() << "Trigger Off";
   }
   return;
#endif
/*
   // Test Camera parameters
   ILLUMINATOR illu = u.getIlluminator();
   if (illu.m2aModeIndex)  // Manual mode
   {
      illu.mGainIndex++;
      if (illu.mGainIndex > 7)
         illu.mGainIndex = 0;
      u.sendCmdToCamera(CMD_GAIN, illu.mGainIndex + 7);
      u.setIlluminator(&illu);
   }
   return;
*/
#ifdef LIDARCAM
   Utils& u = Utils::get();
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

   if (mCamSetting.focus == (QString)"AUTO")
   {  // Change to Manual mode
      mCamSetting.focus = (QString)"MANUAL";
      int zoomValue = mCamSetting.zoom.toInt(); // Current Zoom number
      if (u.getDisplayUnits() == 1)  // kmH
      {
         mCamSetting.focus1 = focusM[zoomValue - 1]; // Remember it
//         DEBUG() << "Manual Focus (kmH): " << mCamSetting.focus1;
      }
      else  // MPH or KNOTS
      {
         mCamSetting.focus1 = focusFt[zoomValue - 1]; // Remember it
//         DEBUG() << "Manual Focus (mph): " << mCamSetting.focus1;
      }
   }
   else
   {  // Change to Auto mode
      mCamSetting.focus = (QString)"AUTO";
   }
   displayZoom( mCamSetting.zoom.toInt());

   u.sendCmdToCamera(CMD_FOCUS, (int)mCamSetting.focus.toLatin1().data());

   int retv = u.db()->updateEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting);
   if (retv != 0)
      qWarning() << "set camera focus failed";
#endif
}

void topView::initZoom()
{
   queryCamSetting();
   Utils& u = Utils::get();
   int zoomValue = mCamSetting.zoom.toInt();
   if( (zoomValue >= ZOOM_MIN ) && (zoomValue <=  ZOOM_MAX) )
   {
      int retv = u.sendCmdToCamera(CMD_ZOOM, zoomValue);
	
      if (retv != 0)
         qWarning() << "set camera zoom failed";
   }
   else
      DEBUG() << "Invalid zoom value from the db " << zoomValue ;   

   // Setup the focus mode
   DEBUG() << "Focus mode: " << mCamSetting.focus.toLatin1().data() ;
   u.sendCmdToCamera(CMD_FOCUS, (int)mCamSetting.focus.toLatin1().data());
}
#endif

// Read camera configuration information from database
void topView::queryCamSetting()
{
   int retv = 0;
   Utils& u = Utils::get();
   int location = u.location();
   if (location > 0)
      mCamSetting.index = location;
   else
      mCamSetting.index = CAMS_DEFAULT_INDEX;

   //  DEBUG() << "mCamSetting.index " << mCamSetting.index;

   int ct = u.db()->queryEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting, QRY_BY_KEY);
   if (ct >= 1)
   {  // Found the settings for this (default) location
      retv = u.db()->getNextEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting);
   }
   else
   {
      if (location > 0)
      {  // Did not find camera setting entry for this location, use 'default' for it.
         mCamSetting.index = CAMS_DEFAULT_INDEX;
         ct = u.db()->queryEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting, QRY_BY_KEY);
         Q_ASSERT(ct == 1);

         retv = u.db()->getNextEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting);
         if (!retv)
         {
            //add cam setting entry with the default settings
            mCamSetting.index = location;
            int retv1 = u.db()->addEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting);
            if (retv1)
               DEBUG() << "add camera initial setting failed for location " << location;
         }
         else
            DEBUG() << "get default camera setting failed";
      }
      else
      {  // No 'location' set yet. But, did not find 'default' either and something was wrong
         DEBUG() << "camera default setting is empty";
         // Recreate camera default setting entry
         retv = u.db()->deleteEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting);
         retv = u.db()->addEntry(TBL_CAMERA_SETTING, NULL);
         ct = u.db()->queryEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting, QRY_BY_KEY);
         if (ct >= 1)
            retv = u.db()->getNextEntry(TBL_CAMERA_SETTING, (DBStruct *)&mCamSetting);
         else
            retv = 1;   // failure
      }
   }

   if (retv)
   {  // if for any reason, did not get the camera settting, use default
      mCamSetting.mode = QString("AUTO");
      mCamSetting.shutter = QString("AUTO");
      mCamSetting.ev = QString("0");
      mCamSetting.gain = QString("AUTO");
      DEBUG() << "Error. Use Default Camera Setting";
   }
}
#ifdef LIDARCAM
void topView::displayZoom( int zoomNum )
{
   // Update zoom info on the main menu
   // Display zoom value on top screen
   QString d;
   int displayUnits = Utils::get().getDisplayUnits();
   if (mCamSetting.focus == (QString)"AUTO")
   {  // Auto focus mode
      if (displayUnits == 1)  // kmH
         d = QString("%1 M").arg(focusM[zoomNum-1], 0, 'f', 0, '0');
      else  // MPH or KNOTS
         d = QString("%1 FT").arg(focusFt[zoomNum-1],0, 'f', 0, '0');
      DEBUG() << "zoomNum " << zoomValue << " " << d;

      d = "AF " + d;
   }
   else
   {  // Manual focus mode
      if (displayUnits == 1)  // kmH
         d = QString("%1 M").arg(mCamSetting.focus1, 0, 'f', 0, '0');
      else  // MPH or KNOTS
         d = QString("%1 FT").arg(mCamSetting.focus1, 0, 'f', 0, '0');

      d = "MF " + d;
   }
   ui->lb_zoom->setText(d);   // Focus value

   d = QString("x%1 ZOOM").arg(zoomValue[zoomNum-1] / 1000);
   // tim hoth turned off displaying the zoom value
   //ui->lb_zoomTitle->setText(d);

   return;
}
#endif

#ifdef HH1
void topView::processTargets()
{
  if(!m_oprScene) {
    return;
  }

  // make sure activity does not affect tracking
  // bump activity will show that activity is happening.
  activity++;
  
  // Raw data from the senson
  pSensor->ReadSensor(&theta_vs, &theta_rs, &theta_hs);
  
  // Relative values cacuated here
  float theta_hs_rel = theta_hs - theta_hs_ref;
  if(theta_hs_rel > PI) theta_hs_rel = PI - theta_hs_rel;
  else if(theta_hs_rel < -PI) theta_hs_rel = PI + theta_hs_rel;
  
  float theta_rs_rel = theta_rs - theta_rs_ref;
  if(theta_rs_rel > PI) theta_rs_rel = PI - theta_rs_rel;
  else if(theta_rs_rel < -PI) theta_rs_rel = PI + theta_rs_rel;
  
  float theta_vs_rel = theta_vs - theta_vs_ref;
  if(theta_vs_rel > PI) theta_vs_rel = PI - theta_vs_rel;
  else if(theta_vs_rel < -PI) theta_vs_rel = PI + theta_vs_rel;

  //    printf("Reference az angle = %f, Relative az angle = %f\n", theta_hs_ref*DEG_PER_RAD, theta_hs_rel*DEG_PER_RAD);
  //    printf("Reference rl angle = %f, Relative rl angle = %f\n", theta_rs_ref*DEG_PER_RAD, theta_rs_rel*DEG_PER_RAD);
  //    printf("Reference el angle = %f, Relative el angle = %f\n", theta_vs_ref*DEG_PER_RAD, theta_vs_rel*DEG_PER_RAD);
 
  // Set up the transform with the current sensor data
  Transforms.UpdateSensor(theta_vs_rel, theta_rs_rel, theta_hs_rel);


  //
  // Begin roll line display
    float rollAngle = theta_rs_rel;
    float pitchAngle = theta_vs_rel;
    int camWidth = m_oprScene->width() - m_oprFrames->width();
    int camHeight = m_oprScene->height();
    int rWidth = camWidth * 0.2;  //Map to screen size
    int rHeight = camHeight * 0.2;
    if(rWidth < 1) rWidth = 1;
    if(rHeight < 1) rHeight = 1;
    int centerScreenX = 0.5 * (camWidth-1);
    int centerScreenY = 0.5 * (camHeight-1);

    // Rotate the roll line position to the unit axes
    float xr = theta_hs_rel;
    float yr = 0.0f;
    float zr = -theta_vs_rel;
    // Rotate through roll angle around Y axis
    float rx = xr * cosf(-rollAngle) - zr * sinf(-rollAngle);  // Rotate around Z axis
    float ry = yr;
    float rz = xr * sinf(-rollAngle) + zr * cosf(-rollAngle);
    // Roll through pitch Angle around X axis
    float ux = rx;                                                  // Rotate around X axis
    //    float uy = ry * cosf(-pitchAngle) - rz * sinf(-pitchAngle);   // Note: Y is not used
    float uz = ry * sinf(-pitchAngle) + rz * cosf(-pitchAngle);

    float pitchDisplay = uz * 2.0f * DEG_PER_RAD/RadarConfig.FOVv;
    float horizDisplay = ux * 2.0f * DEG_PER_RAD/RadarConfig.FOVh;

    int centerX = centerScreenX - centerScreenX * horizDisplay;
    if(centerX < 0) centerX = 0;
    else if(centerX > camWidth - 1) centerX = camWidth -1;
    
    int centerY = centerScreenY - centerScreenY * pitchDisplay;
    if(centerY < 0) centerY = 0;
    else if(centerY > camHeight - 1) centerY = camHeight -1;

    //#define ROLL_LINE_DEBUG
#ifdef ROLL_LINE_DEBUG
    printf("Sensor Relative: ");
    printf("hs %f rs %f vs %f\n", theta_hs_rel, theta_rs_rel, theta_vs_rel);
    printf("FOVv %f FOVh %f\n", RadarConfig.FOVv, RadarConfig.FOVh);
    printf("Xdev = %f Ydev = %f Zdev = %f\n", ux, 0.0f, uz);
    printf("pitchDisplay = %f horizDisplay = %f\n", pitchDisplay, horizDisplay);
    printf("Line position: ");
    printf("X: %6d ", centerX);
    printf("Y: %6d\n",centerY);
#endif

    if(m_rollLine)
    {
        m_oprScene->removeItem(m_rollLine);
        delete m_rollLine;
        m_rollLine = NULL;
    }
    QColor rollColor;
    rollColor = QColor(255, 255, 255, 255);
    QPen rollPen(rollColor);
    rollPen.setWidth(2);
    m_rollLine = m_oprScene->addLine(centerX + rHeight*sinf(rollAngle)/2.0f, centerY + rHeight*cosf(rollAngle)/2.0f,
                                     centerX - rHeight*sinf(rollAngle)/2.0f, centerY - rHeight*cosf(rollAngle)/2.0f, rollPen);

    // End roll line display

    int fastest_Targets[4];
    float fastest_Speeds[4];
    //UINT32 Radar_Target_IDs[4];

    for(int i = 0; i < 4; i++) {
    fastest_Targets[i] = -1;  // Init indexes for fastest targets to invalid index
    fastest_Speeds[i] = 0.0f; // and speeds to zero
  }
  
  //  Note:  We could begin processing targets against violation rules here.
  //         It could also be done after sorting for fastest targets if that provides an advantage

  // Section to save metaData
  // get timeMS this is the timeStamp used for anything associated with this 
  timeStamp = sysTimer.elapsed();

  // build structure
  struct metaDataSet data;

  data.hdr.timeMilliSecs = timeStamp;
  data.hdr.theta_vs = theta_vs;
  data.hdr.theta_rs = theta_rs;
  data.hdr.theta_hs = theta_hs;
  data.target = &mpRadarData->Data.Targets;
   
  // save the data
  HH1MetaData& md = HH1MetaData::get();
  md.setMD( &data );
  
  //    DEBUG() << "numTargets = " << mpRadarData->Data.Targets.numTargets;
  float speed;
  float averageDelay = 1/15 + 0.20; // suppose average delay is 1/15 + 0.2 s
  float threshold = 0;
  bool takePicture = false;
  float d;
  int num_fastest = 0;  // Init to no fastest targets
  float FOVf = 2.0f * tanf(RadarConfig.FOVh * PI/360.0f);  // Factor of camera horizontal FOV at target distance, width = FOVf * Video_Coords->R
  QString base_file_name;

  coord_struct Radar_Coords[MAX_TARGETS];
  coord_struct Video_Coords[MAX_TARGETS];
  coord_struct Roadway_Coords[MAX_TARGETS];
  for(int i = 0; i < mpRadarData->Data.Targets.numTargets; i++)
  {
      RadarTargetResponse_t *target = &mpRadarData->Data.Targets.RadarTargets[i];

      // Same target may sit in different location
      if (target->targetId != targetID[i])
      {
          for (int j=i+1; j < MAX_TARGETS; j++)
          {
            // search for the last target
            if (target->targetId == targetID[j])
            {
              d = lastDistance[j];
              lastDistance[j] = lastDistance[i];
              lastDistance[i] = d;

              d = maxSpeed[j];
              maxSpeed[j] = maxSpeed[i];
              maxSpeed[i] = d;

              targetID[j] = targetID[i];
              targetID[i] = target->targetId;

              break;
            }

            // No more search when there is no more targets. This is a new target.
            if(targetID[j] == 0)
            {
                targetID[j] = targetID[i];
                targetID[i] = target->targetId;
                maxSpeed[j] = maxSpeed[i];
                maxSpeed[i] = 0;
                lastDistance[j] = lastDistance[i];
                lastDistance[i] = 0;

                break;
            }
          }
      }

    memset( &Radar_Coords[i], 0, sizeof(coord_struct));
    memset( &Video_Coords[i], 0, sizeof(coord_struct));
    memset( &Roadway_Coords[i], 0, sizeof(coord_struct));

    Radar_Coords[i].type = radar;
    Radar_Coords[i].X = target->xCoord;
    Radar_Coords[i].Y = target->yCoord;
    // Since the 3D radar does not measure target height (Z axis), fake it
    //    Radar_Coords.Z = target->zCoord;
    Radar_Coords[i].Z = mpRadarData->Data.RadarOrientation.targetHeight - mpRadarData->Data.RadarOrientation.radarHeight;  // Usually negative

    Radar_Coords[i].R = sqrtf(target->xCoord * target->xCoord +
               target->yCoord * target->yCoord +
               target->zCoord * target->zCoord);

    Radar_Coords[i].Vx = target->xVelocity;
    Radar_Coords[i].Vy = target->yVelocity;
    Radar_Coords[i].Vz = target->zVelocity;

    Radar_Coords[i].V = sqrtf(target->xVelocity * target->xVelocity +
               target->yVelocity * target->yVelocity +
               target->zVelocity * target->zVelocity);

    // TODO      Radar_Coords[i].Theta_Y = RadarConfig.Theta_hs;
    // TODO      Radar_Coords[i].Theta_Z = RadarConfig.Theta_vs;
    Radar_Coords[i].Theta_Vy = 0.0f;
    Radar_Coords[i].Theta_Vz = 0.0f;
    Radar_Coords[i].Ix = 0.0f;
    Radar_Coords[i].Iz = 0.0f;

    Roadway_Coords[i].type = roadway;
    Video_Coords[i].type = video;

    Transforms.Transform(&Radar_Coords[i], &Roadway_Coords[i]);

    Transforms.Transform(&Roadway_Coords[i], &Video_Coords[i]);

    // We will invalidate targets if the cosine angle is too near 90 degrees (> 80 degrees and <100 degrees) since dividing by
    // the cosine to get true speed will also magnify speed errors. (80 degrees = 1.396263 radians, 100 degrees = 1.745329 radians)

    float targetAngle = fabsf(atan2f(Roadway_Coords[i].X, Roadway_Coords[i].Y));
    if((targetAngle > 1.396263f) && (targetAngle < 1.745329f))
    {
        printf("Invalidating target %d due to high cos angle at X = %f, Y = %f\n", i, Roadway_Coords[i].X, Roadway_Coords[i].Y);
    }

    // speed is always positive
    speed = Roadway_Coords[i].V;
    if (speed > maxSpeed[i])
        maxSpeed[i] = speed;

    // Compensate for the delayed picture taken because of the speed
    if (topView_mConf.direction == 0)
    {
        // process those approaching targets only
        if (Radar_Coords[i].Vy > 0)
        {
            lastDistance[i] = Roadway_Coords[i].Y;
            continue;
        }
        threshold = threshold0 + speed /3.6 * averageDelay;
        takePicture = (lastDistance[i] > threshold) && (Roadway_Coords[i].Y < threshold);
    }
    else if (topView_mConf.direction == 1)
    {
        // process those receding targets only
        if (Radar_Coords[i].Vy < 0)
        {
            lastDistance[i] = Roadway_Coords[i].Y;
            continue;
        }
        threshold = threshold0 - speed /3.6 * averageDelay;
        takePicture = (lastDistance[i] < threshold) && (Roadway_Coords[i].Y > threshold);
    }
    else
    {
        if (Radar_Coords[i].Vy > 0)
        {
            threshold = threshold0 - speed /3.6 * averageDelay;
            takePicture = (lastDistance[i] < threshold) && (Roadway_Coords[i].Y > threshold);
        }
        else
        {
            threshold = threshold0 + speed /3.6 * averageDelay;
            takePicture = (lastDistance[i] > threshold) && (Roadway_Coords[i].Y < threshold);
        }
    }

    if (takePicture && maxSpeed[i] > mCaptureSpeed)
    {
        logDebug << timeStamp << "ms : Target " << target->targetId << " approached picture distance from " \
                 << lastDistance[i] << "m to " << Roadway_Coords[i].Y << "m at speed " << Roadway_Coords[i].Vy << "km/h, compensated picture distance is " \
                 << threshold << ", configured picture distance is " << threshold0 << std::endl;
        DEBUG()  << timeStamp << "ms : Target " << target->targetId << " approached picture distance from " \
                 << lastDistance[i] << "m to " << Roadway_Coords[i].Y << "m at speed " << Roadway_Coords[i].Vy << "km/h, compensated picture distance is " \
                 << threshold << ", configured picture distance is " << threshold0 << Roadway_Coords[0].Vy << Roadway_Coords[1].Vy << Roadway_Coords[2].Vy \
                 << Radar_Coords[0].Vy << Radar_Coords[1].Vy << Radar_Coords[2].Vy;
    }
    lastDistance[i] = Roadway_Coords[i].Y;

    // This is where violations are caught,
    if ((speed >= mCaptureSpeed) && (m_recording == false) && ui->pb_record->isEnabled() && ((autoTrigger == 1) || triggerPulled ))
    {
        DEBUG() << "Start record speeding for target " << targetID << ", speed is " << speed << ", speedLimit " << mCaptureSpeed;
        violation();
    }

    // take pictures only for those that had violated speeding at least once
    if (m_recording && takePicture && (maxSpeed[i] > mCaptureSpeed))
    {
        Utils& u = Utils::get();
        base_file_name = u.getRecordingFileName();
        QString pictureName = base_file_name;
        //QString pictureName = m_RecordingFile;
        pictureName.append("-");
        pictureName.append(QString::number(target->targetId));
        pictureName.append(".jpg");
        u.takePhoto(pictureName, Roadway_Coords[i].Y);

        QString targetJSONFile = base_file_name;
        targetJSONFile.append("-");
        targetJSONFile.append(QString::number(targetID[i]));
        targetJSONFile.append(".tkt");

        // Calculate the rectangle coordinates of the target
        float FOVh = FOVf * Video_Coords[i].R;  // Width of camera horizontal FOV at target distance
        float fract;
        if(FOVh > 3.03f)
          fract = 1.5f / FOVh;  // Fraction of screen width for 3 meter wide box
        else
          fract = 0.49f;  // but limit to 99% of screen size

        int hWidth = camWidth * fract;  //Map to screen size, half size of target width
        int hHeight = camHeight * fract; // half size of target height
        if(hWidth < 1)
            hWidth = 1;
        if(hHeight < 1)
            hHeight = 1;

        int x0 = (0.5 * Video_Coords->X + 0.5) * (camWidth-1);
        int y0 = (-0.5 * Video_Coords->Z + 0.5) * (camHeight-1);
        int targetTop = y0 - hHeight;
        int targetBottom = y0 + hHeight;
        int targetLeft = x0 - hWidth;
        int targetRight = x0 + hWidth;

        QFile json_file(targetJSONFile);
        json_file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream json_writer(&json_file);

        int id = u.getEvidenceId();
        SysConfig & cfg = u.getConfiguration();
        Location loc1 =u.getCurrentLoc();
        Session *currentSession = u.session();
        int units = cfg.units;
        QString us;
        float s = 0;
        double lat = atof( (const char*)u.GPSBuf()->Latitude );
        double lon = atof( (const char*)u.GPSBuf()->Longitude );

        json_writer << "{\n";
        json_writer << "  \"id\": " << id <<",\n";
        json_writer << "  \"stalker-id\": \"" << id <<"\",\n";
        json_writer << "  \"serial-number\": " << "\"" << cfg.serialNumber << "\"" <<",\n";
        json_writer << "  \"time-local\": \"" << QDateTime::currentDateTimeUtc().toString( "yyyy-MM-dd hh:mm:ss" ) << "\",\n";
        json_writer << "  \"time-utc\": \"" << QDateTime::currentDateTimeUtc().toString( "yyyy-MM-dd hh:mm:ss" ) << "\",\n";
        json_writer << "  \"location\": \"" << loc1.description << "\",\n";

        if (currentSession)
        {
           json_writer << "  \"officer\": \"" << currentSession->user()->loginName << "\",\n";
        }

        json_writer << "  \"units\": {\n";
        if (units == 1)
        {
           us = "km/h";
           s = maxSpeed[i];
           json_writer << "    \"speed\": \"km/h\",\n";
           json_writer << "    \"stalker-speed-units\": \"KM/h\",\n";
           json_writer << "    \"distance\": \"meter\",\n";
           json_writer << "    \"stalker-range-units\": \"M\"\n";
        }
        else if (units == 2)
        {
            us = "knotes";
            s = maxSpeed[i] * 0.54; // km/h to knot
           json_writer << "    \"speed\": \"knots\",\n";
           json_writer << "    \"stalker-speed-units\": \"KNOTS\",\n";
           json_writer << "    \"distance\": \"feet\",\n";
           json_writer << "    \"stalker-range-units\": \"FT\"\n";
        }
        else
        {
            us = "mph";
            s = maxSpeed[i] * 0.621; // km/h to mph
           json_writer << "    \"speed\": \"mph\",\n";
           json_writer << "    \"stalker-speed-units\": \"MPH\",\n";
           json_writer << "    \"distance\": \"feet\",\n";
           json_writer << "    \"stalker-range-units\": \"FT\"\n";
        }
        json_writer << "  },\n";

        json_writer << "  \"targetId\": \"" << targetID[i] << "\",\n";
        json_writer << "  \"timeMillieSecs\": \"" << timeStamp << "\",\n";
        json_writer << "  \"speed-limit\": \"" << speedLimit << "\",\n";
        json_writer << "  \"speed-max\": \"" << s << "\",\n";
        //json_writer << "  \"stalker-speed-limit\": \"" << speedLimit << "\",\n";
        //json_writer << "  \"stalker-speed-actual\": \"" << maxSpeed[i] << "\",\n";

        json_writer << "  \"latitude\": " << lat << ",\n";
        json_writer << "  \"longitude\": " << lon << ",\n";
        //json_writer << "  \"stalker-latitude\": \"" << lat << "\",\n";
        //json_writer << "  \"stalker-longitude\": \"" << lon << "\",\n";
        json_writer << "  \"stalker-range\": \"" << cfg.pictureDist << "\",\n";

        json_writer << "  \"approaching\": true,\n";
        json_writer << "  \"violations\": {\n";
        json_writer << "    \"speeding\": true\n";
        json_writer << "  },\n";
        json_writer << "  \"timestamp-local\": 0,\n";
        json_writer << "  \"timestamp-utc\": 0,\n";
        json_writer << "  \"targetLeft\": \"" <<targetLeft << "\",\n";
        json_writer << "  \"targetTop\": \"" <<targetTop << "\",\n";
        json_writer << "  \"targetRight\": \"" <<targetRight << "\",\n";
        json_writer << "  \"targetBottom\": \"" <<targetBottom << "\"\n";
        json_writer << "}\n";

        json_file.close();
    }

    bool inserted = false;
    for(int j = 0; j < num_fastest; j++)
    {
      if(speed > fastest_Speeds[j])
      {
        // insert new speed
        int max_index = 3;
        if(num_fastest < RadarConfig.num_to_show -1) max_index = num_fastest;
        for(int k = max_index; k > j; k--)
        {
          fastest_Targets[k] = fastest_Targets[k-1];
          fastest_Speeds[k] = fastest_Speeds[k-1];
        }
        fastest_Targets[j] = i;
        fastest_Speeds[j] = speed;
        inserted = true;
        if(num_fastest < RadarConfig.num_to_show) num_fastest++;
        break;
      }
    }
    if(! inserted)
    {
      // Add new speed at end
      if(num_fastest < RadarConfig.num_to_show)
      {
        fastest_Targets[num_fastest] = i;
        fastest_Speeds[num_fastest] = speed;
        if(num_fastest < RadarConfig.num_to_show) num_fastest++;
      }
    }

  }

  // clear those deprecated targets
  for (int i = mpRadarData->Data.Targets.numTargets + 1; i < MAX_TARGETS; i++)
  {
      if (targetID[i])
          targetID[i] = 0;
      else
          break;
  }
    
  for(int i = RadarConfig.num_to_show - 1; i >= 0; i--)  // Iterate backwards so fastest targets are drawn last
  {
    int j = fastest_Targets[i];
    showTheTarget(i, j, &Video_Coords[j]);
  }
  
  // For some reason, I don't need to call the following function.
  // m_oprScene->update();
  return;
}

void topView::showTheTarget(int showNum, int targetNum, coord_struct * Video_Coords)
{
    if(targetNum >=0)
    {
//      coord_struct Radar_Coords;
//      coord_struct Video_Coords[MAX_TARGETS];
//      coord_struct Roadway_Coords;
      
      float FOVh = 2.0f * tanf(RadarConfig.FOVh * PI/360.0f) * Video_Coords->R;  // Width of camera horizontal FOV at target distance
      float fract;
      if(FOVh > 3.03f) {
        fract = 3.0f / FOVh;  // Fraction of screen width for 3 meter wide box
      } else {
        fract = 0.99f;                    // but limit to 99% of screen size
      }
      
      Q_ASSERT(m_oprScene && m_oprFrames);
      int camWidth = m_oprScene->width() - m_oprFrames->width();
      int camHeight = m_oprScene->height();
      int rWidth = camWidth * fract;  //Map to screen size
      int rHeight = camHeight * fract;
      if(rWidth < 1) rWidth = 1;
      if(rHeight < 1) rHeight = 1;

      int centerX = (0.5 * Video_Coords->X + 0.5) * (camWidth-1);
      int centerY = (-0.5 * Video_Coords->Z + 0.5) * (camHeight-1);
      
      // speed and distance strings
      char speedsArray[10], distancesArray[10];
      memset(speedsArray, 0, 10);
      memset(distancesArray, 0,10);
      
      // based on units and speedTenths and rangeTenths      // Speed Need unit and decimal place info here also
      switch( topView_mConf.units ) {
      case 0: // MPH
      case 2: // KNOTS
	{
	  float speed = Video_Coords->V / 1.609344 ;
	  float dst = Video_Coords->R  * 3.28084;
	  if( topView_mConf.speedTenths == 0 ) {
	    sprintf(speedsArray,"%3.0f MPH", speed);
	  }else{
	    sprintf(speedsArray,"%4.1f MPH", speed);
	  }
	  if( topView_mConf.rangeTenths == 0 ) {
	    sprintf(distancesArray,"%3.0f FT", dst);
	  }else{
	    sprintf(distancesArray,"%4.1f FT", dst);
	  }
	}
	break;
	case 1: // km/h
	if( topView_mConf.speedTenths == 0 ) {
      sprintf(speedsArray,"%3.0f km/h", Video_Coords->V);
	}else{
      sprintf(speedsArray,"%4.1f km/h", Video_Coords->V);
	}
	if( topView_mConf.rangeTenths == 0 ) {
      sprintf(distancesArray,"%3.0f m", Video_Coords->R);
	}else{
      sprintf(distancesArray,"%4.1f m", Video_Coords->R);
	}
	break;
      default:
	break;
      }

      
      // Distance

      QString speedsString = QString(speedsArray);
      QString distancesString = QString(distancesArray);
      
      if(showNum == 0)
      {
	if (m_targetRect1)
	{
	  m_oprScene->removeItem(m_targetRect1);
	  delete m_targetRect1;
	  m_targetRect1 = NULL;
	}
	mRect.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
	m_targetRect1 = m_oprScene->addRect(mRect, mP1);
	m_targetRect1->stackBefore(m_oprFramesProxy);
	
	// Show text
	m_oprFrames->setSpeedDistance(OPR_SDLABEL1, speedsString, distancesString);
      }
      else if(showNum == 1)
      {
	if (m_targetRect2)
	{
	  m_oprScene->removeItem(m_targetRect2);
	  delete m_targetRect2;
	  m_targetRect2 = NULL;
	}
	mRect.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
	m_targetRect2 = m_oprScene->addRect(mRect, mP2);
	m_targetRect2->stackBefore(m_oprFramesProxy);
	
	// Show text
	m_oprFrames->setSpeedDistance(OPR_SDLABEL2, speedsString, distancesString);
      }
      else if(showNum == 2)
      {
	if (m_targetRect3)
	{
	  m_oprScene->removeItem(m_targetRect3);
	  delete m_targetRect3;
	  m_targetRect3 = NULL;
	}
	mRect.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
	m_targetRect3 = m_oprScene->addRect(mRect, mP3);
	m_targetRect3->stackBefore(m_oprFramesProxy);
	
	// Show text
	m_oprFrames->setSpeedDistance(OPR_SDLABEL3, speedsString, distancesString);
      }
      else if(showNum == 3)
      {
	if (m_targetRect4)
	{
	  m_oprScene->removeItem(m_targetRect4);
	  delete m_targetRect4;
	  m_targetRect4 = NULL;
	}
	mRect.setRect(centerX - (rWidth / 2), centerY - (rHeight / 2), rWidth, rHeight);
	m_targetRect4 = m_oprScene->addRect(mRect, mP4);
	m_targetRect4->stackBefore(m_oprFramesProxy);
	
	// Show text
	m_oprFrames->setSpeedDistance(OPR_SDLABEL4, speedsString, distancesString);
      }
    }
    else
    {
        QString s("");
        if(showNum == 0)
        {
            if (m_targetRect1)
            {
                m_oprScene->removeItem(m_targetRect1);
                delete m_targetRect1;
                m_targetRect1 = NULL;
            }

            m_oprFrames->setSpeedDistance(OPR_SDLABEL1, s, s);
        }
        else if(showNum == 1)
        {
            if (m_targetRect2)
            {
                m_oprScene->removeItem(m_targetRect2);
                delete m_targetRect2;
                m_targetRect2 = NULL;
            }
            m_oprFrames->setSpeedDistance(OPR_SDLABEL2, s, s);
        }
        else if(showNum == 2)
        {
            if (m_targetRect3)
            {
                m_oprScene->removeItem(m_targetRect3);
                delete m_targetRect3;
                m_targetRect3 = NULL;
            }
            m_oprFrames->setSpeedDistance(OPR_SDLABEL3, s, s);
        }
        else if(showNum == 3)
        {
            if (m_targetRect4)
            {
                m_oprScene->removeItem(m_targetRect4);
                delete m_targetRect4;
                m_targetRect4 = NULL;
            }
            m_oprFrames->setSpeedDistance(OPR_SDLABEL4, s, s);
        }
    }
}
#endif

void topView::PlaybackExit()
{
    #ifdef IS_TI_ARM
    Utils& u = Utils::get();
    int retv = u.sendCmdToCamera(CMD_STOPPLAY, NULL);
    if(retv) DEBUG() << "Error: Stop Play, ret " << retv;
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
    #endif

    if(m_oprFrames)
    {
        m_oprFrames->hide();
        DEBUG() << "turn off operatFrame";
    }

    if(m_playbackScreen)
    {
        openMenuScreen(); //resume top view
        m_playbackScreen = false;
    }
}

#define DEFAULT_VEDIO_PATH  "/mnt/mmc/ipnc"
#define DEFAULT_FILTER      QDir::AllEntries

void topView::displayJPG()
{

 Utils::get().sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );


 if ( iconFrameShow == false ) {
   //   DEBUG() << "show ";
   if ( m_iconFrame ) {
     m_iconFrame->show();
     iconFrameShow = true;
   }
 }else{
   //   DEBUG() << "hide ";
   if ( m_iconFrame ) {
     m_iconFrame->hide();
     iconFrameShow = false;
   }
 }
 
#ifdef JUNK
  // Find the newest file to be display
  // Then possibily print ticket
  QDir m_videoDir;        // Video files path
  QFileInfoList m_FIList;

  ui->pb_displayjpg->setVisible(false);
  ui->pb_displayjpg->setVisible(true);

  QString vfPath = QString (qgetenv("LIDARCAM_VEDIO_PATH"));
  if (vfPath.isEmpty())
	vfPath = QString(DEFAULT_VEDIO_PATH);
  
  vfPath.append( "/");
  
  vfPath.append( Utils::get().session()->user()->loginName);
  if ( !QDir( vfPath ).exists() )
  {
    // if not add it.
    QDir().mkdir( vfPath );
  }
  
  m_videoDir.setPath(vfPath);

  //  DEBUG() << "Video Path: " << vfPath;
  
  QStringList m_defaultNameFilter;
  m_defaultNameFilter << "*.jpg";
  
  m_videoDir.setNameFilters(m_defaultNameFilter);
  
  m_videoDir.setFilter(DEFAULT_FILTER);
  m_videoDir.setSorting(QDir::Time);

  m_FIList.clear();
  m_FIList = m_videoDir.entryInfoList();
  
  //  DEBUG() << "Count " << m_FIList.count() ;

  // Need at least on entry
  if ( m_FIList.count() < 1 ) {
	return;
  }

  QFileInfo fi = m_FIList.at(0);
  displayFileName = fi.absoluteFilePath();

#ifdef LIDARCAM
   static bool display1 = true;
   if (display1 == true)
   {  // Display
      int value = (int)displayFileName.toLatin1().data();
      Utils::get().sendCmdToCamera(CMD_DISPLAY_PHOTO, value);
      display1 = false;
      ui->pb_menu->setEnabled(false);
      ui->pb_record->setEnabled(true);
      printButtonActive = true;
      ui->pb_record->setText("PRINT");
   }
   else
   {  // Remove the photo from displaying
      Utils::get().sendCmdToCamera(CMD_REMOVE_PHOTO, 0);
      display1 = true;
      ui->pb_menu->setEnabled(true);
      ui->pb_record->setEnabled(true);
      printButtonActive = false;
      ui->pb_record->setText("RECORD");
   }
#endif
#endif

   return;
}

//
//protected functions
//
#ifdef JUNK
bool topView::eventFilter(QObject *o, QEvent *e)
{
    static int i;
    QGraphicsView *v = qobject_cast<QGraphicsView *> (o);

    if (v && v == m_view && e->type() == QEvent::MouseButtonPress) {
        if (state::get().getState() == STATE_OPERATING_MENU) {
            qDebug() << "view event " << ++i << qPrintable (o->objectName());
            emit this->sig_topViewScreenReq(OPR_SCREEN);
            return true;
        }
        else if (state::get().getState() == STATE_OPERATING) {


        }
    }
    return false;
}
#endif

//
//private functions
//


int topView::openStartScreen()
{
    if (!this->m_startScene)
        this->m_startScene = new QGraphicsScene (this);
    if (!this->m_startScene)
        return -2;

    const QBrush b(QImage(":/hh1/splash480svg"));
    m_startScene->setBackgroundBrush(b);
    m_startScene->setSceneRect(0, 0, FULL_WIDTH, FULL_HEIGHT);
    m_view->setScene(m_startScene);
    return 0;
}

void* StartRadarPoll(void* radarDataArgs)
{
    UINT32 ret;

    topView * menuClass;
    radarDataArgs_t *pRadarDataArgs = (radarDataArgs_t *)radarDataArgs;
    //    RadarData_t * Radar = pRadarDataArgs->Radar;
    menuClass = (topView *)(pRadarDataArgs->menuClass);
    RadarMemory *pRadarData = Utils::get().RADARBuf();
    timespec start_time;
    timespec next_time;
    UINT64 delta_t_ns = 66666667;  // 1/15 second
    //    UINT64 delta_t_ns = 200000000;   // 1/5 second
    clock_gettime(CLOCK_MONOTONIC, &start_time);  // Get initial time
    long frame = 1;
    //    printf("Start time = %ld.%09lu\n", start_time.tv_sec, start_time.tv_nsec);

    // Set up 4 radar data display box
    QColor color;
    color = QColor(255, 150, 148, 255); // QColor(255, 128, 128, 255);
    QPen pen(color);
    menuClass->mP1 = pen;
    menuClass->mP1.setWidth(3);
    color = QColor(255, 255, 0, 255);
    pen.setColor(color);
    menuClass->mP2 = pen;
    menuClass->mP2.setWidth(3);
    color = QColor(216, 216, 255, 255); // QColor(204, 102, 255, 255);
    pen.setColor(color);
    menuClass->mP3 = pen;
    menuClass->mP3.setWidth(3);
    color = QColor(97, 213, 255, 255); // QColor(0, 204, 255, 255);
    pen.setColor(color);
    menuClass->mP4 = pen;
    menuClass->mP4.setWidth(3);

    //    DEBUG() << "radarPollId " << radarPollId ;
    while(radarPollId)
    {
        UINT64 frac = frame * delta_t_ns;
        frac = frac + start_time.tv_nsec;
        //        printf("frac = %llu\n", frac);
        long secs = frac/1000000000;
        frac = frac - secs * 1000000000;
        next_time.tv_sec = start_time.tv_sec + secs;
        next_time.tv_nsec = frac;
        frame++;
	// printf("Next time = %ld.%09lu\n", next_time.tv_sec, next_time.tv_nsec);

        if(!(ret = pRadarDataArgs->RadarData->CheckResponse()))
        {
            printf("Uh-oh...Did not complete Radar Data transfer on time\n");
        }
        else
        {
            // for(int i = 0; i < mpRadarData->Data.Targets.numTargets; i++)
            // {
            //   float speed = sqrtf( mpRadarData->Data.Targets.RadarTargets[i].xVelocity * mpRadarData->Data.Targets.RadarTargets[i].xVelocity +
            //                        mpRadarData->Data.Targets.RadarTargets[i].yVelocity * mpRadarData->Data.Targets.RadarTargets[i].yVelocity +
             //                        mpRadarData->Data.Targets.RadarTargets[i].zVelocity * mpRadarData->Data.Targets.RadarTargets[i].zVelocity);
            //   float distance = sqrtf( mpRadarData->Data.Targets.RadarTargets[i].xCoord * mpRadarData->Data.Targets.RadarTargets[i].xCoord +
            //                           mpRadarData->Data.Targets.RadarTargets[i].yCoord * mpRadarData->Data.Targets.RadarTargets[i].yCoord +
            //                           mpRadarData->Data.Targets.RadarTargets[i].zCoord * mpRadarData->Data.Targets.RadarTargets[i].zCoord);
            //   printf("Target %d speed = %8.1f distance = %8.1f\n", i, speed, distance);
            // }
            menuClass->emitHandleTargets();
        }
        pRadarDataArgs->RadarData->MessageA(&pRadarData->Data.Targets);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
	//
#ifdef JUNK
	// timespec cur_time;
        clock_gettime(CLOCK_MONOTONIC, &cur_time);
        if(cur_time.tv_nsec - next_time.tv_nsec > 10000000)  // Error > 10 milliseconds?
        {
            printf("Next time = %ld.%09lu\n", next_time.tv_sec, next_time.tv_nsec);
            printf("Woke up at  %ld.%09lu\n\n", cur_time.tv_sec, cur_time.tv_nsec);
        }
#endif
	//
    }
    pthread_exit(&ret);
    return NULL;
}

float topView::getNumber ( QString file, QString element )
{
  QString cmd;
  QProcess process;
  
  float value = 0.0;
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append(element);
  cmd.append(" ");
  cmd.append(file);
  cmd.append( "\"");
  
  //  DEBUG() << cmd;

  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  QString stdout = process.readAllStandardOutput();

  //  DEBUG() << stdout;

  QStringList list = stdout.split('\"');
  
  value =  atof(list.at(3).toLocal8Bit().constData());

  //DEBUG() << element << value;
    
  return value;
}

void topView::getData( QString file )
{
    //DEBUG() << "getData from json file " << file;

    violationData.theta_vs_ref = getNumber( file, "theta_ref_vs");
    violationData.theta_rs_ref = getNumber( file, "theta_ref_rs");
    violationData.theta_hs_ref = getNumber( file, "theta_ref_hs");

    violationData.theta_vs = getNumber( file, "theta_vs");
    violationData.theta_rs = getNumber( file, "theta_rs");
    violationData.theta_hs = getNumber( file, "theta_hs");

    violationData.Targets.numTargets = (int)getNumber( file, "numTargets");
    int i;

    //DEBUG() << MAX_TARGETS << " "  << violationData.Targets.maxTargets << " " << violationData.Targets.numTargets;

    for( i=0; i<violationData.Targets.numTargets; i++ ) {
      QString num("_");
      num.append(QString::number(i));
      violationData.Targets.RadarTargets[i].xCoord = getNumber( file,  QString("xCoord").append(num) );
      violationData.Targets.RadarTargets[i].yCoord = getNumber( file,  QString("yCoord").append(num) );
      violationData.Targets.RadarTargets[i].zCoord = getNumber( file,  QString("zCoord").append(num) );
      violationData.Targets.RadarTargets[i].xVelocity = getNumber( file,  QString("xVelocity").append(num) );
      violationData.Targets.RadarTargets[i].yVelocity = getNumber( file,  QString("yVelocity").append(num) );
      violationData.Targets.RadarTargets[i].zVelocity = getNumber( file,  QString("zVelocity").append(num) );
    }
}

void* PlaybackThread(void* radarDataArgs)
{
    DEBUG() << "playback: " << currentPlaybackFileName;

    state& v = state::get();

    timespec start_time;
    timespec next_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    //DEBUG() << "Start time" <<  start_time.tv_sec << start_time.tv_nsec;
    next_time = start_time;

    topView * menuClass;
    radarDataArgs_t *pRadarDataArgs = (radarDataArgs_t *)radarDataArgs;
    menuClass = (topView *)(pRadarDataArgs->menuClass);

    menuClass->json_md_fd = 0; //check if *.json and *md files exist

    int lastPoint = currentPlaybackFileName.lastIndexOf(".");
    QString fileNameNoExt = currentPlaybackFileName.left(lastPoint);
    QString jsonFile = fileNameNoExt;
    jsonFile.append(".json"); //zxm test for simulation
    QFile filejson(jsonFile);

    QString mdFile = fileNameNoExt;
    mdFile.append(".md");
    QFile file(mdFile);

    if(!filejson.open(QIODevice::ReadOnly | QFile::Truncate )) {
      DEBUG() << "Failed to open json file " << jsonFile;
      menuClass->json_md_fd = 0;
    }
    else
    {
        menuClass->json_md_fd++;
        filejson.close();

        DEBUG() << "mdFile " << mdFile;

        if(!file.open(QIODevice::ReadOnly | QFile::Truncate )) {
          DEBUG() << "Failed to open metat data file " << mdFile;
          menuClass->json_md_fd = 0;
        }else menuClass->json_md_fd++;
    }
    CCoordTransforms Transforms;

    if(menuClass->json_md_fd) //zxm get speed & distance and time stamp from meta file
    {
        DEBUG() << "getData from json file " << jsonFile;
        menuClass->getData(jsonFile);

        // Init the coordinate transformations
        Transforms.InitCoordTransforms(menuClass->RadarConfig.Xs,
                       menuClass->RadarConfig.Zs,
                       menuClass->RadarConfig.Zt,
                       menuClass->RadarConfig.FOVh * PI/180.0f,      // Convert to radians
                       menuClass->RadarConfig.FOVv * PI/180.0f);     // Convert to radians

        int targetNum;
        coord_struct Radar_Coords;
        coord_struct Roadway_Coords;

        int i;
        char data[sizeof(struct metaDataGet)];

        UINT64 ts1=0, ts2=0, nsec=0;
        long delta=0, secs=0, msec=0;
        menuClass->frameCnt=0;

        i=0;

        while(v.getPlaybackState() && i<(FRAMESPERSECOND * MAX_RECORDING_SECS))
        {
            memset(data, 0, sizeof(struct metaDataGet));

            file.read((char *)data, sizeof(struct metaDataGet)); //zxm read one record of 1012 bytes

            struct metaDataGet *ptr;

            ptr = (struct metaDataGet *)data;

            menuClass->violationData.timeMillieSecs = ptr->hdr.timeMilliSecs;
            menuClass->violationData.theta_vs = ptr->hdr.theta_vs;
            menuClass->violationData.theta_rs = ptr->hdr.theta_rs;
            menuClass->violationData.theta_hs = ptr->hdr.theta_hs;
            memcpy( (void *)&menuClass->violationData.Targets, (void *)&ptr->target, sizeof(Targets_t) );

            float theta_hs_rel = menuClass->violationData.theta_hs_ref - menuClass->violationData.theta_hs;
            if(theta_hs_rel > PI) theta_hs_rel = PI - theta_hs_rel;
            else if(theta_hs_rel < -PI) theta_hs_rel = PI + theta_hs_rel;

            float theta_rs_rel = menuClass->violationData.theta_rs - menuClass->violationData.theta_rs_ref;
            if(theta_rs_rel > PI) theta_rs_rel = PI - theta_rs_rel;
            else if(theta_rs_rel < -PI) theta_rs_rel = PI + theta_rs_rel;

            float theta_vs_rel = menuClass->violationData.theta_vs - menuClass->violationData.theta_vs_ref;
            if(theta_vs_rel > PI) theta_vs_rel = PI - theta_vs_rel;
            else if(theta_vs_rel < -PI) theta_vs_rel = PI + theta_vs_rel;

            Transforms.UpdateSensor(theta_vs_rel, theta_rs_rel, theta_hs_rel);

            for( targetNum=0; targetNum < menuClass->violationData.Targets.numTargets; targetNum++ )
            {
              RadarTargetResponse_t *target = &menuClass->violationData.Targets.RadarTargets[targetNum];

              memset( &Radar_Coords, 0, sizeof(coord_struct));
              memset( &menuClass->Video_Coords, 0, sizeof(coord_struct));
              memset( &Roadway_Coords, 0, sizeof(coord_struct));

              Radar_Coords.type = radar;
              Radar_Coords.X = target->xCoord;
              Radar_Coords.Y = target->yCoord;
              // Since the 3D radar does not measure target height (Z axis), fake it
              //    Radar_Coords.Z = target->zCoord;
              //    Radar_Coords.Z = mpRadarData->Data.RadarOrientation.targetHeight - mpRadarData->Data.RadarOrientation.radarHeight;  // Usually negative
              Radar_Coords.Z = 1.0f - 3.0f;  // Usually negative

              Radar_Coords.R = sqrtf(target->xCoord * target->xCoord +
                         target->yCoord * target->yCoord +
                         target->zCoord * target->zCoord);

              Radar_Coords.Vx = target->xVelocity;
              Radar_Coords.Vy = target->yVelocity;
              Radar_Coords.Vz = target->zVelocity;

              Radar_Coords.V = sqrtf(target->xVelocity * target->xVelocity +
                         target->yVelocity * target->yVelocity +
                         target->zVelocity * target->zVelocity);

              Radar_Coords.Theta_Vy = 0.0f;
              Radar_Coords.Theta_Vz = 0.0f;
              Radar_Coords.Ix = 0.0f;
              Radar_Coords.Iz = 0.0f;

              Roadway_Coords.type = roadway;

              Transforms.Transform(&Radar_Coords, &Roadway_Coords);

              menuClass->Video_Coords[targetNum].type = video;
              Transforms.Transform(&Radar_Coords, &menuClass->Video_Coords[targetNum]);

                if(i==0)
                {
                    ts1 = menuClass->violationData.timeMillieSecs;
                    ts2 = ts1;
                }

                delta = menuClass->violationData.timeMillieSecs - ts2;

                if(delta)
                {
                    menuClass->frameCnt++;
                    //DEBUG() <<  "read frame" << menuClass->frameCnt << "delta" << delta << "ms";
                    ts2 = menuClass->violationData.timeMillieSecs;
                    if(delta>200) delta = 67; //cut short long delay
                    nsec = next_time.tv_nsec + delta*1000000;
                    secs = nsec/1000000000;
                    next_time.tv_sec += secs;
                    if (secs) next_time.tv_nsec = (nsec - secs*1000000000);
                    else next_time.tv_nsec = nsec;
                    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
                }
            }
            i++;
        }//end of while loop for next read (each read gets four targets data)

        secs = (ts2-ts1)/1000;
        msec = (ts2-ts1) - secs*1000;
        DEBUG() <<  "total time elapsed" << secs << "s" << msec << "ms";

        file.close();
    }
    else //when json and *.md files not exist
    {
        UINT64 delta_t_ns = 66666667;  // 1/15 second
        int frame=1;

        while(v.getPlaybackState() && frame < 300) //timeout at 20s, 60*15=900 for 60s
        {
            UINT64 frac = frame * delta_t_ns;
            frac = frac + start_time.tv_nsec;
            long secs = frac/1000000000;
            frac = frac - secs * 1000000000;
            next_time.tv_sec = start_time.tv_sec + secs;
            next_time.tv_nsec = frac;
            frame++;
        #ifndef IS_TI_ARM //run on ubuntu
            //DEBUG() << "frame " << frame << "time "<<  secs << "s" << frac/1000000 << "ms";
        #endif
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL); //this line will hold/wait processing
        }//end of while

        #ifndef IS_TI_ARM //run on ubuntu
            DEBUG() << "playbackThread exit @ frame " << frame;
            DEBUG() << "playbackThread time = " << (next_time.tv_sec - start_time.tv_sec);
        #endif
    }

    v.setPlaybackState(0); //end playback
    pthread_exit(NULL);
    return NULL;
}

//
//private slots
//
void topView::initProcesses()
{
  // Get Tilt Sensor interface
  pSensor = new TiltSensor;
  pSensor->init();
}

void topView::powerDownSystem( void)
{
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
  // msgbox
  QString quest_str = QString("System Powering Down Continue?");
  QMessageBox msgBox;
  msgBox.setText(QObject::tr(quest_str.toStdString().c_str()));
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.setIcon(QMessageBox::Warning);
  QPalette p;
  p.setColor(QPalette::Window, Qt::red);
  msgBox.setPalette(p);
  if(msgBox.exec() == QMessageBox::No){
    return;
  } 

  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
  
  system( "/sbin/halt -p");
  // Should never return here, 
  return;
}

#ifdef LIDARCAM
void topView::lidarCamTriggerPulled()
{
#ifndef SPEED_INROOM_DEBUG
  // Normal operation
  m_newRange = u.lidarRange();
  mNewSpeed = u.lidarSpeed();
#else
  // In room
  mNewSpeed = 41.5;
  m_newRange -= 10;
#endif
  
  //DISPLAY the radar distance and range
  // DEBUG()  << " m_range " << m_range << " m_newRange " << m_newRange;
  if ( m_distance )
  {
    m_topScene->removeItem( m_distance);
    delete m_distance;
    m_distance = NULL;
  }
  
  if ( m_range != m_newRange )
  {
    m_range = m_newRange;
    displayRange( m_range );
  }
  
  //display the radar speed
  // need to determine if this is km/h or mph
  if (mRangeOnly == false &&  mSpeed != mNewSpeed )
  {
    mSpeed = mNewSpeed;
    if( m_speed )
    {
      m_topScene->removeItem(m_speed);
      delete m_speed;
      m_speed = NULL;
    }
    switch( u.lidarDataBuf()->lidarStruct.DISPLAY_UNITS )
    {
    case 0: // MPH
      if ((mSpeed > 6.0) || (mSpeed < -6.0) )
	displaySpeed( mSpeed );
      break;
    case 1: // km/h
      if ((mSpeed > 10.0) || (mSpeed < -10.0) )
	displaySpeed( mSpeed );
      break;
    default:
      break;
    }
  }
  
  setRecordingText();  // Set Watermark
  if ((m_recording == false) && (ui->pb_record->isEnabled() == true))
  {
    // Monitor the Speed, start recording if needed
    monitorSpeed();
  }
}

void topView::lidarCamTriggerNotPulled()
{
  if (mAutoRecording == true)
  {  // Auto recording is still on
    exeRecord();   // Stop it
    mAutoRecording = false;
  }
  return;
}
#endif //LIDARCAM

#ifdef HH1
void topView::hh1TriggerPulled( void )
{
  // connected to hh1Trigger and will be activated from hardbuttons.cpp via emit
  int ss = state::get().getState(); 

  //  DEBUG() << "State " << ss;

  switch ( ss ) {
  case STATE_START:
  case STATE_TOP_VIEW:
  case STATE_OPERATING:
    break;
    // trigger is ignored for all other states
  default:
    return;
  }
  
  // Make sure trigger does not stop current violation
  if ((m_recording == false) && (ui->pb_record->isEnabled() == true)) {
    if ( ss == STATE_OPERATING ) {
      // if in the "START" screen it is a violation
      violation();
    }else{
      // STATE_TOP_VIEW
      // trigger means START
      openOperateScreen();
    }
  }
  return;
}

#ifdef NOTUSEDNOW
void topView::hh1TriggerNotPulled( void )
{
  return;
}
#endif // NOTUSEDNOW
#endif //HH1

void topView::violation( void )
{
  violationTimeStamp = timeStamp;

  backGround::get().saveViolation( violationTimeStamp,
				   theta_vs_ref, theta_rs_ref, theta_hs_ref, // base from the start
				   theta_vs,     theta_rs,     theta_hs,     // current values
				   RadarConfig.Xs,
                                   RadarConfig.Zs,
                                   RadarConfig.Zt,
                                   RadarConfig.FocalLength,
				   &mpRadarData->Data.Targets );             // Radar data

  //  hexDump((char *)"metaData", &mpRadarData->Data.Targets, sizeof(Targets_t));

  exeRecord();

  //QTextStream logDebug(logFile);
  logDebug << "Start recording: ";

  float x,y,z,r;
  for (int i = 0; i < 4; i++)
  {
        if (i >= mpRadarData->Data.Targets.numTargets)
            continue;

        x = mpRadarData->Data.Targets.RadarTargets[i].xCoord;
        y = mpRadarData->Data.Targets.RadarTargets[i].yCoord;
        z = mpRadarData->Data.RadarOrientation.targetHeight - mpRadarData->Data.RadarOrientation.radarHeight;  // Usually negative
        r = sqrtf(x*x + y*y + z*z);

        logDebug << timeStamp << " : target " << mpRadarData->Data.Targets.RadarTargets[i].targetId << " is at " << r << "m; ";
        DEBUG()  << timeStamp << " : target " << mpRadarData->Data.Targets.RadarTargets[i].targetId << " is at " << r << "m";
        lastDistance[i] = 0;  // important to clear it to 0 here
        maxSpeed[i] = 0; // clear the max speed of the targets
  }
  logDebug << std::endl << std::flush;
  return;
}

void topView::lowBattery( void )
{
  DEBUG();
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
  // msgbox
  QString quest_str = QString("Please Replace Battery");
  QMessageBox msgBox;
  msgBox.setText(QObject::tr(quest_str.toStdString().c_str()));
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setIcon(QMessageBox::Warning);
  QPalette p;
  p.setColor(QPalette::Window, Qt::red);
  msgBox.setPalette(p);
  msgBox.exec();
  
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  return;
}

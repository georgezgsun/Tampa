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

#ifdef HH1
#include "widgets/icon_frame/icon_frame.h"
#endif
#include "db_types.h"
#include "play_back.h"
#include "printTicket.h"
#include "back_ground.h"
#include "hardButtons.h"
#include "hh1MetaData.h"

#define ONE_BILLON      1000000000L
#define FOVH  ( 6.2f/35.0f * 180.0/PI )
#define FOVV  ( 4.65f/35.0f * 180.0/PI )
#define THETA_HS -5.0f
#define THETA_VS -3.0f

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

bool printButtonActive = false;
QString displayFileName;

void* StartRadarPoll(void* radarDataArgs);

pthread_t radarPollId;

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

    this->TopViewInitTimer();

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
    
    //    DEBUG() << "loc spdLimit " << loc.speedLimit;
    //    DEBUG() << "loc capSpd " << loc.captureSpeed;

    // Display zoom value on top screen
    
#ifdef LIDARCAM
    // figure this out for HH1
    int percent = u.FGBuf()->State_Of_Charge;
    DEBUG() << "Percent " << percent;
    if( percent > 10 ) {
      u.Send_Msg_To_PIC( Set_Power_Led_Green );
    }else{
      u.Send_Msg_To_PIC( Set_Power_Led_Red );
    }
#endif

    this->m_view->installEventFilter(this);

    capture_file_is_open = false;

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

    hardButtons& uu = hardButtons::get();
    uu.settopView( this );
    
    int vol = 0;
    // Get volume from the database
    SysConfig & cfg = u.getConfiguration();
    vol = cfg.volume;

    // amixer sset PCM 100%
    vol = vol * 25;

    u.setVolume( vol );
}

topView::~topView()
{
  //    DEBUG("Running topView destructor for object %08lx\n", (long unsigned int)this);
  if (m_TopViewTimer) {
    m_TopViewTimer->stop();
    delete m_TopViewTimer;
  }

#ifdef HH1
  pthread_t radarPollIdSave = radarPollId;
  radarPollId = 0;
  if( radarPollIdSave != 0 ) {
    pthread_join( radarPollIdSave, NULL );
  }
#endif

  delete ui;
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
  
    Q_ASSERT(m_oprFrames && m_oprScene && m_iconFrame);
  
    DEBUG() << "switch opr screen flag " << flag << "State " << state::get().getState();
    Utils& u = Utils::get();
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
    
    if (flag == OPR_SCREEN) {
      if ( state::get().getState() == STATE_OPERATING) {
        return;
      }
      m_iconFrame->hide();
      m_oprFrames->setCurrentIndex(0);
      state::get().setState(STATE_OPERATING );
    } else if (flag == OPR_MENU_SCREEN) {
	  if ( state::get().getState() == STATE_OPERATING_MENU) {
		return;
    }
      m_iconFrame->show();
      m_oprFrames->setCurrentIndex(1);
      state::get().setState(STATE_OPERATING_MENU );
    } else if (flag == REOPEN_HOME_SCREEN) {
#ifdef CAPTURE_TEXT
      if (capture_file_is_open)
      {
          fclose(capture_fd);
      }
      DEBUG() << "Capture File closed";
#endif
      this->openHomeScreen(REOPEN_HOME_SCREEN);
    }
    else {
	//error
    }
}

void topView::openOperateScreen(void)
{
  unsigned int response;
  
  // DEBUG() << "openOperateScreen " << "State " << state::get().getState();
#ifdef IS_TI_ARM
#ifdef CAPTURE_TEXT
  capture_fd = fopen(TEXT_CAPTURE_FILE, "w");
  capture_file_is_open = true;
#endif
  
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  pSensor->ReadSensor(&theta_vs_ref, &theta_rs_ref, &theta_hs_ref);
  
  if(theta_hs_ref > PI) theta_hs_ref = PI - theta_hs_ref;
  else if(theta_hs_ref < -PI) theta_hs_ref = PI + theta_hs_ref;
  
  if(theta_rs_ref > PI) theta_rs_ref = PI - theta_rs_ref;
  else if(theta_rs_ref < -PI) theta_rs_ref = PI + theta_rs_ref;
  
  if(theta_vs_ref > PI) theta_vs_ref = PI - theta_vs_ref;
  else if(theta_vs_ref < -PI) theta_vs_ref = PI + theta_vs_ref;
  
#ifdef CAPTURE_TEXT
  if (capture_file_is_open)
  {
    fprintf(capture_fd, "Sensor Reference\n  theta_hs_ref = %7.4f\n  theta_rs_ref = %7.4f\n  theta_vs_ref = %7.4f\n",
	    theta_hs_ref, theta_rs_ref, theta_vs_ref);
  }
#endif
#else
  theta_hs_ref = 0.0f;
  theta_rs_ref = 0.0f;
  theta_vs_ref = 0.0f;
#endif
  
  mpRadarData->Data.RadarMode.trackingDirection = 2;
  mpRadarData->Data.RadarMode.transmit = 1;
  CRadarData &RadarData = backGround::get().getRadarData();	// This object provides the Oculii radar interface
  RadarData.Message4(&mpRadarData->Data.RadarMode);  // Set the Radar Mode
  while((response = RadarData.CheckResponse()) == 0);
#ifdef CAPTURE_TEXT
  if (capture_file_is_open)
  {
    fprintf(capture_fd, "Radar Mode:\n");
    fprintf(capture_fd, "  Tracking direction: %hhu\n  Transmit: %hhu\n", mpRadarData->Data.RadarMode.trackingDirection, mpRadarData->Data.RadarMode.transmit);
  }
#endif
  //QGraphicsScene *tmp = new QGraphicsScene (this);
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
  
  //add left frame
  if( m_iconFrame ) {
    delete m_iconFrame;
    m_iconFrame = NULL;
  }
  
  m_iconFrame = new iconFrame(40, 240);
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

void topView::updateRecordView(bool endRecord)
{
  UNUSED( endRecord );
  
  Q_ASSERT(m_topScene);
  
   if (!m_recordMark)
   {
      QRect r(10, 14, 12, 12);
      m_recordMark = m_topScene->addEllipse(r);
      m_recordMark->setBrush(Qt::red);
      m_recordMark->setPos(6, 6);
   }

   m_topScene->removeItem(m_recordMark);
   m_topScene->addItem(m_recordMark);
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
    struct SysConfig config = u.getConfiguration();
    backLightTime = config.backlightOff * 60;  // Time is in seconds, config is minutes, 
    powerOffTime = config.powerOff * 60; // Time is in seconds, config is minutes, 
    
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
        mpRadarData->Data.RadarOrientation.targetHeight = 1.0f;
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
  connect( ui->pb_zoomin, SIGNAL(clicked()), this, SLOT(zoomin()) );
  u.setHardButtonMap( 0, ui->pb_zoomin);
  connect( ui->pb_focus, SIGNAL(clicked()), this, SLOT(focusMode()) );
  u.setHardButtonMap( 1, ui->pb_focus);
  connect( ui->pb_menu, SIGNAL(clicked()), this, SLOT(openMenuScreen()) );
  u.setHardButtonMap( 2, ui->pb_menu);

#ifdef HH1
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
 
int htSeen = 0;
void topView::emitHandleTargets()
{
  if (state::get().getState() == STATE_OPERATING || state::get().getState() == STATE_OPERATING_MENU) {
    emit handleTargets();
#ifdef JUNK
    if( htSeen % (3*FRAMESPERSECOND) == 0 ) {
      DEBUG() << "Elapsed " << sysTimer.elapsed();
    }
    htSeen++;
    //  QCoreApplication::instance()->processEvents();
#endif
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
    m_TopViewTimer->start( 1000 );
}

void topView::TopViewTimerHit()
{
  //  static int count;
  currentSeconds++;
  //  DEBUG() << "Elapsed " << sysTimer.elapsed() << "CurrentSeconds " << currentSeconds;
	
#if defined(LIDARCAM) || defined(HH1)

   // monitor speed and record if over the speed limit
   // every thing happens when the trigger is Pulled
   if (true == triggerPulled )
   {
#ifdef LIDARCAM
     Utils& u = Utils::get();
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
#endif
   }
   else
   {  // Trigger not pulled
      if (mAutoRecording == true)
      {  // Auto recording is still on
         exeRecord();   // Stop it
         mAutoRecording = false;
      }
   }

  if ( true == m_recording )
  {   // recording is going     
#ifdef LIDARCAM
     int zoomDis;
     int zoomNum = mCamSetting.zoom.toInt();
     int displayUnits = u.getDisplayUnits();
     if (mCamSetting.focus == (QString)"AUTO")
     {  // Auto focus mode
        if (displayUnits == 1)  // kmH
           zoomDis = focusM[zoomNum-1];
        else  // MPH or KNOTS
           zoomDis = focusFt[zoomNum-1];
     }
     else
     {  // Manual focus mode
        zoomDis = mCamSetting.focus1;
     }

     if (!mPhotoNum && m_range <= (zoomDis + 10))
     {
        QString fileName = u.getRecordingFileName() + "_1.jpg";
        u.takePhoto(fileName, m_range);
        mPhotoNum++;
     }
#endif
     // how long the recording has been going?
     mRecordingSecs++;
     //     DEBUG() << "elapsed time " << sysTimer.elapsed() << "mRecordingSecs " << mRecordingSecs;
     if (mRecordingSecs > MAX_RECORDING_SECS)
     {   // Stop recording since maximum seconds are reached
        exeRecord();
        mAutoRecording = false;
     }
  }
#else
//   mNewSpeed = 38.5;    // Remove later
//   m_newRange = 176;
//   setRecordingText();  // Set Watermark
#endif

	// Activity moniotoring
   if( activity == 0 )
   {
     // No activity
     if( inactive == false )
     {
       // changing active to inactive and save the start of inactive
       // DEBUG() << "Start inactive !!!";
       inactive = true;
       inactiveStart = currentSeconds;
     }
     else
     {
       // This is where we are inactive and determining how long we are inactive
       qint64 elapsedTime = currentSeconds - inactiveStart;
       
       // Sleep the coldfire if tilt
       // Hardcoded to 60 seconds for a possible tilt SLEEP to coldfire
      if( coldFireSleep == false )
      {
        if( (elapsedTime % SLEEPCOLDFIREWHENTILTED) == 0 )
        {
	  DEBUG() << "Inactivity " <<  SLEEPCOLDFIREWHENTILTED  << " seconds seen" << currentSeconds ;
	  // check if tilt active and over tilt to do the sleep
#ifdef LIDARCAM
	  if( u.tiltSeen() == true )
	    {
	      // sleep the coldFire
	      DEBUG() << "coldFireSleep set" << currentSeconds ;
	      u.sendMbPacket( (unsigned char) CMD_SLEEP, 0, NULL, NULL );
	      u.Send_Msg_To_PIC( Set_Power_Led_Both );
	      coldFireSleep = true;
	    }
#endif
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
	    hardButtons& h = hardButtons::get();
	    h.Send_Display_Brightness( 0 );
	    backLightOn = false;
	  }
	}
      }
      
      // possible logout
      
#ifdef LIDARCAM
      // possible powerdown
      if( powerOffTime > 0 ) {
	if( ( elapsedTime % powerOffTime) == 0 ) {
	  DEBUG() << "Inactivity " << powerOffTime << " seconds seen POWER OFF" << currentSeconds ;
	  
	  // close the local database
	  u.closeUserDB();
	  
	  // sync the linux
	  system("/bin/sync");
	  
	  // send message to pic and turn off power
	  u.Send_Msg_To_PIC( Set_Power_Off);
	  m_TopViewTimer->stop();
	  QApplication::exit(0);
	  return;
	}
      }
#endif
     }
   }
   else
   {  // Active
     // There is activity
     if( inactive == true )
     {
       // Changing from inactive to active!!
       // do what is needed
       // look at turn on touch screen
       // unSLEEP the coldfire
       //		DEBUG() << "Reactivate !!!";
       if ( coldFireSleep == true )
       {
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
       if ( backLightOn == false )
       {
	 DEBUG() << "Turn on Backlight" << currentSeconds ;
#ifdef LIDARCAM
	 hardButtons& h = hardButtons::get();
	 int b = u.lidarDataBuf()->lidarStruct.HUD_BRIGHTNESS;
	 h.Send_Display_Brightness( b * 20 );
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
   state& v = state::get();
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

    return;
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
    
    printTicket::get().print(ticketFileName);

    return;
  }
  
   Utils& u = Utils::get();
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

   if ((m_recording == false) && (ui->pb_record->isEnabled() == true))
   {
      DEBUG() << "recording start ";
      m_recording = true;
      mPhotoNum = 0; // Can take photo now
      ui->pb_record->setText("STOP\nRECORD");

      updateRecordView(false);
      ui->pb_menu->setEnabled(false);
#if defined(LIDARCAM) || defined(HH1)
      hardButtons::get().setHardButtonMap( 2, NULL);
      int retv = u.sendCmdToCamera(CMD_RECORD, 0);
      if (retv != 0)
        qWarning() << "set camera record failed";
#endif
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
      
      QString fileName = u.getRecordingFileName() + ".md";
      
      DEBUG() << "Filename " << fileName;
      
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
	// init the zoom on the camera
	initZoom();

#if defined(LIDARCAM) || defined(HH1)
   Utils& u = Utils::get();
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

   //   DEBUG() << "loc spdLimit " << loc.speedLimit;
   //   DEBUG() << "loc capSpd " << loc.captureSpeed;

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
   m_rollLine = m_topScene->addLine(halfLen, centerY, centerX + halfLen, centerY, p);
   m_vertLine = m_topScene->addLine(centerX, centerY - 30, centerX, centerY + 30, p);
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


void topView::monitorSpeed()
{
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
      exeRecord();   // Start recording
      mAutoRecording = true;
   }
   return;
}

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
	       SysConfig & cfg = u.getConfiguration();
               memcpy(SN_str, cfg.serialNumber.toStdString().c_str(), 8);
               
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

void topView::zoomin()
{
#ifdef LIDARCAM
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
#endif
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
#ifdef LIDARCAM
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
#endif
}

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
      mCamSetting.zoom = QString("1");
      mCamSetting.focus = QString("AUTO");
      mCamSetting.focus1 = 500;
      mCamSetting.shutter = QString("AUTO");
      mCamSetting.color = QString("AUTO");
      mCamSetting.iris = QString("AUTO");
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

#ifdef JUNK
static int ptSeen = 0;
static int ptSeen1 = 0;

if( ptSeen % (3*FRAMESPERSECOND) == 0 ) {
    DEBUG() << "Elapsed " << sysTimer.elapsed();
  }
  ptSeen++;
#endif

  if(!m_oprScene) {
#ifdef JUNK
    if( ptSeen1 % (3*FRAMESPERSECOND) == 0 ) {
      DEBUG() << "Elapsed " << sysTimer.elapsed();
    }
    ptSeen1++;
#endif
    return;
  }

  // Raw data from the senson
  pSensor->ReadSensor(&theta_vs, &theta_rs, &theta_hs);
  
  // Relative values cacuated here
  float theta_hs_rel = theta_hs_ref - theta_hs;
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
  
#ifdef CAPTURE_TEXT
  if (capture_file_is_open)
  {
    fprintf(capture_fd, "Sensor Position\n  theta_hs_rel = %7.4f\n  theta_rs_rel = %7.4f\n  theta_vs_rel = %7.4f\n",
	    theta_hs_rel, theta_rs_rel, theta_vs_rel);
  }
#endif
  
  // Set up the transform with the current sensor data
  Transforms.UpdateSensor(theta_vs_rel, theta_rs_rel, theta_hs_rel);

#ifdef JUNK
  // Debug
  float accTotal = sqrtf(((float)(accBuf.xAxis) * (float)(accBuf.xAxis)) +
			 ((float)(accBuf.yAxis) * (float)(accBuf.yAxis)) +
			 ((float)(accBuf.zAxis) * (float)(accBuf.zAxis)));
  frameMetaData.accData_x = (float)(accBuf.xAxis)/accTotal;
  frameMetaData.accData_y = (float)(accBuf.yAxis)/accTotal;
  frameMetaData.accData_z = (float)(accBuf.zAxis)/accTotal;
  
  float magTotal = sqrtf(((float)(magBuf.xAxis) * (float)(magBuf.xAxis)) +
			 ((float)(magBuf.yAxis) * (float)(magBuf.yAxis)) +
			 ((float)(magBuf.zAxis) * (float)(magBuf.zAxis)));
  frameMetaData.magData_x = (float)(magBuf.xAxis)/magTotal;
  frameMetaData.magData_y = (float)(magBuf.yAxis)/magTotal;
  frameMetaData.magData_z = (float)(magBuf.zAxis)/magTotal;
  //    float rollAngle = atan2f(frameMetaData.accData_x, frameMetaData.accData_z);
  //    float pitchAngle = atan2f(frameMetaData.accData_y, frameMetaData.accData_z);
#endif

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
    else if(centerY > camHeight - 1) centerX = camHeight -1;

//#define ROLL_LINE_DEBUG
#ifdef ROLL_LINE_DEBUG
    printf("Sensor Relative: ");
    printf("hs %f rs %f vs %f\n", theta_hs_rel * DEG_PER_RAD, theta_rs_rel * DEG_PER_RAD, theta_vs_rel * DEG_PER_RAD);
    printf("Xdev = %f Ydev = %f Zdev = %f\n", xr, 0.0f, zr);
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
  
  for(int i = 0; i < 4; i++) {
    fastest_Targets[i] = -1;  // Init indexes for fastest targets to invalid index
    fastest_Speeds[i] = 0.0f; // and speeds to zero
  }
  
  //  Note:  We could begin processing targets against violation rules here.
  //         It could also be done after sorting for fastest targets if that provides an advantage
  
#ifdef CAPTURE_TEXT
  if (capture_file_is_open)
  {
    fprintf(capture_fd, "Targets Poll:\n");
        fprintf(capture_fd, "  Number of Targets: %d\n", mpRadarData->Data.Targets.numTargets);
  }
#endif

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
  int num_fastest = 0;  // Init to no fastest targets
  for(int i = 0; i < mpRadarData->Data.Targets.numTargets; i++)
  {
    //        speed = mpRadarData->Data.extraData[i].speed;
    speed = sqrtf( mpRadarData->Data.Targets.RadarTargets[i].xVelocity * mpRadarData->Data.Targets.RadarTargets[i].xVelocity +
		   mpRadarData->Data.Targets.RadarTargets[i].yVelocity * mpRadarData->Data.Targets.RadarTargets[i].yVelocity +
		   mpRadarData->Data.Targets.RadarTargets[i].zVelocity * mpRadarData->Data.Targets.RadarTargets[i].zVelocity);
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
  
  //    printf("num_fastest = %d, numTargets = %d\n", num_fastest, mpRadarData->Data.Targets.numTargets);
  //    for(int j = 0; j < num_fastest; j++)
  //    {
  //    printf("  Fastest speed %d: Speed = %f, index = %i\n", j, fastest_Speeds[j], fastest_Targets[j]);
  //    }
  
  
  for(int i = RadarConfig.num_to_show - 1; i >= 0; i--)  // Iterate backwards so fastest targets are drawn last
  {
    showTheTarget(i, fastest_Targets[i]);
  }
  
  // For some reason, I don't need to call the following function.
  // m_oprScene->update();
  return;
}

void topView::showTheTarget(int showNum, int targetNum)
{
    if(targetNum >=0)
    {
      coord_struct Radar_Coords;
      coord_struct Video_Coords[MAX_TARGETS];
      coord_struct Roadway_Coords;
      RadarTargetResponse_t *target = &mpRadarData->Data.Targets.RadarTargets[targetNum];
      
      memset( &Radar_Coords, 0, sizeof(coord_struct));
      memset( &Video_Coords, 0, sizeof(coord_struct));
      memset( &Roadway_Coords, 0, sizeof(coord_struct));
	      
      Radar_Coords.type = radar;
      Radar_Coords.X = target->xCoord;
      Radar_Coords.Y = target->yCoord;
      // Since the 3D radar does not measure target height (Z axis), fake it
      //    Radar_Coords.Z = target->zCoord;
      Radar_Coords.Z = mpRadarData->Data.RadarOrientation.targetHeight - mpRadarData->Data.RadarOrientation.radarHeight;  // Usually negative
      
      Radar_Coords.R = sqrtf(target->xCoord * target->xCoord +
			     target->yCoord * target->yCoord +
			     target->zCoord * target->zCoord);
      
      //      Radar_Coords.Vx = target->xVelocity;
      Radar_Coords.Vy = target->yVelocity;
      Radar_Coords.Vz = target->zVelocity;
      
      Radar_Coords.V = sqrtf(target->xVelocity * target->xVelocity +
			     target->yVelocity * target->yVelocity +
			     target->zVelocity * target->zVelocity);
      
      // TODO      Radar_Coords.Theta_Y = RadarConfig.Theta_hs;
      // TODO      Radar_Coords.Theta_Z = RadarConfig.Theta_vs;
      Radar_Coords.Theta_Vy = 0.0f;
      Radar_Coords.Theta_Vz = 0.0f;
      Radar_Coords.Ix = 0.0f;
      Radar_Coords.Iz = 0.0f;
      
      Roadway_Coords.type = roadway;
      Video_Coords[targetNum].type = video;
      
      Transforms.Transform(&Radar_Coords, &Roadway_Coords);
      // printf("Target %d at radar X = %f, Y = %f, Z = %f\n", targetNum, Radar_Coords.X, Radar_Coords.Y, Radar_Coords.Z);
      // printf("  maps to roadway X = %f, Y = %f, Z = %f\n", Roadway_Coords.X, Roadway_Coords.Y, Roadway_Coords.Z);
      
      Transforms.Transform(&Radar_Coords, &Video_Coords[targetNum]);
      
      // printf("Got target %d at speed = %f, distance = %f for target %d in showTheTarget()\n", showNum, Radar_Coords.V, Radar_Coords.R, targetNum);
      // printf("Target %d maps to video X = %f, Z = %f\n Ix = %f, Iz = %f\n", targetNum, Video_Coords[targetNum].X, Video_Coords[targetNum].Z,
      //      Video_Coords[targetNum].Ix, Video_Coords[targetNum].Iz);
      
      float FOVh = 2.0f * tanf(RadarConfig.FOVh * PI/360.0f) * Radar_Coords.R;  // Width of camera horizontal FOV at target distance
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
      //    int centerX = m_topScene->width() / 2;
      //    int centerY = m_topScene->height() / 2;

      int centerX = (0.5 * Video_Coords[targetNum].X + 0.5) * (camWidth-1);
      int centerY = (-0.5 * Video_Coords[targetNum].Z + 0.5) * (camHeight-1);
      
 #ifdef CAPTURE_TEXT
      if (capture_file_is_open)
      {
	fprintf(capture_fd, "  Target %d: Speed = %7.1f Distance = %7.1f\n", showNum, Radar_Coords.V, Radar_Coords.R);
      }
#endif
      // speed and distance strings
      char speedsArray[10], distancesArray[10];
      memset(speedsArray, 0, 10);
      memset(distancesArray, 0,10);
      
      // Speed Need unit and decimal place info here also
      sprintf(speedsArray,"%3.0f km/h", Radar_Coords.V);
      QString speedsString = QString(speedsArray);
      
      // Distance
      sprintf(distancesArray,"%4.1f m", Radar_Coords.R);
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

#define DEFAULT_VEDIO_PATH  "/mnt/mmc/ipnc"
#define DEFAULT_FILTER      QDir::AllEntries

void topView::displayJPG()
{

  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  backGround& bg = backGround::get();
  
  // Temp test code 
  // called with the base ref values, current values
  // all data from the radar
  // float   theta_vs_ref, theta_rs_ref, theta_hs_ref
  // float   theta_vs, theta_rs, theta_hs
  // Targets_t *
  
  violationTimeStamp = timeStamp;
  DEBUG() << "elapsed time " << sysTimer.elapsed() << "voilationTimeStamp " << violationTimeStamp;

  bg.saveViolation( violationTimeStamp,
		    theta_vs_ref, theta_rs_ref, theta_hs_ref, // base from the start
		    theta_vs,     theta_rs,     theta_hs,     // current values
		    &mpRadarData->Data.Targets );             // Radar data

  //  hexDump((char *)"metaData", &mpRadarData->Data.Targets, sizeof(Targets_t));

  DEBUG() << "elapsed time " << sysTimer.elapsed();
  exeRecord();
  DEBUG() << "elapsed time " << sysTimer.elapsed();
  //  QString fileName = u.getRecordingFileName() + "_1.jpg";
  //  DEBUG() << fileName << " m_range " << m_range;
  //  DEBUG() << "elapsed time " << sysTimer.elapsed();
  //  u.takePhoto(fileName, m_range);
  //  DEBUG() << "elapsed time " << sysTimer.elapsed();
  
  // End of temp test code
  
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
    color = QColor(255, 128, 128, 255);
    QPen pen(color);
    menuClass->mP1 = pen;
    menuClass->mP1.setWidth(3);
    color = QColor(255, 255, 0, 255);
    pen.setColor(color);
    menuClass->mP2 = pen;
    menuClass->mP2.setWidth(3);
    color = QColor(204, 102, 255, 255);
    pen.setColor(color);
    menuClass->mP3 = pen;
    menuClass->mP3.setWidth(3);
    color = QColor(0, 204, 255, 255);
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
  QString quest_str = QString("System Powering Down");
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

  system( "/sbin/halt -p");
  // Should never return here, 
  return;
}

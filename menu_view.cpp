#include <QDialog>
#include <QLineEdit>
#include <QGraphicsProxyWidget>
#include <QPen>
#include "menu_view.h"
#include "ui_menu_view.h"
#include "loc_setup.h"
#include "lidar_setup.h"
#include "top_view.h"
#include "mode_sel.h"
#include "sys_opt.h"
#include "user_mgr.h"
#include "dev_info.h"
#include "file_mgr.h"
#include "edit_user.h"
#include "video_setup.h"
#include "wifi_setup.h"
#include "bt_setup.h"
#include "enet_setup.h"
#include "admin.h"
#include "user_access.h"
#include "factory.h"
#include "calibrateMag3110.h"
#include "service.h"
#include "serv_opt.h"
#include "calib_data.h"
#include "upload_mgr.h"
#include "loc_save.h"
#include "camera_setup.h"
#include "dist_measure.h"
#include "play_back.h"
#include "hardButtons.h"
#include "state.h"
#include "utils.h"
#include "ColdFireCommands.h"
#include "global.h"
#include "debug.h"
#include "Message_Queue_Struct.h"
#include "ColdFireMsg.h"
#include "security.h"
#ifndef HH1
#include "metaData.h"
#endif
#include "log_file.h"
#include <QMessageBox>
#include "password.h"
#include "illuminator.h"
#include "focus.h"
#include "serv_opt.h"
#include "radarParams.h"
#include "tiltParams.h"
#include "selfTest.h"
#include "PicMsg.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>

menuView::menuView(widgetKeyBoard *vkb, Users *u, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::menuView)
{
  ui->setupUi(this);
  this->installEventFilter(this);
  this->setFocusPolicy(Qt::StrongFocus);
  
  this->setAttribute(Qt::WA_AcceptTouchEvents);
  
  this->initVariables();
  
  m_vkb = vkb;
  m_user = u;
  
  this->connectSignals();
  // remove r1616    this->mapHardButtons();
  
  //add scene
  m_currMenu = new mainMenu();
  this->setMenuViewCommon();
  ((mainMenu *)m_currMenu)->setUser( m_user );

#ifdef LIDARCAM
  Utils& v = Utils::get();
  //  hexDump("GPS", v.GPSBuf(), sizeof( struct GPS_Buff));
  //  hexDump("FuelGauge", v.FGBuf(), sizeof( struct Fuel_Gauge_Buff));

  // toggle the battery
  int percent = v.FGBuf()->State_Of_Charge;
  DEBUG() << "Battery Percent " << percent;
  v.Send_Msg_To_PIC( Set_Power_Led_Green );
  if( percent > 75 ) {
	ui->lb_battery->setPixmap(QPixmap(":/dynamic/battery-4"));
  }else{
	if( percent > 50 ) {
	  ui->lb_battery->setPixmap(QPixmap(":/dynamic/battery-3"));
	}else{
	  if( percent > 25 ) {
		ui->lb_battery->setPixmap(QPixmap(":/dynamic/battery-2"));
	  }else{
		if( percent > 10 ) {
		  ui->lb_battery->setPixmap(QPixmap(":/dynamic/battery-1"));
		}else{
		  v.Send_Msg_To_PIC( Set_Power_Led_Red );
		  ui->lb_battery->setPixmap(QPixmap(":/dynamic/battery-0"));
		}
	  }
	}
  }

  // toggle the GPS connected icon
  if( v.GPSBuf()->GPS_Fixed == true ) {
	ui->lb_GPS->setPixmap(QPixmap(":/dynamic/GPS-on"));
  }else{
	ui->lb_GPS->setPixmap(QPixmap(":/dynamic/GPS-off"));
  }
#endif

  // get the amount of storage being used
  QProcess process;
  process.start("df");
  process.waitForFinished(1000); 
  
  QString stdout = process.readAllStandardOutput();
  QString stderr = process.readAllStandardError();
  //  DEBUG() << "stdout " << stdout << "stderr " << stderr;

  QStringList l = stdout.split('\n');
  for( QStringList::iterator it = ++l.begin(); it != --l.end(); ) {
    if( it->contains("mmcblk0p1") ) {
	  QString current = *it;
	  QString c = current.split(QRegExp("\\s+")).at(4);
	  c.replace(QString("%"), QString(""));
	  int percent = c.toInt();
	  DEBUG() << "Found " << current << "percent used " << percent;
	  if( percent > 95 ) {
		ui->lb_storage->setPixmap(QPixmap(":/dynamic/disk-0"));
		break;
	  }
	  if( percent > 75 ) {
		ui->lb_storage->setPixmap(QPixmap(":/dynamic/disk-4"));
		break;
	  }
	  if( percent > 50 ) {
		ui->lb_storage->setPixmap(QPixmap(":/dynamic/disk-3"));
		break;
	  }
	  if( percent > 25 ) {
		ui->lb_storage->setPixmap(QPixmap(":/dynamic/disk-2"));
		break;
	  }
	  // determine which icon to display
	  ui->lb_storage->setPixmap(QPixmap(":/dynamic/disk-1"));
	  break;
    } else {
      ++it;
    }
  }
  
}

menuView::~menuView()
{
   deleteAccMenu();
   delete ui;
}

void menuView::closeEvent(QCloseEvent *e)
{
    if (m_vkb)
        m_vkb->hide(true);
    QWidget::closeEvent(e);
}

int menuView::initVariables()
{
    this->m_view = this->ui->gv_view;
    this->m_sceneRect = new QRect(0, 0, MENU_SCENE_WIDTH, MENU_VIEW_HEIGHT);
    this->m_currScene = new QGraphicsScene;
    m_currScene->setSceneRect(*m_sceneRect);
    this->m_view->setScene(m_currScene);
    this->m_currProxyWidget = NULL;
    this->m_currMenu = NULL;
    this->m_accMenu = NULL;
    this->m_command = CMD_NONE;
    this->m_prevCmd = CMD_NONE;
    this->m_vkb = NULL;
    this->m_user = NULL;

    return 0;
}

void menuView::connectSignals()
{
  //  DEBUG() << "Entered" ;
  hardButtons& u = hardButtons::get();
  disconnect( ui->pb_exit );
  u.setHardButtonMap( 0, NULL);
  //  DEBUG() << "ui->pb_select " << ui->pb_select;
  disconnect( ui->pb_select );
  u.setHardButtonMap( 1, NULL );
  disconnect(ui->pb_up);
  u.setHardButtonMap( 2, NULL );
  disconnect(ui->pb_down);
  u.setHardButtonMap( 3,NULL );

  connect( ui->pb_exit, SIGNAL(clicked()), this, SLOT(exitPressed()) );
  u.setHardButtonMap( 0, ui->pb_exit);
  //  DEBUG() << "ui->pb_select " << ui->pb_select << "this " << this;
  connect( ui->pb_select, SIGNAL(clicked()), this, SLOT(selectPressed()) );
  u.setHardButtonMap( 1, ui->pb_select);

  Utils::get().setExitButton(ui->pb_exit);
}
  

//
// public slots//
void menuView::mapHardButtons()
{
    hardButtons& u = hardButtons::get();
    u.setHardButtonMap( 0, ui->pb_exit);
    u.setHardButtonMap( 1, ui->pb_select);
    u.setHardButtonMap( 2, ui->pb_up);
    u.setHardButtonMap( 3, ui->pb_down);
}

void menuView::deleteAccMenu()
{
   if (m_accMenu)
   {
      delete m_accMenu;
      m_accMenu = NULL;
   }
}

void menuView::openVKB(vkILine *l)
{
    toggleVKB(l);
    focusLine(l);
    connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(closeVKB())); // New Keyboard
}

void menuView::toggleVKB(vkILine *l)
{
    Q_ASSERT(m_vkb);

  if (m_vkb) {
      if (m_vkb->isVisible()) {
          this->setFocus();
          m_vkb->hide(true);
      } else {
          m_vkb->show(l, m_currProxyWidget->widget());
          //m_vkb->move(this->x(), this->y());  // comment out, need to fix this->x() y()
      }
  } else {
      qDebug() << "virtual keyboard is not created";
  }
}

void menuView::focusLine(vkILine *l)
{
    Q_ASSERT(m_vkb);

    if (m_vkb->isVisible() == false)
        return;

    if (m_vkb->currentTextBox() == l)
        return;

    m_vkb->setActiveForms(l, m_currProxyWidget->widget());
}

void menuView::closeVKB()
{
    Q_ASSERT(m_vkb);
    disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(closeVKB())); // New Keyboard
    if (m_vkb->isVisible()) {
        //this->setFocus();
        m_vkb->hide(true);
    }
}

int menuView::pushMenuStack()
{
  //  DEBUG() << "push m_currProxyWidget " << m_currProxyWidget;
  //  DEBUG() << "push m_currMenu " << m_currMenu;
  Q_ASSERT(m_currProxyWidget);
  m_currMenu->setEnabled(false);
  m_currScene->removeItem(m_currProxyWidget);
  m_menuStack.push(m_currProxyWidget);
  state& v = state::get();
  if (v.getState() == STATE_MAIN_MENU)
	ui->pb_exit->setText("BACK");
  return 0;
}

QGraphicsProxyWidget * menuView::popMenuStatck()
{
  state& v = state::get();
  if (m_menuStack.isEmpty())
	return NULL;
  else {
	m_currProxyWidget = m_menuStack.pop();
	m_currMenu = static_cast <baseMenu *> (m_currProxyWidget->widget());
	//	DEBUG() << "after pop  m_currMenu " << m_currMenu;
	m_currMenu->refreshData();
	//	printf("%s(%d) setState\r\n", __FILE__, __LINE__);
	v.setState( m_currMenu->updateState());
	this->m_command = m_currMenu->command();
	if (v.getState() == STATE_MAIN_MENU)
	  ui->pb_exit->setText("EXIT");
	return m_currProxyWidget;
  }
}

void menuView::setMenuViewCommon()
{
  //  DEBUG() << "this " << this << "m_currMenu " << m_currMenu << "m_currScene " << m_currScene;
  //  printf("%s(%d) setMenuViewCommon \r\n", __FILE__, __LINE__);

  connect(m_currMenu, SIGNAL(selectChanged()), this, SLOT(selectPressed()));
  connect(m_currMenu, SIGNAL(enableSelectButton(bool)), this, SLOT(setSelectButton(bool)));

  hardButtons& u = hardButtons::get();
  disconnect(ui->pb_up);
  u.setHardButtonMap( 2, NULL );
  disconnect(ui->pb_down);
  u.setHardButtonMap( 3,NULL );

  connect(ui->pb_up, SIGNAL(clicked()), m_currMenu, SLOT(exeUpSelect()));
  u.setHardButtonMap( 2, ui->pb_up);
  connect(ui->pb_down, SIGNAL(clicked()), m_currMenu, SLOT(exeDownSelect()));
  u.setHardButtonMap( 3, ui->pb_down);
  
  connect(m_currMenu, SIGNAL(requestVKB(vkILine*)), this, SLOT(openVKB(vkILine*)));
  //  connect(m_currMenu, SIGNAL(exitMenu()), this, SLOT(exitPressed()));
  // backout r1616  connect(m_currMenu, SIGNAL(reMapHardButtons()), this, SLOT(mapHardButtons()));
  connect(m_currMenu, SIGNAL(delAccMenu()), this, SLOT(deleteAccMenu()));

  m_currProxyWidget = m_currScene->addWidget(m_currMenu);

  //m_currProxyWidget->resize(CAM_SCENE_WIDTH, m_currMenu->height());
  m_currProxyWidget->setPos(0, 0);
  
  //m_currScene->setSceneRect(0, 0, MENU_SCENE_WIDTH, m_currMenu->height());
  //this->m_view->setScene(m_currScene);
  //this->showMainPanel();
  m_prevCmd = m_command;
  
  //init m_view vertical scroll
  if (m_currMenu->height() > this->height())
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  else
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
  //init the new menu
  state& v = state::get();
  if( v.getState() != m_currMenu->updateState() ) {
	v.setState(m_currMenu->updateState());
  }
  this->m_command = m_currMenu->command();
  //m_currMenu->setInitFocus();
  QTimer::singleShot(100, m_currMenu, SLOT(setInitFocus()));
}

void menuView::exitPressed()
{
  // state& v = state::get();
  //  printf("%s(%d) state %d\n\r", __FILE__, __LINE__, v.getState());
  // STATE_TOP_VIEW is 3

	Utils& u = Utils::get();
	u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

    if ( m_menuStack.isEmpty() == true)
    {
        // switch to top view
        //	printf("%s(%d) calling closeMenuView state %d\n\r", __FILE__, __LINE__, v.getState());
        hardButtons::get().EnableHardButtons(false);    // Disable Hard Button
        emit this->closeMenuView();
    }
    else
    {
        //	DEBUG() << " m_currScene " << m_currScene;
        m_currScene->removeItem(m_currProxyWidget);
        m_currScene->clear();

        //	DEBUG() << " m_currProxyWidget " << m_currProxyWidget;
        //	m_currProxyWidget->deleteLater();
        m_currProxyWidget->close();

        delete m_currProxyWidget ;
        m_currProxyWidget = NULL;

        int oldState = this->stateStack.pop();

        QGraphicsProxyWidget *p;
        p = this->popMenuStatck();
        Q_ASSERT(p);

        //	DEBUG() << p ;
        //	DEBUG() << m_currMenu;
        //	DEBUG() << "oldState " << oldState << "State " << v.getState();

        switch ( oldState )
        {
        case STATE_MAIN_MENU:
          {
            delete  p ;
            m_currMenu = new mainMenu();
            //		DEBUG() << "m_currMenu " << m_currMenu;
            //		DEBUG() << "oldState " << oldState << "State " << v.getState();
            this->setMenuViewCommon();
            ((mainMenu *)m_currMenu)->setUser(m_user);
            break;
          }
        default:
          {
            m_currScene->addItem(p);
            m_currScene->update();
            // m_currScene->setSceneRect(0, 0, CAM_SCENE_WIDTH, m_currMenu->height());
            //m_view->setScene(m_currScene);
            //this->showMainPanel();
            break;
          }
        }
	
        //init m_view vertical scroll
        if (m_currMenu->height() > this->height())
          m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        else
          m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        m_prevCmd = CMD_NONE;
  }
  return;
}

void menuView::openSubMenu(baseMenu *bm)
{
    this->pushMenuStack();
    m_currMenu = bm;
	//	DEBUG() << "m_currMenu From bm" << m_currMenu ;
    this->setMenuViewCommon();
}

void menuView::setSelectButton(bool b)
{
    ui->pb_select->setEnabled(b);
}

void menuView::selectPressed()
{
	state& v = state::get();
    Utils& u = Utils::get();

	u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

    m_command = m_currMenu->command();
    m_index = m_currMenu->listIndex();

	//    QObject *o = QObject::sender();
	QString commandHex= QString("%1").arg(m_command , 0, 16);
	QString prevHex= QString("%1").arg(m_prevCmd , 0, 16);
    DEBUG() << "m_command " << commandHex << " m_prevCmd " << prevHex;

    if (!(m_command & 0xf) && ((m_command == CMD_NONE) || (m_prevCmd == m_command)))
        return;

#ifndef HH1
    //workaround to handle a double select signal
	// special case to allow a command to be toggled multiple times
	// workaround caused service.cpp to miss clicks on CMD_SRV_REF_CLK so hacked a special case to allow it to be called multiple times
	// in a row.
	switch ( m_command ) {
	case CMD_SRV_REF_CLK:
	  break;
	default:
	  if (m_command == m_prevCmd) return;
	  break;
	}
	//workaround code done
#endif

    if (m_command & 0xf) {
        //toggle fields
        m_prevCmd = m_command;
        this->m_currMenu->toggleValue(m_command, m_index);
        return;
    }

    if (m_command == CMD_CANCEL) {
        this->m_prevCmd = m_command;
        this->m_currMenu->exeCancelClick();
        return;
    }

    baseMenu *bm = NULL;

    switch (m_command) {
    //case CMD_CANCEL:
      //  m_prevCmd = m_command;
        //return this->m_currMenu->exeCancelClick();

    case CMD_MAIN_LOC_SETUP:
	  this->stateStack.push(v.getState());
        bm = new locSetup;
        bm->setVKB(m_vkb);
        break;
    case CMD_MAIN_LIDAR_SETUP:
	  this->stateStack.push(v.getState());
        bm = new lidarSetup;
        break;
    case CMD_MAIN_MODE_SEL:
	  this->stateStack.push(v.getState());
        bm = new modeSel;
        break;
    case CMD_MAIN_SYS_OPT:
	  this->stateStack.push(v.getState());
        bm = new sysOpt;
        break;
    case CMD_MAIN_USER_MGR:
	  this->stateStack.push(v.getState());
        bm = new userMgr( m_user, m_vkb);
        break;
    case CMD_MAIN_FILE_MGR:
	  this->stateStack.push(v.getState());
        bm = new fileMgr;
        break;
    case CMD_MAIN_DEV_INFO:
	  this->stateStack.push(v.getState());
        bm = new devInfo;
        break;

    //user_mgr sub-menus
    case CMD_ADD_USER:
    case CMD_EDIT_USER:
    {   QString loginName = (qobject_cast<userMgr *>(m_currMenu))->getEditUserLogin();
	  this->stateStack.push(v.getState());
        bm = new editUser(m_command, loginName);
        break;
    }

    //sysOpt sub-menus
    case CMD_VIDEO:
	  this->stateStack.push(v.getState());
        bm = new videoSetup;
        break;
    case CMD_WIFI:
	  this->stateStack.push(v.getState());
        bm = new wifiSetup;
        break;
    case CMD_BLUETOOTH:
	  this->stateStack.push(v.getState());
        bm = new BTSetup;
        break;
    case CMD_ETHERNET:
	  this->stateStack.push(v.getState());
        bm = new enetSetup;
        break;
    case CMD_ADMIN:
	  this->stateStack.push(v.getState());
        bm = new admin;
        break;
    case CMD_CAMERA:
	  if (m_accMenu){
		delete m_accMenu;
		this->m_accMenu = NULL;
	  }
	  this->stateStack.push(v.getState());
	  m_accMenu = new cameraSetup; //better to use no-parent widget
	  m_accMenu->show();
	  return;

    case CMD_SECURITY_OPTIONS:
	  this->stateStack.push(v.getState());
      bm = new securityOptions;
	  break;
    case CMD_USER_ACCESS:
	  this->stateStack.push(v.getState());
	  bm = new userAccess;
	  break;
#ifndef HH1
    case CMD_METADATA:
       stateStack.push(v.getState());
       bm = new metaData;
       break;
#endif
    case CMD_FACTORY:
    case CMD_SERVICE:
        if (m_accMenu)
        {
          delete m_accMenu;
          this->m_accMenu = NULL;
        }
        this->stateStack.push(v.getState());
        if (u.passwordEntered() == false)
        {
            admin * adm = qobject_cast<admin *>(m_currMenu);
            m_accMenu = new password(); //better to use no-parent widget
            m_accMenu->show();
            password * pd = qobject_cast<password *>(m_accMenu);
            pd->setPushButton(adm->getButton(m_command));
            pd->init(m_command);
            return;
        }
        else
        {
            u.setPasswordStatus(false);
            if (m_command == CMD_FACTORY)
                bm = new factory;
            else
                bm = new service;
            break;
        }
    case CMD_FAC_START:
	  DEBUG();
	  return;
    case CMD_FAC_STOP:
	  DEBUG();
	  return;
#ifdef HH1
    case CMD_FAC_CACLMAG3110:
      this->stateStack.push(v.getState());
        bm = new calibrateMag3110;
        break;

    case CMD_FAC_RADAR_PARAMS:
        stateStack.push(v.getState());
        bm = new radarParams;
        bm->setVKB(m_vkb);
        break;

    case CMD_FAC_TILT_PARAMS:
        stateStack.push(v.getState());
        bm = new tiltParams;
        bm->setVKB(m_vkb);
        break;

    case CMD_SRV_REF_CLK:
        stateStack.push(v.getState());
        bm = new servOpt(m_command);
        break;
#endif
    case CMD_LOG_FILE:
      this->stateStack.push(v.getState());
        bm = new logFile;
        break;
/*
// Don't need them, 8/17/2018
    case CMD_FAC_SERV_OPTS:
    case CMD_SRV_SERV_OPTS:
	  this->stateStack.push(v.getState());
        bm = new servOpt (m_command);
        break;
*/
    case CMD_FAC_CALIB_DATA:
    case CMD_SRV_CALIB_DATA:
        if (m_accMenu)
            delete m_accMenu;
        this->stateStack.push(v.getState());
        m_accMenu = new calibData(m_command);
        m_accMenu->show();
        return;
    case CMD_UPLOAD:
    {
	  fileMgr * fm = qobject_cast<fileMgr *>(m_currMenu);
	  this->stateStack.push(v.getState());
	  bm = new uploadMgr (0, fm->fileInfoList(), fm->currTWItem());
	  break;
    }
    case CMD_LOC_LOAD:
    case CMD_LOC_SAVE:
	  this->stateStack.push(v.getState());
        bm = new locSave(m_command);
        break;
    case CMD_LOC_CAMERA:
	  if (m_accMenu) {
		delete m_accMenu;
		this->m_accMenu = NULL;
	  }
	  this->stateStack.push(v.getState());
	  m_accMenu = new cameraSetup; //better to use no-parent widget
	  m_accMenu->show();
	  return;
    case CMD_MODE_AUTO:
    case CMD_MODE_ZONE:
    case CMD_MODE_FTC:
      if (m_accMenu)
      {
         delete m_accMenu;
         this->m_accMenu = NULL;
      }
      if (u.getMode() == CMD_NONE)
         return;   // Uncheck the checkbox, exit here
      this->stateStack.push(v.getState());
         m_accMenu = new distMeasure(); //better to use no-parent widget
      ((distMeasure *)m_accMenu)->init(m_command);
      m_accMenu->show();
      return;

    case CMD_PLAYBACK:
    {
      // This assumes that fileMgr is calling this, beware if not called from file_mgr.cpp
      fileMgr * fm = qobject_cast<fileMgr *>(m_currMenu);
      if (fm->currTWItem() == NULL )
      {
         DEBUG() << "CMD_PLAYBACK no file to playback";
         return;
      }

      if (m_accMenu)
      {
         delete m_accMenu;
         m_accMenu = NULL;
      }
      this->stateStack.push(v.getState());
      DEBUG() << fm->fileInfoList().count() << fm->currTWItem()->text();
      for ( int i=0; i < fm->fileInfoList().count(); i++ )
      {
         QFileInfo fi = fm->fileInfoList().at(i);

         if( fi.fileName() == fm->currTWItem()->text() )
         {
             currentPlaybackFileName = fi.absoluteFilePath();
             m_accMenu = new playBack();
             m_accMenu->show();
             playBack *ptr1 = (playBack *)m_accMenu;
             ptr1->setFileName(currentPlaybackFileName);
             ptr1->startPlay();
             return;
         }
      }
      return;
    }

    case CMD_UPLOAD_WIFI:
    case CMD_UPLOAD_ENET:
    {
      QString quest_str = QString("Function not implemented.");
      QMessageBox msgBox;
      msgBox.setText(tr(quest_str.toStdString().c_str()));
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setDefaultButton(QMessageBox::Ok);
      msgBox.setIcon(QMessageBox::Information);
      QPalette p;
      p.setColor(QPalette::Window, Qt::red);
      msgBox.setPalette(p);
      msgBox.exec();
      return;
    }

    case CMD_UPLOAD_USB:
    {
      fileMgr& bb = fileMgr::get();
      bb.uploadUSB();
	  DEBUG() << "dumpAlltoUSB done  ";
	  return;
    }

    case CMD_MAIN_SELF_TEST:
    {
#ifdef LIDARCAM
	  Utils& u = Utils::get();
	  u.sendMbPacket( (unsigned char) CMD_SELF_TEST, 0, NULL, NULL );

     char buf[64];
     unsigned short len1;
     bool ret = u.getAlertPacket(SELF_TEST_PASS_ALERT_ID, buf, &len1, 50);
     if ( ret == true )
     {
        QMessageBox msgBox;
        msgBox.setText("Self Test Passed!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        QPalette p;
        p.setColor(QPalette::Window, Qt::red);
        msgBox.setPalette(p);
        msgBox.exec();
     }
     else
        DEBUG() << "Failed to get packet TIMEOUT";
#endif
#ifdef HH1
     if (m_accMenu)
     {
        delete m_accMenu;
        m_accMenu = NULL;
     }
//     stateStack.push(v.getState());
     m_accMenu = new selfTest(); //better to use no-parent widget
//     ((selfTest *)m_accMenu)->init(m_command);
     m_accMenu->move(50, 0);
     m_accMenu->show();
#endif
      return;
    }
    
    case CMD_UPGRADE_SW:
      {
	DEBUG() << "CMD_UPGRADE_SW";

	QString stdout;
	QString errout;
	QString cmd;
	QProcess process;
	
	cmd = QString(" /bin/sh -c \"/bin/mkdir -p /usr/local/stalker/tmp ");
      
	DEBUG() << cmd;
	
	process.start( cmd );
	process.waitForFinished(-1); // will wait forever until finished
	stdout = process.readAllStandardOutput();
	errout = process.readAllStandardError();
	
	DEBUG() << "Stdout " << stdout;;
	DEBUG() << "Errout " << errout;;

	cmd = QString(" /bin/sh -c \"/usr/bin/wget -q -P /usr/local/stalker/tmp ");
	cmd.append("http://stalkerradar.azurewebsites.net/ACIApps/portland/portland_update.tar");
      
	DEBUG() << cmd;
	
	process.start( cmd );
	process.waitForFinished(-1); // will wait forever until finished
	stdout = process.readAllStandardOutput();
	errout = process.readAllStandardError();
	
	DEBUG() << "Stdout " << stdout;;
	DEBUG() << "Errout " << errout;;

	// untar the file.
	cmd = QString(" /bin/sh -c \"cd /usr/local/stalker/tmp; /bin/tar -xf portland_update.tar");
      
	DEBUG() << cmd;
	
	process.start( cmd );
	process.waitForFinished(-1); // will wait forever until finished
	stdout = process.readAllStandardOutput();
	errout = process.readAllStandardError();
	
	DEBUG() << "Stdout " << stdout;;
	DEBUG() << "Errout " << errout;;

#ifdef LIDARCAM
	// run the install script
	cmd = QString(" /bin/sh -c \"cd /usr/local/stalker/tmp; /bin/sh -c ./update_portland.sh");
      
	DEBUG() << cmd;
	
	process.start( cmd );
	process.waitForFinished(-1); // will wait forever until finished
	stdout = process.readAllStandardOutput();
	errout = process.readAllStandardError();
	
	DEBUG() << "Stdout " << stdout;;
	DEBUG() << "Errout " << errout;;
	DEBUG() << "Updated ";
	
#endif
	
#ifdef JUNK
         // Mount the USB drive
         QStringList mount_command = QStringList() << "-c" << "mount /dev/sda1 /mnt/USBdisk";
         QProcess mount_process;
         mount_process.start("/bin/sh",mount_command);
         mount_process.waitForFinished(-1);
         mount_process.close();

         // Search for update package
         QString update_package_path;
         bool update_package_found = false;

         QDirIterator iterator("/mnt/USBdisk",QDirIterator::Subdirectories);
         while(iterator.hasNext())
         {
           QFileInfo file_info(iterator.next());
           QString file_type = file_info.suffix();

           if(file_type == "pup")
           {
             update_package_found = true;
             update_package_path = file_info.filePath();
           }
         }

         if ( false == update_package_found ) {
           QMessageBox msgBox;
           msgBox.setText("Missing USB device or update package!");
           msgBox.setStandardButtons(QMessageBox::Ok);
           msgBox.setDefaultButton(QMessageBox::Ok);
           msgBox.setIcon(QMessageBox::Warning);
           QPalette p;
           p.setColor(QPalette::Window, Qt::red);
           msgBox.setPalette(p);
           msgBox.exec();
            return;
         }

         // release the display
         // run LidarCam_II_Updater
         system(" /usr/local/stalker/bin/LidarCam_II_Updater &");
         // terminate lidarCam
         system("killall runLidarCam.sh");
         printf("%s(%d) Upgrade SW lidarCam terminating\n\r", __FILE__, __LINE__);
         close();
         qApp->quit();
#endif
         return;
      }

    case CMD_ILLIMINATOR:
         if (m_accMenu != NULL)
            delete m_accMenu;
#ifdef HH1
         m_accMenu = new focus(); //better to use no-parent widget
#else
         m_accMenu = new illuminator(); //better to use no-parent widget
#endif
         m_accMenu->show();
         return;

    default:
      QString valueInHex= QString("%1").arg(m_command , 0, 16);
      DEBUG() << "select m_command not supported " << m_command << valueInHex;
      return;
    }

    return openSubMenu(bm);
}

bool menuView::eventFilter (QObject * object, QEvent *event)
{

  UNUSED(object);

  //DEBUG() << "Entered" << event;
  if ( event->type() == QEvent::WindowActivate) {
	//	DEBUG() << "Window Activated fix hard";
	this->mapHardButtons();
  }
  return false;
}


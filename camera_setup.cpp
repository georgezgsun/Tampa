#include "camera_setup.h"
#include "ui_camera_setup.h"
#include <QDebug>
#include "utils.h"
#include "state.h"
#include "hardButtons.h"
#include "utils.h"
#include "utils.h"
#include "ColdFireCommands.h"
#include "debug.h"

cameraSetup::cameraSetup(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::cameraSetup)
{
    ui->setupUi(this);

    m_redText = QStringLiteral("color: rgb(255, 0, 0);");
    m_blueText = QStringLiteral("color: rgb(0, 0, 255);");

    this->initLists();
    this->buildHashTables();
    this->setInittoggleValues();

    m_command = CMD_ZOOM;
    m_listIndex = 0;
	state& v = state::get();
    v.setState(STATE_ACC_MENU);

    this->setInitFocus();

    //set initial color
    ui->wt_liveView->setStyleSheet(QStringLiteral("background-color: rgb(0, 0, 0);"));

	hardButtons& u = hardButtons::get();

    connect(ui->pb_exit, SIGNAL(clicked()), this, SLOT(onPbExitClicked()));
	u.setHardButtonMap( 0, ui->pb_exit);
    connect(ui->pb_select, SIGNAL(clicked()), this, SLOT(onPbSelectClicked()));
	u.setHardButtonMap( 1, ui->pb_select);
    connect(ui->pb_up, SIGNAL(clicked()), this, SLOT(onPbUpClicked()));
	u.setHardButtonMap( 2, ui->pb_up);
    connect(ui->pb_down, SIGNAL(clicked()), this, SLOT(onPbDownClicked()));
	u.setHardButtonMap( 3, ui->pb_down);

//    connect(ui->pb_select, SIGNAL(pressed()), this, SLOT(onPbSelectPressed()));
}

cameraSetup::~cameraSetup()
{
    delete ui;
}

void cameraSetup::initLists()
{
    m_list << ui->lb_zoomVal
           << ui->lb_focusVal
           << ui->lb_shutterVal
           << ui->lb_colorVal
           << ui->lb_irisVal
           << ui->lb_gainVal;

    m_cmdList << CMD_ZOOM
              << CMD_FOCUS
              << CMD_SHUTTER
              << CMD_COLOR
              << CMD_IRIS
              << CMD_GAIN;

    m_buddyList << ui->lb_zoom
              << ui->lb_focus
              << ui->lb_shutter
              << ui->lb_color
              << ui->lb_iris
              << ui->lb_gain;
}

void cameraSetup::buildHashTables()
{
    int i;
    QString s;

    //zoom hashes
    //m_zoomList << "";
    for (i = ZOOM_MIN; i <= ZOOM_MAX; i++) {
        s = QString("%1").arg(i);
        m_zoomList << s;
    }
    m_hashValueList[CMD_ZOOM] = &m_zoomList;
    m_hashValueIndex[CMD_ZOOM] = &m_zoomIdx;

    //focus hashes
    m_focusList << "AUTO" << "MANUAL";
    m_hashValueList[CMD_FOCUS] = &m_focusList;
    m_hashValueIndex[CMD_FOCUS] = &m_focusIdx;

    //shutter hashes
    m_shutterList << "AUTO" << "1s" << "1/8" << "1/100" << "1/1000" << "1/2000" << "1/5000";
    m_hashValueList[CMD_SHUTTER] = &m_shutterList;
    m_hashValueIndex[CMD_SHUTTER] = &m_shutterIdx;

    //color hashes
    m_colorList << "AUTO" << "MONO";
    m_hashValueList[CMD_COLOR] = &m_colorList;
    m_hashValueIndex[CMD_COLOR] = &m_colorIdx;

    //iris hashes
    m_irisList << "AUTO" << "1.8" << "2.5" << "3.6" << "4.0" << "5.6" << "8" << "11" << "16";
    m_hashValueList[CMD_IRIS] = &m_irisList;
    m_hashValueIndex[CMD_IRIS] = &m_irisIdx;

    //gain hashes
    m_gainList << "AUTO";
    for (i = GAIN_MIN; i <= GAIN_MAX; i++) {
        s = QString("%1").arg(i);
        m_gainList << s;
    }
    m_hashValueList[CMD_GAIN] = &m_gainList;
    m_hashValueIndex[CMD_GAIN] = &m_gainIdx;
}

void cameraSetup::setInittoggleValues()
{
    m_focusIdx =
            m_zoomIdx =
            m_shutterIdx =
            m_colorIdx =
            m_irisIdx =
            m_gainIdx = 0;

    this->querySetting();
    this->setDisplay();

	// Only zoomIdx is set correctly here, others are zero
	// Dislay is correct, indexs are wrong
	//	DEBUG() << QRegExp("^" + QRegExp::escape(m_camSetting.zoom));
	m_zoomIdx = m_zoomList.indexOf(QRegExp("^" + QRegExp::escape(m_camSetting.zoom)) );
	DEBUG() << " index " << m_camSetting.index;
	DEBUG() << " zoom " << m_camSetting.zoom;
	DEBUG() << " zoomIdx " << m_zoomIdx;
	DEBUG() << " focus" << m_camSetting.focus;
	DEBUG() << " focus1" << m_camSetting.focus1;
	DEBUG() << " shutter" << m_camSetting.shutter;
	DEBUG() << " color" << m_camSetting.color;
	DEBUG() << " iris" << m_camSetting.iris;
	DEBUG() << " gain" << m_camSetting.gain;

#if 0
    ui->lb_zoomVal->setText(m_zoomList[m_zoomIdx]);
    ui->lb_focusVal->setText(m_focusList[m_focusIdx]);
    ui->lb_shutterVal->setText(m_shutterList[m_shutterIdx]);
    ui->lb_colorVal->setText(m_colorList[m_colorIdx]);
    ui->lb_irisVal->setText(m_irisList[m_irisIdx]);
    ui->lb_gainVal->setText(m_gainList[m_gainIdx]);
#endif

}

void cameraSetup::querySetting()
{
    int retv = 0;
    Utils& u = Utils::get();
    int location = u.location();
    if (location > 0) {
        m_camSetting.index = location;
    } else
        m_camSetting.index = CAMS_DEFAULT_INDEX;

    int ct = u.db()->queryEntry(TBL_CAMERA_SETTING, (DBStruct *)&m_camSetting, QRY_BY_KEY);
    if (ct == 1) {
        retv = u.db()->getNextEntry(TBL_CAMERA_SETTING, (DBStruct *)&m_camSetting);
        if (retv) {
            DEBUG() << "get default camera setting failed";
        }
    }
    if (ct == 0 && location > 0) {
        m_camSetting.index = CAMS_DEFAULT_INDEX;
        ct = u.db()->queryEntry(TBL_CAMERA_SETTING, (DBStruct *)&m_camSetting, QRY_BY_KEY);
        Q_ASSERT(ct == 1);

        retv = u.db()->getNextEntry(TBL_CAMERA_SETTING, (DBStruct *)&m_camSetting);
        if (retv) {
            DEBUG() << "get default camera setting failed";
        }

        //add cam setting entry with the default settings
        m_camSetting.index = location;
        int retv = u.db()->addEntry(TBL_CAMERA_SETTING, (DBStruct *)&m_camSetting);
        if (retv) {
            DEBUG() << "add camera initial setting failed for location " << location;
            return;
        }
    } else if (m_camSetting.index == CAMS_DEFAULT_INDEX) {
        if (ct == 0)
            Q_ASSERT_X(0, "cameraSetup::querySetting", "camera default setting is empty");
        else if (ct == 1)
            DEBUG() << "Location in NOT set, use default camera setting";
    }
}

void cameraSetup::setDisplay()
{
    struct CameraSetting& c = m_camSetting;
    ui->lb_zoomVal->setText(c.zoom);
    ui->lb_focusVal->setText(c.focus);
    ui->lb_shutterVal->setText(c.shutter);
    ui->lb_colorVal->setText(c.color);
    ui->lb_irisVal->setText(c.iris);
    ui->lb_gainVal->setText(c.gain);
}

int cameraSetup::updateSetting()
{
    struct CameraSetting& o = m_camSetting;
    struct CameraSetting c;
    c.zoom = ui->lb_zoomVal->text();
    c.focus = ui->lb_focusVal->text();
    c.shutter = ui->lb_shutterVal->text();
    c.color = ui->lb_colorVal->text();
    c.iris = ui->lb_irisVal->text();
    c.gain = ui->lb_gainVal->text();


    if ( c.zoom != o.zoom ||
         c.focus != o.focus ||
         c.shutter != o.shutter ||
         c.color != o.color ||
         c.iris != o.iris ||
         c.gain != o.gain ) {
        c.index = o.index;
        Utils& u = Utils::get();
        int retv = u.db()->updateEntry(TBL_CAMERA_SETTING, (DBStruct *)&c);
        if (retv) {
            DEBUG() << "update camera setting failed for location " << c.index;
            return retv;
        }
    }
    return 0;
}

void cameraSetup::toggleValue(int cmd, int idx, int /*f*/) {

    switch (cmd) {
    case CMD_FOCUS_MANUAL:
        return;
    default:
        baseMenu::toggleValue(cmd, idx, 1);
        break;
    }
}

//slots
void cameraSetup::onPbExitClicked()
{
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
  this->updateSetting();
  emit reMapHardButtons();
  this->close();
}

void cameraSetup::onPbDownClicked() {
    QLabel *lb = qobject_cast<QLabel *>(m_list[m_listIndex]);
	Utils& u = Utils::get();
	u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
	//    lb->setStyleSheet("");
	//    m_buddy->setStyleSheet("");
    lb->setStyleSheet(m_blueText);
    m_buddy->setStyleSheet(m_blueText);

    baseMenu::exeDownSelect();

    lb = qobject_cast<QLabel *>(m_list[m_listIndex]);
    m_buddy = qobject_cast<QLabel *>(m_buddyList[m_listIndex]);

    lb->setStyleSheet(m_redText);
    m_buddy->setStyleSheet(m_redText);
}

void cameraSetup::onPbUpClicked() {
    QLabel *lb = qobject_cast<QLabel *>(m_list[m_listIndex]);
	Utils& u = Utils::get();
	u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
	//    lb->setStyleSheet("");
	//    m_buddy->setStyleSheet("");
    lb->setStyleSheet(m_blueText);
    m_buddy->setStyleSheet(m_blueText);

    baseMenu::exeUpSelect();

    lb = qobject_cast<QLabel *>(m_list[m_listIndex]);
    m_buddy = qobject_cast<QLabel *>(m_buddyList[m_listIndex]);

    lb->setStyleSheet(m_redText);
    m_buddy->setStyleSheet(m_redText);
}

void cameraSetup::onPbSelectClicked()
{
   //DEBUG() << "onPbSelectClicked called";

   Utils& u = Utils::get();
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

   if (m_command == CMD_FOCUS_MANUAL)
         return;

   this->toggleValue(m_command, m_listIndex);

   //DEBUG() << "m_command = " << m_command;
   //DEBUG() << "m_listIndex = " << m_listIndex;


   if (m_command == CMD_ZOOM)
   {
      QLabel *lb = qobject_cast<QLabel *>(m_list.at(m_listIndex));

      int zoomValue = 0;
      zoomValue = lb->text().toInt();

      //DEBUG() << "lb text " << lb->text();

#ifdef LIDARCAM
      int retv = u.sendCmdToCamera(m_command, zoomValue);

      if (retv != 0)
         qWarning() << "set camera zoom failed";
#endif
   }
   else if (m_command == CMD_COLOR)
   {
      QLabel *lb = qobject_cast<QLabel *>(m_list.at(m_listIndex));
      QProcess Play;
      QString str = "/usr/local/stalker/bin/stalkerDayNight";
      str.append( " " );

      if (lb->text() == "AUTO")
      {
         str.append( QString::number( 1 ));		// auto is color

         //		  DEBUG() << str;

         Play.start( str );
         Play.waitForFinished(1000);
         int ret = Play.exitCode();

         if( ret != 0 )
            DEBUG() << "ret " << ret;
      }
      else
      {
         str.append( QString::number( 2 ));		// auto is night
         Play.start( str );
         Play.waitForFinished(1000);
         int ret = Play.exitCode();

         if( ret != 0 )
            DEBUG() << "ret " << ret;
      }
   }
   else if (m_command == CMD_FOCUS)
   {
#ifdef LIDARCAM
      QLabel *lb = qobject_cast<QLabel *>(m_list.at(m_listIndex));

      int retv = u.sendCmdToCamera(m_command, (int)lb->text().toLatin1().data());

      if (retv != 0)
         qWarning() << "set camera zoom failed";
#endif
   }
}

void cameraSetup::onPbSelectPressed() {
    DEBUG() << "onPbSelectPressed called";

    if (m_command != CMD_FOCUS_MANUAL)
        return;


}

void cameraSetup::setInitFocus() {
    baseMenu::setInitFocus();
    QLabel *lb = qobject_cast<QLabel *>(m_list[m_listIndex]);
    m_buddy = qobject_cast<QLabel *>(m_buddyList[m_listIndex]);
    lb->setStyleSheet(m_redText);
    m_buddy->setStyleSheet(m_redText);
}

void cameraSetup::onPbExitPressed() {
  
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  hardButtons& h = hardButtons::get();
  
  disconnect(ui->pb_exit);
  h.setHardButtonMap( 0, NULL);
  disconnect(ui->pb_select);
  h.setHardButtonMap( 1, NULL);
  disconnect(ui->pb_up);
  h.setHardButtonMap( 2, NULL);
  disconnect(ui->pb_down);
  h.setHardButtonMap( 3, NULL);
  close();
}

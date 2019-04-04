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

    m_command = CMD_MODE;
    m_listIndex = 0;
    state& v = state::get();
    v.setState(STATE_CAMERACONFIG);
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
    m_list << ui->pb_modeVal
           << ui->pb_shutterVal
           << ui->pb_gainVal
           << ui->pb_evVal;

    m_cmdList << CMD_MODE
              << CMD_SHUTTER
              << CMD_GAIN
              << CMD_EV;

    m_buddyList << ui->lb_mode
                << ui->lb_shutter
                << ui->lb_gain
                << ui->lb_ev;
}

void cameraSetup::buildHashTables()
{
    m_modeList << "AUTO" << "Shutter" << "Manual" ;

    m_hashValueList[CMD_MODE] = &m_modeList;
    m_hashValueIndex[CMD_MODE] = &m_modeIdx;

    //shutter hashes
    m_shutterList << "AUTO" << "1/10000 " << "1/1000" << "1/500" << "1/250" << "1/100" << "1/60" << "1/30"<<"1/15"<<"1/7.5";
    m_hashValueList[CMD_SHUTTER] = &m_shutterList;
    m_hashValueIndex[CMD_SHUTTER] = &m_shutterIdx;


    //ev hashes
    m_evList << "0" << "50" << "100" << "150" << "200" << "250";
    m_hashValueList[CMD_EV] = &m_evList;
    m_hashValueIndex[CMD_EV] = &m_evIdx;

    //gain hashes
    m_gainList << "AUTO" << "0dB" << "6dB" << "12dB" << "18dB" << "24dB";

    m_hashValueList[CMD_GAIN] = &m_gainList;
    m_hashValueIndex[CMD_GAIN] = &m_gainIdx;
}

void cameraSetup::setInittoggleValues()
{
            m_modeIdx =
            m_shutterIdx =
            m_evIdx =
            m_gainIdx = 0;

    this->querySetting();
    this->setDisplay();

    // Only zoomIdx is set correctly here, others are zero
    // Dislay is correct, indexs are wrong
    //	DEBUG() << QRegExp("^" + QRegExp::escape(m_camSetting.zoom));
    m_modeIdx = m_modeList.indexOf(QRegExp("^" + QRegExp::escape(m_camSetting.mode)) );
    DEBUG() << " index " << m_camSetting.index;
    DEBUG() << " mode " << m_camSetting.mode;
    DEBUG() << " modeIdx " << m_modeIdx;

    DEBUG() << " shutter" << m_camSetting.shutter;

    DEBUG() << " ev" << m_camSetting.ev;
    DEBUG() << " gain" << m_camSetting.gain;

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
    ui->pb_modeVal->setText(c.mode);
    ui->pb_shutterVal->setStyleSheet(m_blueText);
    ui->pb_shutterVal->setText(c.shutter);
    ui->pb_gainVal->setStyleSheet(m_blueText);
    ui->pb_gainVal->setText(c.gain);
    ui->pb_evVal->setStyleSheet(m_blueText);
    ui->pb_evVal->setText(c.ev);
}

int cameraSetup::updateSetting()
{
    struct CameraSetting& o = m_camSetting;
    struct CameraSetting c;
    c.mode = ui->pb_modeVal->text();
    c.shutter = ui->pb_shutterVal->text();
    c.ev = ui->pb_evVal->text();
    c.gain = ui->pb_gainVal->text();


    if ( c.mode != o.mode ||
         c.shutter != o.shutter ||
         c.ev != o.ev ||
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

    default:
        baseMenu::toggleValue(cmd, idx, 2);
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
    QPushButton *pb = qobject_cast<QPushButton *>(m_list[m_listIndex]);
    Utils& u = Utils::get();
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
    //    lb->setStyleSheet("");
    //    m_buddy->setStyleSheet("");
    pb->setStyleSheet(m_blueText);
    m_buddy->setStyleSheet(m_blueText);

    baseMenu::exeDownSelect();

    pb = qobject_cast<QPushButton *>(m_list[m_listIndex]);
    m_buddy = qobject_cast<QLabel *>(m_buddyList[m_listIndex]);

    pb->setStyleSheet(m_redText);
    m_buddy->setStyleSheet(m_redText);
}

void cameraSetup::onPbUpClicked() {
    QPushButton *pb = qobject_cast<QPushButton*>(m_list[m_listIndex]);
    Utils& u = Utils::get();
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
    //    lb->setStyleSheet("");
    //    m_buddy->setStyleSheet("");
    pb->setStyleSheet(m_blueText);
    m_buddy->setStyleSheet(m_blueText);

    baseMenu::exeUpSelect();

    pb = qobject_cast<QPushButton *>(m_list[m_listIndex]);
    m_buddy = qobject_cast<QLabel *>(m_buddyList[m_listIndex]);

    pb->setStyleSheet(m_redText);
    m_buddy->setStyleSheet(m_redText);
}

void cameraSetup::onPbSelectClicked()
{
   //DEBUG() << "onPbSelectClicked called";

   Utils& u = Utils::get();
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

   this->toggleValue(m_command, m_listIndex);

}

void cameraSetup::onPbSelectPressed() {
    DEBUG() << "onPbSelectPressed called";

    if (m_command != CMD_FOCUS_MANUAL)
        return;


}
void cameraSetup::setInitFocus() {
    baseMenu::setInitFocus();
    QPushButton *pb = qobject_cast<QPushButton *>(m_list[m_listIndex]);
    m_buddy = qobject_cast<QLabel *>(m_buddyList[m_listIndex]);
    pb->setStyleSheet(m_redText);
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

void cameraSetup::on_pb_modeVal_clicked()
{
    if (m_modeIdx < (m_modeList.size() - 1))
           m_modeIdx++;
       else
           m_modeIdx = 0;
       ui->pb_modeVal->setText(m_modeList.at(m_modeIdx));
       Utils::get().sendCmdToCamera(CMD_MODE, m_modeIdx);

}

void cameraSetup::on_pb_shutterVal_clicked()
{
    if (m_shutterIdx < (m_shutterList.size() - 1))
           m_shutterIdx++;
       else
           m_shutterIdx = 0;
       ui->pb_shutterVal->setText(m_shutterList.at(m_shutterIdx));
       Utils::get().sendCmdToCamera(CMD_SHUTTER, m_shutterIdx);
}

void cameraSetup::on_pb_gainVal_clicked()
{

    if (m_gainIdx < (m_gainList.size() - 1))
           m_gainIdx++;
       else
           m_gainIdx = 0;
       ui->pb_gainVal->setText(m_gainList.at(m_gainIdx));
       Utils::get().sendCmdToCamera(CMD_GAIN, m_gainIdx);
}

void cameraSetup::on_pb_evVal_clicked()
{

    if (m_evIdx < (m_evList.size() - 1))
           m_evIdx++;
       else
           m_evIdx = 0;
       ui->pb_evVal->setText(m_evList.at(m_evIdx));
       Utils::get().sendCmdToCamera(CMD_EV, m_evIdx);
}

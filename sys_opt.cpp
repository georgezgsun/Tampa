#include "sys_opt.h"
#include "ui_sys_opt.h"
#include "state.h"

sysOpt::sysOpt(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::sysOpt)
{
    ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_SYS_OPT_MENU);
    m_listIndex =
            m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);
}

sysOpt::~sysOpt()
{
    delete ui;
}

void sysOpt::initLists()
{
    m_list << ui->pb_camera
           << ui->pb_video
           << ui->pb_wifi
           << ui->pb_blueTooth
           << ui->pb_ethernet
           << ui->pb_device;

    m_cmdList << CMD_CAMERA
           << CMD_VIDEO
           << CMD_WIFI
           << CMD_BLUETOOTH
           << CMD_ETHERNET
           << CMD_MAIN_DEV_INFO;

    this->connectWidgetSigs();

#ifdef HH1
    ui->gridLayout->removeWidget(ui->pb_camera);
    ui->pb_camera->setVisible(false);
    ui->gridLayout->removeWidget(ui->pb_wifi);
    ui->gridLayout->addWidget(ui->pb_wifi, 0, 0);
    ui->gridLayout->removeWidget(ui->pb_ethernet);
    ui->gridLayout->addWidget(ui->pb_ethernet, 1, 0);
#endif
}


void sysOpt::setInittoggleValues()
{   
#ifndef HH1
   // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
   ui->pb_camera->setEnabled(false);
   ui->pb_wifi->setEnabled(false);
   ui->pb_blueTooth->setEnabled(false);
#endif
}

void sysOpt::toggleValue(int cmd, int /*idx*/, int /*f*/)
{
    switch (cmd) {
    case CMD_PWR_OFF:
    case CMD_BCKLT_OFF:
        return;
    default:
        return;
    }
}

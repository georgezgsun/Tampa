#include "wifi_setup.h"
#include "ui_wifi_setup.h"
#include "state.h"

wifiSetup::wifiSetup(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::wifiSetup)
{
    ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_WIFI_SETUP);
    m_listIndex = m_prevListIndex = 1;
    m_command = m_cmdList.at(m_listIndex);
}

wifiSetup::~wifiSetup()
{
    delete ui;
}

void wifiSetup::initLists()
{
    m_list << ui->pb_scanWifi
           << ui->cb_wifiNetList
           << ui->le_wifiUsername
           << ui->le_wifiPasswd;

    m_cmdList << CMD_WIFI_SCAN
              << CMD_WIFI_LIST
              << CMD_WIFI_USERNAME
              << CMD_WIFI_PASSWD;

    this->connectWidgetSigs();
}

void wifiSetup::setInittoggleValues()
{

}

void wifiSetup::toggleValue(int, int, int )
{
    return;
}

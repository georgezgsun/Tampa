#include "bt_setup.h"
#include "ui_bt_setup.h"
#include "state.h"

BTSetup::BTSetup(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::BTSetup)
{
    ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_BT_SETUP);
    m_listIndex = m_prevListIndex = 1;
    m_command = m_cmdList.at(m_listIndex);
}

BTSetup::~BTSetup()
{
    delete ui;
}

void BTSetup::initLists()
{
    m_list << ui->pb_scanBT
           << ui->cb_BTDevList
           << ui->le_BTPasswd;

    m_cmdList << CMD_BT_SCANNING
           << CMD_BT_LIST
           << CMD_BT_PASSWD;

    this->connectWidgetSigs();
}

void BTSetup::setInittoggleValues()
{

}

void BTSetup::toggleValue(int, int, int)
{

}

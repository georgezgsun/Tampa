#include "user_access.h"
#include "ui_user_access.h"
#include <QDebug>
#include "state.h"
#include "debug.h"
#include "utils.h"

userAccess::userAccess(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::userAccess)
{
    ui->setupUi(this);

    this->initLists();
    this->buildHashTables();

    setInittoggleValues();
    m_listIndex = m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);
	state& v = state::get();
    v.setState(STATE_SUB_MENU3);
}

userAccess::~userAccess()
{
    //Is anything changed?
    if (memcmp((void *)&mAdmin, (void *)&mOldAdmin, sizeof(Administration)))
    {   // Yes
        Utils::get().setAdmin(mAdmin);
    }
    delete ui;
}

void userAccess::initLists()
{
    m_list << ui->pb_admin
           << ui->pb_transfer
           << ui->pb_delete;

    m_cmdList << CMD_USER_ACCESS_ADMIN
              << CMD_USER_ACCESS_TRANSFER
              << CMD_USER_ACCESS_DELETE;
    this->connectWidgetSigs();
}

void userAccess::buildHashTables()
{
    m_adminList << "None" << "Level 1" << "Levels 1-2" << "Levels 1-3" << "Levels 1-4";
    m_transferList << "None" << "Level 1" << "Levels 1-2" << "Levels 1-3" << "Levels 1-4";
    m_deleteList << "None" << "Level 1" << "Levels 1-2" << "Levels 1-3" << "Levels 1-4";
    m_hashValueList[CMD_USER_ACCESS_ADMIN] = &m_adminList;
    m_hashValueList[CMD_USER_ACCESS_TRANSFER] = &m_transferList;
    m_hashValueList[CMD_USER_ACCESS_DELETE] = &m_deleteList;

    m_hashValueIndex[CMD_USER_ACCESS_ADMIN] = &mAdmin.usrAccAdmin;
    m_hashValueIndex[CMD_USER_ACCESS_TRANSFER] = &mAdmin.usrAccTransfer;
    m_hashValueIndex[CMD_USER_ACCESS_DELETE] = &mAdmin.usrAccDelete;
}

void userAccess::setInittoggleValues()
{
    mAdmin = mOldAdmin = Utils::get().getAdmin();

    ui->pb_admin->setText(m_adminList.at(mAdmin.usrAccAdmin));
    ui->pb_transfer->setText(m_transferList.at(mAdmin.usrAccTransfer));
    ui->pb_delete->setText(m_deleteList.at(mAdmin.usrAccDelete));
}

void userAccess::toggleValue(int cmd, int idx, int /*f*/) {

    switch (cmd) {
    case CMD_FOCUS_MANUAL:
        return;
    default:
        baseMenu::toggleValue(cmd, idx, 1);
        break;
    }

    return;
}

void userAccess::on_pb_admin_clicked()
{
    if (++mAdmin.usrAccAdmin >= m_adminList.size())
    {
        mAdmin.usrAccAdmin = 0;
    }
    ui->pb_admin->setText(m_adminList.at(mAdmin.usrAccAdmin));
}

void userAccess::on_pb_transfer_clicked()
{
    if (++mAdmin.usrAccTransfer >= m_transferList.size())
    {
        mAdmin.usrAccTransfer = 0;
    }
    ui->pb_transfer->setText(m_transferList.at(mAdmin.usrAccTransfer));

}

void userAccess::on_pb_delete_clicked()
{
    if (++mAdmin.usrAccDelete >= m_deleteList.size())
    {
        mAdmin.usrAccDelete = 0;
    }
    ui->pb_delete->setText(m_deleteList.at(mAdmin.usrAccDelete));
}

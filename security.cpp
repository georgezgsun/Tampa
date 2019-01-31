#include <QMessageBox>
#include <QDebug>
#include "security.h"
#include "ui_security.h"
#include "state.h"
#include "debug.h"
#include "utils.h"
#include "ColdFireCommands.h"

securityOptions::securityOptions(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::securityOptions)
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

securityOptions::~securityOptions()
{
    if (ui->cb_userLogin->isChecked() == true)
        mAdmin.userLogin = true;
    else
        mAdmin.userLogin = false;
    if (ui->cb_compression->isChecked() == true)
        mAdmin.compression = true;
    else
        mAdmin.compression = false;
    if (ui->cb_autoDelete->isChecked() == true)
        mAdmin.autoDelete = true;
    else
        mAdmin.autoDelete = false;
    if (ui->cb_encryption->isChecked() == true)
        mAdmin.encryption = true;
    else
        mAdmin.encryption= false;

    Utils& u = Utils::get();
    if(mAdmin.password != ui->le_password->text())
    {
        struct Users usr;
        userDB *userDB = u.db();
        int ct, retv;
        ct = userDB->queryEntry(TBL_USERS, (DBStruct *)&usr, QRY_ALL_ENTRIES);
        for (int i = 0; i < ct; i++)
        {
            retv = userDB->getNextEntry(TBL_USERS, (DBStruct *)&usr);
            if (!retv)
            {
                if( usr.loginName == "admin")
                {
                    mAdmin.password = usr.password = ui->le_password->text();
                    retv = u.db()->updateEntry(TBL_USERS, (DBStruct *)&usr );
                    break;
                }
            }
        }
        if( retv == 1 )
            DEBUG() << "Update failed";
        else
            DEBUG() << "Update Succeeded";
    }
    //Is anything changed?
    if (memcmp((void *)&mAdmin, (void *)&mOldAdmin, sizeof(Administration)))
    {   // Yes
        u.setAdmin(mAdmin);
    }
    delete ui;
}


void securityOptions::initLists()
{
   m_list << ui->pb_counters << ui->le_password << ui->pb_dateFormat << ui->pb_language;

   m_cmdList << CMD_SECURITY_COUNTERS << CMD_SECURITY_PASSWORD
             << CMD_SECURITY_DATEFORMAT << CMD_SECURITY_LANGUAGE;
   connectWidgetSigs();
}

void securityOptions::buildHashTables()
{
   m_dateFormatList << "YYYY-MM-DD" << "DD-MM-YYYY" ;
   m_languageList << "ENGLISH" << "SPANISH" ;
   m_hashValueList[CMD_SECURITY_DATEFORMAT] = &m_dateFormatList;
   m_hashValueList[CMD_SECURITY_LANGUAGE] = &m_languageList;

   m_hashValueIndex[CMD_SECURITY_DATEFORMAT] = &mAdmin.dateFormat;
   m_hashValueIndex[CMD_SECURITY_LANGUAGE] = &mAdmin.language;
}

void securityOptions::setInittoggleValues()
{
   mAdmin = mOldAdmin = Utils::get().getAdmin();

   if (mAdmin.userLogin == true)
       ui->cb_userLogin->setChecked(true);
   if (mAdmin.compression == true)
       ui->cb_compression->setChecked(true);
   if (mAdmin.autoDelete == true)
       ui->cb_autoDelete->setChecked(true);
   if (mAdmin.encryption == true)
       ui->cb_encryption->setChecked(true);

   ui->le_password->setText(mAdmin.password);
   ui->pb_dateFormat->setText(m_dateFormatList.at(mAdmin.dateFormat));
    ui->pb_language->setText(m_languageList.at(mAdmin.language));

   // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
   ui->cb_autoDelete->setEnabled(false);
   ui->cb_compression->setEnabled(false);
   ui->cb_encryption->setEnabled(false);
   ui->cb_userLogin->setEnabled(false);
}

void securityOptions::toggleValue(int cmd, int idx, int /*f*/) {

    switch (cmd) {
    case CMD_FOCUS_MANUAL:
        return;
    default:
        baseMenu::toggleValue(cmd, idx, 1);
        break;
    }

    return;
}

void securityOptions::on_pb_dateFormat_clicked()
{
    if (++mAdmin.dateFormat >= m_dateFormatList.size())
    {
        mAdmin.dateFormat = 0;
    }
    ui->pb_dateFormat->setText(m_dateFormatList.at(mAdmin.dateFormat));
}

void securityOptions::on_pb_language_clicked()
{
    if (++mAdmin.language >= m_languageList.size())
    {
        mAdmin.language = 0;
    }
    ui->pb_language->setText(m_languageList.at(mAdmin.language));

}

void securityOptions::on_pb_counters_clicked()
{
   Utils::get().sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   QMessageBox msgBox;
   msgBox.setText("Are you sure you want to reset all the counts?");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   msgBox.setDefaultButton(QMessageBox::No);
   msgBox.setIcon(QMessageBox::Question);
   QPalette p;
   p.setColor(QPalette::Window, Qt::red);
   msgBox.setPalette(p);

   if(msgBox.exec() == QMessageBox::Yes)
   {
      Utils::get().setEvidenceNum(0);
   }
}

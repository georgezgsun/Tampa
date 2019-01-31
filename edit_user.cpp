#include "edit_user.h"
#include "ui_edit_user.h"

#include <QMessageBox>
#include <string>
#include "utils.h"
#include "state.h"

editUser::editUser(int type, QString& editLogin, QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::editUser)
{
    ui->setupUi(this);

    m_editLogin = editLogin;
    m_type = type;
	state& v = state::get();
    if (type == CMD_EDIT_USER) {
	  v.setState(STATE_EDIT_USER);
	  ui->le_loginName->setEnabled(false);
    }
    else
	  v.setState(STATE_ADD_USER);

    this->initLists();
    this->setInittoggleValues();

    m_listIndex =
            m_prevListIndex = 1;
    m_command = m_cmdList.at(m_listIndex);

    connect(ui->pb_save, SIGNAL(clicked()), this, SLOT(saveUser()));
	//    connect(ui->pb_cancel, SIGNAL(clicked()), this, SLOT(cancelEdit()));
    ui->pb_save->setEnabled(false);

    m_baseTimer->start(500);
}

editUser::~editUser()
{
    if (m_baseTimer) {
        m_baseTimer->stop();
    }
    delete ui;
}

void editUser::initLists()
{
  //           << ui->pb_cancel
    m_list << ui->pb_save
           << ui->le_firstName
           << ui->le_lastName
           << ui->le_badgeNum
           << ui->le_userLevel
           << ui->le_loginName
           << ui->le_passWord;

	//              << CMD_CANCEL
    m_cmdList << CMD_SAVE
              << CMD_FIRST_NAME
              << CMD_LAST_NAME
              << CMD_BADGE_NUM
              << CMD_USER_LEVEL
              << CMD_LOGINNAME
              << CMD_PASSWORD;

    this->connectWidgetSigs();
}

void editUser::setInittoggleValues()
{
    if (m_type == CMD_EDIT_USER) {
        Utils& u = Utils::get();
        int retv = u.getTransitData(CMD_EDIT_USER, (DBStruct *) &m_editUser);
        if (!retv) {
            if (m_editLogin != m_editUser.loginName) {
                qWarning() << "edit user mismatch, expect " << m_editLogin << " get " << m_editUser.loginName;
                //data mismatch, get editUser from DB
            } else {
                //get right data
                this->fillEditData();
            }
        } else {
            qWarning() << "get transit data failed";
            //get data from DB
        }
    }
}

void editUser::toggleValue(int, int, int )
{

}

void editUser::timerHit()
{
	state& v = state::get();
    if (v.getState() == STATE_ADD_USER) {
#if 0
    //test code
    ui->pb_save->setEnabled(!ui->le_loginName->text().isEmpty());
#else
    ui->pb_save->setEnabled(!ui->le_firstName->text().isEmpty() &&
                            !ui->le_lastName->text().isEmpty() &&
                            !ui->le_badgeNum->text().isEmpty() &&
                            !ui->le_loginName->text().isEmpty() &&
                            !ui->le_passWord->text().isEmpty() &&
                            !ui->le_userLevel->text().isEmpty());
#endif
    } else {
        // edit user
        ui->pb_save->setEnabled(ui->le_firstName->text() != m_editUser.firstName ||
                                ui->le_lastName->text() != m_editUser.lastName ||
                                ui->le_badgeNum->text() != m_editUser.bagdeNumber ||
                                ui->le_passWord->text() != m_editUser.password ||
                                ui->le_userLevel->text() != m_editUser.userLevel);
    }
}

void editUser::fillEditData()
{
    Q_ASSERT(m_type == CMD_EDIT_USER);
    ui->le_loginName->setText(m_editUser.loginName);
    ui->le_firstName->setText(m_editUser.firstName);
    ui->le_lastName->setText(m_editUser.lastName);
    ui->le_badgeNum->setText(m_editUser.bagdeNumber);
    ui->le_passWord->setText(m_editUser.password);
    ui->le_userLevel->setText(m_editUser.userLevel);
}

void editUser::cancelEdit()
{
    if (m_type == CMD_ADD_USER) {
        //clear all fields
        ui->le_loginName->setText("");
        ui->le_firstName->setText("");
        ui->le_lastName->setText("");
        ui->le_badgeNum->setText("");
        ui->le_passWord->setText("");
        ui->le_userLevel->setText("");
    } else {
        //reset to original data
        this->fillEditData();
    }

}

void editUser::saveUser()
{
    Utils& u = Utils::get();
	userDB *m_userDB;
    m_userDB = u.db();
    Q_ASSERT(m_userDB);
    int retv = 0;
    int sendExitSig = 0;

    qDebug() << "save clicked";

    struct Users user;
    user.firstName = ui->le_firstName->text();
    user.lastName = ui->le_lastName->text();
    user.bagdeNumber = ui->le_badgeNum->text();
    user.loginName = ui->le_loginName->text();
    user.password = ui->le_passWord->text();
    user.userLevel = ui->le_userLevel->text();

	state& v = state::get();
    if (v.getState() == STATE_ADD_USER) {
        retv = m_userDB->queryEntry(TBL_USERS, (DBStruct *)&user, QRY_BY_KEY);
        if (retv == 0) {
            //add user
            retv = m_userDB->addEntry(TBL_USERS, (DBStruct *)&user);
            if (retv != 0) {
                //addEtry failed
                QString quest_str = QString("Could not add user!");
                QMessageBox msgBox;
                msgBox.setText(tr(quest_str.toStdString().c_str()));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.setIcon(QMessageBox::Critical);
                QPalette p;
                p.setColor(QPalette::Window, Qt::red);
                msgBox.setPalette(p);
                msgBox.exec();
            }else{
	      //addEntry succeeded
          QString quest_str = QString("User saved!");
          QMessageBox msgBox;
          msgBox.setText(tr(quest_str.toStdString().c_str()));
          msgBox.setStandardButtons(QMessageBox::Ok);
          msgBox.setDefaultButton(QMessageBox::Ok);
          msgBox.setIcon(QMessageBox::Information);
          QPalette p;
          p.setColor(QPalette::Window, Qt::red);
          msgBox.setPalette(p);
          msgBox.exec();

	      // add user name to evidence file area
	      // determine if loginname is in the evidence directory
	      QString folder = QString("%1%2").arg("/mnt/mmc/ipnc/", (char *)user.loginName.toStdString().c_str());

	      //	      qDebug() << "Folder " << folder <<endl;

	      if ( !QDir( folder ).exists() ) {
		// if not add it.
		QDir().mkdir( folder );
	      }
	    }
            sendExitSig = 1;
        } else if(retv > 0) {
            // user with same login exist already
            QString quest_str = QString("USER NAME already used!");
            QMessageBox msgBox;
            msgBox.setText(tr(quest_str.toStdString().c_str()));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            QPalette p;
            p.setColor(QPalette::Window, Qt::red);
            msgBox.setPalette(p);
            msgBox.exec();
        } else {
            // query failed
            QString quest_str = QString("User query failed!");
            QMessageBox msgBox;
            msgBox.setText(tr(quest_str.toStdString().c_str()));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            QPalette p;
            p.setColor(QPalette::Window, Qt::red);
            msgBox.setPalette(p);
            msgBox.exec();
        }
    }
    else {
        //edit user
        retv = m_userDB->updateEntry(TBL_USERS, (DBStruct *) &user);
        if (!retv) {
            //succeeded
            m_editUser.firstName = user.firstName;
            m_editUser.lastName = user.lastName;
            m_editUser.bagdeNumber = user.bagdeNumber;
            m_editUser.password = user.password;
            m_editUser.userLevel = user.userLevel;
            QString quest_str = QString("User %1 updated!").arg(user.loginName);
            QMessageBox msgBox;
            msgBox.setText(tr(quest_str.toStdString().c_str()));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Information);
            QPalette p;
            p.setColor(QPalette::Window, Qt::red);
            msgBox.setPalette(p);
            msgBox.exec();
            sendExitSig = 1;
        } else {
            //failed
            QString quest_str = QString("User %1 update failed!").arg(user.loginName);
            QMessageBox msgBox;
            msgBox.setText(tr(quest_str.toStdString().c_str()));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            QPalette p;
            p.setColor(QPalette::Window, Qt::red);
            msgBox.setPalette(p);
            msgBox.exec();
        }
    }

    if (sendExitSig) {
        emit exitMenu();
    }

    return;
}

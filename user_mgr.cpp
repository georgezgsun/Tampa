#include "user_mgr.h"
#include "ui_user_mgr.h"
#include "utils.h"
#include <QMessageBox>
#include <QTranslator>
#include "state.h"
#include "debug.h"

userMgr::userMgr(Users *u, widgetKeyBoard *vkb, QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::userMgr)
{
    ui->setupUi(this);

    initLists();
    setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_USER_MGR_MENU);
    m_listIndex =
            m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);

    m_lwFocused = false;
    connect(ui->lw_users, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(usersItemClicked(QListWidgetItem*)));

    //local services
    this->setUser(u);
    this->setVKB(vkb);

    updateUsersFromDB();
    this->enableButtons(false);
}

userMgr::~userMgr()
{
    emit enableSelectButton(true);
    delete ui;
}

void userMgr::initLists()
{
    m_list << ui->pb_add
           << ui->pb_edit
           << ui->pb_delete
           << ui->lw_users;

    m_cmdList << CMD_ADD_USER
              << CMD_EDIT_USER
              << CMD_DEL_USER
              << CMD_LIST_USERS;

    this->connectWidgetSigs();
}

void userMgr::setInittoggleValues()
{
    //get users from db
}

void userMgr::toggleValue(int, int)
{
  DEBUG();
    //process locally
}

void userMgr::refreshData()
{
    this->updateUsersFromDB();
}

void userMgr::updateUsersFromDB()
{
    QListWidget *lw = ui->lw_users;
    Users u;
    int ct = 0;
    int retv;
	userDB *m_userDB;
	Utils& uu = Utils::get();
    m_userDB = uu.db();

    ct = m_userDB->queryEntry(TBL_USERS, (DBStruct *)&u, QRY_ALL_ENTRIES);

    DEBUG() << "total users " << ct;

    lw->clear();

    for (int i = 0; i < ct; i++) {
        retv = m_userDB->getNextEntry(TBL_USERS, (DBStruct *)&u);
        if (!retv) {
            QListWidgetItem *item = new QListWidgetItem(u.loginName + '\t' + u.firstName + " " + u.lastName);
            ui->lw_users->addItem(item);
        }
    }

    DEBUG() << "current row " << ui->lw_users->currentRow() << " count " << ui->lw_users->count();
}

void userMgr::exeDownSelect() {
    int checkAgain = 0;
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->lw_users) {
        int row = ui->lw_users->currentRow();
        row++;
        if (row < ui->lw_users->count()) {
            m_lwFocused = true;
            ui->lw_users->setCurrentRow(row);
            this->enableButtons(true);
            return;
        } else {
            m_lwFocused = false;
            goto baseSlot;
        }
    } else
        checkAgain = 1;

baseSlot:
    this->enableButtons(false);
    baseMenu::exeDownSelect();

    if (checkAgain)
        setLWFocus();
}

void userMgr::exeUpSelect() {
    int checkAgain = 0;
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->lw_users) {
        int row = ui->lw_users->currentRow();
        row--;
        if (row < 0) {
            m_lwFocused = false;
            goto baseSlot;
        }
        m_lwFocused = true;
        ui->lw_users->setCurrentRow(row);
        this->enableButtons(true);
        return;
    } else
        checkAgain = 1;

baseSlot:
    this->enableButtons(false);
    baseMenu::exeUpSelect();

    if (checkAgain) {
        setLWFocus();
    }
}

void userMgr::setLWFocus()
{
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->lw_users) {
        m_lwFocused = true;
        int row = ui->lw_users->currentRow();
        if (row < 0 || row >= ui->lw_users->count())
            row = 0;
        ui->lw_users->setCurrentRow(row);
        this->enableButtons(true);
    }
}

void userMgr::enableButtons(bool b)
{
    if (b == true
		&& (ui->lw_users->currentItem()->text().contains("admin")
			|| ui->lw_users->currentItem()->text().contains("Admin"))) {
        ui->pb_delete->setEnabled(false);
        ui->pb_edit->setEnabled(false);
	} else{
	  ui->pb_delete->setEnabled(b);
	  ui->pb_edit->setEnabled(b);
	}
    /*
    if (b == false) {                       //TODO: may not needed
        ui->lw_users->setCurrentRow(-1);  // clear focus of LW
        m_lwFocused = false;
    }
    */

    //if true, the cursor is in LW, SELECT should disabled
    if (b == true) {
        emit enableSelectButton(false);
    } else {
        emit enableSelectButton(true);
    }
}

void userMgr::usersItemClicked(QListWidgetItem * )
{
   // qDebug() << "listIndex " << m_listIndex;
    //qDebug() << "pre index " << m_prevListIndex;
    m_lwFocused = true;
    baseMenu::setCmd(ui->lw_users);
    this->setLWFocus();
    //this->setInitFocus();
    //qDebug() << "listIndex " << m_listIndex;
    //qDebug() << "pre index " << m_prevListIndex;
}

void userMgr::setCmd()
{
	userDB *m_userDB;
	Utils& uu = Utils::get();
    m_userDB = uu.db();

    QObject *o = QObject::sender();

    //baseMenu::setCmd();
    this->setIndexAndCmd(dynamic_cast<QWidget *> (o));
    this->enableButtons(false);

    if (o->objectName() == ui->pb_add->objectName()) {
        //this->enableButtons(false);
        emit selectChanged();
    }
    else if (o->objectName() == ui->pb_edit->objectName()) {
        //get user key
        QString loginName = ui->lw_users->currentItem()->text();
        struct Users editUser;
        editUser.loginName = loginName.section('\t', 0, 0);

        m_editUserLogin = editUser.loginName;

        //query and get the user
        int ct = m_userDB->queryEntry(TBL_USERS, (DBStruct *)&editUser, QRY_BY_KEY);
        if (ct == 1) {
            int retv = m_userDB->getNextEntry(TBL_USERS, (DBStruct *)&editUser);
            if (retv)
                qCritical() << "getNextEntry failed";
        } else {
            //query failed, no user or multiple users
            qCritical() << "Query user failed";
        }

        //set trasitive data
        Utils& u = Utils::get();
        u.setTransitData(CMD_EDIT_USER, (DBStruct *)&editUser);

        //this->enableButtons(false);
        emit selectChanged();
    }
    else if (o->objectName() == ui->pb_delete->objectName()) {
        //delet a user
        QString loginName = ui->lw_users->currentItem()->text();
        struct Users u;
        u.loginName = loginName.section('\t', 0, 0);

        //confirm the deletion
        QString quest_str = QString("Delete user %1?").arg(u.loginName);
        QMessageBox msgBox;
		msgBox.setText(tr(quest_str.toStdString().c_str()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Question);
		QPalette p;
        p.setColor(QPalette::Window, Qt::red);
		msgBox.setPalette(p);
		
		if(msgBox.exec() == QMessageBox::No){
		  //qDebug() << "No was clicked";
		  this->enableButtons(true);
		  return;
		}
		
        int retv = m_userDB->deleteEntry(TBL_USERS, (DBStruct *) &u);
        if (!retv) {
            QString quest_str = QString("User %1 deleted!").arg(u.loginName);
            QMessageBox msgBox;
            msgBox.setText(tr(quest_str.toStdString().c_str()));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Information);
            QPalette p;
            p.setColor(QPalette::Window, Qt::red);
            msgBox.setPalette(p);
            msgBox.exec();

	      QString folder = QString("%1%2").arg("/mnt/mmc/ipnc/", (char *)u.loginName.toStdString().c_str());

	      qDebug() << "Folder " << folder <<endl;

	      if ( QDir( folder ).exists() ) {
		// if foudn delete it.
		QDir().rmdir( folder );
	      }
            this->updateUsersFromDB();
            //this->enableButtons(false);
        }
        else {
            // deletion failed
            QString quest_str =  QString("Delete user %1 failed!").arg(u.loginName);
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
}

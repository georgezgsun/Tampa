#include "main_menu.h"
#include "ui_main_menu.h"
#include <QDebug>
#include "top_view.h"
#include "state.h"
#include "debug.h"
#include <QMessageBox>
#include "utils.h"
#include "ColdFireCommands.h"

mainMenu::mainMenu(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::mainMenu)
{
  //  printf("%s(%d) ENTERED\r\n", __FILE__, __LINE__);
  ui->setupUi(this);
  this->initLists();
  state& v = state::get();
  v.setState(STATE_MAIN_MENU);
  m_command = m_cmdList.at(m_listIndex);
}

mainMenu::~mainMenu()
{
    delete ui;
}

/*
void mainMenu::setInitFocus()
{
    if (m_prevListIndex == 0)
        ui->pb_locSetup->setFocus();
    else
        baseMenu::setInitFocus();
}
*/

void mainMenu::initLists()
{

    m_list << ui->tb_logout
             << ui-> pb_locSetup
             << ui->pb_modeSel
             << ui->pb_lidarSetup
             << ui->pb_fileMgr
             << ui->pb_userMgr
             << ui->pb_sysOpt
             << ui->pb_selfTest
             << ui->pb_camera
             << ui->pb_admin
             << ui->pb_prtTicket;

    m_cmdList << CMD_MAIN_LOGOUT
             << CMD_MAIN_LOC_SETUP
             << CMD_MAIN_MODE_SEL
             << CMD_MAIN_LIDAR_SETUP
             << CMD_MAIN_FILE_MGR
             << CMD_MAIN_USER_MGR
             << CMD_MAIN_SYS_OPT
             << CMD_MAIN_SELF_TEST
             << CMD_ILLIMINATOR
             << CMD_ADMIN
             << CMD_MAIN_PRT_TICKET;

    this->connectWidgetSigs();

#ifdef HH1
    //ui->gridLayout->removeWidget(ui->pb_modeSel);
   // ui->pb_modeSel->setVisible(false);
    //    ui->gridLayout->removeWidget(ui->pb_camera);
    //    ui->pb_camera->setVisible(false);
    ui->pb_camera->setText("FOCUS");
    //ui->gridLayout->removeWidget(ui->pb_userMgr);
    //ui->gridLayout->addWidget(ui->pb_userMgr, 0, 1);
    //ui->pb_userMgr->setGeometry(115, 6, 99, 40);
#endif
}

void mainMenu::setUser(struct Users *u)
{
    baseMenu::setUser(u);
    QString name = u->firstName + " " + u->lastName;
    ui->lb_userName->setText(name.toUpper());
}


#if 0
void mainMenu::updateSelect()
{
    if (ui->pb_locSetup->isEnabled() != true)
        return;

    m_command = m_cmdList.at(m_listIndex);

    if (m_list.at(m_listIndex)->hasFocus() == false)
        m_command = CMD_NONE;

    emit selectChanged(m_command, m_listIndex);
}
#endif

void mainMenu::on_tb_logout_clicked()
{
  Utils& u = Utils::get();
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  QMessageBox msgBox;
  msgBox.setText("Are you sure you want to log out?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setIcon(QMessageBox::Question);
  QPalette p;
  p.setColor(QPalette::Window, Qt::red);
  msgBox.setPalette(p);

 if(msgBox.exec() == QMessageBox::Yes){
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   //    DEBUG() << "Yes was clicked";
   state& v = state::get();
   v.setState(STATE_START);
   QApplication::exit(1);
  }else {
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   //    DEBUG() << "Yes was *not* clicked";
  }
}

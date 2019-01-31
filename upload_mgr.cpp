#include "upload_mgr.h"
#include "ui_upload_mgr.h"
#include "state.h"

uploadMgr::uploadMgr(QWidget *parent, QFileInfoList &fil, const QListWidgetItem *twi) :
    baseMenu(parent),
    ui(new Ui::uploadMgr)
{
    ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();

    state& v = state::get();
    v.setState(STATE_SUB_MENU2);
    m_command = m_cmdList.at(m_listIndex);

    QString s = this->styleSheet() + QStringLiteral("QListWidget {font: BOLD 10pt;}");
    this->setStyleSheet(s);

    m_fileInfoList.clear();
    if (twi == NULL) {
        m_fileInfoList = fil;
    } else {
        for (int i = 0; i < fil.count(); i++) {
            QFileInfo fi = fil.at(i);
            if (fi.fileName() == twi->text()) {
                m_fileInfoList.append(fi);
                break;
            }
        }
    }
    this->loadVideoFiles();
}

uploadMgr::~uploadMgr()
{
    delete ui;
}

void uploadMgr::initLists()
{
    m_list << ui->pb_wifi
           << ui->pb_usb
           << ui->pb_enet
           << ui->tw_fileList;

    m_cmdList << CMD_UPLOAD_WIFI
              << CMD_UPLOAD_USB
              << CMD_UPLOAD_ENET
              << CMD_UPLOAD_FLIST;

    this->connectWidgetSigs();
}

void uploadMgr::setInittoggleValues()
{

   // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
   ui->pb_enet->setEnabled(false);
   ui->pb_wifi->setEnabled(false);
}

void uploadMgr::loadVideoFiles()
{
    ui->tw_fileList->clear();
    for (int i = 0; i <m_fileInfoList.count(); i++) {
        QFileInfo fi = m_fileInfoList.at(i);
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(fi.fileName());
        ui->tw_fileList->addItem(item);
    }
}

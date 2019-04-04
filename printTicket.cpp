#include "printTicket.h"
#include "ui_printTicket.h"
#include <QMessageBox>
#include <cstdio>
#include <QAbstractItemView>
#include "state.h"
#include "utils.h"
#include "debug.h"
#include "ColdFireCommands.h"
#include <QPrinter>
#include <QPainter>
//#include <QImage>
#include <QPrintDialog>
#include <QFontDatabase>
#include "doTicket.h"


printTicket::printTicket(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::printTicket)
{
    ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();
    state& v = state::get();
    v.setState(STATE_PRNT_TICKET_MENU);
    m_listIndex = m_prevListIndex = 0;

    m_command = m_cmdList.at(m_listIndex);

    QString s = this->styleSheet() + QStringLiteral("QListWidget {font: BOLD 10pt;}");
    this->setStyleSheet(s);

    connect(ui->tw_files, SIGNAL(itemSelectionChanged()), this, SLOT(twItemChanged()));
    m_twFocused = false;
    m_currTWItem = NULL;
    m_fmState = FM_SELECT_ALL;
    setVideoPath();
    loadVideoFiles();

#ifdef LIDARCAM
    // Restart AV server
    DEBUG() << "AV Server ON";
    char data1[APPRO_DATA_LEN];
    data1[0] = 1; // 1 -> start
    Utils::get().SndApproMsg(APPRO_AVONOFF, data1, NULL);  // 1 -> start
#endif

}

printTicket::~printTicket()
{
   DEBUG();

   emit enableSelectButton(true);
   delete ui;
}

void printTicket::initLists()
{
  //  DEBUG();
  m_list << ui->pb_print
         << ui->tw_files;

  m_cmdList << CMD_TICKET_PLAYBACK
            << CMD_LIST_FILES;

  connectWidgetSigs();

}

void printTicket::setInittoggleValues()
{
  //  DEBUG();
  // get file tree from file system
}


void printTicket::toggleValue(int, int, int /*f*/)
{
  DEBUG() << m_command;
    switch (m_command) {
    case CMD_SEL_ALL:
        m_fmState = FM_SELECT_ALL;
        break;
//    case CMD_SEL_LAST:
//        m_fmState = FM_SELECT_LAST;
//        break;
    case CMD_SEL_NEW:
        m_fmState = FM_SELECT_NEW;
        break;
    default:
        break;
    }
    this->loadVideoFiles();
}


void printTicket::setVideoPath(void)
{
   QString vfPath = QString (qgetenv("LIDARCAM_VEDIO_PATH"));
   if (vfPath.isEmpty())
      vfPath = QString(DEFAULT_VEDIO_PATH);

   vfPath.append( "/");

   vfPath.append( Utils::get().session()->user()->loginName);
   if ( !QDir( vfPath ).exists() )
   {
      // if not add it.
      QDir().mkdir( vfPath );
   }

   m_videoDir.setPath(vfPath);
   DEBUG() << "Video Path: " << vfPath;
}

void printTicket::loadVideoFiles()
{
    QStringList m_defaultNameFilter;
    m_defaultNameFilter << "*-*.jpg";

    m_videoDir.setNameFilters(m_defaultNameFilter);

    switch (m_fmState)
    {
       case FM_SELECT_ALL:
           m_videoDir.setFilter(DEFAULT_FILTER);
           //        m_videoDir.setSorting(DEFAULT_SORT_FLAG);
           m_videoDir.setSorting(QDir::Name | QDir::Reversed);
           break;
           //    case FM_SELECT_LAST:
       case FM_SELECT_NEW:
           m_videoDir.setFilter(DEFAULT_FILTER);
           m_videoDir.setSorting(QDir::Name | QDir::Reversed);
           break;

       default:
           break;
    }

    m_currTWItem = NULL;
    ui->tw_files->clear();
    m_listItemList.clear();
    m_FIList.clear();
    m_FIList = m_videoDir.entryInfoList();

    QFileInfoList tmpFIL;
    tmpFIL.clear();

   DEBUG() << "m_fmState " << m_fmState << "Count " << m_FIList.count() ;

    //display the file list in tw_files
    for (int i = 0; i <m_FIList.count(); i++)
    {
        QFileInfo fi = m_FIList.at(i);
        if (m_fmState == FM_SELECT_LAST)
        {
            if (i > 0)
            {
               m_FIList.clear();
               m_FIList = tmpFIL;
               break;
            }
            else
               tmpFIL.append(fi);
        }
        else if (m_fmState == FM_SELECT_NEW)
        {
            QDate today = QDateTime::currentDateTime().date();
            if (fi.lastModified().date() != today)
            {
               m_FIList.clear();
               m_FIList = tmpFIL;
               break;
            }
            else
               tmpFIL.append(fi);
        }

    // QListWidgetItem *item = new QListWidgetItem(QListWidgetItem::Type);
        QListWidgetItem *item = new QListWidgetItem();
        item->setText( fi.fileName());
    // QDateTime dt = fi.lastModified();
    // item->setText(1, dt.toString("dd MMM yyyy hh:mm:ss"));
        m_listItemList.append(item);
        ui->tw_files->addItem(item);
    }

    this->enableButtons(true);
    this->m_twFocused = true;
}

void printTicket::enableButtons(bool b)
{
  //    DEBUG() << "Called with " << b;
  
  if ( b == false) {
    ui->tw_files->clearFocus();
    m_twFocused = false;
  }
  
  //if true, the cursor is in LW, SELECT should disabled
  if (b == true) {
    emit enableSelectButton(false);
  } else {
    emit enableSelectButton(true);
  }
}

void printTicket::setTWFocus(int flag)
{
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->tw_files) {
        QListWidgetItem *item = NULL;
        m_twFocused = true;

        if (m_listItemList.isEmpty())
            return;

        if (flag == ARROW_DOWN) {
            item = m_listItemList.at(0);
        } else {
            //arrow up
            int c = m_listItemList.count();
            item = m_listItemList.at(c-1);
        }
        ui->tw_files->setCurrentItem(item);
        m_currTWItem = item;
        this->enableButtons(true);
    }
}

void printTicket::twItemChanged()
{
   QListWidgetItem *item = ui->tw_files->currentItem();
   //   QModelIndex curItemIndex = ui->tw_files->indexFromItem(item);
   if (item == m_currTWItem)
      return;
   QWidget *w = dynamic_cast<QWidget *>(ui->tw_files);

   setIndexAndCmd(w);
   ui->tw_files->setCurrentItem(item);
   m_currTWItem = item;
   enableButtons(true);
   m_twFocused = true;
}

#ifdef JUNK
void printTicket::setCmd()
{
    QObject *o = QObject::sender();
    this->setIndexAndCmd(dynamic_cast<QWidget *>(o));

    if ( (o->objectName() == ui->pb_playback->objectName())
         || (o->objectName() == ui->pb_upload->objectName()) ) {
      emit selectChanged();
    } else if (o->objectName() == ui->pb_selAll->objectName()) {
      Utils& u = Utils::get();
      u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
      m_fmState = FM_SELECT_ALL;
      this->loadVideoFiles();
    } else if (o->objectName() == ui->pb_selNew->objectName()) {
      Utils& u = Utils::get();
      u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
      m_fmState = FM_SELECT_NEW;
      this->loadVideoFiles();
    } else if(o->objectName() == ui->pb_print->objectName()) {
      Utils& u = Utils::get();
      u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

      if( m_currTWItem == NULL ) {
	DEBUG() << "No File selected for printing";
	// msgbox
	QString quest_str = QString("No JPG file selected for printing");
	QMessageBox msgBox;
	msgBox.setText(QObject::tr(quest_str.toStdString().c_str()));
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.setIcon(QMessageBox::Warning);
	QPalette p;
	p.setColor(QPalette::Window, Qt::red);
	msgBox.setPalette(p);
	msgBox.exec();
	//	    u.getExitButton()->clicked();   // Clicked 'Exit' button
	//	    DEBUG() << 	u.getExitButton();   // Clicked 'Exit' button
	return;
      }

      // Print ticket
      QString ticketFileName = QString( m_currTWItem->text());
      DEBUG() << ticketFileName;
      QString newticketFileName = m_videoDir.absoluteFilePath( ticketFileName );
      DEBUG() << newticketFileName;
      
      doTicket * aa = new doTicket();
      aa->display( newticketFileName);
      return;      

#ifdef JUNK
      // clear the picked file from the screen
      m_currTWItem = NULL;
      m_twFocused = false;
      loadVideoFiles();

      return;
#endif
    }
    else if (o->objectName() == ui->pb_delete->objectName())
    { // Delete
      Utils& u = Utils::get();
      u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
      if (m_FIList.count() == 0)
         return;

      if (m_currTWItem)
      {
         //delet a file
         QString fileName = m_currTWItem->text();

         QString quest_str = QString("Are you sure you want to delete %1?").arg(fileName);
         QMessageBox msgBox;
         msgBox.setText(tr(quest_str.toStdString().c_str()));
         msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
         msgBox.setDefaultButton(QMessageBox::No);
         msgBox.setIcon(QMessageBox::Question);
         QPalette p;
         p.setColor(QPalette::Window, Qt::red);
         msgBox.setPalette(p);

         if(msgBox.exec() == QMessageBox::No)
         {
       //qDebug() << "No was clicked";
           enableButtons(true);
       return;
     }

     bool b = m_videoDir.remove(fileName);

         if (b == false)
     {
       QString quest_str = QString("Delete file %1 failed!").arg(fileName);
       QMessageBox msgBox;
       msgBox.setText(tr(quest_str.toStdString().c_str()));
       msgBox.setStandardButtons(QMessageBox::Ok);
       msgBox.setDefaultButton(QMessageBox::Ok);
       msgBox.setIcon(QMessageBox::Warning);
       QPalette p;
       p.setColor(QPalette::Window, Qt::red);
       msgBox.setPalette(p);
       msgBox.exec();
     }
         else
     {
       QStringList pieces = fileName.split(".");
       if (pieces.length() >= 1)
           {
               QString json1 = pieces.value(0);
               json1 += ".json";
               m_videoDir.remove(json1);
               QString mdFile = pieces.value(0);
               mdFile += ".md";
               m_videoDir.remove(mdFile);
            }
            loadVideoFiles();
         }
       }
       else
       {
         //delete multiple files
         int count = m_FIList.count();
         QString quest_str;

         //confirm the deletion
         if (count == 1)
            quest_str = QString("Are you sure you want to delete %1?").arg(m_FIList.at(0).fileName());
         else
            quest_str = QString("Are you sure you want to delete %1 files?").arg(count);
            QMessageBox msgBox;
            msgBox.setText(tr(quest_str.toStdString().c_str()));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Question);
            QPalette p;
            p.setColor(QPalette::Window, Qt::red);
            msgBox.setPalette(p);

         if(msgBox.exec() == QMessageBox::No)
         {
       //qDebug() << "No was clicked";
           enableButtons(true);
       return;
     }

         for (int i = 0; i < count; i++)
         {
            const QString fName = m_FIList.at(i).fileName();
            bool b = m_videoDir.remove(fName);
            if (b == false)
            {
               QString quest_str = QString("Delete file %1 failed!").arg(fName);
               QMessageBox msgBox;
               msgBox.setText(tr(quest_str.toStdString().c_str()));
               msgBox.setStandardButtons(QMessageBox::Ok);
               msgBox.setDefaultButton(QMessageBox::Ok);
               msgBox.setIcon(QMessageBox::Warning);
               QPalette p;
               p.setColor(QPalette::Window, Qt::red);
               msgBox.setPalette(p);
               msgBox.exec();
            }
            else
            {
          // delete the json and md file
          QStringList pieces = fName.split(".");
          if (pieces.length() >= 1)
          {
        QString json1 = pieces.value(0);
        json1 += ".json";
        m_videoDir.remove(json1);
        QString mdFile = pieces.value(0);
        mdFile += ".md";
        m_videoDir.remove(mdFile);
          }
            }
         }
         loadVideoFiles();
       }
    }
}
#endif

void printTicket::exeDownSelect()
{
    qDebug();
    int checkAgain = 0;
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->tw_files) {
        QListWidgetItem *item = NULL;
        this->enableButtons(true);
        if (m_twFocused == true) {  //already in the TW
            ui->tw_files->setFocus();
        int index = ui->tw_files->currentRow();
        item = ui->tw_files->item( index + 1);
            if (item) {
                ui->tw_files->setCurrentItem(item);
                m_currTWItem = ui->tw_files->currentItem();
                return;
            } else {
                m_currTWItem = NULL;
                goto baseSlot;
            }

        } else {
            //first in TW, should not get into this block(?)
            if ((item = m_listItemList.at(0))) {
                this->m_twFocused = true;
                this->ui->tw_files->setCurrentItem(m_listItemList.at(0));
                m_currTWItem = ui->tw_files->currentItem();
            }
            return;
        }
     } else
        checkAgain = 1;

baseSlot:
    ui->tw_files->setCurrentItem(NULL);
    this->m_twFocused = false;
    //    this->enableButtons(false);
    baseMenu::exeDownSelect();

    if (checkAgain)
        setTWFocus();
}


void printTicket::exeUpSelect()
{
  qDebug();
    int checkAgain = 0;
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->tw_files) {
        QListWidgetItem *item = NULL;
        this->enableButtons(true);
        if (m_twFocused == true) {  //already in the TW
            ui->tw_files->setFocus();
        int index = ui->tw_files->currentRow();
        item = ui->tw_files->item(index - 1);
            if (item) {
                ui->tw_files->setCurrentItem(item);
                m_currTWItem = ui->tw_files->currentItem();
                return;
            } else {
                m_currTWItem = NULL;
                goto baseSlot;
            }

        } else {
            //first in TW, should not get into this block(?)
            int c = m_listItemList.count();
            if ((item = m_listItemList.at(c-1))) {
                this->m_twFocused = true;
                this->ui->tw_files->setCurrentItem(item);
                m_currTWItem = item;
            }
            return;
        }
    } else
        checkAgain = 1;

baseSlot:
    ui->tw_files->setCurrentItem(NULL);
    this->m_twFocused = false;
    //    this->enableButtons(false);
    baseMenu::exeUpSelect();

    if (checkAgain)
        setTWFocus(ARROW_UP);

}

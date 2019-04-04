#include <QMessageBox>
#include "file_mgr.h"
#include "ui_file_mgr.h"
#include <cstdio>
#include <QAbstractItemView>
#include "state.h"
#include "utils.h"
#include "debug.h"
#include "ColdFireCommands.h"
#include <QPrinter>
#include <QPainter>
#include <QImage>
#include <QPrintDialog>
#include <QFontDatabase>

fileMgr::fileMgr(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::fileMgr)
{
    //  DEBUG();
    ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();
    state& v = state::get();
    v.setState(STATE_FILE_MGR_MENU);
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

    // Restart AV server
    DEBUG() << "AV Server ON";
    char data1[APPRO_DATA_LEN];
    data1[0] = 1; // 1 -> start
    Utils::get().SndApproMsg(APPRO_AVONOFF, data1, NULL);  // 1 -> start
}

fileMgr::~fileMgr()
{
   DEBUG();
#ifdef LIDARCAM
   // Suspend AV server
   DEBUG() << "AV Server OFF";
   char data1[APPRO_DATA_LEN];
   data1[0] = 0; // 0 -> suspend
   Utils::get().SndApproMsg(APPRO_AVONOFF, data1, NULL);
#endif
   emit enableSelectButton(true);
   delete ui;
}

void fileMgr::initLists()
{
  //  DEBUG();
  m_list << ui->pb_playback
         << ui->pb_upload
         << ui->tw_files
         << ui->pb_delete
         << ui->pb_selNew
         << ui->pb_selAll;
  
  m_cmdList << CMD_PLAYBACK
            << CMD_UPLOAD
            << CMD_LIST_FILES
            << CMD_DEL_FILE
            << CMD_SEL_NEW
            << CMD_SEL_ALL;
  
   connectWidgetSigs();
   ui->pb_print->setVisible(false);
}

void fileMgr::setInittoggleValues()
{
  //  DEBUG();
  // get file tree from file system
}


void fileMgr::toggleValue(int, int, int /*f*/)
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

void fileMgr::setVideoPath(void)
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

void fileMgr::loadVideoFiles()
{    
    QStringList m_defaultNameFilter;
    m_defaultNameFilter << "*.avi" << "*.jpg"<<"*.psev"<<"*.json"<<"*.md";

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

void fileMgr::enableButtons(bool b)
{
  //    DEBUG() << "Called with " << b;
  
    //ui->pb_delete->setEnabled(b);
    ui->pb_playback->setEnabled(b);
    ui->pb_upload->setEnabled(b);
	
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

void fileMgr::setTWFocus(int flag)
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

void fileMgr::twItemChanged()
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

void fileMgr::setCmd()
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
          ui->tw_files->clearSelection();
	  this->loadVideoFiles();
          ui->tw_files->selectAll();
    } else if (o->objectName() == ui->pb_selNew->objectName()) {
	  Utils& u = Utils::get();
	  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
	  m_fmState = FM_SELECT_NEW;
	  this->loadVideoFiles();
    }
#ifdef JUNK
    else if(o->objectName() == ui->pb_print->objectName()) {
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

     printTicket::get().print(ticketFileName);
      
      // clear the picked file from the screen
      m_currTWItem = NULL;
      m_twFocused = false;
      loadVideoFiles();
      
      return;

    }
#endif
    else if (o->objectName() == ui->pb_delete->objectName())
    { // Delete
      Utils& u = Utils::get();
      u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
      if (m_FIList.count() == 0)
         return;

      if (ui->tw_files->selectedItems().count() == 1) //m_currTWItem)
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

void fileMgr::exeDownSelect()
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


void fileMgr::exeUpSelect()
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
#include <QProgressDialog>
void fileMgr::uploadUSB()
{
  switch ( m_fmState )
    {
    case FM_SELECT_ALL:
      {      
		QProcess doCopy;
		QProgressDialog *dialog = new QProgressDialog;
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->setRange(0,0);
		connect(&doCopy, SIGNAL(finished(int)), dialog, SLOT(close()));
		doCopy.start("/usr/local/stalker/bin/dumpAlltoUSB");
		dialog->exec();

		int ret = doCopy.exitCode();
        if ( ret == 0 ) {
          qDebug() << "ret " << ret << "m_fmState " << m_fmState;
        }else{
          qDebug() << "ret " << ret << "m_fmState " << m_fmState;
		  QMessageBox *msgBox = new QMessageBox(this);
		  msgBox->setText("Upload:USB");
		  msgBox->setWindowModality(Qt::NonModal);
		  msgBox->setInformativeText("Missing USB Device!");
		  msgBox->setStandardButtons(QMessageBox::Ok);
		  msgBox->setDefaultButton(QMessageBox::Ok);
		  msgBox->exec();
		}
        break;
      }
    default:
      qDebug() << "m_fmState " << m_fmState;
    }
  return;
}

void fileMgr::on_pb_print_clicked()
{
   // Utils& u = Utils::get();

   //  u.getDisplayUnits();
}

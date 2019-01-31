#include "operat_frames.h"
#include "ui_operat_frames.h"
#include <QDebug>
#include "global.h"
#include "state.h"
#include "hardButtons.h"
#include "utils.h"
#include "ColdFireCommands.h"

#ifdef IS_TI_ARM
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif


OperatFrames::OperatFrames(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::OperatFrames)
{
    ui->setupUi(this);

    m_recording = 0;
    mLoop1 = 0;

    setCurrentIndex(OPR_SCREEN);
    state::get().setState(STATE_OPERATING );
    ui->lb_sd1->installEventFilter(this);
    ui->lb_sd2->installEventFilter(this);
    ui->lb_sd3->installEventFilter(this);
    ui->lb_sd4->installEventFilter(this);

    hardButtons& hd = hardButtons::get();
    hd.setHardButtonMap( 0, ui->pb_exit);
    hd.setHardButtonMap( 1, ui->pb_spare1);
    hd.setHardButtonMap( 2, ui->pb_record);
    hd.setHardButtonMap( 3, ui->pb_selfTest);
}

OperatFrames::~OperatFrames()
{
   hardButtons& hd = hardButtons::get();
   hd.setHardButtonMap(0, NULL);
   hd.setHardButtonMap(1, NULL);
   hd.setHardButtonMap(2, NULL);
   hd.setHardButtonMap(3, NULL);
   delete ui;
}

bool OperatFrames::eventFilter(QObject *o, QEvent *e)
{
   //for label click
   QLabel *lb = qobject_cast<QLabel *>(o);
   if (lb && e->type() == QEvent::MouseButtonPress)
   {
      emit sig_oprScreenReq(OPR_MENU_SCREEN);
      return true;
   }

   return false;
}


void OperatFrames::on_pb_exit_clicked()
{
   if ( state::get().getState() == STATE_OPERATING)
   {  // Switch to Operation Menu screen
      emit sig_oprScreenReq(OPR_MENU_SCREEN);
   }
   else
   {
      mLoop1 = 0;
      emit sig_oprScreenReq(REOPEN_HOME_SCREEN);
   }
}

void OperatFrames::setSpeedDistance(int index, QString &speed, QString &distance)
{
    if (currentIndex() != OPR_SCREEN)
        return;

    // Update the screen every 3rd time to save CPU power
    if (mLoop1 >= 12)
       mLoop1 = 0;
    if (mLoop1++ >= 4)
      return;

    QString s = speed + QString("\n") + distance;

    switch (index) {
    case OPR_SDLABEL1:
        ui->lb_sd1->setText(s);
        break;
    case OPR_SDLABEL2:
        ui->lb_sd2->setText(s);
        break;
    case OPR_SDLABEL3:
        ui->lb_sd3->setText(s);
        break;
    case OPR_SDLABEL4:
        ui->lb_sd4->setText(s);
        break;
    default:
        break;
    }
   // DEBUG() << sty;
}

void OperatFrames::on_pb_record_clicked()
{
   if ( state::get().getState() == STATE_OPERATING)
   {  // Switch to Operation Menu screen
      emit sig_oprScreenReq(OPR_MENU_SCREEN);
   }
   else
   {
      QString noRecord(QLatin1String("image: url(:/hh1/record); background-color: rgba(255, 255, 255, 0);"));
      QString inRecord(QLatin1String("image: url(:/hh1/stop); background-color: rgba(255, 255, 255, 0);"));

      Utils& u = Utils::get();
      u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

      if (m_recording == 0) {  //start recording
          m_recording = 1;
          ui->pb_record->setText("STOP");
          ui->pb_record->setStyleSheet(inRecord);
        //  emit this->sig_startRecord();

  #ifdef IS_TI_ARM
  #define SD_PATH     "/mnt/mmc/ipnc"
          time_t timep;
          tm * tm_struct;
          char filename[256];
          struct stat filestat;
          char serial_num[10] = "HH001000";
          time(&timep);
          tm_struct = gmtime(&timep);
          sprintf(filename, "%s/%s_%4d-%02d-%02d_%02d-%02d-%02d", SD_PATH, serial_num, tm_struct->tm_year+1900, tm_struct->tm_mon+1, tm_struct->tm_mday,
                  tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
          int ext;
          for(ext = 1; ext < 100; ext++)
          {
              char fullfile [256];
              sprintf(fullfile, "%s%2d.avi",filename, ext);
              int ret = stat(fullfile, &filestat);
              if(ret != 0) break;
          }
          sprintf(filename, "%s_%02d.avi", filename, ext);
          char cmd[256];
          sprintf(cmd, "/opt/ipnc/Appro_avi_save %s %d %d %d %d %d %d %d %d &\n",
                  filename, 999999999, 15, 15, 0, 1, 3, 0, 0);
          printf("Running command: %s\n", cmd);
          system(cmd);
  #endif
      }
      else {                  //stop recording
  #ifdef IS_TI_ARM
          system("killall -2 Appro_avi_save");
  #endif
          m_recording = 0;
          ui->pb_record->setText("RECORD");
          ui->pb_record->setStyleSheet(noRecord);
          //emit this->sig_stopRecord();
      }
   }
}

void OperatFrames::on_pb_selfTest_clicked()
{
   if ( state::get().getState() == STATE_OPERATING)
   {  // Switch to Operation Menu screen
      emit sig_oprScreenReq(OPR_MENU_SCREEN);
   }
}

void OperatFrames::on_pb_spare1_clicked()
{
   if ( state::get().getState() == STATE_OPERATING)
   {  // Switch to Operation Menu screen
      emit sig_oprScreenReq(OPR_MENU_SCREEN);
   }
   else
   {
      emit sig_oprScreenReq(OPR_SCREEN);
   }
}

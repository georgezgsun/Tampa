#include "play_back.h"
#include "ui_play_back.h"
#include "debug.h"
#include "hardButtons.h"
#include "utils.h"
#include "ColdFireCommands.h"

playBack::playBack(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::playBack)
{
   ui->setupUi(this);

   this->initLists();

   ui->pb_forward->setEnabled(false);
}

playBack::~playBack()
{
    delete ui;
}

void playBack::initLists()
{
  // Map Hard Button
  hardButtons& hd = hardButtons::get();
  hd.setHardButtonMap( 0, ui->pb_exit);
  hd.setHardButtonMap( 1, ui->pb_pause);
  //    hd.setHardButtonMap( 2, ui->pb_forward);
  hd.setHardButtonMap( 2, NULL);
  hd.setHardButtonMap( 3, ui->pb_restart);
}

void playBack::setFileName(QString &filename)
{
   mFileName = filename;
   if (mFileName.contains(".jpg") == true)
   {
      ui->pb_pause->setEnabled(false);
   }
}

void playBack::startPlay(void)
{
#ifdef IS_TI_ARM
   int value = (int)mFileName.toLatin1().data();
   int retv = Utils::get().sendCmdToCamera(CMD_PLAYBACK, value);
   if(retv)
      DEBUG() << "Error: Start Play, ret " << retv;

   // TODO determine if jpeg or video
   // if jpeg draw boxes based on json file input
   // else start timer to read metadata file and draw boxes
#else
   DEBUG() << mFileName;
   
   QPixmap image( mFileName );
   
   QPixmap resizedImage = image.scaled( DISPLAYSCREENSIZE_X, DISPLAYSCREENSIZE_Y, Qt::KeepAspectRatio );
   
   //  DEBUG() << "elapsed time " << sysTimer.elapsed() << "ui " << ui;
   ui->picture->setPixmap( resizedImage);

   // TODO determine if jpeg or video
   // if jpeg draw boxes based on json file input
   // else start timer to read metadata file and draw boxes

#endif
}

void playBack::displayPhoto(void)
{
   ui->pb_pause->setEnabled(false);
   ui->pb_forward->setEnabled(false);
   ui->pb_restart->setEnabled(false);

#ifdef IS_TI_ARM
   int value = (int)mFileName.toLatin1().data();
   int retv = Utils::get().sendCmdToCamera(CMD_DISPLAY_PHOTO, value);
   if(retv)
      DEBUG() << "Error: Display Photo, ret " << retv;
#endif
}

void playBack::on_pb_exit_clicked()
{
   // Stop
#ifdef IS_TI_ARM
   Utils& u = Utils::get();
   int retv = u.sendCmdToCamera(CMD_STOPPLAY, NULL);
   if(retv)
      DEBUG() << "Error: Stop Play, ret " << retv;
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
#endif
   close();
}

void playBack::on_pb_restart_clicked()
{
#ifdef IS_TI_ARM
   Utils& u = Utils::get();
   u.sendCmdToCamera(CMD_STOPPLAY, NULL);
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   ui->pb_pause->setText("PAUSE");
   u.sendCmdToCamera(CMD_PLAYBACK, (int)mFileName.toLatin1().data());
#endif
}

void playBack::on_pb_pause_clicked()
{
#ifdef IS_TI_ARM
   Utils& u = Utils::get();
   if (ui->pb_pause->text() == "PAUSE")
   {  // Pause
      u.sendCmdToCamera(CMD_PAUSEPLAY, NULL);
      ui->pb_pause->setText("RESUME");
   }
   else
   {  // Resume
      u.sendCmdToCamera(CMD_RESUMEPLAY, NULL);
      ui->pb_pause->setText("PAUSE");
   }
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
#endif
}

#include "illuminator.h"
#include "ui_illuminator.h"
#include "debug.h"
#include "hardButtons.h"
#include "ColdFireCommands.h"

illuminator::illuminator(QWidget *parent) :
   baseMenu(parent),
   ui(new Ui::illuminator)
{
   ui->setupUi(this);

   buildHashTables();
   setInittoggleValues();
}

illuminator::~illuminator()
{
   Utils::get().setIlluminator(&mIlluminator);
   delete ui;
}

void illuminator::buildHashTables()
{
    mIrcutList << "AUTO" << "OFF" << "ON";
    mIrisList << "F1.6" << "F2.3" << "F3.2" << "F4.5" << "F6.4";
    m2aModeList << "Auto" << "Manual";
    mShutterList << "1/100000" << "1/10000" << "1/2000" << "1/1500" << "1/1000" << "1/750" << "1/500"
                 << "1/250" << "1/120" << "1/60" << "1/30" << "1/15" << "1/8" << "1/4" << "1/2";
    mGainList << "42 db" << "36 db" << "30 db" << "24 db" << "18 db" << "12 db" << "6 db" << "0 db";
    mBoxList << "OFF" << "ON";
}

void illuminator::setInittoggleValues()
{
   Utils& u = Utils::get();
   ILLUMINATOR illu = u.getIlluminator();
   if (illu.mBoxIndex == -1)
   {  // Not initialized yet
      // Default values
      mIlluminator.mIrcutIndex = 0;    // Auto
      mIlluminator.mIrisIndex = 0;     // F1.6
      mIlluminator.m2aModeIndex = 0;   // Auto
      mIlluminator.mShutterIndex = 10; // 1/30
      mIlluminator.mGainIndex = mGainList.size() - 1;  // 0 db
      mIlluminator.mBoxIndex = 1;                      // On
      u.setIlluminator(&mIlluminator);
   }
   else
      memcpy((void *)&mIlluminator, (void *)&illu, sizeof(ILLUMINATOR));

   ui->pb_ircut->setText(mIrcutList.at(mIlluminator.mIrcutIndex));
   ui->pb_iris->setText(mIrisList.at(mIlluminator.mIrisIndex));
   ui->pb_2amode->setText(m2aModeList.at(mIlluminator.m2aModeIndex));
   ui->pb_shutter->setText(mShutterList.at(mIlluminator.mShutterIndex));
   ui->pb_gain->setText(mGainList.at(mIlluminator.mGainIndex));
   ui->pb_box->setText(mBoxList.at(mIlluminator.mBoxIndex));

   if (!mIlluminator.m2aModeIndex)
   {  // Auto mode
      ui->pb_shutter->setEnabled(false);
      ui->pb_gain->setEnabled(false);
   }

#ifdef LIDARCAM
   hardButtons& hd = hardButtons::get();
   hd.setHardButtonMap( 0, ui->pb_exit);
   hd.setHardButtonMap( 1, NULL);
   hd.setHardButtonMap( 2, NULL);
   hd.setHardButtonMap( 3, NULL);
#endif
}

void illuminator::on_pb_exit_clicked()
{
#ifdef LIDARCAM
   Utils::get().sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
#endif
   close();
}

void illuminator::on_pb_ircut_clicked()
{
   if (++mIlluminator.mIrcutIndex >= mIrcutList.size())
   {
       mIlluminator.mIrcutIndex = 0;
   }
   ui->pb_ircut->setText(mIrcutList.at(mIlluminator.mIrcutIndex));

   Utils::get().sendCmdToCamera(CMD_CAMERA_IRCUT, mIlluminator.mIrcutIndex);
   ui->pb_ircut->setEnabled(false);
}

void illuminator::on_pb_iris_clicked()
{
   if (++mIlluminator.mIrisIndex >= mIrisList.size())
   {
       mIlluminator.mIrisIndex = 0;
   }
   ui->pb_iris->setText(mIrisList.at(mIlluminator.mIrisIndex));

   Utils::get().sendCmdToCamera(CMD_IRIS, mIlluminator.mIrisIndex);
}

void illuminator::on_pb_2amode_clicked()
{
   if (++mIlluminator.m2aModeIndex >= m2aModeList.size())
   {
       mIlluminator.m2aModeIndex = 0;
   }
   ui->pb_2amode->setText(m2aModeList.at(mIlluminator.m2aModeIndex));
   if (!mIlluminator.m2aModeIndex)
   {  // Auto mode
      ui->pb_shutter->setEnabled(false);
      ui->pb_gain->setEnabled(false);
   }
   else
   {  // Manual mode
      ui->pb_shutter->setEnabled(true);
      ui->pb_gain->setEnabled(true);
   }
   int i1 = 1; // Auto
   if (mIlluminator.m2aModeIndex)
      i1 = 3;  // Manual
   Utils::get().sendCmdToCamera(CMD_CAMERA_2A_MODE, i1);
}

void illuminator::on_pb_shutter_clicked()
{
   if (++mIlluminator.mShutterIndex >= mShutterList.size())
   {
       mIlluminator.mShutterIndex = 0;
   }
   ui->pb_shutter->setText(mShutterList.at(mIlluminator.mShutterIndex));

   Utils::get().sendCmdToCamera(CMD_SHUTTER, mIlluminator.mShutterIndex);
}

void illuminator::on_pb_gain_clicked()
{
   if (++mIlluminator.mGainIndex >= mGainList.size())
   {
       mIlluminator.mGainIndex = 0;
   }
   ui->pb_gain->setText(mGainList.at(mIlluminator.mGainIndex));

   Utils::get().sendCmdToCamera(CMD_GAIN, mIlluminator.mGainIndex);
}

void illuminator::on_pb_box_clicked()
{
   if (++mIlluminator.mBoxIndex >= mBoxList.size())
   {
       mIlluminator.mBoxIndex = 0;
   }
   ui->pb_box->setText(mBoxList.at(mIlluminator.mBoxIndex));

   Utils::get().sendCmdToCamera(CMD_CAMERA_BOX, mIlluminator.mBoxIndex, 20);
}

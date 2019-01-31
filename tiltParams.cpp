#include "tiltParams.h"
#include "ui_tiltParams.h"
#include "utils.h"

tiltParams::tiltParams(QWidget *parent) :
   baseMenu(parent),
   ui(new Ui::tiltParams)
{
   ui->setupUi(this);

   initLists();
   buildHashTables();
   setInittoggleValues();

   m_listIndex = m_prevListIndex = 0;
   m_command = m_cmdList.at(m_listIndex);
}

tiltParams::~tiltParams()
{
   Utils& u = Utils::get();
   if ( mMagXmax != mSensor.magXmax
       || mMagXmin != mSensor.magXmin
       || mMagYmax != mSensor.magYmax
       || mMagYmin != mSensor.magYmin
       || mMagZmax != mSensor.magZmax
       || mMagZmin != mSensor.magZmin
       || mMagThetaX != mSensor.magThetaX
       || mMagThetaY != mSensor.magThetaY
       || mMagThetaZ != mSensor.magThetaZ
       || mAccXmax != mSensor.accXmax
       || mAccXmin != mSensor.accXmin
       || mAccYmax != mSensor.accYmax
       || mAccYmin != mSensor.accYmin
       || mAccZmax != mSensor.accZmax
       || mAccZmin != mSensor.accZmin
       || mAccThetaX != mSensor.accThetaX
       || mAccThetaY != mSensor.accThetaY
       || mAccThetaZ != mSensor.accThetaZ)
   {
      u.setSensorInDB(mSensor);
   }

   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagXmax()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagXmin()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagYmax()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagYmin()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagZmax()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagZmin()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagThetaX()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagThetaY()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagThetaZ()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccXmax()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccXmin()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccYmax()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccYmin()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccZmax()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccZmin()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccThetaX()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccThetaY()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccThetaZ()));

   delete ui;
}

void tiltParams::initLists()
{
   m_list << ui->le_magXmax
          << ui->le_magXmin
          << ui->le_magYmax
          << ui->le_magYmin
          << ui->le_magZmax
          << ui->le_magZmin
          << ui->le_magThetaX
          << ui->le_magThetaY
          << ui->le_magThetaZ
          << ui->le_accXmax
          << ui->le_accXmin
          << ui->le_accYmax
          << ui->le_accYmin
          << ui->le_accZmax
          << ui->le_accZmin
          << ui->le_accThetaX
          << ui->le_accThetaY
          << ui->le_accThetaZ;

   m_cmdList << CMD_FAC_TILT_MAG_XMAX
             << CMD_FAC_TILT_MAG_XMIN
             << CMD_FAC_TILT_MAG_YMAX
             << CMD_FAC_TILT_MAG_YMIN
             << CMD_FAC_TILT_MAG_ZMAX
             << CMD_FAC_TILT_MAG_ZMIN
             << CMD_FAC_TILT_MAG_THETAX
             << CMD_FAC_TILT_MAG_THETAY
             << CMD_FAC_TILT_MAG_THETAZ
             << CMD_FAC_TILT_ACC_XMAX
             << CMD_FAC_TILT_ACC_XMIN
             << CMD_FAC_TILT_ACC_YMAX
             << CMD_FAC_TILT_ACC_YMIN
             << CMD_FAC_TILT_ACC_ZMAX
             << CMD_FAC_TILT_ACC_ZMIN
             << CMD_FAC_TILT_ACC_THETAX
             << CMD_FAC_TILT_ACC_THETAY
             << CMD_FAC_TILT_ACC_THETAZ;

   connectWidgetSigs();
}


void tiltParams::setInittoggleValues()
{
#ifdef HH1
   Utils& u = Utils::get();
   mSensor = u.getSensorFromDB();

   mMagXmax = mSensor.magXmax;
   mMagXmin = mSensor.magXmin;
   mMagYmax = mSensor.magYmax;
   mMagYmin = mSensor.magYmin;
   mMagZmax = mSensor.magZmax;
   mMagZmin = mSensor.magZmin;
   mMagThetaX = mSensor.magThetaX;
   mMagThetaY = mSensor.magThetaY;
   mMagThetaZ = mSensor.magThetaZ;
   mAccXmax = mSensor.accXmax;
   mAccXmin = mSensor.accXmin;
   mAccYmax = mSensor.accYmax;
   mAccYmin = mSensor.accYmin;
   mAccZmax = mSensor.accZmax;
   mAccZmin = mSensor.accZmin;
   mAccThetaX = mSensor.accThetaX;
   mAccThetaY = mSensor.accThetaY;
   mAccThetaZ = mSensor.accThetaZ;

   QString qs1 = QString::number(mMagXmax);
   ui->le_magXmax->setText(qs1);
   qs1 = QString::number(mMagXmin);
   ui->le_magXmin->setText(qs1);
   qs1 = QString::number(mMagYmax);
   ui->le_magYmax->setText(qs1);
   qs1 = QString::number(mMagYmin);
   ui->le_magYmin->setText(qs1);
   qs1 = QString::number(mMagZmax);
   ui->le_magZmax->setText(qs1);
   qs1 = QString::number(mMagZmin);
   ui->le_magZmin->setText(qs1);
   qs1 = QString::number(mMagThetaX);
   ui->le_magThetaX->setText(qs1);
   qs1 = QString::number(mMagThetaY);
   ui->le_magThetaY->setText(qs1);
   qs1 = QString::number(mMagThetaZ);
   ui->le_magThetaZ->setText(qs1);
   qs1 = QString::number(mAccXmax);
   ui->le_accXmax->setText(qs1);
   qs1 = QString::number(mAccXmin);
   ui->le_accXmin->setText(qs1);
   qs1 = QString::number(mAccYmax);
   ui->le_accYmax->setText(qs1);
   qs1 = QString::number(mAccYmin);
   ui->le_accYmin->setText(qs1);
   qs1 = QString::number(mAccZmax);
   ui->le_accZmax->setText(qs1);
   qs1 = QString::number(mAccZmin);
   ui->le_accZmin->setText(qs1);
   qs1 = QString::number(mAccThetaX);
   ui->le_accThetaX->setText(qs1);
   qs1 = QString::number(mAccThetaY);
   ui->le_accThetaY->setText(qs1);
   qs1 = QString::number(mAccThetaZ);
   ui->le_accThetaZ->setText(qs1);
#endif
}

void tiltParams::toggleValue(int cmd, int idx, int /*f*/)
{
   idx = idx;
   switch (cmd)
   {
      case CMD_FAC_TILT_MAG_XMAX:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagXmax()));
         break;
      case CMD_FAC_TILT_MAG_XMIN:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagXmin()));
         break;
      case CMD_FAC_TILT_MAG_YMAX:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagYmax()));
         break;
      case CMD_FAC_TILT_MAG_YMIN:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagYmin()));
         break;
      case CMD_FAC_TILT_MAG_ZMAX:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagZmax()));
         break;
      case CMD_FAC_TILT_MAG_ZMIN:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagZmin()));
         break;
      case CMD_FAC_TILT_MAG_THETAX:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagThetaX()));
         break;
      case CMD_FAC_TILT_MAG_THETAY:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagThetaY()));
         break;
      case CMD_FAC_TILT_MAG_THETAZ:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkMagThetaZ()));
         break;
      case CMD_FAC_TILT_ACC_XMAX:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccXmax()));
         break;
      case CMD_FAC_TILT_ACC_XMIN:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccXmin()));
         break;
      case CMD_FAC_TILT_ACC_YMAX:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccYmax()));
         break;
      case CMD_FAC_TILT_ACC_YMIN:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccYmin()));
         break;
      case CMD_FAC_TILT_ACC_ZMAX:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccZmax()));
         break;
      case CMD_FAC_TILT_ACC_ZMIN:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccZmin()));
         break;
      case CMD_FAC_TILT_ACC_THETAX:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccThetaX()));
         break;
      case CMD_FAC_TILT_ACC_THETAY:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccThetaY()));
         break;
      case CMD_FAC_TILT_ACC_THETAZ:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkAccThetaZ()));
         break;
//      default:
//        baseMenu::toggleValue(cmd, idx);
    }
}

void tiltParams::checkMagXmax()
{
   float f1 = ui->le_magXmax->text().toFloat();
   if (f1 < 0.0 || f1 > 1000.0 || mSensor.magXmax <= mSensor.magXmin)
   {
      QString qs1 = QString::number(mSensor.magXmax);
      ui->le_magXmax->setText(qs1); // Go back to old data
   }
   else
      mSensor.magXmax = f1;
}

void tiltParams::checkMagXmin()
{
   float f1 = ui->le_magXmin->text().toFloat();
   if (f1 < -100.0 || f1 > 0.0 || mSensor.magXmin >= mSensor.magXmax)
   {
      QString qs1 = QString::number(mSensor.magXmin);
      ui->le_magXmin->setText(qs1); // Go back to old data
   }
   else
      mSensor.magXmin = f1;
}

void tiltParams::checkMagYmax()
{
   float f1 = ui->le_magYmax->text().toFloat();
   if (f1 < 0.0 || f1 > 1000.0 || mSensor.magYmax <= mSensor.magYmin)
   {
      QString qs1 = QString::number(mSensor.magYmax);
      ui->le_magYmax->setText(qs1); // Go back to old data
   }
   else
      mSensor.magYmax = f1;
}

void tiltParams::checkMagYmin()
{
   float f1 = ui->le_magYmin->text().toFloat();
   if (f1 < -100.0 || f1 > 0.0 || mSensor.magYmin >= mSensor.magYmax)
   {
      QString qs1 = QString::number(mSensor.magYmin);
      ui->le_magYmin->setText(qs1); // Go back to old data
   }
   else
      mSensor.magYmin = f1;
}

void tiltParams::checkMagZmax()
{
   float f1 = ui->le_magZmax->text().toFloat();
   if (f1 < 1000.0 || f1 > 3000.0 || mSensor.magZmax <= mSensor.magZmin)
   {
      QString qs1 = QString::number(mSensor.magZmax);
      ui->le_magZmax->setText(qs1); // Go back to old data
   }
   else
      mSensor.magZmax = f1;
}

void tiltParams::checkMagZmin()
{
   float f1 = ui->le_magZmin->text().toFloat();
   if (f1 < 1000.0 || f1 > 3000.0 || mSensor.magZmin >= mSensor.magZmax)
   {
      QString qs1 = QString::number(mSensor.magZmin);
      ui->le_magZmin->setText(qs1); // Go back to old data
   }
   else
      mSensor.magZmin = f1;
}

void tiltParams::checkMagThetaX()
{
   float f1 = ui->le_magThetaX->text().toFloat();
   if (f1 < 0.0 || f1 > 100.0)
   {
      QString qs1 = QString::number(mSensor.magThetaX);
      ui->le_magThetaX->setText(qs1); // Go back to old data
   }
   else
      mSensor.magThetaX = f1;
}

void tiltParams::checkMagThetaY()
{
   float f1 = ui->le_magThetaY->text().toFloat();
   if (f1 < 0.0 || f1 > 100.0)
   {
      QString qs1 = QString::number(mSensor.magThetaY);
      ui->le_magThetaY->setText(qs1); // Go back to old data
   }
   else
      mSensor.magThetaY = f1;
}

void tiltParams::checkMagThetaZ()
{
   float f1 = ui->le_magThetaZ->text().toFloat();
   if (f1 < 0.0 || f1 > 100.0)
   {
      QString qs1 = QString::number(mSensor.magThetaZ);
      ui->le_magThetaZ->setText(qs1); // Go back to old data
   }
   else
      mSensor.magThetaZ = f1;
}

void tiltParams::checkAccXmax()
{
   float f1 = ui->le_accXmax->text().toFloat();
   if (f1 > 17500.0 || mSensor.accXmax <= mSensor.accXmin)
   {
      QString qs1 = QString::number(mSensor.accXmax);
      ui->le_accXmax->setText(qs1); // Go back to old data
   }
   else
      mSensor.accXmax = f1;
}

void tiltParams::checkAccXmin()
{
   float f1 = ui->le_accXmin->text().toFloat();
   if (f1 < -17500.0 || mSensor.accXmin >= mSensor.accXmax)
   {
      QString qs1 = QString::number(mSensor.accXmin);
      ui->le_accXmin->setText(qs1); // Go back to old data
   }
   else
      mSensor.accXmin = f1;
}

void tiltParams::checkAccYmax()
{
   float f1 = ui->le_accYmax->text().toFloat();
   if (f1 > 17500.0 || mSensor.accYmax <= mSensor.accYmin)
   {
      QString qs1 = QString::number(mSensor.accYmax);
      ui->le_accYmax->setText(qs1); // Go back to old data
   }
   else
      mSensor.accYmax = f1;
}

void tiltParams::checkAccYmin()
{
   float f1 = ui->le_accYmin->text().toFloat();
   if (f1 < -17500.0 || mSensor.accYmin >= mSensor.accYmax)
   {
      QString qs1 = QString::number(mSensor.accYmin);
      ui->le_accYmin->setText(qs1); // Go back to old data
   }
   else
      mSensor.accYmin = f1;
}

void tiltParams::checkAccZmax()
{
   float f1 = ui->le_accZmax->text().toFloat();
   if (f1 > 17500.0 || mSensor.accZmax <= mSensor.accZmin)
   {
      QString qs1 = QString::number(mSensor.accZmax);
      ui->le_accZmax->setText(qs1); // Go back to old data
   }
   else
      mSensor.accZmax = f1;
}

void tiltParams::checkAccZmin()
{
   float f1 = ui->le_accZmin->text().toFloat();
   if (f1 < -17500.0 || mSensor.accZmin >= mSensor.accZmax)
   {
      QString qs1 = QString::number(mSensor.accZmin);
      ui->le_accZmin->setText(qs1); // Go back to old data
   }
   else
      mSensor.accZmin = f1;
}

void tiltParams::checkAccThetaX()
{
   float f1 = ui->le_accThetaX->text().toFloat();
   if (f1 < 0.0 || f1 > 100.0)
   {
      QString qs1 = QString::number(mSensor.accThetaX);
      ui->le_accThetaX->setText(qs1); // Go back to old data
   }
   else
      mSensor.accThetaX = f1;
}

void tiltParams::checkAccThetaY()
{
   float f1 = ui->le_accThetaY->text().toFloat();
   if (f1 < 0.0 || f1 > 100.0)
   {
      QString qs1 = QString::number(mSensor.accThetaY);
      ui->le_accThetaY->setText(qs1); // Go back to old data
   }
   else
      mSensor.accThetaY = f1;
}
void tiltParams::checkAccThetaZ()
{
   float f1 = ui->le_accThetaZ->text().toFloat();
   if (f1 < 0.0 || f1 > 100.0)
   {
      QString qs1 = QString::number(mSensor.accThetaZ);
      ui->le_accThetaZ->setText(qs1); // Go back to old data
   }
   else
      mSensor.accThetaZ = f1;
}

#include "radarParams.h"
#include "ui_radarParams.h"
#include "utils.h"

radarParams::radarParams(QWidget *parent) :
   baseMenu(parent),
   ui(new Ui::radarParams)
{
   ui->setupUi(this);

   initLists();
   buildHashTables();
   setInittoggleValues();

   m_listIndex = m_prevListIndex = 0;
   m_command = m_cmdList.at(m_listIndex);
}

radarParams::~radarParams()
{
   Utils& u = Utils::get();
   if (mFrequency != mConf.frequency
       || mBandwidth != mConf.bandwidth
       || mPowerIndex != mConf.radarPower
       || mSensitivityIndex != mConf.sensitivity)
   {
      u.setConfiguration(mConf);
   }

   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkFreqRange()));
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkBandwidthRange()));

   delete ui;
}

void radarParams::initLists()
{
   m_list << ui->le_radarFreq
          << ui->le_radarBandwidth;

   m_cmdList << CMD_FAC_RADAR_FREQ
             << CMD_FAC_RADAR_BANDWIDTH;

   connectWidgetSigs();
}


void radarParams::setInittoggleValues()
{
#ifdef HH1
   Utils& u = Utils::get();
   mConf = u.getConfiguration();

   mFrequency = mConf.frequency;
   mBandwidth = mConf.bandwidth;
   mPowerIndex = mConf.radarPower;
   mSensitivityIndex = mConf.sensitivity;

   QString qs1 = QString::number(mFrequency);
   ui->le_radarFreq->setText(qs1);
   qs1 = QString::number(mBandwidth);
   ui->le_radarBandwidth->setText(qs1);
   ui->pb_radarPower->setText(mPowerList.at(mPowerIndex));
   ui->pb_radarSensitivity->setText(mSensitivityList.at(mSensitivityIndex));
#endif
}

void radarParams::buildHashTables()
{
   mPowerList << "1" << "2" << "3";
   mSensitivityList << "1" << "2" << "3" << "4" << "5";
}

void radarParams::toggleValue(int cmd, int idx, int /*f*/)
{
   switch (cmd)
   {
      case CMD_FAC_RADAR_FREQ:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkFreqRange()));
         break;
      case CMD_FAC_RADAR_BANDWIDTH:
         m_vkb->setNumKeyboard();
         connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(checkBandwidthRange()));
         break;
      default:
        baseMenu::toggleValue(cmd, idx);
    }
}

void radarParams::checkFreqRange()
{
   float f1 = ui->le_radarFreq->text().toFloat();
   if (f1 < 24 || f1 > 24.25)
   {
      QString qs1 = QString::number(mConf.frequency);
      ui->le_radarFreq->setText(qs1); // Go back to old data
   }
   else
      mConf.frequency = f1;
}

void radarParams::checkBandwidthRange()
{
   float f1 = ui->le_radarBandwidth->text().toFloat();
   if (f1 < 50 || f1 > 250)
   {
      QString qs1 = QString::number(mConf.bandwidth);
      ui->le_radarBandwidth->setText(qs1); // Go back to old data
   }
   else
      mConf.bandwidth = f1;
}

void radarParams::on_pb_radarPower_clicked()
{
  if (++mConf.radarPower >= (unsigned int)mPowerList.size())
   {
       mConf.radarPower = 0;
   }
   ui->pb_radarPower->setText(mPowerList.at(mConf.radarPower));
}

void radarParams::on_pb_radarSensitivity_clicked()
{
  if (++mConf.sensitivity >= (unsigned int)mSensitivityList.size())
   {
       mConf.sensitivity = 0;
   }
   ui->pb_radarSensitivity->setText(mSensitivityList.at(mConf.sensitivity));
}

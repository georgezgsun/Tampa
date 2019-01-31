#include "service.h"
#include "ui_service.h"
#include "state.h"
#include "utils.h"
#include "debug.h"
#include "ColdFireCommands.h"

service::service(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::service)
{
    ui->setupUi(this);

    this->initLists();
    this->buildHashTables();
    this->setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_SUB_MENU3);
    m_listIndex = m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);
}

service::~service()
{
    delete ui;
}

void service::initLists()
{
    m_refClockIndex = 0;

    m_list << ui->pb_refClock
           << ui->pb_lidarSetup
           << ui->pb_verification;

    m_cmdList << CMD_SRV_REF_CLK
              << CMD_SRV_LIDAR_SETUP
              << CMD_SRV_CALIB_DATA;

    this->connectWidgetSigs();
}

void service::buildHashTables()
{
    m_refClockList << "REFERENCE CLOCK OFF" << "REFERENCE CLOCK ON";

    m_hashValueList[CMD_SRV_REF_CLK] = &m_refClockList;
    m_hashValueIndex[CMD_SRV_REF_CLK] = &m_refClockIndex;
}

void service::setInittoggleValues()
{
#ifdef HH1
   ui->pb_refClock->setText("SERVICE OPTIONS");
   ui->pb_lidarSetup->setText("FACTORY RESTORE");
   ui->pb_verification->setText("CERTIFICATION");
#else
   ui->pb_refClock->setText(m_refClockList.at(m_refClockIndex));

   // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
   ui->pb_lidarSetup->setEnabled(false);
   ui->pb_refClock->setEnabled(false);
#endif
}

void service::toggleValue(int cmd, int idx, int f)
{
  UNUSED(idx);
  UNUSED(f);
  
   switch (cmd)
   {
      case CMD_SRV_REF_CLK:
         if( m_refClockIndex == 0 )
         {
            DEBUG() << "m_refClockIndex " << m_refClockIndex << "DISABLE";
#ifdef LIDARCAM
            Utils& u = Utils::get();
            u.sendMbPacket( (unsigned char) CMD_REF_CLOCK_OUT_DISABLE, 0, NULL, NULL );
#endif
         }
         else
         {
            DEBUG() << "m_refClockIndex " << m_refClockIndex << "ENABLE";
#ifdef LIDARCAM
            Utils& u = Utils::get();
            u.sendMbPacket( (unsigned char) CMD_REF_CLOCK_OUT_ENABLE, 0, NULL, NULL );
#endif
         }
         break;
      case CMD_SRV_LIDAR_SETUP:
         break;
      case CMD_SRV_CALIB_DATA:
         break;
      default:
         //	baseMenu::toggleValue(cmd, idx, 1);
         break;
   }
   return;
}

void service::on_pb_refClock_clicked()
{
#ifndef HH1
    if (++m_refClockIndex >= m_refClockList.size())
    {
        m_refClockIndex = 0;
    }
    ui->pb_refClock->setText(m_refClockList.at(m_refClockIndex));
#endif
}

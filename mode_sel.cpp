#include "mode_sel.h"
#include "ui_mode_sel.h"
#include "state.h"
#include "utils.h"
#include "lidarMsg.h"
#include "Lidar_Buff.h"
#include "ColdFireCommands.h"

//#define LIDAR_DEBUG

modeSel::modeSel(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::modeSel)
{
    ui->setupUi(this);
    Utils& u = Utils::get();
    SysConfig mConf = u.getConfiguration();
       int units = mConf.units;
       if (units  == 0)
           {
               ui->lb_radarH_2->setText("Feet");
               ui->lb_distFrR_2->setText("Feet");
               ui->lb_targetH_2->setText("Feet");
           }
       else if ( units == 1)
       {
           ui->lb_radarH_2->setText("Meters");
           ui->lb_distFrR_2->setText("Meters");
           ui->lb_targetH_2->setText("Meters");
       }
       else if (units  == 2)
       {
           ui->lb_radarH_2->setText("Feet");
           ui->lb_distFrR_2->setText("Feet");
           ui->lb_targetH_2->setText("Feet");
       }


    this->initLists();
    this->buildHashTables();
    this->setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_MODE_SEL_MENU);
    m_listIndex = m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);

  //  connect(ui->cb_zone, SIGNAL(stateChanged(int)), this, SLOT(setCmd()));
   // connect(ui->cb_autoObs, SIGNAL(stateChanged(int)), this, SLOT(setCmd()));
    //connect(ui->cb_ftc, SIGNAL(stateChanged(int)), this, SLOT(setCmd()));
}

modeSel::~modeSel()
{
    mConf.radarH = ui->le_radarH->text().toFloat();
    mConf.distFrR = ui->le_distFrR->text().toFloat();
    mConf.targetH = ui->le_targetH->text().toFloat();
    mConf.lensFocal = ui->pb_lensFocal->text().toInt();
    Utils& u = Utils::get();
     if (mConf.radarH != mOldRadarH
      || mConf.distFrR != mOldDistFrR
      || mConf.targetH != mOldTargetH
      || mConf.lensFocal!= mOldLensFocal)

      u.setConfiguration(mConf);

    delete ui;
}

void modeSel::initLists()
{
    m_list << ui->le_radarH
           << ui->le_distFrR
           << ui->le_targetH
           << ui->pb_lensFocal;


    m_cmdList << CMD_RADAR_H
              << CMD_DIST_FRR
              << CMD_TARGET_H
              << CMD_LENSFOCAL;
    this->connectWidgetSigs();
}

void modeSel::buildHashTables()
{
    m_lensFocalList << "25" << "30" << "50" << "75";
    m_lensFocalIndex = 0;
    m_hashValueList[CMD_LENSFOCAL] = &m_lensFocal;
    m_hashValueIndex[CMD_LENSFOCAL] = &m_lensFocalIndex;

}

void modeSel::setInittoggleValues()
{

    Utils& u = Utils::get();
   // int operMode = u.getMode();
    mConf = u.getConfiguration();
    mOldRadarH = mConf.radarH;
    mOldDistFrR= mConf.distFrR;
    mOldTargetH = mConf.targetH;
    mOldLensFocal = mConf.lensFocal;

    ui->le_radarH->setText(QString::number(mOldRadarH));
    ui->le_distFrR ->setText(QString::number(mOldDistFrR));
    ui->le_targetH ->setText(QString::number( mOldTargetH));
    ui->pb_lensFocal->setText(QString::number( mOldLensFocal));

}

void modeSel::toggleValue(int cmd, int idx, int /*f*/)
{
   switch (cmd) {
   case CMD_RADAR_H:
   case CMD_DIST_FRR:
   case CMD_TARGET_H:
   m_vkb ->setNumKeyboard();
   break;
   case CMD_LENSFOCAL:
   break;
    default:
        baseMenu::toggleValue(cmd, idx);
    }
}

void modeSel::on_pb_lensFocal_clicked()
{
    if (m_lensFocalIndex < (m_lensFocalList.size() - 1))
        m_lensFocalIndex++;
    else
        m_lensFocalIndex = 0;
    ui->pb_lensFocal->setText(m_lensFocalList.at(m_lensFocalIndex));
}






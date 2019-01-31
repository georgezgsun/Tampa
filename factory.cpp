#include "factory.h"
#include "ui_factory.h"
#include "state.h"

factory::factory(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::factory)
{
    ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();
    this->buildHashTables();

	state& v = state::get();
    v.setState(STATE_SUB_MENU3);
    m_listIndex = m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);
}

factory::~factory()
{
    delete ui;
}

void factory::initLists()
{
    m_list << ui->pb_alignment
#ifdef HH1
           << ui->pb_factoryCalibrate
           << ui->pb_radarParams
#endif
	<< ui->pb_factoryDefault;

    m_cmdList << CMD_FAC_ALIGNMENT
#ifdef HH1
              << CMD_FAC_CACLMAG3110
              << CMD_FAC_RADAR_PARAMS
              << CMD_FAC_TILT_PARAMS
#endif
			  << CMD_FAC_DEFAULTS;
	
    this->connectWidgetSigs();

#ifndef HH1
	ui->pb_factoryCalibrate->setVisible(false);
   ui->pb_radarParams->setVisible(false);
   ui->pb_tiltParams->setVisible(false);
#endif
	
}


void factory::setInittoggleValues()
{
//    m_refClkIdx = 0;
}

void factory::buildHashTables()
{
//    m_refClkList << "OFF" << "OUTPUT";

//    m_hashValueList[CMD_FAC_REF_CLK] = &m_refClkList;
//    m_hashValueIndex[CMD_FAC_REF_CLK] = &m_refClkIdx;
}



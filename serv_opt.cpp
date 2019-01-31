#include "serv_opt.h"
#include "ui_serv_opt.h"
#include <QDebug>
#include "state.h"

servOpt::servOpt(int type, QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::servOpt),
    m_menuType(type)
{
   ui->setupUi(this);

   initLists();
   setInittoggleValues();
   buildHashTables();

	state& v = state::get();
	v.setState(STATE_SUB_MENU4);
   m_command = CMD_NONE;
}

servOpt::~servOpt()
{
    delete ui;
}


void servOpt::initLists()
{
}


void servOpt::setInittoggleValues()
{
   m_spdTrigOnIdx =
            m_spdTrigYesIdx =
            m_zoneModeOnIdx =
            m_zoneModeYesIdx =
            m_tdModeOnIdx =
            m_tdModeYesIdx = 0;
   ui->pb_on1->setText("ON");
   ui->pb_yes1->setText("YES");
}


void servOpt::buildHashTables()
{
}

void servOpt::on_pb_on1_clicked()
{
   if (m_spdTrigOnIdx)
   {
       m_spdTrigOnIdx = 0;
       ui->pb_on1->setText("ON");
   }
   else
   {
       m_spdTrigOnIdx = 1;
       ui->pb_on1->setText("OFF");
   }
}

void servOpt::on_pb_yes1_clicked()
{
   if (m_spdTrigYesIdx)
   {
       m_spdTrigYesIdx = 0;
       ui->pb_yes1->setText("YES");
   }
   else
   {
       m_spdTrigYesIdx = 1;
       ui->pb_yes1->setText("NO");
   }
}

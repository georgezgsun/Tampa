#include "metaData.h"
#include "ui_metaData.h"
#include "state.h"
#include "utils.h"

metaData::metaData(QWidget *parent) :
   baseMenu(parent),
   ui(new Ui::metaData)
{
   ui->setupUi(this);

   mInitialized = false;   // Initialization not done yet
   initLists();

   state& v = state::get();
   v.setState(STATE_SUB_MENU3);
   m_command = m_cmdList.at(m_listIndex);

   mAdmin = mOldAdmin = Utils::get().getAdmin();
   ui->le_metaLine1->setText(mAdmin.metaData1);
   ui->le_metaLine2->setText(mAdmin.metaData2);

   QStringList list1 = mAdmin.metaData1.split(" ");
   QStringList list2 = mAdmin.metaData2.split(" ");
   QString current1;

   for (QStringList::iterator it = list1.begin(); it != list1.end(); it++)
   {
      current1 = *it;
      if (current1 == (QString)"%0")
         ui->cb_deviceID->setChecked(true);
      else if (current1 == (QString)"%1")
         ui->cb_userID->setChecked(true);
      else if (current1 == (QString)"%2")
         ui->cb_recordNumber->setChecked(true);
      else if (current1 == (QString)"%3")
         ui->cb_speedLimit->setChecked(true);
      else if (current1 == (QString)"%4")
         ui->cb_speed->setChecked(true);
      else if (current1 == (QString)"%5")
         ui->cb_distance->setChecked(true);
      else if (current1 == (QString)"%6")
         ui->cb_location->setChecked(true);
   }

   for (QStringList::iterator it = list2.begin(); it != list2.end(); it++)
   {
      current1 = *it;
      if (current1 == (QString)"%0")
         ui->cb_deviceID->setChecked(true);
      else if (current1 == (QString)"%1")
         ui->cb_userID->setChecked(true);
      else if (current1 == (QString)"%2")
         ui->cb_recordNumber->setChecked(true);
      else if (current1 == (QString)"%3")
         ui->cb_speedLimit->setChecked(true);
      else if (current1 == (QString)"%4")
         ui->cb_speed->setChecked(true);
      else if (current1 == (QString)"%5")
         ui->cb_distance->setChecked(true);
      else if (current1 == (QString)"%6")
         ui->cb_location->setChecked(true);
   }

   mLine1 = true;
   ui->le_metaLine2->setEnabled(false);   // Disable line2 input
   mPosition = mAdmin.metaPosition;
   if (mPosition == WATERMARK_TOPLEFT)
      ui->pb_position->setText((QString)"SCREEN TOP");
   ui->pb_position->setEnabled(false); // Disable this feature right now.
   mInitialized = true;    // Initialization is done
}

metaData::~metaData()
{
   mAdmin.metaData1 = ui->le_metaLine1->text();
   mAdmin.metaData2 = ui->le_metaLine2->text();
   mAdmin.metaPosition = mPosition;
   //Is anything changed?
   if (memcmp((void *)&mAdmin, (void *)&mOldAdmin, sizeof(Administration)))
   {   // Yes
       Utils::get().setAdmin(mAdmin);
   }
   delete ui;
}

void metaData::initLists()
{
   m_list << ui->le_metaLine1 << ui->le_metaLine2 << ui->pb_line << ui->pb_position;

   m_cmdList << CMD_METADATA_METALINE1 << CMD_METADATA_METALINE2
             << CMD_METADATA_LINE1 << CMD_METADATA_TOP_BOTTOM;

   connectWidgetSigs();
}

void metaData::toggleValue(int cmd, int idx, int flag)
{
  UNUSED(idx);
  UNUSED(flag);
  
   switch (cmd)
   {
      default:
//       baseMenu::toggleValue(cmd, idx, 1);
       break;
   }
}

void metaData::on_pb_line_clicked()
{
   if (ui->pb_line->text() == (QString)"LINE 1")
   {  // Switch to LINE 2
      ui->pb_line->setText("LINE 2");
      mLine1 = false;
      ui->le_metaLine1->setEnabled(false);
      ui->le_metaLine2->setEnabled(true);
   }
   else
   {  // Switch to LINE 1
      ui->pb_line->setText("LINE 1");
      mLine1 = true;
      ui->le_metaLine1->setEnabled(true);
      ui->le_metaLine2->setEnabled(false);
   }
}

void metaData::on_cb_deviceID_stateChanged(int arg1)
{
   if (mInitialized == false)
      return;

   if (arg1 == Qt::Checked)
   {  // Just Checked
      if (mLine1 == true)
      {  // Line 1
         mAdmin.metaData1 = ui->le_metaLine1->text();
         mAdmin.metaData1 += "%0 ";
         ui->le_metaLine1->setText(mAdmin.metaData1);
      }
      else
      {  // Line 2
         mAdmin.metaData2 = ui->le_metaLine2->text();
         mAdmin.metaData2 += "%0 ";
         ui->le_metaLine2->setText(mAdmin.metaData2);
      }
   }
   else
   {  // Just Unchecked
      mAdmin.metaData1 = ui->le_metaLine1->text();
      mAdmin.metaData2 = ui->le_metaLine2->text();
      mAdmin.metaData1.remove("%0 ", Qt::CaseSensitive);
      mAdmin.metaData2.remove("%0 ", Qt::CaseSensitive);
      ui->le_metaLine1->setText(mAdmin.metaData1);
      ui->le_metaLine2->setText(mAdmin.metaData2);
   }
}

void metaData::on_cb_userID_stateChanged(int arg1)
{
   if (mInitialized == false)
      return;

   if (arg1 == Qt::Checked)
   {  // Just Checked
      if (mLine1 == true)
      {  // Line 1
         mAdmin.metaData1 = ui->le_metaLine1->text();
         mAdmin.metaData1 += "%1 ";
         ui->le_metaLine1->setText(mAdmin.metaData1);
      }
      else
      {  // Line 2
         mAdmin.metaData2 = ui->le_metaLine2->text();
         mAdmin.metaData2 += "%1 ";
         ui->le_metaLine2->setText(mAdmin.metaData2);
      }
   }
   else
   {  // Just Unchecked
      mAdmin.metaData1 = ui->le_metaLine1->text();
      mAdmin.metaData2 = ui->le_metaLine2->text();
      mAdmin.metaData1.remove("%1 ", Qt::CaseSensitive);
      mAdmin.metaData2.remove("%1 ", Qt::CaseSensitive);
      ui->le_metaLine1->setText(mAdmin.metaData1);
      ui->le_metaLine2->setText(mAdmin.metaData2);
   }
}

void metaData::on_cb_recordNumber_stateChanged(int arg1)
{
   if (mInitialized == false)
      return;

   if (arg1 == Qt::Checked)
   {  // Just Checked
      if (mLine1 == true)
      {  // Line 1
         mAdmin.metaData1 = ui->le_metaLine1->text();
         mAdmin.metaData1 += "%2 ";
         ui->le_metaLine1->setText(mAdmin.metaData1);
      }
      else
      {  // Line 2
         mAdmin.metaData2 = ui->le_metaLine2->text();
         mAdmin.metaData2 += "%2 ";
         ui->le_metaLine2->setText(mAdmin.metaData2);
      }
   }
   else
   {  // Just Unchecked
      mAdmin.metaData1 = ui->le_metaLine1->text();
      mAdmin.metaData2 = ui->le_metaLine2->text();
      mAdmin.metaData1.remove("%2 ", Qt::CaseSensitive);
      mAdmin.metaData2.remove("%2 ", Qt::CaseSensitive);
      ui->le_metaLine1->setText(mAdmin.metaData1);
      ui->le_metaLine2->setText(mAdmin.metaData2);
   }
}

void metaData::on_cb_speedLimit_stateChanged(int arg1)
{
   if (mInitialized == false)
      return;

   if (arg1 == Qt::Checked)
   {  // Just Checked
      if (mLine1 == true)
      {  // Line 1
         mAdmin.metaData1 = ui->le_metaLine1->text();
         mAdmin.metaData1 += "%3 ";
         ui->le_metaLine1->setText(mAdmin.metaData1);
      }
      else
      {  // Line 2
         mAdmin.metaData2 = ui->le_metaLine2->text();
         mAdmin.metaData2 += "%3 ";
         ui->le_metaLine2->setText(mAdmin.metaData2);
      }
   }
   else
   {  // Just Unchecked
      mAdmin.metaData1 = ui->le_metaLine1->text();
      mAdmin.metaData2 = ui->le_metaLine2->text();
      mAdmin.metaData1.remove("%3 ", Qt::CaseSensitive);
      mAdmin.metaData2.remove("%3 ", Qt::CaseSensitive);
      ui->le_metaLine1->setText(mAdmin.metaData1);
      ui->le_metaLine2->setText(mAdmin.metaData2);
   }
}

void metaData::on_cb_speed_stateChanged(int arg1)
{
   if (mInitialized == false)
      return;

   if (arg1 == Qt::Checked)
   {  // Just Checked
      if (mLine1 == true)
      {  // Line 1
         mAdmin.metaData1 = ui->le_metaLine1->text();
         mAdmin.metaData1 += "%4 ";
         ui->le_metaLine1->setText(mAdmin.metaData1);
      }
      else
      {  // Line 2
         mAdmin.metaData2 = ui->le_metaLine2->text();
         mAdmin.metaData2 += "%4 ";
         ui->le_metaLine2->setText(mAdmin.metaData2);
      }
   }
   else
   {  // Just Unchecked
      mAdmin.metaData1 = ui->le_metaLine1->text();
      mAdmin.metaData2 = ui->le_metaLine2->text();
      mAdmin.metaData1.remove("%4 ", Qt::CaseSensitive);
      mAdmin.metaData2.remove("%4 ", Qt::CaseSensitive);
      ui->le_metaLine1->setText(mAdmin.metaData1);
      ui->le_metaLine2->setText(mAdmin.metaData2);
   }
}

void metaData::on_cb_distance_stateChanged(int arg1)
{
   if (mInitialized == false)
      return;

   if (arg1 == Qt::Checked)
   {  // Just Checked
      if (mLine1 == true)
      {  // Line 1
         mAdmin.metaData1 = ui->le_metaLine1->text();
         mAdmin.metaData1 += "%5 ";
         ui->le_metaLine1->setText(mAdmin.metaData1);
      }
      else
      {  // Line 2
         mAdmin.metaData2 = ui->le_metaLine2->text();
         mAdmin.metaData2 += "%5 ";
         ui->le_metaLine2->setText(mAdmin.metaData2);
      }
   }
   else
   {  // Just Unchecked
      mAdmin.metaData1 = ui->le_metaLine1->text();
      mAdmin.metaData2 = ui->le_metaLine2->text();
      mAdmin.metaData1.remove("%5 ", Qt::CaseSensitive);
      mAdmin.metaData2.remove("%5 ", Qt::CaseSensitive);
      ui->le_metaLine1->setText(mAdmin.metaData1);
      ui->le_metaLine2->setText(mAdmin.metaData2);
   }

}

void metaData::on_cb_location_stateChanged(int arg1)
{
   if (mInitialized == false)
      return;

   if (arg1 == Qt::Checked)
   {  // Just Checked
      if (mLine1 == true)
      {  // Line 1
         mAdmin.metaData1 = ui->le_metaLine1->text();
         mAdmin.metaData1 += "%6 ";
         ui->le_metaLine1->setText(mAdmin.metaData1);
      }
      else
      {  // Line 2
         mAdmin.metaData2 = ui->le_metaLine2->text();
         mAdmin.metaData2 += "%6 ";
         ui->le_metaLine2->setText(mAdmin.metaData2);
      }
   }
   else
   {  // Just Unchecked
      mAdmin.metaData1 = ui->le_metaLine1->text();
      mAdmin.metaData2 = ui->le_metaLine2->text();
      mAdmin.metaData1.remove("%6 ", Qt::CaseSensitive);
      mAdmin.metaData2.remove("%6 ", Qt::CaseSensitive);
      ui->le_metaLine1->setText(mAdmin.metaData1);
      ui->le_metaLine2->setText(mAdmin.metaData2);
   }

}

void metaData::on_pb_position_clicked()
{
   if (ui->pb_position->text() == (QString)"SCREEN BOTTOM")
   {  // Switch to TOP
      ui->pb_position->setText("SCREEN TOP");
      mPosition = WATERMARK_TOPLEFT;
   }
   else
   {  // Switch to BOTTOM
      ui->pb_position->setText("SCREEN BOTTOM");
      mPosition = WATERMARK_BOTTOMLEFT;
   }
}

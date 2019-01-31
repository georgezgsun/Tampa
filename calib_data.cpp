#include <QMessageBox>
#include <QCalendarWidget>
#include "calib_data.h"
#include "ui_calib_data.h"
#include "state.h"
#include "utils.h"
#include "hardButtons.h"
#include "ColdFireCommands.h"
#include "vkiline.h"

calibData::calibData(int type, QWidget *parent) :
   baseMenu(parent),
   ui(new Ui::calibData),
   m_menuType (type)
{
   ui->setupUi(this);

   setInittoggleValues();
   connect(ui->le_authority, SIGNAL(linePressed(vkILine*)), this, SLOT(vkeyboard_clicked(vkILine*)));
   connect(ui->le_certification, SIGNAL(linePressed(vkILine*)), this, SLOT(vkeyboard_clicked(vkILine*)));
}

calibData::~calibData()
{
   delete ui;
}

void calibData::setInittoggleValues()
{
   mAdmin = Utils::get().getAdmin();

   ui->le_authority->setText(mAdmin.authority);
   ui->le_certification->setText(mAdmin.certification);

   if (mAdmin.dateFormat)
   {   // dd-MM-yyyy format
      ui->de_expiration->setDisplayFormat("dd-MM-yyyy");
      ui->de_date->setDisplayFormat("dd-MM-yyyy");
   }
   DisplayDate(mAdmin.expiration, ui->de_expiration);
   DisplayDate(mAdmin.serviceDate, ui->de_date);
#ifdef LIDARCAM
   // Map Hard Button
   hardButtons& hd = hardButtons::get();
   hd.setHardButtonMap( 0, ui->pb_exit);
   hd.setHardButtonMap( 1, NULL);
   hd.setHardButtonMap( 2, NULL);
   hd.setHardButtonMap( 3, NULL);
#endif
   mKeyInputs.setPlaceholderText("Enter Data");
   ui->de_expiration->calendarWidget()->setFirstDayOfWeek(Qt::Sunday);
   ui->de_date->calendarWidget()->setFirstDayOfWeek(Qt::Sunday);

#ifdef HH1
   ui->label_2->setText("CERTIFICATION MENU");
#endif
}

void calibData::DisplayDate(QString &date1,  QDateEdit *pQd)
{
    // Note: In SQL database, the data is always saved as 'YYYY-MM-DD'
   QByteArray ba = date1.toLatin1();
   char *ptr, *str1 = ba.data();
   int year, month, day;
   char buf1[16] = "";
   bool flag1 = false;

   ptr = strtok(str1, "-");
   if (ptr != NULL)
   {
      sscanf(ptr, "%d", &year);
      ptr = strtok(NULL, "-");
      if (ptr != NULL)
      {
         sscanf(ptr, "%d", &month);
         ptr = strtok(NULL, "-");
         if (ptr != NULL)
         {
            sscanf(ptr, "%d", &day);
            sprintf(buf1, "%02d/%02d/%d", day, month, year);
            flag1 = true;
         }
      }
   }
   QDate qd;
   if (flag1 == true)  // Get the correct date from the database?
      qd = QDate::fromString((QString)buf1,"dd/MM/yyyy");
   else
      qd = QDate::currentDate();  // Use current date
   pQd->setDate(qd);
}

void calibData::openVKB(vkILine *l)
{
   toggleVKB(l);
   focusLine(l);
   connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(hideVKB()));
}

void calibData::closeVKB()
{
   Q_ASSERT(m_vkb);
   disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(hideVKB()));
   if (m_vkb->isVisible())
   {
      m_vkb->hide(true);
   }
}

void calibData::toggleVKB(vkILine *l)
{
   Q_ASSERT(m_vkb);

   if (m_vkb)
   {
      if (m_vkb->isVisible())
      {
         setFocus();
         m_vkb->hide(true);
      }
      else
      {
         m_vkb->show(l, focusWidget());
         m_vkb->move(x(), y());
      }
   }
   else
   {
      qDebug() << "virtual keyboard is not created";
   }
}

void calibData::hideVKB()
{
   if (m_command == CMD_CALIB_AUTHORITY)
      ui->le_authority->setText(mKeyInputs.text());
   else
      ui->le_certification->setText(mKeyInputs.text());
   closeVKB();
}

void calibData::focusLine(vkILine *l)
{
   Q_ASSERT(m_vkb);

   if (m_vkb->isVisible() == false)
      return;

   if (m_vkb->currentTextBox() == l)
      return;
}

void calibData::vkeyboard_clicked(vkILine* vkl)
{
   Utils& u = Utils::get();
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   if (!m_vkb)
   {
      m_vkb = u.vkb();
   }
   else
   {
      if (m_vkb->isVisible())
         return;
   }
   openVKB(vkl);
   mKeyInputs.setText("");
   if (ui->le_authority == vkl)
   {
      m_command = CMD_CALIB_AUTHORITY;
      mKeyInputs.setText(ui->le_authority->text());
   }
   else // if (ui->le_certification == vkl)
   {
      m_command = CMD_CALIB_CERT;
      mKeyInputs.setText(ui->le_certification->text());
   }
   m_vkb->show(&mKeyInputs, this);
   m_vkb->setActiveForms(&mKeyInputs, this);
}

// This method will convert the date to the correct format to be saved in database
void calibData::ConvertDate(QDate &da1, QString &outDate)
{
   int year = da1.year(), month = da1.month(), day = da1.day();
   // Note: In SQL database, the data is always saved as 'YYYY-MM-DD'
   char buf1[16];
   sprintf(buf1, "%d-%02d-%02d", year, month, day);
   outDate = QString((QString)buf1);
}

void calibData::on_pb_exit_clicked()
{
   Utils& u = Utils::get();
   hardButtons& hd = hardButtons::get();
   hd.setHardButtonMap( 0, NULL);   // Disconnect 'exit' hardbutton
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

   bool changed = false;

   if(mAdmin.expiration != ui->de_expiration->text())
   {
      QString date1 = ui->de_expiration->text();
      if(mAdmin.dateFormat)
      {
         QDate da1 = ui->de_expiration->date();
         ConvertDate(da1, date1);
      }
      mAdmin.expiration = date1;
      changed = true;

   }
   if(mAdmin.authority != ui->le_authority->text())
   {
      mAdmin.authority = ui->le_authority->text();
      changed = true;
   }
   if(mAdmin.certification != ui->le_certification->text())
   {
      mAdmin.certification = ui->le_certification->text();
      changed = true;
   }
   if(mAdmin.serviceDate != ui->de_date->text())
   {
      QString date1 = ui->de_date->text();
      if(mAdmin.dateFormat)
      {
         QDate da1 = ui->de_date->date();
         ConvertDate(da1, date1);
      }
      mAdmin.serviceDate = date1;
      changed = true;
   }

   //Is anything changed?
   if (changed == true)
   {   // Yes
      u.setAdmin(mAdmin);
   }

   close();
}


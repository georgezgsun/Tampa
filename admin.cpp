#include <QMessageBox>
#include "admin.h"
#include "ui_admin.h"
#include "state.h"
#include "utils.h"
#include "ColdFireCommands.h"
#include "debug.h"

admin::admin(QWidget *parent) :
   baseMenu(parent),
   ui(new Ui::admin)
{
   ui->setupUi(this);

   initLists();

	state& v = state::get();
   v.setState(STATE_SUB_MENU2);
   m_command = m_cmdList.at(m_listIndex);
}

admin::~admin()
{
   delete ui;
}

void admin::initLists()
{
   m_list << ui->pb_saveSettings
          << ui->pb_loadSettings
          << ui->pb_logFile
          << ui->pb_security
          << ui->pb_userAccess
#ifndef HH1
          << ui->pb_metaData
#endif
          << ui->pb_upgradeSW
          << ui->pb_service
          << ui->pb_factory;

   m_cmdList << CMD_SAVE_SETTINGS
             << CMD_LOAD_SETTINGS
             << CMD_LOG_FILE
             << CMD_SECURITY_OPTIONS
             << CMD_USER_ACCESS
#ifndef HH1
             << CMD_METADATA
#endif
             << CMD_UPGRADE_SW
             << CMD_SERVICE
             << CMD_FACTORY;

   connectWidgetSigs();

#ifdef HH1
    ui->gridLayout->removeWidget(ui->pb_metaData);
    ui->pb_metaData->setVisible(false);
    ui->gridLayout->removeWidget(ui->pb_upgradeSW);
    ui->gridLayout->addWidget(ui->pb_upgradeSW, 1, 2);
#else
    // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
    ui->pb_logFile->setEnabled(false);
    ui->pb_factory->setEnabled(false);
    ui->pb_userAccess->setEnabled(false);
#endif
}

QPushButton *admin::getButton(int cmdIndex)
{
   switch (cmdIndex)
   {
      case CMD_SERVICE:
         return ui->pb_service;
      case CMD_FACTORY:
         return ui->pb_factory;
   }
   return NULL;
}

#define LOC_ENTRY_FLAGS    0X01F    // 5 entries
#define USR_ENTRY_FLAGS    0X03F    // 6 entries

void admin::on_pb_loadSettings_clicked()
{
   Utils& u = Utils::get();
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   QMessageBox msgBox;
   msgBox.setText("This operation will overwrite all locations and user information. Are you sure?");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   msgBox.setDefaultButton(QMessageBox::No);
   msgBox.setIcon(QMessageBox::Question);
   QPalette p;
   p.setColor(QPalette::Window, Qt::red);
   msgBox.setPalette(p);

   if(msgBox.exec() != QMessageBox::Yes)
      return;

#ifdef LIDARCAM
   FILE *fp1 = fopen("/usr/local/stalker/admin.cfg", "rb");
#else
   FILE *fp1 = fopen("./admin.cfg", "rb");
#endif
   if (fp1 == NULL)
   {
      qCritical() << "Cannot open location and user configuration file!";
      return;
   }

   char str[128];
   bool locStatus = false, usrStatus = false;
   bool locClear = false, usrClear = false;
   int index1, row = 0, entryNum, retv;
   Location loc1;
   Users user1;
   userDB *pDB = u.db();

   while (fgets(str, sizeof(str), fp1))
   {
      if (str[0] == '#' || str[0] == '\n')   // Comment or blank line
         continue;

      if (!strncmp(str, "$Location", strlen("$Location")))
      {  // Location section
         locStatus = true; // In 'Location' section
         usrStatus = false;
         entryNum = 0;  // No entry yet
         if (locClear == false)
         {  // Delete all location entries first
            for (index1 = 1; index1 <= MAX_LOCATION_ENTRIES; index1++)
            {
               loc1.index = index1;
               u.db()->deleteEntry(TBL_LOCATION, (DBStruct *)&loc1);
            }
            locClear = true;  // Do this only once
         }
         continue;
      }
      else if (!strncmp(str, "$Users", strlen("$Users")))
      {  // Users section
         locStatus = false;
         usrStatus = true; // In 'User' section
         entryNum = 0;  // No entry yet
         if (usrClear == false)
         {  // Delete all user entries first
            int ct = pDB->queryEntry(TBL_USERS, (DBStruct *)&user1, QRY_ALL_ENTRIES);
            //    DEBUG() << "total Users " << ct;

            ct = (ct > MAX_LOCATION_ENTRIES) ? MAX_LOCATION_ENTRIES : ct;
            Users usrs[ct];
            for (index1 = 0; index1 < ct; index1++)
            {
               retv = pDB->getNextEntry(TBL_USERS, (DBStruct *)&usrs[index1]);
            }
            // Skip 'admin' user and delete all others
            for (index1 = 1; index1 < ct; index1++)
               pDB->deleteEntry(TBL_USERS, (DBStruct *)&usrs[index1]);
            usrClear = true;  // Do this only once
         }
         continue;
      }

      if (locStatus == true)
      {  // Location section
         index1 = strlen(str) - 1;
         if (str[index1] == '\n')
            str[index1] = '\0';  // Remove '\n' at the end of string
         if (!strncmp(str, "Address:", strlen("Address:")))
         {
            index1 = strlen("Address:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            loc1.description = QString::fromUtf8(&str[index1]);
            entryNum |= 1;  // Got 'Address' entry
         }
         if (!strncmp(str, "SpeedLimit:", strlen("SpeedLimit:")))
         {
            index1 = strlen("SpeedLimit:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            loc1.speedLimit = QString::fromUtf8(&str[index1]);
            if (entryNum)  // Have we got 'Address' entry yet?
               entryNum |= 0x02;
         }
         if (!strncmp(str, "CaptureSpeed:", strlen("CaptureSpeed:")))
         {
            index1 = strlen("CaptureSpeed:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            loc1.captureSpeed = QString::fromUtf8(&str[index1]);
            if (entryNum)
               entryNum |= 0x04;
         }
         if (!strncmp(str, "RoadCondition:", strlen("RoadCondition:")))
         {
            index1 = strlen("RoadCondition:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            loc1.roadCondition = QString::fromUtf8(&str[index1]);
            if (entryNum)
               entryNum |= 0x08;
         }
         if (!strncmp(str, "NumberLanes:", strlen("NumberLanes:")))
         {
            index1 = strlen("NumberLanes:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            if (entryNum && sscanf(&str[index1], "%d", &loc1.numberLanes) == 1)
               entryNum |= 0x10;
         }

         if (entryNum == LOC_ENTRY_FLAGS)
         {  // Got all data for this location
            //update database
            loc1.index = row + 1;
            row++;
            retv = u.db()->addEntry(TBL_LOCATION, (DBStruct *)&loc1);
            if (retv)
               qCritical() << "Error: Create location" << loc1.index << "(" << loc1.description << ")";
            entryNum = 0;  // Done with this location
         }
      }  // locStatus

      if (usrStatus == true)
      {  // User section
         index1 = strlen(str) - 1;
         if (str[index1] == '\n')
            str[index1] = '\0';  // Remove '\n' at the end of string
         if (!strncmp(str, "FirstName:", strlen("FirstName:")))
         {
            index1 = strlen("FirstName:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            user1.firstName = QString::fromUtf8(&str[index1]);
            entryNum |= 1;  // Got 'FirstName' entry
         }
         if (!strncmp(str, "LastName:", strlen("LastName:")))
         {
            index1 = strlen("LastName:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            user1.lastName = QString::fromUtf8(&str[index1]);
            entryNum |= 0x02;  // Got 'LastName' entry
         }
         if (!strncmp(str, "BadgeNum:", strlen("BadgeNum:")))
         {
            index1 = strlen("BadgeNum:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            user1.bagdeNumber = QString::fromUtf8(&str[index1]);
            entryNum |= 0x04;  // Got 'BadgeNum' entry
         }
         if (!strncmp(str, "UserLevel:", strlen("UserLevel:")))
         {
            index1 = strlen("UserLevel:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            user1.userLevel = QString::fromUtf8(&str[index1]);
            entryNum |= 0x08;  // Got 'UserLevel' entry
         }
         if (!strncmp(str, "UserName:", strlen("UserName:")))
         {
            index1 = strlen("UserName:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            user1.loginName = QString::fromUtf8(&str[index1]);
            entryNum |= 0x10;  // Got 'UserName' entry
         }
         if (!strncmp(str, "Password:", strlen("Password:")))
         {
            index1 = strlen("Password:");
            while (str[index1] == ' ' || str[index1] == '\t')
               index1++;
            user1.password = QString::fromUtf8(&str[index1]);
            entryNum |= 0x20;  // Got 'Password' entry
         }

         if (entryNum == USR_ENTRY_FLAGS)
         {  // Got all data for this user
            int retv = pDB->queryEntry(TBL_USERS, (DBStruct *)&user1, QRY_BY_KEY);
            //update database
            if (!retv)  // If this entry does not exist yet
               retv = pDB->addEntry(TBL_USERS, (DBStruct *)&user1);
            if (retv)
               qCritical() << "Error: Create User" << user1.firstName << " " << user1.lastName;
            entryNum = 0;  // Done with this User
         }
      }  // usrStatus
   }
   fclose(fp1);
}

void admin::on_pb_saveSettings_clicked()
{
   Utils& uu = Utils::get();
   uu.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   QMessageBox msgBox;
   msgBox.setText("This operation will export all locations and user information. Are you sure?");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   msgBox.setDefaultButton(QMessageBox::No);
   msgBox.setIcon(QMessageBox::Question);
   QPalette p;
   p.setColor(QPalette::Window, Qt::red);
   msgBox.setPalette(p);

   if(msgBox.exec() != QMessageBox::Yes)
      return;

   userDB *pDB;
   pDB = uu.db();
   int retv, ct, i, locNum, userNum;

   // Export Locations first
   Location l, locs[MAX_LOCATION_ENTRIES];

   ct = pDB->queryEntry(TBL_LOCATION, (DBStruct *)&l, QRY_ALL_ENTRIES);
   //    DEBUG() << "total locations " << ct;

   for (i = 0, locNum = 0; i < ct; i++)
   {
      retv = pDB->getNextEntry(TBL_LOCATION, (DBStruct *)&l);
      if (!retv && l.index <= MAX_LOCATION_ENTRIES)
      {
//         locs[locNum].index = l.index;
         locs[locNum].description = l.description;
         locs[locNum].speedLimit = l.speedLimit;
         locs[locNum].captureSpeed = l.captureSpeed;
         locs[locNum].roadCondition = l.roadCondition;
         locs[locNum].numberLanes = l.numberLanes;
         locNum++;
      }
   }

   // Export Users
   Users u;

   ct = pDB->queryEntry(TBL_USERS, (DBStruct *)&u, QRY_ALL_ENTRIES);
   //    DEBUG() << "total Users " << ct;

   ct = (ct > MAX_LOCATION_ENTRIES) ? MAX_LOCATION_ENTRIES : ct;
   Users usrs[ct];
   for (i = 0, userNum = 0; i < ct; i++)
   {
      retv = pDB->getNextEntry(TBL_USERS, (DBStruct *)&u);
      if (!retv)
      {
         usrs[userNum].loginName = u.loginName;
         usrs[userNum].password = u.password;
         usrs[userNum].firstName = u.firstName;
         usrs[userNum].lastName = u.lastName;
         usrs[userNum].bagdeNumber = u.bagdeNumber;
         usrs[userNum].userLevel = u.userLevel;
         userNum++;
      }
   }

#ifdef LIDARCAM
   FILE *fp1 = fopen("/usr/local/stalker/admin.cfg", "wb");
#else
   FILE *fp1 = fopen("./admin.cfg", "wb");
#endif
   if (fp1 == NULL)
      return;

   // Write Locations to file
   ct = locNum;  // real legal entries for location
   fprintf(fp1, "#any line that starts with # is comments\n");
   fprintf(fp1, "#any line that starts with $ is Section Keyword\n\n");
   fprintf(fp1, "$Location\n\n");   // Keyword
   fprintf(fp1, "#Address must be the 1st entry for a location\n");
   fprintf(fp1, "#RoadCondition: NORMAL, RAIN, SLEET, SNOW, FOG, ICE\n\n");
   for (i = 0; i < ct; i++)
   {
      fprintf(fp1, "Address:\t%s\n", locs[i].description.toLatin1().data());
      fprintf(fp1, "SpeedLimit:\t%s\n", locs[i].speedLimit.toLatin1().data());
      fprintf(fp1, "CaptureSpeed:\t%s\n", locs[i].captureSpeed.toLatin1().data());
      fprintf(fp1, "RoadCondition:\t%s\n", locs[i].roadCondition.toLatin1().data());
      fprintf(fp1, "NumberLanes:\t%d\n\n", locs[i].numberLanes);
   }

   // Write Users to file
   ct = userNum;
   fprintf(fp1, "\n\n$Users\n\n");
   // Skip i = 0 since that is 'Admin'
   for (i = 1; i < ct; i++)
   {
      fprintf(fp1, "FirstName:\t%s\n", usrs[i].firstName.toLatin1().data());
      fprintf(fp1, "LastName:\t%s\n", usrs[i].lastName.toLatin1().data());
      fprintf(fp1, "BadgeNum:\t%s\n", usrs[i].bagdeNumber.toLatin1().data());
      fprintf(fp1, "UserLevel:\t%s\n", usrs[i].userLevel.toLatin1().data());
      fprintf(fp1, "UserName:\t%s\n", usrs[i].loginName.toLatin1().data());
      fprintf(fp1, "Password:\t%s\n\n", usrs[i].password.toLatin1().data());
   }

   fclose(fp1);
}

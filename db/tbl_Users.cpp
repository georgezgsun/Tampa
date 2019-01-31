#include "user_db.h"

int userDB::Users_createTbl()
{
   QString s =  "CREATE TABLE Users "
                "(LoginName TEXT PRIMARY KEY, "
                "FirstName TEXT NOT NULL, "
                "LastName TEXT NOT NULL, "
                "BadgeNumber NUMERIC, "
                "UserLevel NUMERIC NOT NULL, "
                "PassWord TEXT NOT NULL)";

   QSqlQuery query(m_db);
   if (!query.exec(s))
   {
      qDebug() << "Create table Users failed: number";
      return (-ERR_CREATE_TBL);
   }
   return 0;
}

int userDB::Users_addEntry(Users *u)
{
   m_query->prepare("INSERT INTO Users "
                    "(LoginName,  FirstName,  LastName,  BadgeNumber,  UserLevel,  PassWord) "
            "VALUES (:LoginName, :FirstName, :LastName, :BadgeNumber, :UserLevel, :PassWord)");
   if (!u)
   {
      if (m_dbOpened)
         return 0;       //recreated admin is not allowed, return sillently

      // add user admin
      m_query->bindValue(":LoginName", "admin");
      m_query->bindValue(":FirstName", "Admin");
      m_query->bindValue(":LastName", "Administrator");
      m_query->bindValue(":BadgeNumber", 12345678);
      m_query->bindValue(":UserLevel", 101);
      m_query->bindValue(":PassWord", ADMINISTRATION_PASSWORD);
      if (!m_query->exec())
      {
         qDebug() << m_query->lastError();
         qFatal("insert User admin failed");
      }

      //test code
      m_query->bindValue(":LoginName", "jsmith");
      m_query->bindValue(":FirstName", "John");
      m_query->bindValue(":LastName", "Smith");
      m_query->bindValue(":BadgeNumber", 34567890);
      m_query->bindValue(":UserLevel", 501);
      m_query->bindValue(":PassWord", "user-1");
      if (!m_query->exec())
      {
         qDebug() << m_query->lastError();
         qCritical() << "ERROR: Insert User: jsmith";
      }

      return 0;
   }

  // struct Users *u = (struct Users *)user;
   m_query->bindValue(":LoginName", u->loginName);
   m_query->bindValue(":FirstName", u->firstName);
   m_query->bindValue(":LastName", u->lastName);
   m_query->bindValue(":BadgeNumber", u->bagdeNumber.toInt());
   m_query->bindValue(":UserLevel", u->userLevel.toInt());
   m_query->bindValue(":PassWord", u->password);
   if (!m_query->exec())
   {
      qDebug() << m_query->lastError();
      qCritical() << "ERROR: Insert User: " << u->loginName;
      return (-ERR_ADD_ENTRY);
   }

   return 0;
}

int userDB::Users_delEntry(Users *u, DB_QUERY_FLAG flag)
{
   if (flag == QRY_BY_KEY)
   {
      m_query->prepare("DELETE FROM Users WHERE LoginName = ?");
      m_query->addBindValue(u->loginName);
      if (!m_query->exec())
      {
         qDebug() << m_query->lastError();
         qCritical() << "ERROR: delete User: " << u->loginName;
         return (-ERR_DEL_ENTRY);
      }
   }
   else
      qDebug() << "delete User by multi-field is not supported";

    return 0;
}

#define USERS_LOGIN_NAME    0x1
#define USERS_FIRST_NAME    0x2
#define USERS_LAST_NAME     0x4
#define USERS_BADGE_NUM     0x8
#define USERS_USER_LEVL     0x10
#define USERS_PASSWORD      0x20

int userDB::Users_query(Users *u, DB_QUERY_FLAG flag)
{
   int fields = 0;
   QString s = "SELECT * FROM Users";

   if (flag == QRY_ALL_ENTRIES)
      goto exec;
   else
      s = s + " WHERE";

   if (flag == QRY_BY_KEY)
   {
      s = s + " LoginName = (:LoginName)";
      fields |= USERS_LOGIN_NAME;
   }
   else if (flag == QRY_BY_MULTI_FIELDS)
   {
      if (u->loginName.isEmpty() == false)
      {
         s = s + " LoginName = (:LoginName)";
         fields |= USERS_LOGIN_NAME;
      }
      if (u->firstName.isEmpty() == false)
      {
         if (fields)
            s = s + " AND FirstName = (:FirstName)";
         else
            s = s + " FirstName = (:FirstName)";
         fields |= USERS_FIRST_NAME;
      }
      if (u->lastName.isEmpty() == false)
      {
         if (fields)
            s = s + " AND LastName = (:LastName)";
         else
            s = s + " LastName = (:LastName)";
         fields |= USERS_LAST_NAME;
      }
      if (u->bagdeNumber.isEmpty() == false)
      {
         if (fields)
            s = s + " AND BadgeNumber = (:BadgeNumber)";
         else
            s = s + " BadgeNumber = (:BadgeNumber)";
         fields |= USERS_BADGE_NUM;
      }
      if (u->userLevel.isEmpty() == false)
      {
         if (fields)
            s = s + " AND UserLevel = (:UserLevel)";
         else
            s = s + " UserLevel = (:UserLevel)";
         fields |= USERS_USER_LEVL;
      }
      if (u->password.isEmpty() == false)
      {
         if (fields)
            s = s + " AND PassWord = (:PassWord)";
         else
            s = s + " PassWord = (:PassWord)";
         fields |= USERS_PASSWORD;
      }
   }
   else //ERROR
      qDebug() << "Invalid Users query flag " << flag;

exec:
   m_query->prepare(s);
   if (fields)
   {
      if (fields & USERS_LOGIN_NAME)
         m_query->bindValue(":LoginName", u->loginName);
      if (fields & USERS_FIRST_NAME)
         m_query->bindValue(":FirstName", u->firstName);
      if (fields & USERS_LAST_NAME)
         m_query->bindValue(":LastName", u->lastName);
      if (fields & USERS_BADGE_NUM)
         m_query->bindValue(":BadgeNumber", u->bagdeNumber.toInt());
      if (fields & USERS_USER_LEVL)
         m_query->bindValue(":UserLevel", u->userLevel.toInt());
      if (fields & USERS_PASSWORD)
         m_query->bindValue(":PassWord", u->password);
   }

   if (!m_query->exec())
   {
      qDebug() << "query Users failed "<< m_query->lastError();
      return (-ERR_QUERY_EXEC);
   }

   int count = 0;
   if (m_query->driver()->hasFeature(QSqlDriver::QuerySize))
       count = m_query->size();
   else
   {
      bool b = m_query->last();
      if (b)
         count = m_query->at() + 1;
   }

   return count;
}

int userDB::Users_getNextEntry(Users *u)
{
   bool retv;

   if (m_firstEntry)
      retv = m_query->next();
   else
   {
      retv = m_query->first();
      m_firstEntry = 1;
   }

   if (retv)
   {
      // qDebug() << "next return " << retv;
      u->loginName = m_query->value(m_query->record().indexOf("LoginName")).toString();
      u->password = m_query->value(m_query->record().indexOf("PassWord")).toString();
      u->firstName = m_query->value(m_query->record().indexOf("FirstName")).toString();
      u->lastName = m_query->value(m_query->record().indexOf("LastName")).toString();
      u->bagdeNumber = m_query->value(m_query->record().indexOf("BadgeNumber")).toString();
      u->userLevel = m_query->value(m_query->record().indexOf("UserLevel")).toString();
   }
   else
   {
      qDebug() << "query get Users entry failed "<< m_query->lastError();
      return -ERR_QUERY_ENTRY;
   }

    return 0;
}

int userDB::Users_updateEntry(Users *u, DB_QUERY_FLAG flag)
{
   int retv = 0;

   if (flag == QRY_BY_KEY)
   {
      QString s = "UPDATE Users SET FirstName = (:FirstName),";
              s = s + " LastName = (:LastName),";
              s = s + " BadgeNumber = (:BadgeNumber),";
              s = s + " PassWord = (:PassWord),";
              s = s + " UserLevel = (:UserLevel)";
              s = s + " WHERE LoginName = (:LoginName)";

      if (m_query->prepare(s))
      {
         m_query->bindValue(":FirstName", u->firstName);
         m_query->bindValue(":LastName", u->lastName);
         m_query->bindValue(":BadgeNumber", u->bagdeNumber.toInt());
         m_query->bindValue(":PassWord", u->password);
         m_query->bindValue(":UserLevel", u->userLevel.toInt());
         m_query->bindValue(":LoginName", u->loginName);

         if (!m_query->exec())
            retv = -ERR_UPDATE_EXEC;
      }
      else
         retv = -ERR_UPDATE_PREPARE;
   }
   // else    //not supported

   return retv;
}


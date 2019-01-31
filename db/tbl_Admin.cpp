#include "user_db.h"

int userDB::Admin_createTbl()
{
    QString s = "CREATE TABLE Administration "
                "(AdminIndex NUMERIC PRIMARY KEY, "
                "UserLogin NUMERIC, "
                "Compression NUMERIC, "
                "AutoDelete NUMERIC, "
                "Encryption NUMERIC, "
                "Password TEXT NOT NULL, "
                "DateFormat NUMERIC, "
                "Language NUMERIC, "
                "Expiration TEXT, "
                "Authority TEXT, "
                "Certification TEXT, "
                "ServiceDate TEXT, "
                "MetaData1 TEXT, "
                "MetaData2 TEXT, "
                "MetaPosition NUMERIC, "
                "UsrAccAdmin NUMERIC, "
                "UsrAccTransfer NUMERIC, "
                "UsrAccDelete NUMERIC)";


    QSqlQuery query(m_db);
    if (!query.exec(s))
    {
        qDebug() << "Create table Administration failed";
        qDebug() << query.lastError();
        return (-ERR_CREATE_TBL);
    }
    return 0;
}

int userDB::Admin_addEntry(Administration *l)
{
    m_query->prepare("INSERT INTO Administration "
                     "(AdminIndex,  UserLogin,  Compression,  AutoDelete,  Encryption,  Password,  DateFormat,  Language,  Expiration, "
                     " Authority,  Certification,  ServiceDate,  MetaData1,  MetaData2,  MetaPosition, UsrAccAdmin,  UsrAccTransfer,  UsrAccDelete)"
             "VALUES (:AdminIndex, :UserLogin, :Compression, :AutoDelete, :Encryption, :Password, :DateFormat, :Language, :Expiration, "
                     ":Authority, :Certification, :ServiceDate, :MetaData1, :MetaData2, :MetaPosition, :UsrAccAdmin, :UsrAccTransfer, :UsrAccDelete)");

    m_query->bindValue(":AdminIndex", l->index);
    m_query->bindValue(":UserLogin", l->userLogin);
    m_query->bindValue(":Compression", l->compression);
    m_query->bindValue(":AutoDelete", l->autoDelete);
    m_query->bindValue(":Encryption", l->encryption);
    m_query->bindValue(":Password", l->password);
    m_query->bindValue(":DateFormat", l->dateFormat);
    m_query->bindValue(":Language", l->language);
    m_query->bindValue(":Expiration", l->expiration);
    m_query->bindValue(":Authority", l->authority);
    m_query->bindValue(":Certification", l->certification);
    m_query->bindValue(":ServiceDate", l->serviceDate);
    m_query->bindValue(":MetaData1", l->metaData1);
    m_query->bindValue(":MetaData2", l->metaData2);
    m_query->bindValue(":MetaPosition", l->metaPosition);
    m_query->bindValue(":UsrAccAdmin", l->usrAccAdmin);
    m_query->bindValue(":UsrAccTransfer", l->usrAccTransfer);
    m_query->bindValue(":UsrAccDelete", l->usrAccDelete);

    if (!m_query->exec())
    {
        qDebug() << m_query->lastError();
        qFatal("insert Administration failed");
        return (-ERR_ADD_ENTRY);
    }

    return 0;
}

int userDB::Admin_delEntry(Administration *l, DB_QUERY_FLAG flag)
{
    if (flag == QRY_BY_KEY)
    {
        m_query->prepare("DELETE FROM Administration WHERE AdminIndex = ?");
        m_query->addBindValue(l->index);
        if (!m_query->exec())
        {
            qDebug() << m_query->lastError();
            qFatal("delete Administration failed");
            return (-ERR_DEL_ENTRY);
        }
    }
    else
    {
        qDebug() << "delete Administration by multi-field is not supported";
    }

    return 0;
}

#define ADMIN_INDEX         0x1
#define ADMIN_USER_LOGIN    0x2
#define ADMIN_COMPRESSION   0x4
#define ADMIN_AUTO_DELETE   0x08
#define ADMIN_ENCRYPTION    0x10
#define ADMIN_PASSWORD      0x20
#define ADMIN_DATE_FORMAT   0x40
#define ADMIN_LANGUAGE      0x080
#define ADMIN_EXPIRATION    0x100
#define ADMIN_AUTHORITY     0x200
#define ADMIN_CERTIFICATION 0x400
#define ADMIN_SERVICE_DATE  0x0800


int userDB::Admin_query(Administration *l, DB_QUERY_FLAG flag)
{
    int fields = 0;
    QString s = "SELECT * FROM Administration";

    if (flag == QRY_ALL_ENTRIES)
        goto exec;
    else
        s = s + " WHERE";

    if (flag == QRY_BY_KEY)
    {
        s = s + " AdminIndex = (:AdminIndex)";
        fields |= ADMIN_INDEX;
    }
    else if (flag == QRY_BY_MULTI_FIELDS)
    {
        if (l->index >= 0)
        {
            s = s + " AdminIndex = (:AdminIndex)";
            fields |= ADMIN_INDEX;
        }
        if (l->password.isEmpty() == false)
        {
            if (fields)
                s = s + " AND Password = (:Password)";
            else
                s = s + " Password = (:Password)";
            fields |= ADMIN_PASSWORD;
        }
        if (l->expiration.isEmpty() == false)
        {
            if (fields)
                s = s + " AND Expiration = (:Expiration)";
            else
                s = s + " Expiration = (:Expiration)";
            fields |= ADMIN_EXPIRATION;
        }
        if (l->authority.isEmpty() == false)
        {
            if (fields)
                s = s + " AND Authority = (:Authority)";
            else
                s = s + " Authority = (:Authority)";
            fields |= ADMIN_AUTHORITY;
        }
        if (l->certification.isEmpty() == false)
        {
            if (fields)
                s = s + " AND Certification = (:Certification)";
            else
                s = s + " Certification = (:Certification)";
            fields |= ADMIN_CERTIFICATION;
        }
        if (l->serviceDate.isEmpty() == false)
        {
            if (fields)
                s = s + " AND ServiceDate = (:ServiceDate)";
            else
                s = s + " ServiceDate = (:ServiceDate)";
            fields |= ADMIN_SERVICE_DATE;
        }
    }
    else
    {   //ERROR
        qDebug() << "Invalid Users query flag " << flag;
    }

exec:
    m_query->prepare(s);
    if (fields)
    {
        if (fields & ADMIN_INDEX)
            m_query->bindValue(":AdminIndex", l->index);
        if (fields & ADMIN_PASSWORD)
            m_query->bindValue(":Password", l->password);
        if (fields & ADMIN_EXPIRATION)
            m_query->bindValue(":Expiration", l->expiration);
        if (fields & ADMIN_AUTHORITY)
            m_query->bindValue(":Authority", l->authority);
        if (fields & ADMIN_CERTIFICATION)
            m_query->bindValue(":Certification", l->certification);
        if (fields & ADMIN_SERVICE_DATE)
            m_query->bindValue(":ServiceDate", l->serviceDate);
    }

    if (!m_query->exec()) {
        qDebug() << "query Administration failed "<< m_query->lastError();
        return (-ERR_QUERY_EXEC);
    }

    int count = 0;
    if (m_query->driver()->hasFeature(QSqlDriver::QuerySize))
    {
        count = m_query->size();
    }
    else
    {
        bool b = m_query->last();
        if (b)
            count = m_query->at() + 1;
    }

    return count;
}

int userDB::Admin_getNextEntry(Administration *l)
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
        l->index = m_query->value(m_query->record().indexOf("AdminIndex")).toInt();
        l->userLogin = m_query->value(m_query->record().indexOf("UserLogin")).toBool();
        l->compression = m_query->value(m_query->record().indexOf("Compression")).toBool();
        l->autoDelete = m_query->value(m_query->record().indexOf("AutoDelete")).toBool();
        l->encryption = m_query->value(m_query->record().indexOf("Encryption")).toBool();
        l->password = m_query->value(m_query->record().indexOf("Password")).toString();
        l->dateFormat = m_query->value(m_query->record().indexOf("DateFormat")).toInt();
        l->language = m_query->value(m_query->record().indexOf("Language")).toInt();
        l->expiration = m_query->value(m_query->record().indexOf("Expiration")).toString();
        l->authority = m_query->value(m_query->record().indexOf("Authority")).toString();
        l->certification = m_query->value(m_query->record().indexOf("Certification")).toString();
        l->serviceDate = m_query->value(m_query->record().indexOf("ServiceDate")).toString();
        l->metaData1 = m_query->value(m_query->record().indexOf("MetaData1")).toString();
        l->metaData2 = m_query->value(m_query->record().indexOf("MetaData2")).toString();
        l->metaPosition = m_query->value(m_query->record().indexOf("MetaPosition")).toInt();
        l->usrAccAdmin = m_query->value(m_query->record().indexOf("UsrAccAdmin")).toInt();
        l->usrAccTransfer = m_query->value(m_query->record().indexOf("UsrAccTransfer")).toInt();
        l->usrAccDelete = m_query->value(m_query->record().indexOf("UsrAccDelete")).toInt();
    }
    else
    {
        qDebug() << "query get Administration entry failed "<< m_query->lastError();
        return -ERR_QUERY_ENTRY;
    }

    return 0;
}

int userDB::Admin_updateEntry(Administration *l, DB_QUERY_FLAG flag)
{
    int retv = 0;

    if (flag == QRY_BY_KEY)
    {
        QString s = "UPDATE Administration SET UserLogin = (:UserLogin),";
                s = s + " Compression = (:Compression),";
                s = s + " AutoDelete = (:AutoDelete),";
                s = s + " Encryption = (:Encryption),";
                s = s + " Password = (:Password),";
                s = s + " DateFormat = (:DateFormat),";
                s = s + " Language = (:Language),";
                s = s + " Expiration = (:Expiration),";
                s = s + " Authority = (:Authority),";
                s = s + " Certification = (:Certification),";
                s = s + " ServiceDate = (:ServiceDate),";
                s = s + " MetaData1 = (:MetaData1),";
                s = s + " MetaData2 = (:MetaData2),";
                s = s + " MetaPosition = (:MetaPosition),";
                s = s + " UsrAccAdmin = (:UsrAccAdmin),";
                s = s + " UsrAccTransfer = (:UsrAccTransfer),";
                s = s + " UsrAccDelete = (:UsrAccDelete)";
                s = s + " WHERE AdminIndex = (:AdminIndex)";

        if (m_query->prepare(s))
        {
            m_query->bindValue(":UserLogin", l->userLogin);
            m_query->bindValue(":Compression", l->compression);
            m_query->bindValue(":AutoDelete", l->autoDelete);
            m_query->bindValue(":Encryption", l->encryption);
            m_query->bindValue(":Password", l->password);
            m_query->bindValue(":DateFormat", l->dateFormat);
            m_query->bindValue(":Language", l->language);
            m_query->bindValue(":Expiration", l->expiration);
            m_query->bindValue(":Authority", l->authority);
            m_query->bindValue(":Certification", l->certification);
            m_query->bindValue(":ServiceDate", l->serviceDate);
            m_query->bindValue(":MetaData1", l->metaData1);
            m_query->bindValue(":MetaData2", l->metaData2);
            m_query->bindValue(":MetaPosition", l->metaPosition);
            m_query->bindValue(":UsrAccAdmin", l->usrAccAdmin);
            m_query->bindValue(":UsrAccTransfer", l->usrAccTransfer);
            m_query->bindValue(":UsrAccDelete", l->usrAccDelete);
            m_query->bindValue(":AdminIndex", l->index);

            if (!m_query->exec())
                retv = -ERR_UPDATE_EXEC;
        }
        else
            retv = -ERR_UPDATE_PREPARE;
    }
    else
    {
        //not supported
    }

    return retv;
}



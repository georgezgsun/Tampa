#include "user_db.h"


int userDB::Evid_createTbl()
{
    QString str1 = "CREATE TABLE Evidence "
        "(EvidIndex NUMERIC PRIMARY KEY, "
        "EvidNum NUMERIC, "
        "ImageNum NUMERIC, "
        "TicketNum NUMERIC)";

    QSqlQuery query(m_db);
    if (!query.exec(str1))
    {
        qDebug() << "Create Evidence Table failed";
        qDebug() << query.lastError();
        return (-ERR_CREATE_TBL);
    }

    return 0;
}

int userDB::Evid_addEntry(Evidence *l)
{
    m_query->prepare("INSERT INTO Evidence "
                     "(EvidIndex,  EvidNum,  ImageNum,  TicketNum) "
             "VALUES (:EvidIndex, :EvidNum, :ImageNum, :TicketNum)");

    m_query->bindValue(":EvidIndex", l->index);
    m_query->bindValue(":EvidNum", l->evidNum);
    m_query->bindValue(":ImageNum", l->imageNum);
    m_query->bindValue(":TicketNum", l->ticketNum);

    if (!m_query->exec())
    {
        qDebug() << m_query->lastError();
        qFatal("insert Evidence failed");
        return (-ERR_ADD_ENTRY);
    }

    return 0;
}

int userDB::Evid_delEntry(Evidence *l, DB_QUERY_FLAG flag)
{
    if (flag == QRY_BY_KEY)
    {
        m_query->prepare("DELETE FROM Evidence WHERE EvidIndex = ?");
        m_query->addBindValue(l->index);
        if (!m_query->exec())
        {
            qDebug() << m_query->lastError();
            qFatal("delete Evidence failed");
            return (-ERR_DEL_ENTRY);
        }
    }
    else
    {
        qDebug() << "delete Evidence by multi-field is not supported";
    }

    return 0;
}

#define EVIDENCE_INDEX          0x1
#define EVIDENCE_NUMBER         0x2
#define IMAGE_NUMBER            0x4
#define TICKET_NUMBER           0x8


int userDB::Evid_query(Evidence *l, DB_QUERY_FLAG flag)
{
    int fields = 0;
    QString s = "SELECT * FROM Evidence";

    if (flag != QRY_ALL_ENTRIES)
    {
        s = s + " WHERE";
        if (flag == QRY_BY_KEY)
        {
            s = s + " EvidIndex = (:EvidIndex)";
            fields |= EVIDENCE_INDEX;
        }
        else if (flag == QRY_BY_MULTI_FIELDS)
        {
            if (l->index >= 0)
            {
                s = s + " EvidIndex = (:EvidIndex)";
                fields |= EVIDENCE_INDEX;
            }
            if (l->evidNum > 0)
            {
                if (fields)
                    s = s + " AND EvidNum = (:EvidNum)";
                else
                    s = s + " EvidNum = (:EvidNum)";
                fields |= EVIDENCE_NUMBER;
            }
            if (l->imageNum > 0)
            {
                if (fields)
                    s = s + " AND ImageNum = (:ImageNum)";
                else
                    s = s + " ImageNum = (:ImageNum)";
                fields |= IMAGE_NUMBER;
            }
            if (l->ticketNum > 0)
            {
                if (fields)
                    s = s + " AND TicketNum = (:TicketNum)";
                else
                    s = s + " TicketNum = (:TicketNum)";
                fields |= TICKET_NUMBER;
            }
        }
        else
        {      //ERROR
            qDebug() << "Invalid Users query flag " << flag;
        }
    }

    m_query->prepare(s);
    if (fields)
    {
        if (fields & EVIDENCE_INDEX)
            m_query->bindValue(":EvidIndex", l->index);
        if (fields & EVIDENCE_NUMBER)
            m_query->bindValue(":EvidNum", l->evidNum);
        if (fields & IMAGE_NUMBER)
            m_query->bindValue(":ImageNum", l->imageNum);
        if (fields & TICKET_NUMBER)
            m_query->bindValue(":TicketNum", l->ticketNum);
    }

    if (!m_query->exec())
    {
        qDebug() << "query Evidence failed "<< m_query->lastError();
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

int userDB::Evid_getNextEntry(Evidence *l)
{
    bool retv;

    if (m_query->isSelect() == false)
    {   // Set SELECT query
        QString s = "SELECT * FROM Evidence WHERE EvidIndex = (:EvidIndex)";
        m_query->prepare(s);
        m_query->bindValue(":EvidIndex", l->index);
        if (!m_query->exec())
        {
            qDebug() << "query Evidence failed "<< m_query->lastError();
            return (-ERR_QUERY_EXEC);
        }
    }
    retv = m_query->first();
    if (retv == false)
        retv = m_query->last();

    if (retv)
    {
        l->index = m_query->value(m_query->record().indexOf("EvidIndex")).toInt();
        l->evidNum = m_query->value(m_query->record().indexOf("EvidNum")).toUInt();
        l->imageNum = m_query->value(m_query->record().indexOf("ImageNum")).toUInt();
        l->ticketNum = m_query->value(m_query->record().indexOf("TicketNum")).toUInt();
    }
    else
    {
        qDebug() << "query get Evidence entry failed "<< m_query->lastError();
        return -ERR_QUERY_ENTRY;
    }
    return 0;
}

int userDB::Evid_updateEntry(Evidence *l, DB_QUERY_FLAG flag)
{
    int retv = 0;

    if (flag == QRY_BY_KEY)
    {
        QString s = "UPDATE Evidence SET EvidNum = (:EvidNum),";
                s = s + " ImageNum = (:ImageNum),";
                s = s + " TicketNum = (:TicketNum)";
                s = s + " WHERE EvidIndex = (:EvidIndex)";

        if (m_query->prepare(s))
        {
            m_query->bindValue(":EvidNum", l->evidNum);
            m_query->bindValue(":ImageNum", l->imageNum);
            m_query->bindValue(":TicketNum", l->ticketNum);
            m_query->bindValue(":EvidIndex", l->index);

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

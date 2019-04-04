#include "user_db.h"
#include "debug.h"
#include "global.h"

int userDB::Cams_createTbl()
{
    QString s =  "CREATE TABLE CameraSetting "
                 "(CamsIndex NUMERIC PRIMARY KEY, "
                 "Mode TEXT NOT NULL, "
                 "Shutter TEXT NOT NULL, "
                 "Ev TEXT NOT NULL,"
                 "Gain TEXT NOT NULL)";

    QSqlQuery query(m_db);
    if (!query.exec(s))
    {
        DEBUG() << "Create table CameraSetting failed";
        DEBUG() << query.lastError();
        return (-ERR_CREATE_TBL);
    }

    return 0;
}

int userDB::Cams_addEntry(CameraSetting *s)
{
    m_query->prepare("INSERT INTO CameraSetting "
                     "(CamsIndex,  Mode,  Shutter, Ev,  Gain) "
             "VALUES (:CamsIndex, :Mode, :Shutter, :Ev, :Gain)");

    if (!s) {
        //add the default entry
        CameraSetting cs;
        cs.index = CAMS_DEFAULT_INDEX;
        cs.mode = QString("AUTO");
        cs.shutter = QString("AUTO");
        cs.ev = QString("AUTO");
        cs.gain = QString("AUTO");

        m_query->bindValue(":CamsIndex", cs.index);
        m_query->bindValue(":Mode", cs.mode);
        m_query->bindValue(":Shutter", cs.shutter);
        m_query->bindValue(":Ev", cs.ev);
        m_query->bindValue(":Gain", cs.gain);

        if (!m_query->exec()) {
            DEBUG() << m_query->lastError();
            qFatal("insert default CameraSetting failed");
            return (-ERR_ADD_ENTRY);
        }

        return 0;
    }

    m_query->bindValue(":CamsIndex", s->index);
    m_query->bindValue(":Mode", s->mode);
    m_query->bindValue(":Shutter", s->shutter);
    m_query->bindValue(":Ev", s->ev);
    m_query->bindValue(":Gain", s->gain);

    if (!m_query->exec()) {
        DEBUG() << m_query->lastError();
        qFatal("insert CameraSetting failed");
        return (-ERR_ADD_ENTRY);
    }

    return 0;
}

int userDB::Cams_delEntry(CameraSetting *s, DB_QUERY_FLAG flag)
{
    if (flag == QRY_BY_KEY) {
        m_query->prepare("DELETE FROM CameraSetting WHERE CamsIndex = ?");
        m_query->addBindValue(s->index);
        if (!m_query->exec()) {
            DEBUG() << m_query->lastError();
            qFatal("delete CameraSetting entry failed");
            return (-ERR_DEL_ENTRY);
        }
    } else {
        DEBUG() << "delete CameraSetting by multi-field is not supported";
    }

    return 0;
}

#define CAMS_INDEX          0x1
#define CAMS_MODE           0x2
#define CAMS_SHUTTER        0x4
#define CAMS_EV             0x8
#define CAMS_GAIN           0x10


int userDB::Cams_query(CameraSetting *c, DB_QUERY_FLAG flag)
{
    int fields = 0;
    QString s = "SELECT * FROM CameraSetting";

    if (flag == QRY_ALL_ENTRIES) {
        goto exec;
    }
    else {
        s = s + " WHERE";
    }

    if (flag == QRY_BY_KEY) {
        s = s + " CamsIndex = (:CamsIndex)";
        fields |= CAMS_INDEX;
    }
    else if (flag == QRY_BY_MULTI_FIELDS) {
        if (c->index > 0) {
            s = s + " CamsIndex = (:CamsIndex)";
            fields |= CAMS_INDEX;
        }
        if (c->mode.isEmpty() == false) {
            if (fields)
                s = s + " AND";
            s = s + " Mode = (:Mode)";
            fields |= CAMS_MODE;
        }
        if (c->shutter.isEmpty() == false) {
            if (fields)
                s = s + " AND";
            s = s + " Shutter = (:Shutter)";
            fields |= CAMS_SHUTTER;
        }
        if (c->ev.isEmpty() == false) {
            if (fields)
                s = s + " AND";
            s = s + " Ev = (:Ev)";
            fields |= CAMS_EV;
        }if (c->gain.isEmpty() == false) {
            if (fields)
                s = s + " AND";
            s = s + " Gain = (:Gain)";
            fields |= CAMS_GAIN;
        }
    }
    else {      //ERROR
        DEBUG() << "Invalid Users query flag " << flag;
    }

exec:
    m_query->prepare(s);
    if (fields) {
        if (fields & CAMS_INDEX)
            m_query->bindValue(":CamsIndex", c->index);
        if (fields & CAMS_MODE)
            m_query->bindValue(":Mode", c->mode);
        if (fields & CAMS_SHUTTER)
            m_query->bindValue(":Shutter", c->shutter);
        if (fields & CAMS_EV)
            m_query->bindValue(":Ev", c->ev);
        if (fields & CAMS_GAIN)
            m_query->bindValue(":Gain", c->gain);
    }

    if (!m_query->exec()) {
        DEBUG() << "query CameraSetting failed "<< m_query->lastError();
        return (-ERR_QUERY_EXEC);
    }

    int count = 0;
    if (m_query->driver()->hasFeature(QSqlDriver::QuerySize)) {
        count = m_query->size();
    } else {
        bool b = m_query->last();
        if (b)
            count = m_query->at() + 1;
    }

    return count;
}

int userDB::Cams_getNextEntry(CameraSetting *s)
{
    bool retv;

    if (m_firstEntry)
        retv = m_query->next();
    else {
        retv = m_query->first();
        m_firstEntry = 1;
    }

    if (retv) {
      //        DEBUG() << "CameraSetting next return " << retv;
        s->index = m_query->value(m_query->record().indexOf("CamsIndex")).toInt();
        s->mode = m_query->value(m_query->record().indexOf("Mode")).toString();
        s->shutter = m_query->value(m_query->record().indexOf("Shutter")).toString();
        s->ev = m_query->value(m_query->record().indexOf("Ev")).toString();
        s->gain = m_query->value(m_query->record().indexOf("Gain")).toString();
    } else {
        DEBUG() << "query get CameraSetting next entry failed "<< m_query->lastError();
        return -ERR_QUERY_ENTRY;
    }

    return 0;
}

int userDB::Cams_updateEntry(CameraSetting *c, DB_QUERY_FLAG flag)
{
  UNUSED(flag);

    int retv = 0;
    retv = this->Cams_delEntry(c, QRY_BY_KEY);
    if (retv)
        DEBUG() << m_query->lastError();
    retv = this->Cams_addEntry(c);
    if (retv)
        DEBUG() << m_query->lastError();

#if 0
    if (flag == QRY_BY_KEY) {
        QString s = "UPDATE CameraSetting SET Mode = (:Mode),";
                s = s + " Shutter = (:Shutter),";
                s = s + " Ev = (:Ev)";
                s = s + " Gain = (:Gain)";
                s = s + " WHERE CamsIndex = (:CamsIndex)";

        if (m_query->prepare(s)) {

            m_query->bindValue(":CamsIndex", c->index);
            m_query->bindValue(":Mode", c->mode);
            m_query->bindValue(":Shutter", c->shutter);
            m_query->bindValue(":Ev", c->ev);
            m_query->bindValue(":Gain", c->gain);

            if (!m_query->exec()) {
                DEBUG() << m_query->lastError();
                retv = -ERR_UPDATE_EXEC;
            }
        } else {
            DEBUG() << m_query->lastError();
            retv = -ERR_UPDATE_PREPARE;
        }
    } else {
        //not supported
    }
#endif

    return retv;
}




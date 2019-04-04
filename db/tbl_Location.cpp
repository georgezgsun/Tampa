#include "user_db.h"

int userDB::Loc_createTbl()
{
    QString s =  "CREATE TABLE Location "
                 "(LocIndex NUMERIC PRIMARY KEY, "
                 "Description TEXT NOT NULL, "
                 "SpeedLimit TEXT NOT NULL, "
                 "CaptureSpeed TEXT NOT NULL, "
                 "CaptureDistance TEXT NOT NULL, "
                 "RoadCondition TEXT, "
                 "NumberOfLanes NUMERIC)";


    QSqlQuery query(m_db);
    if (!query.exec(s))
    {
        qDebug() << "Create table Location failed";
        qDebug() << query.lastError();
        return (-ERR_CREATE_TBL);
    }
    return 0;
}

int userDB::Loc_addEntry(Location *l)
{
    m_query->prepare("INSERT INTO Location "
                     "(LocIndex,  Description,  SpeedLimit,  CaptureSpeed, CaptureDistance, RoadCondition,  NumberOfLanes) "
             "VALUES (:LocIndex, :Description, :SpeedLimit, :CaptureSpeed, :CaptureDistance, :RoadCondition, :NumberOfLanes)");

    m_query->bindValue(":LocIndex", l->index);
    m_query->bindValue(":Description", l->description);
    m_query->bindValue(":SpeedLimit", l->speedLimit);
    m_query->bindValue(":CaptureSpeed", l->captureSpeed);
    m_query->bindValue(":CaptureDistance", l->captureDistance);
    m_query->bindValue(":RoadCondition", l->roadCondition);
    m_query->bindValue(":NumberOfLanes", l->numberOfLanes);

    if (!m_query->exec()) {
        qDebug() << m_query->lastError();
        qFatal("insert Location failed");
        return (-ERR_ADD_ENTRY);
    }

    return 0;
}

int userDB::Loc_delEntry(Location *l, DB_QUERY_FLAG flag)
{
    if (flag == QRY_BY_KEY) {
        m_query->prepare("DELETE FROM Location WHERE LocIndex = ?");
        m_query->addBindValue(l->index);
        if (!m_query->exec()) {
            qDebug() << m_query->lastError();
            qFatal("delete Location failed");
            return (-ERR_DEL_ENTRY);
        }
    } else {
        qDebug() << "delete Location by multi-field is not supported";
    }

    return 0;
}

#define LOCATION_INDEX             0x1
#define LOCATION_DESCRIPTION       0x2
#define LOCATION_SPEED_LIMIT       0x4
#define LOCATION_CAPTURE_SPEED     0x8
#define LOCATION_CAPTURE_DISTANCE  0x10
#define LOCATION_ROAD_CONDITION    0x20
#define LOCATION_NUMBER_OF_LANES   0x40


int userDB::Loc_query(Location *l, DB_QUERY_FLAG flag)
{
    int fields = 0;
    QString s = "SELECT * FROM Location";

    if (flag == QRY_ALL_ENTRIES) {
        goto exec;
    }
    else {
        s = s + " WHERE";
    }

    if (flag == QRY_BY_KEY) {
        s = s + " LocIndex = (:LocIndex)";
        fields |= LOCATION_INDEX;
    }
    else if (flag == QRY_BY_MULTI_FIELDS) {
        if (l->index > 0) {
            s = s + " LocIndex = (:LocIndex)";
            fields |= LOCATION_INDEX;
        }
        if (l->description.isEmpty() == false) {
            if (fields)
                s = s + " AND Description = (:Description)";
            else
                s = s + " Description = (:Description)";
            fields |= LOCATION_DESCRIPTION;
        }
        if (l->speedLimit.isEmpty() == false) {
            if (fields)
                s = s + " AND SpeedLimit = (:SpeedLimit)";
            else
                s = s + " SpeedLimit = (:SpeedLimit)";
            fields |= LOCATION_SPEED_LIMIT;
        }
        if (l->captureSpeed.isEmpty() == false) {
            if (fields)
                s = s + " AND CaptureSpeed = (:CaptureSpeed)";
            else
                s = s + " CaptureSpeed = (:CaptureSpeed)";
            fields |= LOCATION_CAPTURE_SPEED;
        }
        if (l->captureDistance.isEmpty() == false) {
            if (fields)
                s = s + " AND CaptureDistance = (:CaptureDistance)";
            else
                s = s + " CaptureDistance = (:CaptureDistance)";
            fields |= LOCATION_CAPTURE_DISTANCE;
        }
        if (l->roadCondition.isEmpty() == false) {
            if (fields)
                s = s + " AND RoadCondition = (:RoadCondition)";
            else
                s = s + " RoadCondition = (:RoadCondition)";
            fields |= LOCATION_ROAD_CONDITION;
        }
        if (l->numberOfLanes > 0) {
            if (fields)
                s = s + " AND NumberOfLanes = (:NumberOfLanes)";
            else
                s = s + " NumberOfLanes = (:NumberOfLanes)";
            fields |= LOCATION_NUMBER_OF_LANES;
        }
    }
    else {      //ERROR
        qDebug() << "Invalid Users query flag " << flag;
    }

exec:
    m_query->prepare(s);
    if (fields) {
        if (fields & LOCATION_INDEX)
            m_query->bindValue(":LocIndex", l->index);
        if (fields & LOCATION_DESCRIPTION)
            m_query->bindValue(":Description", l->description);
        if (fields & LOCATION_SPEED_LIMIT)
            m_query->bindValue(":SpeedLimit", l->speedLimit);
        if (fields & LOCATION_CAPTURE_SPEED)
            m_query->bindValue(":CaptureSpeed", l->captureSpeed);
        if (fields & LOCATION_CAPTURE_DISTANCE)
            m_query->bindValue(":CaptureDistance", l->captureDistance);
        if (fields & LOCATION_ROAD_CONDITION)
            m_query->bindValue(":RoadCondition", l->roadCondition);
        if (fields & LOCATION_NUMBER_OF_LANES)
            m_query->bindValue(":NumberOfLanes", l->numberOfLanes);
    }

    if (!m_query->exec()) {
        qDebug() << "query Location failed "<< m_query->lastError();
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

int userDB::Loc_getNextEntry(Location *l)
{
    bool retv;

    if (m_firstEntry)
        retv = m_query->next();
    else {
        retv = m_query->first();
        m_firstEntry = 1;
    }

    if (retv) {
//        qDebug() << "Location next return " << retv;
        l->index = m_query->value(m_query->record().indexOf("LocIndex")).toInt();
        l->description = m_query->value(m_query->record().indexOf("Description")).toString();
        l->speedLimit = m_query->value(m_query->record().indexOf("SpeedLimit")).toString();
        l->captureSpeed = m_query->value(m_query->record().indexOf("CaptureSpeed")).toString();
        l->captureDistance = m_query->value(m_query->record().indexOf("CaptureDistance")).toString();
        l->roadCondition = m_query->value(m_query->record().indexOf("RoadCondition")).toString();
        l->numberOfLanes = m_query->value(m_query->record().indexOf("NumberOfLanes")).toInt();
    } else {
        qDebug() << "query get Location entry failed "<< m_query->lastError();
        return -ERR_QUERY_ENTRY;
    }

    return 0;
}

int userDB::Loc_updateEntry(Location *l, DB_QUERY_FLAG flag)
{
    int retv = 0;

    if (flag == QRY_BY_KEY) {
        QString s = "UPDATE Location SET LocIndex = (:LocIndex),";
                s = s + "Description = (:Description),";
                s = s + " SpeedLimit = (:SpeedLimit),";
                s = s + " CaptureSpeed = (:CaptureSpeed),";
                s = s + " CaptureDistance = (:CaptureDistance),";
                s = s + " RoadCondition = (:RoadCondition),";
                s = s + " NumberOfLanes = (:NumberOfLanes)";

        retv = m_query->prepare(s);
        if (retv) {
            m_query->bindValue(":Description", l->description);
            m_query->bindValue(":SpeedLimit", l->speedLimit);
            m_query->bindValue(":CaptureSpeed", l->captureSpeed);
            m_query->bindValue(":CaptureDistance", l->captureDistance);
            m_query->bindValue(":RoadCondition", l->roadCondition);
            m_query->bindValue(":NumberOfLanes", l->numberOfLanes);
            m_query->bindValue(":LocIndex", l->index);

            if (!m_query->exec()) {
                retv = -ERR_UPDATE_EXEC;
            }
        } else {
            retv = -ERR_UPDATE_PREPARE;
        }
    } else {
        //not supported
    }

    return retv;
}



#include "user_db.h"


int userDB::Conf_createTbl()
{
    QString str1 = "CREATE TABLE Configuration "
        "(ConfIndex NUMERIC PRIMARY KEY, "
#ifdef HH1
        "SpeedTenths NUMERIC, "
        "RangeTenths NUMERIC, "
        "AutoTrigger NUMERIC, "
        "TargetSort NUMERIC, "
        "Units NUMERIC, "
        "Direction NUMERIC, "
        "Frequency REAL, "     // These 4 are Radar related
        "Bandwidth REAL, "
        "RadarPower NUMERIC, "
        "Sensitivity NUMERIC, "
        "Volume NUMERIC, "
        "ImageSpacing NUMERIC, "
        "SerialNumber TEXT NOT NULL, "
#endif
        "BacklighOff NUMERIC, "
        "PowerOff NUMERIC, "
        "Frames NUMERIC, "
        "Resolution NUMERIC, "
        "ImagesPerFile NUMERIC, "
        "PreBuf NUMERIC, "
        "PostBuf NUMERIC)";

    QSqlQuery query(m_db);
    if (!query.exec(str1))
    {
        qDebug() << "Create Configuration Table failed";
        qDebug() << query.lastError();
        return (-ERR_CREATE_TBL);
    }

    return 0;
}

int userDB::Conf_addEntry(SysConfig *l)
{
    m_query->prepare("INSERT INTO Configuration "
#ifdef HH1
        "(ConfIndex,  SpeedTenths,  RangeTenths,  AutoTrigger,  TargetSort,  Units,  Direction,  Frequency,  Bandwidth,  RadarPower,  Sensitivity, Volume, \
          ImageSpacing, SerialNumber, BacklighOff,  PowerOff,  Frames,  Resolution,  ImagesPerFile,  PreBuf,  PostBuf) "
         "VALUES (:ConfIndex, :SpeedTenths, :RangeTenths, :AutoTrigger, :TargetSort, :Units, :Direction, :Frequency, :Bandwidth, :RadarPower, :Sensitivity, :Volume, \
                  :imageSpacing, :SerialNumber, :BacklighOff, :PowerOff, :Frames, :Resolution, :ImagesPerFile, :PreBuf, :PostBuf)");
#else
    "(ConfIndex,  BacklighOff,  PowerOff,  Frames,  Resolution,  ImagesPerFile,  PreBuf,  PostBuf) "
      "VALUES (:ConfIndex, :BacklighOff, :PowerOff, :Frames, :Resolution, :ImagesPerFile, :PreBuf, :PostBuf)");
#endif

    m_query->bindValue(":ConfIndex", l->index);
#ifdef HH1
    m_query->bindValue(":SpeedTenths", l->speedTenths);
    m_query->bindValue(":RangeTenths", l->rangeTenths);
    m_query->bindValue(":AutoTrigger", l->autoTrigger);
    m_query->bindValue(":TargetSort", l->targetSort);
    m_query->bindValue(":Units", l->units);
    m_query->bindValue(":Direction", l->direction);
    m_query->bindValue(":Frequency", l->frequency);
    m_query->bindValue(":Bandwidth", l->bandwidth);
    m_query->bindValue(":RadarPower", l->radarPower);
    m_query->bindValue(":Sensitivity", l->sensitivity);
    m_query->bindValue(":Volume", l->volume);
    m_query->bindValue(":ImageSpacing", l->imageSpacing);
    m_query->bindValue(":SerialNumber", l->serialNumber);
#endif
    m_query->bindValue(":BacklighOff", l->backlightOff);
    m_query->bindValue(":PowerOff", l->powerOff);
    m_query->bindValue(":Frames", l->frames);
    m_query->bindValue(":Resolution", l->resolution);
    m_query->bindValue(":ImagesPerFile", l->imagesPerFile);
    m_query->bindValue(":PreBuf", l->preBuf);
    m_query->bindValue(":PostBuf", l->postBuf);

    if (!m_query->exec())
    {
        qDebug() << m_query->lastError();
        qFatal("insert SysConfig failed");
        return (-ERR_ADD_ENTRY);
    }

    return 0;
}

int userDB::Conf_delEntry(SysConfig *l, DB_QUERY_FLAG flag)
{
    if (flag == QRY_BY_KEY)
    {
        m_query->prepare("DELETE FROM Configuration WHERE ConfIndex = ?");
        m_query->addBindValue(l->index);
        if (!m_query->exec())
        {
            qDebug() << m_query->lastError();
            qFatal("delete SysConfig failed");
            return (-ERR_DEL_ENTRY);
        }
    }
    else
    {
        qDebug() << "delete Configuration by multi-field is not supported";
    }

    return 0;
}

#define CONFIGURATION_INDEX          0x00001
#define CONFIGURATION_BACKLIGHTOFF   0x00002
#define CONFIGURATION_POWEROFF       0x00004
#define CONFIGURATION_FRAMES         0x00008
#define CONFIGURATION_RESOLUTION     0x00010
#define CONFIGURATION_IMAGESPERFILE  0x00020
#define CONFIGURATION_PREBUF         0x00040
#define CONFIGURATION_POSTBUF        0x00080
#ifdef HH1
#define CONFIGURATION_SPEEDTENTHS    0x00100
#define CONFIGURATION_RANGETENTHS    0x00200
#define CONFIGURATION_AUTOTRIGGER    0x00400
#define CONFIGURATION_TARGETSORT     0x00800
#define CONFIGURATION_UNITS          0x01000
#define CONFIGURATION_DIRECTION      0x02000
#define CONFIGURATION_IMAGESPACING   0x04000
#define CONFIGURATION_FREQUENCY      0x08000
#define CONFIGURATION_BANDWIDTH      0x10000
#define CONFIGURATION_RADARPOWER     0x20000
#define CONFIGURATION_SENSITIVITY    0x40000
#define CONFIGURATION_SERIALNUMBER   0x80000
#define CONFIGURATION_VOLUME        0x100000
#endif

int userDB::Conf_query(SysConfig *l, DB_QUERY_FLAG flag)
{
    int fields = 0;
    QString s = "SELECT * FROM Configuration";

    if (flag != QRY_ALL_ENTRIES)
    {
        s = s + " WHERE";
        if (flag == QRY_BY_KEY)
        {
            s = s + " ConfIndex = (:ConfIndex)";
            fields |= CONFIGURATION_INDEX;
        }
        else if (flag == QRY_BY_MULTI_FIELDS)
        {
            if (l->index >= 0)
            {
                s = s + " ConfIndex = (:ConfIndex)";
                fields |= CONFIGURATION_INDEX;
            }
#ifdef HH1
            if (l->speedTenths > 0)
            {
                if (fields)
                    s = s + " AND SpeedTenths = (:SpeedTenths)";
                else
                    s = s + " SpeedTenths = (:SpeedTenths)";
                fields |= CONFIGURATION_SPEEDTENTHS;
            }
            if (l->rangeTenths > 0)
            {
                if (fields)
                    s = s + " AND RangeTenths = (:RangeTenths)";
                else
                    s = s + " RangeTenths = (:RangeTenths)";
                fields |= CONFIGURATION_RANGETENTHS;
            }
            if (l->autoTrigger > 0)
            {
                if (fields)
                    s = s + " AND AutoTrigger = (:AutoTrigger)";
                else
                    s = s + " AutoTrigger = (:AutoTrigger)";
                fields |= CONFIGURATION_AUTOTRIGGER;
            }
            if (l->targetSort > 0)
            {
                if (fields)
                    s = s + " AND TargetSort = (:TargetSort)";
                else
                    s = s + " TargetSort = (:TargetSort)";
                fields |= CONFIGURATION_TARGETSORT;
            }
            if (l->units > 0)
            {
                if (fields)
                    s = s + " AND Units = (:Units)";
                else
                    s = s + " Units = (:Units)";
                fields |= CONFIGURATION_UNITS;
            }
            if (l->direction > 0)
            {
                if (fields)
                    s = s + " AND Direction = (:Direction)";
                else
                    s = s + " Direction = (:Direction)";
                fields |= CONFIGURATION_DIRECTION;
            }
            if (l->imageSpacing > 0)
            {
                if (fields)
                    s = s + " AND ImageSpacing = (:ImageSpacing)";
                else
                    s = s + " ImageSpacing = (:ImageSpacing)";
                fields |= CONFIGURATION_IMAGESPACING;
            }
            if (l->frequency > 0)
            {
                if (fields)
                    s = s + " AND Frequency = (:Frequency)";
                else
                    s = s + " Frequency = (:Frequency)";
                fields |= CONFIGURATION_FREQUENCY;
            }
            if (l->bandwidth > 0)
            {
                if (fields)
                    s = s + " AND Bandwidth = (:Bandwidth)";
                else
                    s = s + " Bandwidth = (:Bandwidth)";
                fields |= CONFIGURATION_BANDWIDTH;
            }
            if (l->radarPower > 0)
            {
                if (fields)
                    s = s + " AND RadarPower = (:RadarPower)";
                else
                    s = s + " RadarPower = (:RadarPower)";
                fields |= CONFIGURATION_RADARPOWER;
            }
            if (l->volume > 0)
            {
                if (fields)
                    s = s + " AND Volume = (:Volume)";
                else
                    s = s + " Volume = (:Volume)";
                fields |= CONFIGURATION_VOLUME;
            }
            if (l->sensitivity > 0)
            {
                if (fields)
                    s = s + " AND Sensitivity = (:Sensitivity)";
                else
                    s = s + " Sensitivity = (:Sensitivity)";
                fields |= CONFIGURATION_SENSITIVITY;
            }
            if (l->serialNumber > 0)
            {
                if (fields)
                    s = s + " AND SerialNumber = (:SerialNumber)";
                else
                    s = s + " SerialNumber = (:SerialNumber)";
                fields |= CONFIGURATION_SERIALNUMBER;
            }
#endif
            if (l->backlightOff > 0)
            {
                if (fields)
                    s = s + " AND BacklighOff = (:BacklighOff)";
                else
                    s = s + " BacklighOff = (:BacklighOff)";
                fields |= CONFIGURATION_BACKLIGHTOFF;
            }
            if (l->powerOff > 0)
            {
                if (fields)
                    s = s + " AND PowerOff = (:PowerOff)";
                else
                    s = s + " PowerOff = (:PowerOff)";
                fields |= CONFIGURATION_POWEROFF;
            }
            if (l->frames > 0)
            {
                if (fields)
                    s = s + " AND Frames = (:Frames)";
                else
                    s = s + " Frames = (:Frames)";
                fields |= CONFIGURATION_FRAMES;
            }
            if (l->resolution > 0)
            {
                if (fields)
                    s = s + " AND Resolution = (:Resolution)";
                else
                    s = s + " Resolution = (:Resolution)";
                fields |= CONFIGURATION_RESOLUTION;
            }
            if (l->imagesPerFile > 0)
            {
                if (fields)
                    s = s + " AND ImagesPerFile = (:ImagesPerFile)";
                else
                    s = s + " ImagesPerFile = (:ImagesPerFile)";
                fields |= CONFIGURATION_IMAGESPERFILE;
            }
            if (l->preBuf > 0)
            {
                if (fields)
                    s = s + " AND PreBuf = (:PreBuf)";
                else
                    s = s + " PreBuf = (:PreBuf)";
                fields |= CONFIGURATION_PREBUF;
            }
            if (l->postBuf > 0)
            {
                if (fields)
                    s = s + " AND PostBuf = (:PostBuf)";
                else
                    s = s + " PostBuf = (:PostBuf)";
                fields |= CONFIGURATION_POSTBUF;
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
        if (fields & CONFIGURATION_INDEX)
            m_query->bindValue(":ConfIndex", l->index);
#ifdef HH1
        if (fields & CONFIGURATION_SPEEDTENTHS)
            m_query->bindValue(":SpeedTenths", l->speedTenths);
        if (fields & CONFIGURATION_RANGETENTHS)
            m_query->bindValue(":RangeTenths", l->rangeTenths);
        if (fields & CONFIGURATION_AUTOTRIGGER)
            m_query->bindValue(":AutoTrigger", l->autoTrigger);
        if (fields & CONFIGURATION_TARGETSORT)
            m_query->bindValue(":TargetSort", l->targetSort);
        if (fields & CONFIGURATION_UNITS)
            m_query->bindValue(":Units", l->units);
        if (fields & CONFIGURATION_DIRECTION)
            m_query->bindValue(":Direction", l->direction);
        if (fields & CONFIGURATION_IMAGESPACING)
            m_query->bindValue(":ImageSpacing", l->imageSpacing);
        if (fields & CONFIGURATION_FREQUENCY)
            m_query->bindValue(":Frequency", l->frequency);
        if (fields & CONFIGURATION_BANDWIDTH)
            m_query->bindValue(":Bandwidth", l->bandwidth);
        if (fields & CONFIGURATION_RADARPOWER)
            m_query->bindValue(":RadarPower", l->radarPower);
        if (fields & CONFIGURATION_SENSITIVITY)
            m_query->bindValue(":Sensitivity", l->sensitivity);
        if (fields & CONFIGURATION_VOLUME)
            m_query->bindValue(":Volume", l->volume);
        if (fields & CONFIGURATION_SERIALNUMBER)
            m_query->bindValue(":SerialNumber", l->serialNumber);
#endif
        if (fields & CONFIGURATION_BACKLIGHTOFF)
            m_query->bindValue(":BacklighOff", l->backlightOff);
        if (fields & CONFIGURATION_POWEROFF)
            m_query->bindValue(":PowerOff", l->powerOff);
        if (fields & CONFIGURATION_FRAMES)
            m_query->bindValue(":Frames", l->frames);
        if (fields & CONFIGURATION_RESOLUTION)
            m_query->bindValue(":Resolution", l->resolution);
        if (fields & CONFIGURATION_IMAGESPERFILE)
            m_query->bindValue(":ImagesPerFile", l->imagesPerFile);
        if (fields & CONFIGURATION_PREBUF)
            m_query->bindValue(":PreBuf", l->preBuf);
        if (fields & CONFIGURATION_POSTBUF)
            m_query->bindValue(":PostBuf", l->postBuf);
    }

    if (!m_query->exec())
    {
        qDebug() << "query SysConfig failed "<< m_query->lastError();
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

int userDB::Conf_getNextEntry(SysConfig *l)
{
    bool retv;

    if (m_query->isSelect() == false)
    {   // Set SELECT query
        QString s = "SELECT * FROM Configuration WHERE ConfIndex = (:ConfIndex)";
        m_query->prepare(s);
        m_query->bindValue(":ConfIndex", l->index);
        if (!m_query->exec())
        {
            qDebug() << "query Configuration failed "<< m_query->lastError();
            return (-ERR_QUERY_EXEC);
        }
    }
    retv = m_query->first();
    if (retv == false)
        retv = m_query->last();

    if (retv)
    {
        l->index = m_query->value(m_query->record().indexOf("ConfIndex")).toInt();
#ifdef HH1
        l->speedTenths = m_query->value(m_query->record().indexOf("SpeedTenths")).toUInt();
        l->rangeTenths = m_query->value(m_query->record().indexOf("RangeTenths")).toUInt();
        l->autoTrigger = m_query->value(m_query->record().indexOf("AutoTrigger")).toUInt();
        l->targetSort = m_query->value(m_query->record().indexOf("TargetSort")).toUInt();
        l->units = m_query->value(m_query->record().indexOf("Units")).toUInt();
        l->direction = m_query->value(m_query->record().indexOf("Direction")).toUInt();
        l->imageSpacing = m_query->value(m_query->record().indexOf("ImageSpacing")).toUInt();
        l->frequency = m_query->value(m_query->record().indexOf("Frequency")).toReal();
        l->bandwidth = m_query->value(m_query->record().indexOf("Bandwidth")).toReal();
        l->radarPower = m_query->value(m_query->record().indexOf("RadarPower")).toUInt();
        l->sensitivity = m_query->value(m_query->record().indexOf("Sensitivity")).toUInt();
        l->volume = m_query->value(m_query->record().indexOf("Volume")).toUInt();
        l->serialNumber = m_query->value(m_query->record().indexOf("SerialNumber")).toString();
#endif
        l->backlightOff = m_query->value(m_query->record().indexOf("BacklighOff")).toUInt();
        l->powerOff = m_query->value(m_query->record().indexOf("PowerOff")).toUInt();
        l->frames = m_query->value(m_query->record().indexOf("Frames")).toUInt();
        l->resolution = m_query->value(m_query->record().indexOf("Resolution")).toUInt();
        l->imagesPerFile = m_query->value(m_query->record().indexOf("ImagesPerFile")).toUInt();
        l->preBuf = m_query->value(m_query->record().indexOf("PreBuf")).toUInt();
        l->postBuf = m_query->value(m_query->record().indexOf("PostBuf")).toUInt();
    }
    else
    {
        qDebug() << "query get Configuration entry failed "<< m_query->lastError();
        return -ERR_QUERY_ENTRY;
    }
    return 0;
}

int userDB::Conf_updateEntry(SysConfig *l, DB_QUERY_FLAG flag)
{
    int retv = 0;

    if (flag == QRY_BY_KEY)
    {
#ifdef HH1
        QString s = "UPDATE Configuration SET SpeedTenths = (:SpeedTenths),";
                s = s + " RangeTenths = (:RangeTenths),";
                s = s + " AutoTrigger = (:AutoTrigger),";
                s = s + " TargetSort = (:TargetSort),";
                s = s + " Units = (:Units),";
                s = s + " Direction = (:Direction),";
                s = s + " BacklighOff = (:BacklighOff),";
                s = s + " Frequency = (:Frequency),";
                s = s + " Bandwidth = (:Bandwidth),";
                s = s + " RadarPower = (:RadarPower),";
                s = s + " Sensitivity = (:Sensitivity),";
                s = s + " Volume = (:Volume),";
                s = s + " SerialNumber = (:SerialNumber),";
#else
        QString s = "UPDATE Configuration SET BacklighOff = (:BacklighOff),";
#endif
                s = s + " PowerOff = (:PowerOff),";
                s = s + " Frames = (:Frames),";
                s = s + " Resolution = (:Resolution),";
                s = s + " ImagesPerFile = (:ImagesPerFile),";
#ifdef HH1
                s = s + " ImageSpacing = (:ImageSpacing),";
#endif
                s = s + " PreBuf = (:PreBuf),";
                s = s + " PostBuf = (:PostBuf)";
                s = s + " WHERE ConfIndex = (:ConfIndex)";

        if (m_query->prepare(s))
        {
#ifdef HH1
            m_query->bindValue(":SpeedTenths", l->speedTenths);
            m_query->bindValue(":RangeTenths", l->rangeTenths);
            m_query->bindValue(":AutoTrigger", l->autoTrigger);
            m_query->bindValue(":TargetSort", l->targetSort);
            m_query->bindValue(":Units", l->units);
            m_query->bindValue(":Direction", l->direction);
            m_query->bindValue(":ImageSpacing", l->imageSpacing);
            m_query->bindValue(":Frequency", l->frequency);
            m_query->bindValue(":Bandwidth", l->bandwidth);
            m_query->bindValue(":RadarPower", l->radarPower);
            m_query->bindValue(":Sensitivity", l->sensitivity);
            m_query->bindValue(":Volume", l->volume);
            m_query->bindValue(":SerialNumber", l->serialNumber);
#endif
            m_query->bindValue(":BacklighOff", l->backlightOff);
            m_query->bindValue(":PowerOff", l->powerOff);
            m_query->bindValue(":Frames", l->frames);
            m_query->bindValue(":Resolution", l->resolution);
            m_query->bindValue(":ImagesPerFile", l->imagesPerFile);
            m_query->bindValue(":PreBuf", l->preBuf);
            m_query->bindValue(":PostBuf", l->postBuf);
            m_query->bindValue(":ConfIndex", l->index);

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

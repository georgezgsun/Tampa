#include "user_db.h"


#ifdef HH1
int userDB::Sensor_createTbl()
{
   QString str1 = "CREATE TABLE Sensor "
        "(SensorIndex NUMERIC PRIMARY KEY, "
        "I2cDevice TEXT NOT NULL, "
        "MagXmax REAL, "
        "MagXmin REAL, "
        "MagYmax REAL, "
        "MagYmin REAL, "
        "MagZmax REAL, "
        "MagZmin REAL, "
        "MagThetaX REAL, "
        "MagThetaY REAL, "
        "MagThetaZ REAL, "
        "AccXmax REAL, "
        "AccXmin REAL, "
        "AccYmax REAL, "
        "AccYmin REAL, "
        "AccZmax REAL, "
        "AccZmin REAL, "
        "AccThetaX REAL, "
        "AccThetaY REAL, "
        "AccThetaZ REAL)";

   QSqlQuery query(m_db);
   if (!query.exec(str1))
   {
      qDebug() << "Create Sensor Table failed";
      qDebug() << query.lastError();
      return (-ERR_CREATE_TBL);
   }

   return 0;
}

int userDB::Sensor_addEntry(Sensor *l)
{
    m_query->prepare("INSERT INTO Sensor "
                     "(SensorIndex,  I2cDevice,  MagXmax,  MagXmin,  MagYmax,  MagYmin,  MagZmax,  MagZmin,  MagThetaX,  MagThetaY,  MagThetaZ, \
                       AccXmax,  AccXmin,  AccYmax,  AccYmin,  AccZmax,  AccZmin,  AccThetaX,  AccThetaY,  AccThetaZ) "
             "VALUES (:SensorIndex, :I2cDevice, :MagXmax, :MagXmin, :MagYmax, :MagYmin, :MagZmax, :MagZmin, :MagThetaX, :MagThetaY, :MagThetaZ, \
                      :AccXmax, :AccXmin, :AccYmax, :AccYmin, :AccZmax, :AccZmin, :AccThetaX, :AccThetaY, :AccThetaZ)");

    m_query->bindValue(":SensorIndex", l->index);
    m_query->bindValue(":I2cDevice", l->i2cDevice);
    m_query->bindValue(":MagXmax", l->magXmax);
    m_query->bindValue(":MagXmin", l->magXmin);
    m_query->bindValue(":MagYmax", l->magYmax);
    m_query->bindValue(":MagYmin", l->magYmin);
    m_query->bindValue(":MagZmax", l->magZmax);
    m_query->bindValue(":MagZmin", l->magZmin);
    m_query->bindValue(":MagThetaX", l->magThetaX);
    m_query->bindValue(":MagThetaY", l->magThetaY);
    m_query->bindValue(":MagThetaZ", l->accThetaZ);
    m_query->bindValue(":AccXmax", l->accXmax);
    m_query->bindValue(":AccXmin", l->accXmin);
    m_query->bindValue(":AccYmax", l->accYmax);
    m_query->bindValue(":AccYmin", l->accYmin);
    m_query->bindValue(":AccZmax", l->accZmax);
    m_query->bindValue(":AccZmin", l->accZmin);
    m_query->bindValue(":AccThetaX", l->accThetaX);
    m_query->bindValue(":AccThetaY", l->accThetaY);
    m_query->bindValue(":AccThetaZ", l->accThetaZ);

    if (!m_query->exec())
    {
        qDebug() << m_query->lastError();
        qFatal("insert Sensor failed");
        return (-ERR_ADD_ENTRY);
    }

    return 0;
}

int userDB::Sensor_delEntry(Sensor *l, DB_QUERY_FLAG flag)
{
    if (flag == QRY_BY_KEY)
    {
        m_query->prepare("DELETE FROM Sensor WHERE SensorIndex = ?");
        m_query->addBindValue(l->index);
        if (!m_query->exec())
        {
            qDebug() << m_query->lastError();
            qFatal("delete Sensor failed");
            return (-ERR_DEL_ENTRY);
        }
    }
    else
    {
        qDebug() << "delete Sensor by multi-field is not supported";
    }

    return 0;
}

#define SENSOR_INDEX          0x1
#define SENSOR_I2C_DEVICE     0x2
#define SENSOR_MAG_XMAX       0x4
#define SENSOR_MAG_XMIN       0x8
#define SENSOR_MAG_YMAX       0x10
#define SENSOR_MAG_YMIN       0x20
#define SENSOR_MAG_ZMAX       0x40
#define SENSOR_MAG_ZMIN       0x80
#define SENSOR_MAG_THETA_X    0x100
#define SENSOR_MAG_THETA_Y    0x200
#define SENSOR_MAG_THETA_Z    0x400
#define SENSOR_ACC_XMAX       0x800
#define SENSOR_ACC_XMIN       0x1000
#define SENSOR_ACC_YMAX       0x2000
#define SENSOR_ACC_YMIN       0x4000
#define SENSOR_ACC_ZMAX       0x8000
#define SENSOR_ACC_ZMIN       0x10000
#define SENSOR_ACC_THETA_X    0x20000
#define SENSOR_ACC_THETA_Y    0x40000
#define SENSOR_ACC_THETA_Z    0x80000

int userDB::Sensor_query(Sensor *l, DB_QUERY_FLAG flag)
{
    int fields = 0;
    QString s = "SELECT * FROM Sensor";

    if (flag != QRY_ALL_ENTRIES)
    {
        s = s + " WHERE";
        if (flag == QRY_BY_KEY)
        {
            s = s + " SensorIndex = (:SensorIndex)";
            fields |= SENSOR_INDEX;
        }
        else if (flag == QRY_BY_MULTI_FIELDS)
        {
            if (l->index >= 0)
            {
                s = s + " SensorIndex = (:SensorIndex)";
                fields |= SENSOR_INDEX;
            }
            if (l->i2cDevice > 0)
            {
                if (fields)
                    s = s + " AND I2cDevice = (:I2cDevice)";
                else
                    s = s + " I2cDevice = (:I2cDevice)";
                fields |= SENSOR_I2C_DEVICE;
            }
            if (l->magXmax > 0)
            {
                if (fields)
                    s = s + " AND MagXmax = (:MagXmax)";
                else
                    s = s + " MagXmax = (:MagXmax)";
                fields |= SENSOR_MAG_XMAX;
            }
            if (l->magXmin > 0)
            {
                if (fields)
                    s = s + " AND MagXmin = (:MagXmin)";
                else
                    s = s + " MagXmin = (:MagXmin)";
                fields |= SENSOR_MAG_XMIN;
            }
            if (l->magYmax > 0)
            {
                if (fields)
                    s = s + " AND MagYmax = (:MagYmax)";
                else
                    s = s + " MagYmax = (:MagYmax)";
                fields |= SENSOR_MAG_YMAX;
            }
            if (l->magYmin > 0)
            {
                if (fields)
                    s = s + " AND MagYmin = (:MagYmin)";
                else
                    s = s + " MagYmin = (:MagYmin)";
                fields |= SENSOR_MAG_YMIN;
            }
            if (l->magZmax > 0)
            {
                if (fields)
                    s = s + " AND MagZmax = (:MagZmax)";
                else
                    s = s + " MagZmax = (:MagZmax)";
                fields |= SENSOR_MAG_ZMAX;
            }
            if (l->magZmin > 0)
            {
                if (fields)
                    s = s + " AND MagZmin = (:MagZmin)";
                else
                    s = s + " MagZmin = (:MagZmin)";
                fields |= SENSOR_MAG_ZMIN;
            }
            if (l->magThetaX > 0)
            {
                if (fields)
                    s = s + " AND MagThetaX = (:MagThetaX)";
                else
                    s = s + " MagThetaX = (:MagThetaX)";
                fields |= SENSOR_MAG_THETA_X;
            }
            if (l->magThetaY > 0)
            {
                if (fields)
                    s = s + " AND MagThetaY = (:MagThetaY)";
                else
                    s = s + " MagThetaY = (:MagThetaY)";
                fields |= SENSOR_MAG_THETA_Y;
            }
            if (l->magThetaZ > 0)
            {
                if (fields)
                    s = s + " AND MagThetaZ = (:MagThetaZ)";
                else
                    s = s + " MagThetaZ = (:MagThetaZ)";
                fields |= SENSOR_MAG_THETA_Z;
            }
            if (l->accXmax > 0)
            {
                if (fields)
                    s = s + " AND AccXmax = (:AccXmax)";
                else
                    s = s + " AccXmax = (:AccXmax)";
                fields |= SENSOR_ACC_XMAX;
            }
            if (l->accXmin > 0)
            {
                if (fields)
                    s = s + " AND AccXmin = (:AccXmin)";
                else
                    s = s + " AccXmin = (:AccXmin)";
                fields |= SENSOR_ACC_XMIN;
            }
            if (l->accYmax > 0)
            {
                if (fields)
                    s = s + " AND AccYmax = (:AccYmax)";
                else
                    s = s + " AccYmax = (:AccYmax)";
                fields |= SENSOR_ACC_YMAX;
            }
            if (l->accYmin > 0)
            {
                if (fields)
                    s = s + " AND AccYmin = (:AccYmin)";
                else
                    s = s + " AccYmin = (:AccYmin)";
                fields |= SENSOR_ACC_YMIN;
            }
            if (l->accZmax > 0)
            {
                if (fields)
                    s = s + " AND AccZmax = (:AccZmax)";
                else
                    s = s + " AccZmax = (:AccZmax)";
                fields |= SENSOR_ACC_ZMAX;
            }
            if (l->accZmin > 0)
            {
                if (fields)
                    s = s + " AND AccZmin = (:AccZmin)";
                else
                    s = s + " AccZmin = (:AccZmin)";
                fields |= SENSOR_ACC_ZMIN;
            }
            if (l->accThetaX > 0)
            {
                if (fields)
                    s = s + " AND AccThetaX = (:AccThetaX)";
                else
                    s = s + " AccThetaX = (:AccThetaX)";
                fields |= SENSOR_ACC_THETA_X;
            }
            if (l->accThetaY > 0)
            {
                if (fields)
                    s = s + " AND AccThetaY = (:AccThetaY)";
                else
                    s = s + " AccThetaY = (:AccThetaY)";
                fields |= SENSOR_ACC_THETA_Y;
            }
            if (l->accThetaZ > 0)
            {
                if (fields)
                    s = s + " AND AccThetaZ = (:AccThetaZ)";
                else
                    s = s + " AccThetaZ = (:AccThetaZ)";
                fields |= SENSOR_ACC_THETA_Z;
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
        if (fields & SENSOR_INDEX)
            m_query->bindValue(":SensorIndex", l->index);
        if (fields & SENSOR_I2C_DEVICE)
            m_query->bindValue(":I2cDevice", l->i2cDevice);
        if (fields & SENSOR_MAG_XMAX)
            m_query->bindValue(":MagXmax", l->magXmax);
        if (fields & SENSOR_MAG_XMIN)
            m_query->bindValue(":MagXmin", l->magXmin);
        if (fields & SENSOR_MAG_YMAX)
            m_query->bindValue(":MagYmax", l->magYmax);
        if (fields & SENSOR_MAG_YMIN)
            m_query->bindValue(":MagYmin", l->magYmin);
        if (fields & SENSOR_MAG_ZMAX)
            m_query->bindValue(":MagZmax", l->magZmax);
        if (fields & SENSOR_MAG_ZMIN)
            m_query->bindValue(":MagZmin", l->magZmin);
        if (fields & SENSOR_MAG_THETA_X)
            m_query->bindValue(":MagThetaX", l->magThetaX);
        if (fields & SENSOR_MAG_THETA_Y)
            m_query->bindValue(":MagThetaY", l->magThetaY);
        if (fields & SENSOR_MAG_THETA_Z)
            m_query->bindValue(":MagThetaZ", l->magThetaZ);
        if (fields & SENSOR_ACC_XMAX)
            m_query->bindValue(":AccXmax", l->accXmax);
        if (fields & SENSOR_ACC_XMIN)
            m_query->bindValue(":AccXmin", l->accXmin);
        if (fields & SENSOR_ACC_YMAX)
            m_query->bindValue(":AccYmax", l->accYmax);
        if (fields & SENSOR_ACC_YMIN)
            m_query->bindValue(":AccYmin", l->accYmin);
        if (fields & SENSOR_ACC_ZMAX)
            m_query->bindValue(":AccZmax", l->accZmax);
        if (fields & SENSOR_ACC_ZMIN)
            m_query->bindValue(":AccZmin", l->accZmin);
        if (fields & SENSOR_ACC_THETA_X)
            m_query->bindValue(":AccThetaX", l->accThetaX);
        if (fields & SENSOR_ACC_THETA_Y)
            m_query->bindValue(":AccThetaY", l->accThetaY);
        if (fields & SENSOR_ACC_THETA_Z)
            m_query->bindValue(":AccThetaZ", l->accThetaZ);
    }

    if (!m_query->exec())
    {
        qDebug() << "query Sensor failed "<< m_query->lastError();
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

int userDB::Sensor_getNextEntry(Sensor *l)
{
    bool retv;

    if (m_query->isSelect() == false)
    {   // Set SELECT query
        QString s = "SELECT * FROM Sensor WHERE SensorIndex = (:SensorIndex)";
        m_query->prepare(s);
        m_query->bindValue(":SensorIndex", l->index);
        if (!m_query->exec())
        {
            qDebug() << "query Sensor failed "<< m_query->lastError();
            return (-ERR_QUERY_EXEC);
        }
    }
    retv = m_query->first();
    if (retv == false)
        retv = m_query->last();

    if (retv)
    {
        l->index = m_query->value(m_query->record().indexOf("SensorIndex")).toInt();
        l->i2cDevice = m_query->value(m_query->record().indexOf("I2cDevice")).toString();
        l->magXmax = m_query->value(m_query->record().indexOf("MagXmax")).toReal();
        l->magXmin = m_query->value(m_query->record().indexOf("MagXmin")).toReal();
        l->magYmax = m_query->value(m_query->record().indexOf("MagYmax")).toReal();
        l->magYmin = m_query->value(m_query->record().indexOf("MagYmin")).toReal();
        l->magZmax = m_query->value(m_query->record().indexOf("MagZmax")).toReal();
        l->magZmin = m_query->value(m_query->record().indexOf("MagZmin")).toReal();
        l->magThetaX = m_query->value(m_query->record().indexOf("MagThetaX")).toReal();
        l->magThetaY = m_query->value(m_query->record().indexOf("MagThetaY")).toReal();
        l->magThetaZ = m_query->value(m_query->record().indexOf("MagThetaZ")).toReal();
        l->accXmax = m_query->value(m_query->record().indexOf("AccXmax")).toReal();
        l->accXmin = m_query->value(m_query->record().indexOf("AccXmin")).toReal();
        l->accYmax = m_query->value(m_query->record().indexOf("AccYmax")).toReal();
        l->accYmin = m_query->value(m_query->record().indexOf("AccYmin")).toReal();
        l->accZmax = m_query->value(m_query->record().indexOf("AccZmax")).toReal();
        l->accZmin = m_query->value(m_query->record().indexOf("AccZmin")).toReal();
        l->accThetaX = m_query->value(m_query->record().indexOf("AccThetaX")).toReal();
        l->accThetaY = m_query->value(m_query->record().indexOf("AccThetaY")).toReal();
        l->accThetaZ = m_query->value(m_query->record().indexOf("AccThetaZ")).toReal();
    }
    else
    {
        qDebug() << "query get Sensor entry failed "<< m_query->lastError();
        return -ERR_QUERY_ENTRY;
    }
    return 0;
}

int userDB::Sensor_updateEntry(Sensor *l, DB_QUERY_FLAG flag)
{
    int retv = 0;

    if (flag == QRY_BY_KEY)
    {
        QString s = "UPDATE Sensor SET I2cDevice = (:I2cDevice),";
                s = s + " MagXmax = (:MagXmax),";
                s = s + " MagXmin = (:MagXmin),";
                s = s + " MagYmax = (:MagYmax),";
                s = s + " MagYmin = (:MagYmin),";
                s = s + " MagZmax = (:MagZmax),";
                s = s + " MagZmin = (:MagZmin),";
                s = s + " MagThetaX = (:MagThetaX),";
                s = s + " MagThetaY = (:MagThetaY),";
                s = s + " MagThetaZ = (:MagThetaZ),";
                s = s + " AccXmax = (:AccXmax),";
                s = s + " AccXmin = (:AccXmin),";
                s = s + " AccYmax = (:AccYmax),";
                s = s + " AccYmin = (:AccYmin),";
                s = s + " AccZmax = (:AccZmax),";
                s = s + " AccZmin = (:AccZmin),";
                s = s + " AccThetaX = (:AccThetaX),";
                s = s + " AccThetaY = (:AccThetaY),";
                s = s + " AccThetaZ = (:AccThetaZ)";
                s = s + " WHERE SensorIndex = (:SensorIndex)";

        if (m_query->prepare(s))
        {
           m_query->bindValue(":I2cDevice", l->i2cDevice);
            m_query->bindValue(":MagXmax", l->magXmax);
            m_query->bindValue(":MagXmin", l->magXmin);
            m_query->bindValue(":MagYmax", l->magYmax);
            m_query->bindValue(":MagYmin", l->magYmin);
            m_query->bindValue(":MagZmax", l->magZmax);
            m_query->bindValue(":MagZmin", l->magZmin);
            m_query->bindValue(":MagThetaX", l->magThetaX);
            m_query->bindValue(":MagThetaY", l->magThetaY);
            m_query->bindValue(":MagThetaZ", l->magThetaZ);
            m_query->bindValue(":AccXmax", l->accXmax);
            m_query->bindValue(":AccXmin", l->accXmin);
            m_query->bindValue(":AccYmax", l->accYmax);
            m_query->bindValue(":AccYmin", l->accYmin);
            m_query->bindValue(":AccZmax", l->accZmax);
            m_query->bindValue(":AccZmin", l->accZmin);
            m_query->bindValue(":AccThetaX", l->accThetaX);
            m_query->bindValue(":AccThetaY", l->accThetaY);
            m_query->bindValue(":AccThetaZ", l->accThetaZ);
            m_query->bindValue(":SensorIndex", l->index);

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
#endif

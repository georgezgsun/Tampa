#include "user_db.h"


int userDB::Network_createTbl()
{
    QString str1 = "CREATE TABLE Network "
        "(NetworkIndex NUMERIC PRIMARY KEY, "
        "Discovery NUMERIC, "
        "IpAddr NUMERIC, "
        "SubnetMask NUMERIC, "
        "Gateway NUMERIC, "
        "Dns1 NUMERIC, "
        "Dns2 NUMERIC)";

    QSqlQuery query(m_db);
    if (!query.exec(str1))
    {
        qDebug() << "Create Network Table failed";
        qDebug() << query.lastError();
        return (-ERR_CREATE_TBL);
    }

    return 0;
}

int userDB::Network_addEntry(Network *l)
{
    m_query->prepare("INSERT INTO Network "
                     "(NetworkIndex,  Discovery,  IpAddr,  SubnetMask,  Gateway,  Dns1,  Dns2) "
             "VALUES (:NetworkIndex, :Discovery, :IpAddr, :SubnetMask, :Gateway, :Dns1, :Dns2)");

    m_query->bindValue(":NetworkIndex", l->index);
    m_query->bindValue(":Discovery", l->discovery);
    m_query->bindValue(":IpAddr", l->ipAddr);
    m_query->bindValue(":SubnetMask", l->subnetMask);
    m_query->bindValue(":Gateway", l->gateway);
    m_query->bindValue(":Dns1", l->dns1);
    m_query->bindValue(":Dns2", l->dns2);

    if (!m_query->exec())
    {
        qDebug() << m_query->lastError();
        qFatal("insert Network failed");
        return (-ERR_ADD_ENTRY);
    }

    return 0;
}

int userDB::Network_delEntry(Network *l, DB_QUERY_FLAG flag)
{
    if (flag == QRY_BY_KEY)
    {
        m_query->prepare("DELETE FROM Network WHERE NetworkIndex = ?");
        m_query->addBindValue(l->index);
        if (!m_query->exec())
        {
            qDebug() << m_query->lastError();
            qFatal("delete Network failed");
            return (-ERR_DEL_ENTRY);
        }
    }
    else
    {
        qDebug() << "delete Network by multi-field is not supported";
    }

    return 0;
}

#define NETWORK_INDEX          0x1
#define NETWORK_DISCOVERY      0x2
#define NETWORK_IPADDR         0x4
#define NETWORK_SUBNETMASK     0x8
#define NETWORK_GATEWAY        0x10
#define NETWORK_DNS1           0x20
#define NETWORK_DNS2           0x40


int userDB::Network_query(Network *l, DB_QUERY_FLAG flag)
{
    int fields = 0;
    QString s = "SELECT * FROM Network";

    if (flag != QRY_ALL_ENTRIES)
    {
        s = s + " WHERE";
        if (flag == QRY_BY_KEY)
        {
            s = s + " NetworkIndex = (:NetworkIndex)";
            fields |= NETWORK_INDEX;
        }
        else if (flag == QRY_BY_MULTI_FIELDS)
        {
            if (l->index >= 0)
            {
                s = s + " NetworkIndex = (:NetworkIndex)";
                fields |= NETWORK_INDEX;
            }
            if (l->discovery > 0)
            {
                if (fields)
                    s = s + " AND Discovery = (:Discovery)";
                else
                    s = s + " Discovery = (:Discovery)";
                fields |= NETWORK_DISCOVERY;
            }
            if (l->ipAddr > 0)
            {
                if (fields)
                    s = s + " AND IpAddr = (:IpAddr)";
                else
                    s = s + " IpAddr = (:IpAddr)";
                fields |= NETWORK_IPADDR;
            }
            if (l->subnetMask > 0)
            {
                if (fields)
                    s = s + " AND SubnetMask = (:SubnetMask)";
                else
                    s = s + " SubnetMask = (:SubnetMask)";
                fields |= NETWORK_SUBNETMASK;
            }
            if (l->gateway > 0)
            {
                if (fields)
                    s = s + " AND Gateway = (:Gateway)";
                else
                    s = s + " Gateway = (:Gateway)";
                fields |= NETWORK_GATEWAY;
            }
            if (l->dns1 > 0)
            {
                if (fields)
                    s = s + " AND Dns1 = (:Dns1)";
                else
                    s = s + " Dns1 = (:Dns1)";
                fields |= NETWORK_DNS1;
            }
            if (l->dns2 > 0)
            {
                if (fields)
                    s = s + " AND Dns2 = (:Dns2)";
                else
                    s = s + " Dns2 = (:Dns2)";
                fields |= NETWORK_DNS2;
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
        if (fields & NETWORK_INDEX)
            m_query->bindValue(":NetworkIndex", l->index);
        if (fields & NETWORK_DISCOVERY)
            m_query->bindValue(":Discovery", l->discovery);
        if (fields & NETWORK_IPADDR)
            m_query->bindValue(":IpAddr", l->ipAddr);
        if (fields & NETWORK_SUBNETMASK)
            m_query->bindValue(":SubnetMask", l->subnetMask);
        if (fields & NETWORK_GATEWAY)
            m_query->bindValue(":Gateway", l->gateway);
        if (fields & NETWORK_DNS1)
            m_query->bindValue(":Dns1", l->dns1);
        if (fields & NETWORK_DNS2)
            m_query->bindValue(":Dns2", l->dns2);
    }

    if (!m_query->exec())
    {
        qDebug() << "query Network failed "<< m_query->lastError();
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

int userDB::Network_getNextEntry(Network *l)
{
    bool retv;

    if (m_query->isSelect() == false)
    {   // Set SELECT query
        QString s = "SELECT * FROM Network WHERE NetworkIndex = (:NetworkIndex)";
        m_query->prepare(s);
        m_query->bindValue(":NetworkIndex", l->index);
        if (!m_query->exec())
        {
            qDebug() << "query Network failed "<< m_query->lastError();
            return (-ERR_QUERY_EXEC);
        }
    }
    retv = m_query->first();
    if (retv == false)
        retv = m_query->last();

    if (retv)
    {
        l->index = m_query->value(m_query->record().indexOf("NetworkIndex")).toInt();
        l->discovery = m_query->value(m_query->record().indexOf("Discovery")).toUInt();
        l->ipAddr = m_query->value(m_query->record().indexOf("IpAddr")).toUInt();
        l->subnetMask = m_query->value(m_query->record().indexOf("SubnetMask")).toUInt();
        l->gateway = m_query->value(m_query->record().indexOf("Gateway")).toUInt();
        l->dns1 = m_query->value(m_query->record().indexOf("Dns1")).toUInt();
        l->dns2 = m_query->value(m_query->record().indexOf("Dns2")).toUInt();
    }
    else
    {
        qDebug() << "query get Network entry failed "<< m_query->lastError();
        return -ERR_QUERY_ENTRY;
    }
    return 0;
}

int userDB::Network_updateEntry(Network *l, DB_QUERY_FLAG flag)
{
    int retv = 0;

    if (flag == QRY_BY_KEY)
    {
        QString s = "UPDATE Network SET Discovery = (:Discovery),";
                s = s + " IpAddr = (:IpAddr),";
                s = s + " SubnetMask = (:SubnetMask),";
                s = s + " Gateway = (:Gateway),";
                s = s + " Dns1 = (:Dns1),";
                s = s + " Dns2 = (:Dns2)";
                s = s + " WHERE NetworkIndex = (:NetworkIndex)";

        if (m_query->prepare(s))
        {
            m_query->bindValue(":Discovery", l->discovery);
            m_query->bindValue(":IpAddr", l->ipAddr);
            m_query->bindValue(":SubnetMask", l->subnetMask);
            m_query->bindValue(":Gateway", l->gateway);
            m_query->bindValue(":Dns1", l->dns1);
            m_query->bindValue(":Dns2", l->dns2);
            m_query->bindValue(":NetworkIndex", l->index);

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

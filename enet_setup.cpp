#include "enet_setup.h"
#include "ui_enet_setup.h"
#include "state.h"
#include <QNetworkInterface>
#include <QByteArray>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include "debug.h"
#include "utils.h"

enetSetup::enetSetup(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::enetSetup)
{
    ui->setupUi(this);

    initLists();
    buildHashTables();
    setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_ENET_SETUP);
    m_listIndex = m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);
}

enetSetup::~enetSetup()
{
    bool flag1 = false;

    if ((mNetTypeIndex != (int)(mNet.discovery)))
    {
        mNet.discovery = (unsigned int)mNetTypeIndex;
        flag1 = true;   // data changed
    }
    if (mNetTypeIndex)
    {   // static
        // IP address
        if (mOldIpAddr != ui->le_ipAddress->text())
        {
            QByteArray ba = ui->le_ipAddress->text().toLatin1();
            char *str1 = ba.data();
            unsigned int i1, i2, i3, i4;
            // IP address
            sscanf(str1, "%u.%u.%u.%u", &i1, &i2, &i3, &i4);
            mNet.ipAddr = (i1 << 24) | ((i2 << 16) & 0x0FF0000) | ((i3 << 8) & 0x0FF00) | (i4 & 0x0FF);
            flag1 = true;   // data changed
        }
        // Subnet Mask
        if (mOldSubnetMask != ui->le_subnetMask->text())
        {
            // Subnet Mask
            QByteArray ba = ui->le_subnetMask->text().toLatin1();
            char *str1 = ba.data();
            unsigned int i1, i2, i3, i4;
            sscanf(str1, "%u.%u.%u.%u", &i1, &i2, &i3, &i4);
            mNet.subnetMask = (i1 << 24) | ((i2 << 16) & 0x0FF0000) | ((i3 << 8) & 0x0FF00) | (i4 & 0x0FF);
            flag1 = true;   // data changed
        }
        // Gateway
        if (mOldGateway != ui->le_gateway->text())
        {
            // Subnet Mask
            QByteArray ba = ui->le_gateway->text().toLatin1();
            char *str1 = ba.data();
            unsigned int i1, i2, i3, i4;
            sscanf(str1, "%u.%u.%u.%u", &i1, &i2, &i3, &i4);
            mNet.gateway = (i1 << 24) | ((i2 << 16) & 0x0FF0000) | ((i3 << 8) & 0x0FF00) | (i4 & 0x0FF);
            flag1 = true;   // data changed
        }
        // DNS1
        if (mOldDns1 != ui->le_dns1->text())
        {
            // Subnet Mask
            QByteArray ba = ui->le_dns1->text().toLatin1();
            char *str1 = ba.data();
            unsigned int i1, i2, i3, i4;
            sscanf(str1, "%u.%u.%u.%u", &i1, &i2, &i3, &i4);
            mNet.dns1 = (i1 << 24) | ((i2 << 16) & 0x0FF0000) | ((i3 << 8) & 0x0FF00) | (i4 & 0x0FF);
            flag1 = true;   // data changed
        }
        // Subnet Mask
        if (mOldDns2 != ui->le_dns2->text())
        {
            // Subnet Mask
            QByteArray ba = ui->le_dns2->text().toLatin1();
            char *str1 = ba.data();
            unsigned int i1, i2, i3, i4;
            sscanf(str1, "%u.%u.%u.%u", &i1, &i2, &i3, &i4);
            mNet.dns2 = (i1 << 24) | ((i2 << 16) & 0x0FF0000) | ((i3 << 8) & 0x0FF00) | (i4 & 0x0FF);
            flag1 = true;   // data changed
        }
    }
    if (flag1 == true)
        Utils::get().setNetwork(mNet);
    delete ui;
}

void enetSetup::initLists()
{
    m_list << ui->pb_type
           << ui->le_ipAddress
           << ui->le_subnetMask
           << ui->le_gateway
           << ui->le_dns1
           << ui->le_dns2;

    m_cmdList << CMD_ENET_SETTINGS
              << CMD_ENET_ADDRESS
              << CMD_ENET_SUBNET
              << CMD_ENET_GATEWAY
              << CMD_ENET_DNS1
              << CMD_ENET_DNS2;

    this->connectWidgetSigs();
}

void enetSetup::buildHashTables()
{
    m_enetTypeList << "DHCP" << "STATIC";

    m_hashValueList[CMD_ENET_SETTINGS] = &m_enetTypeList;
    m_hashValueIndex[CMD_ENET_SETTINGS] = &mNetTypeIndex;
}

void enetSetup::setInittoggleValues()
{
    mNet = Utils::get().getNetwork();
    mNetTypeIndex = (int)(mNet.discovery);
    ui->pb_type->setText(m_enetTypeList.at(mNetTypeIndex));

    if (!mNetTypeIndex)
    {   // DHCP
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
        {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            {
                //	  DEBUG() << address.toString();
                //  qDebug() << address.netmask();
                ui->le_ipAddress->setText(address.toString());

                QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
                QNetworkInterface eth;

                foreach(eth, allInterfaces)
                {
                    QList<QNetworkAddressEntry> allEntries = eth.addressEntries();
                    QNetworkAddressEntry entry;
                    foreach (entry, allEntries)
                    {
                        if( address == entry.ip() )
                        {
                            DEBUG() << entry.ip().toString() << "/" << entry.netmask().toString();
                            ui->le_subnetMask->setText(entry.netmask().toString());
                        }
                    }
                }
                // QHostAddress QNetworkAddressEntry::netmask() const	}
            }
        }

        QFile routeFile("/proc/net/route");
  
        if (!routeFile.open(QFile::ReadOnly))
        {
            //qWarn("Couldn't read routing information: %s", qPrintable(routeFile.errorString()));
            DEBUG() << "Couldn't read routing information:";
        }
  
        QByteArray line;
        while (!(line = routeFile.readLine()).isNull())
        {
            QList<QByteArray> parts = line.split('\t');
            QByteArray intf = parts[0];
            QByteArray route = parts[1];
            QByteArray gateway = parts[2];
            QByteArray mask = parts[7];

            // Find make sure the destination address is 0.0.0.0 and the netmask empty
            if (route == "00000000" && mask == "00000000")
            {
              unsigned int x = 0;
              int i;
              // data from the /proc/net/route file is string repersentation of a hex number
              // read 8 bytes of string data, convert to integer
              for( i=0;i<8;i++ )
              {
                char data =  gateway.at(i);
                int y = strtol(&data, NULL, 16);
                x += ( (y & 0x0f) << (28 - (4 *i)));
              }

              unsigned int reverseX = htonl(x);
              QHostAddress addr( reverseX );
              ui->le_gateway->setText( addr.toString());
              break;
            }
        }
        ui->le_ipAddress->setEnabled(false);
        ui->le_subnetMask->setEnabled(false);
        ui->le_gateway->setEnabled(false);
        ui->le_dns1->setEnabled(false);
        ui->le_dns2->setEnabled(false);
    }
    else
    {   // Static
        char buf1[16];
        sprintf(buf1, "%2u.%2u.%2u.%2u", mNet.ipAddr >> 24, (mNet.ipAddr >> 16) & 0x0FF,
                                         (mNet.ipAddr >> 8) & 0x0FF, mNet.ipAddr & 0x0FF);
        ui->le_ipAddress->setText((QString)buf1);
        sprintf(buf1, "%2u.%2u.%2u.%2u", mNet.subnetMask >> 24, (mNet.subnetMask >> 16) & 0x0FF,
                                         (mNet.subnetMask >> 8) & 0x0FF, mNet.subnetMask & 0x0FF);
        ui->le_subnetMask->setText((QString)buf1);
        sprintf(buf1, "%2u.%2u.%2u.%2u", mNet.gateway >> 24, (mNet.gateway >> 16) & 0x0FF,
                                         (mNet.gateway >> 8) & 0x0FF, mNet.gateway & 0x0FF);
        ui->le_gateway->setText((QString)buf1);
        sprintf(buf1, "%2u.%2u.%2u.%2u", mNet.dns1 >> 24, (mNet.dns1 >> 16) & 0x0FF,
                                         (mNet.dns1 >> 8) & 0x0FF, mNet.dns1 & 0x0FF);
        ui->le_dns1->setText((QString)buf1);
        sprintf(buf1, "%2u.%2u.%2u.%2u", mNet.dns2 >> 24, (mNet.dns2 >> 16) & 0x0FF,
                                         (mNet.dns2 >> 8) & 0x0FF, mNet.dns2 & 0x0FF);
        ui->le_dns2->setText((QString)buf1);
    }
    mOldIpAddr = ui->le_ipAddress->text();
    mOldSubnetMask = ui->le_subnetMask->text();
    mOldGateway = ui->le_gateway->text();
    mOldDns1 = ui->le_dns1->text();
    mOldDns2 = ui->le_dns2->text();

    // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
    ui->pb_type->setEnabled(false);
    ui->label->setEnabled(false);
}
 
 void enetSetup::toggleValue(int cmd, int idx, int)
{
    switch (cmd)
    {
        case CMD_ENET_SETTINGS:
            baseMenu::toggleValue(cmd, idx);
            break;
        default:
            break;
    }
    return;
}

void enetSetup::on_pb_type_clicked()
{
    if (++mNetTypeIndex >= m_enetTypeList.size())
    {
        mNetTypeIndex = 0;
    }
    ui->pb_type->setText(m_enetTypeList.at(mNetTypeIndex));
    if (!mNetTypeIndex)
    {   // DHCP
        ui->le_ipAddress->setEnabled(false);
        ui->le_subnetMask->setEnabled(false);
        ui->le_gateway->setEnabled(false);
        ui->le_dns1->setEnabled(false);
        ui->le_dns2->setEnabled(false);
    }
    else
    {   // Static
        ui->le_ipAddress->setEnabled(true);
        ui->le_subnetMask->setEnabled(true);
        ui->le_gateway->setEnabled(true);
        ui->le_dns1->setEnabled(true);
        ui->le_dns2->setEnabled(true);
    }
}

#ifndef ENET_SETUP_H
#define ENET_SETUP_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class enetSetup;
}

class enetSetup : public baseMenu
{
    Q_OBJECT

public:
    explicit enetSetup(QWidget *parent = 0);
    ~enetSetup();

    void toggleValue(int, int, int f=0);

private slots:
    void on_pb_type_clicked();

private:
    Ui::enetSetup *ui;

    void initLists();
    void setInittoggleValues();
    void buildHashTables();

    QStringList m_enetTypeList;
    int mNetTypeIndex;

    Network mNet;
    QString mOldIpAddr;
    QString mOldSubnetMask;
    QString mOldGateway;
    QString mOldDns1;
    QString mOldDns2;
};

#endif // ENET_SETUP_H

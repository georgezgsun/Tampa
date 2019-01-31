#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class wifiSetup;
}

class wifiSetup : public baseMenu
{
    Q_OBJECT

public:
    explicit wifiSetup(QWidget *parent = 0);
    ~wifiSetup();

    void toggleValue(int, int, int f=0);

private:
    Ui::wifiSetup *ui;

    void initLists();
    void setInittoggleValues();

};

#endif // WIFI_SETUP_H

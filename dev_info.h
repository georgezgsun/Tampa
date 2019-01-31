#ifndef DEV_INFO_H
#define DEV_INFO_H

#include <QWidget>

#include "base_menu.h"

namespace Ui {
class devInfo;
}

class devInfo : public baseMenu
{
    Q_OBJECT

public:
    explicit devInfo(QWidget *parent = 0);
    ~devInfo();

private:
    Ui::devInfo *ui;

    void initLists() { };
    void setInittoggleValues();
};

#endif // DEV_INFO_H

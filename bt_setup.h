#ifndef BT_SETUP_H
#define BT_SETUP_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class BTSetup;
}

class BTSetup : public baseMenu
{
    Q_OBJECT

public:
    explicit BTSetup(QWidget *parent = 0);
    ~BTSetup();

    void toggleValue(int, int, int f=0);

private:
    Ui::BTSetup *ui;

    void initLists();
    void setInittoggleValues();
};

#endif // BT_SETUP_H

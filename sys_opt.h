#ifndef SYS_OPT_H
#define SYS_OPT_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class sysOpt;
}

class sysOpt : public baseMenu
{
    Q_OBJECT

public:
    explicit sysOpt(QWidget *parent = 0);
    ~sysOpt();

    void toggleValue(int, int, int f=0);

private slots:

private:
    Ui::sysOpt *ui;

    void initLists();
    void setInittoggleValues();
};

#endif // SYS_OPT_H

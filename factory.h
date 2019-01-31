#ifndef FACTORY_H
#define FACTORY_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class factory;
}

class factory : public baseMenu
{
    Q_OBJECT

public:
    explicit factory(QWidget *parent = 0);
    ~factory();

    //void toggleValue(int, int);

private:
    Ui::factory *ui;

    void initLists();
    void setInittoggleValues();
    void buildHashTables();

    QStringList m_refClkList;
    int m_refClkIdx;
};

#endif // FACTORY_H

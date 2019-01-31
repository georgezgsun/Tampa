#ifndef SERVICE_H
#define SERVICE_H

#include <QWidget>
#include <QPushButton>
#include "global.h"
#include "base_menu.h"

namespace Ui {
class service;
}

class service : public baseMenu
{
    Q_OBJECT

public:
    explicit service(QWidget *parent = 0);
    ~service();

    void toggleValue(int cmd, int idx, int f=0);

private slots:

    void on_pb_refClock_clicked();

private:
    Ui::service *ui;

    void initLists();
    void setInittoggleValues();
    void buildHashTables();

    QStringList m_refClockList;
    int m_refClockIndex;

};

#endif // SERVICE_H

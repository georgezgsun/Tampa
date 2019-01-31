#ifndef USER_ACCESS_H
#define USER_ACCESS_H

#include <QWidget>
#include <QPushButton>
//#include <QList>
#include "global.h"
#include "base_menu.h"

namespace Ui {
class userAccess;
}

class userAccess : public baseMenu
{
    Q_OBJECT

public:
    explicit userAccess(QWidget *parent = 0);
    ~userAccess();

    void toggleValue(int cmd, int idx, int f=0);

private slots:

    void on_pb_admin_clicked();

    void on_pb_transfer_clicked();

    void on_pb_delete_clicked();

private:
    Ui::userAccess *ui;

    void initLists();
    void buildHashTables();
    void setInittoggleValues();

    //values for toggle fields, some use m_onOffList
    QStringList m_adminList;
    QStringList m_transferList;
    QStringList m_deleteList;

    Administration mAdmin;
    Administration mOldAdmin;
};

#endif // USER_ACCESS_H

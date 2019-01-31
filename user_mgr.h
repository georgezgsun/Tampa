#ifndef USER_MGR_H
#define USER_MGR_H

#include <QWidget>

#include "base_menu.h"

class QListWidgetItem;

namespace Ui {
class userMgr;
}

class userMgr : public baseMenu
{
    Q_OBJECT

public:
    explicit userMgr(Users *u, widgetKeyBoard *vkb, QWidget *parent = 0);
    ~userMgr();

    void toggleValue(int, int);
    QString& getEditUserLogin() { return m_editUserLogin; }

    //reimplement virutal slots
    void exeUpSelect();
    void exeDownSelect();
    void refreshData();

private slots:
    void usersItemClicked(QListWidgetItem*);
    void setCmd();

private:
    Ui::userMgr *ui;

    void initLists();
    void setInittoggleValues();

    void updateUsersFromDB();
    QString m_editUserLogin;    //login name being edited
    bool m_lwFocused;       //list widget in focus

    void setLWFocus();      //set the List Widget's focus when the LW is selected
    void enableButtons(bool);
};

#endif // USER_MGR_H

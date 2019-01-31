#ifndef EDIT_USER_H
#define EDIT_USER_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class editUser;
}

class editUser : public baseMenu
{
    Q_OBJECT

public:
    explicit editUser(int type, QString& editLogin, QWidget *parent = 0);
    ~editUser();

    void toggleValue(int, int, int f=0);

private slots:
    virtual void timerHit();
    void saveUser();
    void cancelEdit();

private:
    Ui::editUser *ui;

    void initLists();
    void setInittoggleValues();

    int m_type;     // add or edit user

    QString m_editLogin;        //the login name being edited
    struct Users m_editUser;    //user being edited (or added)
    void fillEditData();

};

#endif // EDIT_USER_H

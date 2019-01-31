#ifndef PASSWORD_H
#define PASSWORD_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class password;
}

class password : public baseMenu
{
    Q_OBJECT

public:
    explicit password(QWidget *parent = 0);
    ~password();
    void init(int);
    void setPushButton(QPushButton *newPb) {mPb = newPb;}
    void closeVKB();
    void toggleVKB(vkILine *l);
    void focusLine(vkILine *l);

private slots:
    void on_pb_exit_clicked();
    void lb_keypad_clicked(vkILine*);
    void hideVKB();

private:
    Ui::password *ui;

    QPushButton *mPb;
    int mMenuType;
    vkILine m_passHolder;   //temporary hold the password

    void initLists(){};
    void openVKB(vkILine *l);
};

#endif // PASSWORD_H

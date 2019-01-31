#ifndef ADMIN_H
#define ADMIN_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class admin;
}

class admin : public baseMenu
{
    Q_OBJECT

public:
    explicit admin(QWidget *parent = 0);
    ~admin();

    QPushButton *getButton(int cmdIndex);

private slots:
    void on_pb_loadSettings_clicked();

    void on_pb_saveSettings_clicked();

private:
    Ui::admin *ui;

    void initLists();

};

#endif // ADMIN_H

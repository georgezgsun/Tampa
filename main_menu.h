#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <QWidget>
#include "global.h"
#include <QPushButton>
//#include <QDialog>
#include <QList>
#include "base_menu.h"

namespace Ui {
class mainMenu;
}

class mainMenu : public baseMenu
//class mainMenu : public QDialog
{
    Q_OBJECT

signals:

public:
    explicit mainMenu(QWidget *parent = 0);
    ~mainMenu();

    //set user ID and name
    void setUser (struct Users *u);
	//	bool eventFilter (QObject * object, QEvent *event);

private slots:

    void on_tb_logout_clicked();

private:
    Ui::mainMenu *ui;

    void initLists();
};

#endif // MAIN_MENU_H

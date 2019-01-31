#ifndef MENU_VIEW_H
#define MENU_VIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QMainWindow>
#include <QTimer>
#include <QString>
#include <QLineEdit>
#include <QStack>
#include "global.h"
#include "base_menu.h"
#include "main_menu.h"
#include "widgetKeyBoard.h"
#include "vkiline.h"
#include "user_db.h"


//#define FULL_VIEW_WITH      480
#define MENU_VIEW_HEIGHT        270
#define MENU_VIEW_WIDTH         340     //480
#define MENU_SCENE_WIDTH        MENU_VIEW_WIDTH


namespace Ui {
  class menuView;
}

class menuView : public QWidget
{
    Q_OBJECT

public:
    explicit menuView(widgetKeyBoard *vkb, struct Users *u, QWidget *parent = 0);
    ~menuView();

    struct Users * user() { return m_user; }
	bool eventFilter (QObject * object, QEvent *event);

private:
    Ui::menuView *ui;

protected:
    void closeEvent(QCloseEvent *);

signals:
    void closeMenuView();

public slots:
    //void setCommand(int cmd, int index);
    void openVKB(vkILine *l);     //prefered api to open a vkb
    void toggleVKB(vkILine *l);
    void focusLine(vkILine *l);
    void closeVKB();

    void selectPressed();
    void exitPressed();

    void setSelectButton(bool b);  //eneble / diable the SELECT button, requested by sub-menus

    void mapHardButtons();
    void deleteAccMenu();

private slots:


private:

    int m_command;
    int m_prevCmd;  //command processed
    int m_index;    //from current menu, index of the widget in m_list

    baseMenu *m_accMenu;    //for camera, playback etc. STATE_ACC_MENU
    baseMenu *m_currMenu;
    QRect *m_sceneRect;
    QGraphicsView *m_view;
    QGraphicsScene *m_currScene;
    QGraphicsProxyWidget *m_currProxyWidget;
    QStack <QGraphicsProxyWidget *> m_menuStack;
    QStack < int > stateStack;

    //saved widget/scene numbers
    QPoint m_point;
    int m_height;
    int m_width;

    widgetKeyBoard *m_vkb;
    QString m_userName;      //current login user: "FirstName LastName"
    int m_userID;            //TODO: may not needed
    struct Users *m_user;    //current login user

    int initVariables();
    void connectSignals();

    int pushMenuStack();
    QGraphicsProxyWidget* popMenuStatck();
    void setMenuViewCommon();

    //open Main Menu's sub-menu
    void openSubMenu(baseMenu *);
};

#endif // MENU_VIEW_H

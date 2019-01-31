#ifndef BASE_MENU_H
#define BASE_MENU_H

#include <QWidget>
#include <QList>
#include <QStringList>
#include <QHash>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include "global.h"
#include "vkiline.h"
#include "user_db.h"
#include "widgetKeyBoard.h"

class baseMenu : public QWidget
{
    Q_OBJECT
public:
    explicit baseMenu(QWidget *parent = 0);

    int command() { return m_command; }
    int listIndex() { return m_listIndex; }
    int updateState();

    void setVKB (widgetKeyBoard *vkb) { m_vkb = vkb; }

    virtual void toggleValue(int, int, int flag = 0);
  //  virtual void setInitFocus();
    virtual void setUser(struct Users *u) { m_user = u; }
    virtual void refreshData() { return; }
    virtual void exeCancelClick() { return; }

signals:
    void selectChanged();
    void requestVKB(vkILine *);
    void enableSelectButton(bool b);
    void exitMenu();        //exit the current menu, move to upper level
    void reMapHardButtons();
    void delAccMenu();

protected slots:
    virtual void updateIndexAndSelect();   //TODO: not used
    void setCmd(vkILine *l);    //from line editor
    virtual void setCmd();              //from push button
    virtual void timerHit() { return; }  //each menu implement its own timer routine

protected:
    void setIndexAndCmd(QWidget *w);

public slots:
    virtual void setInitFocus();


    virtual void exeUpSelect();
    virtual void exeDownSelect();
    virtual void saveSettings() { };
    virtual void restoreSettings() { };

protected:
    virtual void initLists() = 0;

    //the following two lists shares the same index (mlistIndex),
    //so list items must be in exactly the same order.
    QList <QWidget *> m_list;
    QList <int> m_cmdList;
    int m_listIndex;
    int m_prevListIndex;
    int m_command;
    QHash <QWidget *, int> m_hashWidgetToIdx;   //use widget to get index in the list

    void buildWIHashTable();    //build widget to index hash table

    //for DB Users
    QString m_userName;         //TODO: may not needed
    int m_userID;               //TODO: may not needed
    struct Users *m_user;       //TODO: may not needed, keep current login user in menuView
	QTimer *m_baseTimer;

    widgetKeyBoard *m_vkb;

    //for toggle values
    QHash <int, QStringList *> m_hashValueList;   //[cmd, QStringList <toggleValues>];
    QHash <int, int*> m_hashValueIndex;         // [cmd, m_toggleValueIndex];
    QStringList m_onOffList;                    //shared for all fields

    virtual void buildHashTables() { };
    virtual void setInittoggleValues() { };

    //other functions
    void setListIndex();    //TODO: not used
    void updateSelect();
    void connectWidgetSigs();   //connect the signal of widgets for selection update
    void disconnectWidgetSigs();   //disconnect the signal of widgets for selection update
    void setUserID(int id) { m_userID = id; }
    void setUserName(QString& name) { m_userName = name; }

    void setCmd(QWidget *w);    //base function for setCmd() slots

};

#endif // BASE_MENU_H

#ifndef LOC_SAVE_H
#define LOC_SAVE_H

#include <QWidget>
#include <QListWidgetItem>
#include "base_menu.h"

namespace Ui {
class locSave;
}

class locSave : public baseMenu
{
    Q_OBJECT

public:
    explicit locSave(int type, QWidget *parent = 0);
    ~locSave();

    void toggleValue(int, int, int f=0);
    void exeUpSelect();
    void exeDownSelect();
    void exeCancelClick();

private slots:
    void locItemClicked(QListWidgetItem *);

private:
    Ui::locSave *ui;

    int m_menuType;  //save or load
    Location m_locs[MAX_LOCATION_ENTRIES];
    Location m_backupLoc;        //the location replace by save
    Location m_backupTransitLoc; //the locatiooon to be saved
    bool m_lwFocused;       //list widget in focus

    void initLists();
    void setInittoggleValues();

    int loadLocsFromDB();

    void setLWFocus();      //set the List Widget's focus when the LW is selected
    void enableButtons(bool);
};

#endif // LOC_SAVE_H

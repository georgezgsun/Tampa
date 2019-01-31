#ifndef SERV_OPT_H
#define SERV_OPT_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class servOpt;
}

class servOpt : public baseMenu
{
    Q_OBJECT

public:
    explicit servOpt(int type, QWidget *parent = 0);
    ~servOpt();

private slots:
   void on_pb_on1_clicked();

   void on_pb_yes1_clicked();

private:
    Ui::servOpt *ui;

    int m_menuType;       //from Factory Menu or Service Menu

    void initLists();
    void setInittoggleValues();
    void buildHashTables();

    int m_spdTrigOnIdx;
    int m_spdTrigYesIdx;
    int m_zoneModeOnIdx;
    int m_zoneModeYesIdx;
    int m_tdModeOnIdx;
    int m_tdModeYesIdx;
};

#endif // SERV_OPT_H

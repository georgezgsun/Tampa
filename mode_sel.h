#ifndef MODE_SEL_H
#define MODE_SEL_H

#include <QWidget>
#include "base_menu.h"
#include "Lidar.h"

namespace Ui {
class modeSel;
}

class modeSel : public baseMenu
{
    Q_OBJECT

public:
    explicit modeSel(QWidget *parent = 0);
    ~modeSel();

    void toggleValue(int, int, int f=0);

private slots:

    void on_cb_range_stateChanged(int arg1);

    void on_cb_autoObs_stateChanged(int arg1);

    void on_cb_zone_stateChanged(int arg1);

    void on_cb_incWx_stateChanged(int arg1);

    void on_cb_singleShot_stateChanged(int arg1);

    void on_cb_ftc_stateChanged(int arg1);

    void on_cb_logChase_stateChanged(int arg1);

    void on_cb_logStats_stateChanged(int arg1);

private:
    Ui::modeSel *ui;

    void initLists();

    void buildHashTables();
    void setInittoggleValues();
    void enterOperMode(int mode1, bool status);
    // Steven Cao, 12/4/2017
    LIDAR *mpLidar;
    LIDAR mLidar;

};

#endif // MODE_SEL_H

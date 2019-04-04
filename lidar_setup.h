#ifndef LIDAR_SETUP_H
#define LIDAR_SETUP_H

#include <QWidget>
#include "base_menu.h"
#include "Lidar.h"

namespace Ui {
class lidarSetup;
}

class lidarSetup : public baseMenu
{
    Q_OBJECT

public:
    explicit lidarSetup(QWidget *parent = 0);
    ~lidarSetup();

    void toggleValue(int cmd, int idx, int f=0);

private slots:

    void on_pb_power_clicked();

    void on_pb_units_clicked();

    void on_pb_tilt_clicked();

    void on_pb_backlight_clicked();

    void on_cb_speedTenths_stateChanged(int arg1);

    void on_cb_rangeTenths_stateChanged(int arg1);

    void on_cb_antiJamming_stateChanged(int arg1);

    void on_cb_autoTrigger_stateChanged(int arg1);

    void on_pb_targets_clicked();

private:
    Ui::lidarSetup *ui;

    void initLists();

    void buildHashTables();
    void setInittoggleValues();

    //values for toggle fields, some use m_onOffList
    QStringList m_unitsList;
    QStringList m_tiltList;
    QStringList m_backlightList;
    QStringList m_powerList;
    unsigned int m_unitsIndex;
    unsigned int m_tiltIndex;
    unsigned int m_backlightIndex;
    unsigned int m_powerIndex;
    // Steven Cao, 11/30/2017
    LIDAR *mpLidar;
    LIDAR mLidar;
    SysConfig mConf;
#ifdef HH1
    unsigned int mOldSpeedTenth;
    unsigned int mOldRangeTenth;
    unsigned int mOldAutoTrigger;
    unsigned int mOldAudioAlert;
    QStringList m_targetSortList;
    unsigned int m_targetSortIndex;
    unsigned int mOldUnitIndex;
#endif
    unsigned int mOldTiltIndex;
    unsigned int mOldBacklightIndex;
    unsigned int mOldPowerIndex;
};

#endif // LIDAR_SETUP_H

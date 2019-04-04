#ifndef CAMERA_SETUP_H
#define CAMERA_SETUP_H

#include <QWidget>
#include <QStringList>
#include <QLabel>
#include "base_menu.h"

#define FOCUS_MIN   1
#define FOCUS_MAX   1000
#define GAIN_MIN    0
#define GAIN_MAX    20

namespace Ui {
class cameraSetup;
}

class cameraSetup : public baseMenu
{
    Q_OBJECT

public:
    explicit cameraSetup(QWidget *parent = 0);
    ~cameraSetup();

    void toggleValue(int cmd, int idx, int f=0);
    void setInitFocus();
private slots:
    void onPbSelectClicked();
    void onPbExitClicked();
    void onPbSelectPressed();
    void onPbUpClicked();
    void onPbDownClicked();
    void onPbExitPressed();

    void on_pb_modeVal_clicked();

    void on_pb_shutterVal_clicked();

    void on_pb_gainVal_clicked();

    void on_pb_evVal_clicked();

private:
    Ui::cameraSetup *ui;

    void initLists();
    void setInittoggleValues();
    void buildHashTables();

    QLabel *m_buddy;        //the current nameing lable, such as "ZOOM", "FOCUS"...
    QList <QWidget *> m_buddyList;  //the nameing label list
    QString m_redText;
    QString m_blueText;
    QStringList m_modeList;
    QStringList m_shutterList;
    QStringList m_evList;
    QStringList m_gainList;
    int m_modeIdx;
    int m_shutterIdx;
    int m_evIdx;
    int m_gainIdx;

    struct CameraSetting m_camSetting;
    void querySetting();
    void setDisplay();
    int updateSetting();
};

#endif // CAMERA_SETUP_H

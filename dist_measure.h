#ifndef DIST_MEASURE_H
#define DIST_MEASURE_H

#include <QWidget>
#include <QGraphicsProxyWidget>
#include "base_menu.h"
#include "Lidar.h"

namespace Ui {
class distMeasure;
}

class distMeasure : public baseMenu
{
    Q_OBJECT

public:
    explicit distMeasure(QWidget *parent = 0);
    ~distMeasure();
    void init(int);

public slots:
    void closeVKB();
    void toggleVKB(vkILine *l);
    void focusLine(vkILine *l);

private slots:
    void lb_keypad_clicked(vkILine*);

    void on_pb_enter_clicked();

    void on_pb_exit_clicked();

    void timerHit();
    void setCmd() {};
    void hideVKB();

    void on_pb_value_clicked();

private:
    Ui::distMeasure *ui;

    void initLists(){};

    void setInittoggleValues(){};
    void openVKB(vkILine *l);
    void drawLaserSquare(void);
    void ftcMeasure(void);

    int mMenuType;

    QString mDistance;
    vkILine m_distHolder;  //temporary hold the distance input
    float   mDist1;
    float   mDist2;
    float   mMeasure;
    bool    mDistFlag;     // Dist 1 or 2
    bool    mFtcShoot;     // true after Lane distanct is taken in FTC mode
    bool    mFtcTrigger;   // true if Trigger is pulled after 'mFtcShoot' = true
    bool    mFtcDataTrue;  // true after all FTC operations
    int     mState;

    // radar data
    int     mLidarLinked;  // Is Lidar connected?
    float   mRange;     // Current range
};

#endif // DIST_MEASURE_H

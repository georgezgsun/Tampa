#ifndef CALIBRATEMAG3110_H
#define CALIBRATEMAG3110_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class calibrateMag3110;
}

class calibrateMag3110 : public baseMenu
{
    Q_OBJECT

public:
    explicit calibrateMag3110(QWidget *parent = 0);
    ~calibrateMag3110();

    //void toggleValue(int, int);

private slots:
  void start(void);
  void stop(void);
  void calTimerHit();
	
private:
    Ui::calibrateMag3110 *ui;

    void initLists();
    void setInittoggleValues();
    void buildHashTables();

    QStringList m_refClkList;
    int m_refClkIdx;
};

#endif // CALIBRATEMAG3110_H

#ifndef RADARPARAMS_H
#define RADARPARAMS_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class radarParams;
}

class radarParams : public baseMenu
{
   Q_OBJECT

public:
   explicit radarParams(QWidget *parent = 0);
   ~radarParams();

   void toggleValue(int, int, int f=0);

private slots:
   void on_pb_radarPower_clicked();

   void on_pb_radarSensitivity_clicked();

   void checkFreqRange();
   void checkBandwidthRange();
private:
   Ui::radarParams *ui;

   void initLists();
   void setInittoggleValues();
   void buildHashTables();

   SysConfig mConf;
   float mFrequency;
   float mBandwidth;

   QStringList mPowerList;
   QStringList mSensitivityList;
   unsigned int mPowerIndex;
   unsigned int mSensitivityIndex;
};

#endif // RADARPARAMS_H

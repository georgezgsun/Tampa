#ifndef TILTPARAMS_H
#define TILTPARAMS_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class tiltParams;
}

class tiltParams : public baseMenu
{
   Q_OBJECT

public:
   explicit tiltParams(QWidget *parent = 0);
   ~tiltParams();

   void toggleValue(int, int, int f=0);

private slots:
   void checkMagXmax();
   void checkMagXmin();
   void checkMagYmax();
   void checkMagYmin();
   void checkMagZmax();
   void checkMagZmin();
   void checkMagThetaX();
   void checkMagThetaY();
   void checkMagThetaZ();
   void checkAccXmax();
   void checkAccXmin();
   void checkAccYmax();
   void checkAccYmin();
   void checkAccZmax();
   void checkAccZmin();
   void checkAccThetaX();
   void checkAccThetaY();
   void checkAccThetaZ();

private:
   Ui::tiltParams *ui;

   void initLists();
   void setInittoggleValues();
   void buildHashTables(){}

   Sensor mSensor;
   float mMagXmax;
   float mMagXmin;
   float mMagYmax;
   float mMagYmin;
   float mMagZmax;
   float mMagZmin;
   float mMagThetaX;
   float mMagThetaY;
   float mMagThetaZ;
   float mAccXmax;
   float mAccXmin;
   float mAccYmax;
   float mAccYmin;
   float mAccZmax;
   float mAccZmin;
   float mAccThetaX;
   float mAccThetaY;
   float mAccThetaZ;
};

#endif // TILTPARAMS_H

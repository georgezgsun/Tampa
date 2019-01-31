#ifndef SELFTEST_H
#define SELFTEST_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class selfTest;
}

class selfTest : public baseMenu
{
   Q_OBJECT

public:
   explicit selfTest(QWidget *parent = 0);
   ~selfTest();

private slots:
   void on_pb_back_clicked();

private:
   Ui::selfTest *ui;

   void initLists(){}

   void setInittoggleValues(){}
};

#endif // SELFTEST_H

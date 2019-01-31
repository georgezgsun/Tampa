#ifndef ILLUMINATOR_H
#define ILLUMINATOR_H

#include <QWidget>
#include "base_menu.h"
#include "utils.h"

namespace Ui {
   class illuminator;
}

class illuminator : public baseMenu
{
   Q_OBJECT

public:
   explicit illuminator(QWidget *parent = 0);
   ~illuminator();

private slots:
   void on_pb_exit_clicked();

   void on_pb_ircut_clicked();

   void on_pb_iris_clicked();

   void on_pb_2amode_clicked();

   void on_pb_shutter_clicked();

   void on_pb_gain_clicked();

   void on_pb_box_clicked();

private:
   Ui::illuminator *ui;

   void initLists(){};
   void setInittoggleValues();
   void buildHashTables();

   QStringList mIrcutList;
   QStringList mIrisList;
   QStringList m2aModeList;
   QStringList mShutterList;
   QStringList mGainList;
   QStringList mBoxList;
   ILLUMINATOR mIlluminator;
};

#endif // ILLUMINATOR_H

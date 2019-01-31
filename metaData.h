#ifndef METADATA_H
#define METADATA_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
   class metaData;
}

class metaData : public baseMenu
{
   Q_OBJECT

public:
   explicit metaData(QWidget *parent = 0);
   ~metaData();

   void toggleValue(int cmd, int idx, int flag=0);

private slots:
   void on_pb_line_clicked();

   void on_cb_deviceID_stateChanged(int arg1);

   void on_cb_userID_stateChanged(int arg1);

   void on_cb_recordNumber_stateChanged(int arg1);

   void on_cb_speedLimit_stateChanged(int arg1);

   void on_cb_speed_stateChanged(int arg1);

   void on_cb_distance_stateChanged(int arg1);

   void on_cb_location_stateChanged(int arg1);

   void on_pb_position_clicked();

private:
   Ui::metaData *ui;

   void initLists();

   QString mQs1;     // Metadata line 1
   QString mQs2;     // Metadata line 2
   Administration mAdmin;
   Administration mOldAdmin;
   bool mLine1;
   int  mPosition;
   bool mInitialized;
};

#endif // METADATA_H

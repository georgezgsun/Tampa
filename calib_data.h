#ifndef CALIB_DATA_H
#define CALIB_DATA_H

#include <QWidget>
#include <QDateTimeEdit>
#include "base_menu.h"

namespace Ui {
   class calibData;
}

class calibData : public baseMenu
{
   Q_OBJECT

public:
   explicit calibData(int type, QWidget *parent = 0);
   ~calibData();

   void toggleValue(int, int, int f=0){ UNUSED(f); }

public slots:
   void closeVKB();
   void toggleVKB(vkILine *l);
   void focusLine(vkILine *l);

private slots:
   void vkeyboard_clicked(vkILine* vkl);
   void hideVKB();

   void on_pb_exit_clicked();

private:
   Ui::calibData *ui;

   int m_menuType;  // SERVICE or FACTATORY

   void initLists(){}
   void setInittoggleValues();
   void openVKB(vkILine *l);
   void DisplayDate(QString &date1,  QDateEdit *pQd);
   void ConvertDate(QDate &da1, QString &outDate);

   Administration mAdmin;
   vkILine mKeyInputs;   //temporary hold the keyboard input
};

#endif // CALIB_DATA_H

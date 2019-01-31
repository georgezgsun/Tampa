#ifndef FOCUS_H
#define FOCUS_H

#include <QWidget>
#include "base_menu.h"
#include "utils.h"

namespace Ui {
   class focus;
}

class focus : public baseMenu
{
   Q_OBJECT

public:
   static focus& get()
   {
      static focus instance;
      return instance;
   }

   explicit focus(QWidget *parent = 0);
   ~focus();
   QTimer *m_Timer;

private slots:
  void again();

private:
   Ui::focus *ui;
   void initLists(){};

};

#endif // FOCUS_H

#ifndef PRINTTICKET_H
#define PRINTTICKET_H

#include <QWidget>


class printTicket
{
public:
   static printTicket& get()
   {
      static printTicket instance;
      return instance;
   }

   void print( QString fileName );

private:
   printTicket();
   ~printTicket();

};

#endif // PRINTTICKET_H

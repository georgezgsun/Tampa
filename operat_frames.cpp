#include "operat_frames.h"
#include "ui_operat_frames.h"
#include <QDebug>
#include "global.h"
#include "state.h"
#include "hardButtons.h"
#include "utils.h"
#include "ColdFireCommands.h"

#ifdef IS_TI_ARM
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif


OperatFrames::OperatFrames(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::OperatFrames)
{
    ui->setupUi(this);

    mLoop1 = 0;

    setCurrentIndex(OPR_SCREEN);
    state::get().setState(STATE_OPERATING );
    ui->lb_sd1->installEventFilter(this);
    ui->lb_sd2->installEventFilter(this);
    ui->lb_sd3->installEventFilter(this);
    ui->lb_sd4->installEventFilter(this);
    return;
}

OperatFrames::~OperatFrames()
{
   hardButtons& hd = hardButtons::get();
   hd.setHardButtonMap(0, NULL);
   hd.setHardButtonMap(1, NULL);
   hd.setHardButtonMap(2, NULL);
   hd.setHardButtonMap(3, NULL);
   delete ui;
}

bool OperatFrames::eventFilter(QObject *o, QEvent *e)
{
   //for label click
   QLabel *lb = qobject_cast<QLabel *>(o);
   if (lb && e->type() == QEvent::MouseButtonPress)
   {
      emit sig_oprScreenReq(OPR_MENU_SCREEN);
      return true;
   }
   return false;
}

void OperatFrames::setSpeedDistance(int index, QString &speed, QString &distance)
{
    if (currentIndex() != OPR_SCREEN)
        return;

    // Update the screen every 3rd time to save CPU power
    if (mLoop1 >= 12)
       mLoop1 = 0;
    if (mLoop1++ >= 4)
      return;

    QString s = speed + QString("\n") + distance;

    switch (index) {
    case OPR_SDLABEL1:
        ui->lb_sd1->setText(s);
        break;
    case OPR_SDLABEL2:
        ui->lb_sd2->setText(s);
        break;
    case OPR_SDLABEL3:
        ui->lb_sd3->setText(s);
        break;
    case OPR_SDLABEL4:
        ui->lb_sd4->setText(s);
        break;
    default:
        break;
    }
   // DEBUG() << sty;
}


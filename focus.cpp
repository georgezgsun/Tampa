#include "focus.h"
#include "ui_focus.h"
#include "debug.h"
#include "hardButtons.h"
#include <QPixmap>
#include "ColdFireCommands.h"

extern QElapsedTimer sysTimer;
#ifdef IS_TI_ARM
  //#define FILENAME "/mnt/mmc/ipnc/jsmith/190104_153228_JA001001_00131_1.jpg"
  //#define FILENAME "/mnt/mmc/ipnc/jsmith/tmptmp123456789.jpg"
#define FILENAME "/tmp/tmptmp"
#else
#define FILENAME "/public/tbuckley/JA001001/190104_153228_JA001001_00131_1.jpg"
#endif
#define JPGSIZE_X 4000
#define JPGSIZE_Y 3000
  
#define DISPLAYSCREENSIZE_X 378
#define DISPLAYSCREENSIZE_Y 270

int pic = 1;

focus::focus( QWidget *parent ) :
   baseMenu(parent),
   ui(new Ui::focus)
{
  
  ui->setupUi(this);
  
  this->connectWidgetSigs();
  
  m_Timer = new QTimer(this);

  connect(m_Timer, SIGNAL(timeout()), this, SLOT(again()));

  again();

  m_Timer->start( 1000 );
}

focus::~focus()
{
  delete ui;
}


void focus::again()
{
  QString fileName(FILENAME);
  fileName.append(QString::number(pic++));
  fileName.append(".jpg");
  
  DEBUG() << "Start elapsed time " << sysTimer.elapsed() << "fileName " << fileName;

  Utils& u = Utils::get();
  
  QString filename(fileName);
  u.takePhoto(filename, 0);
  
  
  QFile Fout( fileName);
  
#ifdef IS_TI_ARM
  int cnt = 0;
  while ( 1 ) {
    cnt++;
    if(!Fout.exists()) {       
      if( (cnt % 10) == 0) DEBUG() << "Error";
    } else {
      if( (cnt % 10) == 0) DEBUG() << "File good";
      break;
    }
    sleep(1);
  }
#endif
  
  DEBUG() << "et " << sysTimer.elapsed();
  QPixmap image( fileName );
  DEBUG() << "et " << sysTimer.elapsed();
  
  int x = (JPGSIZE_X / 2) - ( DISPLAYSCREENSIZE_X / 2 );
  int y = (JPGSIZE_Y / 2) - ( DISPLAYSCREENSIZE_Y / 2 );
  int xx = DISPLAYSCREENSIZE_X;
  int yy = DISPLAYSCREENSIZE_Y;
  
  
  QRect myROI(x, y, xx, yy);
  
  QPixmap croppedImage = image.copy(myROI);
  DEBUG() << "et " << sysTimer.elapsed();
  
  ui->picture->setPixmap(croppedImage);
  DEBUG() << "et " << sysTimer.elapsed();

#ifdef JUNK  
#ifdef IS_TI_ARM
  QFile file ( FILENAME );
  file.remove();
  //  DEBUG() << "elapsed time " << sysTimer.elapsed() << " removed " << FILENAME;
#endif
#endif
  
  u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  DEBUG() << "After beep elapsed time " << sysTimer.elapsed();

}

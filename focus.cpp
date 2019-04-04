#include "focus.h"
#include "ui_focus.h"
#include "debug.h"
#include "hardButtons.h"
#include <QPixmap>
#include "ColdFireCommands.h"

int zoom = 0;

focus::focus( QWidget *parent ) :
   baseMenu(parent),
   ui(new Ui::focus)
{
  ui->setupUi(this);
  
  this->connectWidgetSigs();
  
  QGraphicsScene *scene = new QGraphicsScene;
  QGraphicsView *view = this->ui->gv_cam;
  QRect *camRect = new QRect(0, 0, CAM_VIEW_WIDTH, this->height());

  //set background color to black
  QColor c(0, 0, 0, 255);
  QBrush bgB(c);
  view->setVisible(false);
  view->setFixedSize(CAM_VIEW_WIDTH, this->height());
  view->setGeometry(*camRect);
  scene->setBackgroundBrush(bgB);
  scene->setSceneRect(*camRect);
  view->setScene(scene);
  // Draw a crosshair
  QPen p(Qt::red);
  qreal centerX, centerY;
  centerX = CAM_VIEW_WIDTH >> 1;
  centerY = height() >> 1;
  qreal halfLen = centerX / 2;
  scene->addLine(halfLen, centerY, centerX + halfLen, centerY, p);
  scene->addLine(centerX, centerY - 30, centerX, centerY + 30, p);
  scene->update();
  view->setVisible(true);
  view->show();
  connect( ui->pb_exit, SIGNAL(clicked()), this, SLOT(exeExitPressed()) );
  connect(ui->pb_down, SIGNAL(clicked()), this, SLOT(exeDownSelect()));
  connect(ui->pb_up, SIGNAL(clicked()), this, SLOT(exeUpSelect()));
  hardButtons::get().setHardButtonMap( 0, ui->pb_exit );
  hardButtons::get().setHardButtonMap( 1, NULL );
  hardButtons::get().setHardButtonMap( 2, ui->pb_up );
  hardButtons::get().setHardButtonMap( 3, ui->pb_down );
}

focus::~focus()
{
  DEBUG();

  // set zoom to "No zoom"
  (void)Utils::get().sendCmdToCamera(CMD_ZOOM, 0);

  delete ui;
}

void focus::exeUpSelect()
{
  DEBUG() << "zoom is " << zoom;
  
  zoom++;

  if ( zoom > 2) {
    zoom = 2;
  }

  // set zoom to "No zoom"
  (void)Utils::get().sendCmdToCamera(CMD_ZOOM, zoom );
  (void)Utils::get().sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
}

void focus::exeDownSelect()
{
  DEBUG() << "zoom is " << zoom;

  zoom--;

  if ( zoom < 0 ) {
    zoom = 0;
  }

  // set zoom to "No zoom"
  (void)Utils::get().sendCmdToCamera(CMD_ZOOM, zoom );

  (void)Utils::get().sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
}

void focus::exeExitPressed()
{
  DEBUG();
  // set zoom to "No zoom"
  (void)Utils::get().sendCmdToCamera(CMD_ZOOM, 0);
  (void)Utils::get().sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

  close();
}

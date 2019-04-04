#include "play_back.h"
#include "ui_play_back.h"
#include "debug.h"
#include "hardButtons.h"
#include "utils.h"
#include "ColdFireCommands.h"
#include "back_ground.h"
#include "json.h"

saveData violation;

#define INDENT 35
#define LINEHEIGTH 130
#define LINELENGTH 800
#define XMAX 4000
#define YMAX 3000

static void doPrint(QPainter *im, int fntsize, int x, int y, char *buff)
{
  im->setFont ( QFont( "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fntsize ) );
  im->setPen ( Qt::white );
  im->drawText ( QPoint( x,y), buff);  

  return;
}

playBack::playBack(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::playBack)
{
   ui->setupUi(this);

   this->initLists();

//   ui->pb_forward->setEnabled(false);

}

playBack::~playBack()
{
    delete ui;
}

void playBack::initLists()
{
  // Map Hard Button
  hardButtons& hd = hardButtons::get();
  hd.setHardButtonMap( 0, ui->pb_exit);
  hd.setHardButtonMap( 1, ui->pb_exit);
  hd.setHardButtonMap( 2, ui->pb_exit);
  hd.setHardButtonMap( 3, ui->pb_exit);
//  hd.setHardButtonMap( 1, ui->pb_pause);
//  hd.setHardButtonMap( 2, ui->pb_forward);
//  hd.setHardButtonMap( 3, ui->pb_restart);

  ui->pb_forward->hide();
  ui->pb_pause->hide();
  ui->pb_restart->hide();
}

void playBack::setFileName(QString &filename)
{
   mFileName = filename;
   if (mFileName.contains(".jpg") == true)
   {
      ui->pb_pause->setEnabled(false);
   }
}

void playBack::startPlay(void)
{
  // determine if jpeg or video
  QList<QString> ext_list;
  ext_list<<"jpg";
  QFileInfo fi(mFileName);
  QString ext = fi.suffix();

  if (ext_list.contains(ext)){
    //playback JPEG
    ui->pb_pause->setEnabled(false);
    //    ui->pb_forward->setEnabled(false);
    ui->pb_restart->setEnabled(false);
    
    DEBUG() << mFileName;
    
    QPixmap image( mFileName );
    
    // Get data from json file
    // add it to the QPixmap with painter
    char buff[LINELENGTH];
    
    memset((void *)&violation, 0, sizeof(violation));
    
    // get the json file name
    // get and fill in violation struct
    int lastPoint = mFileName.lastIndexOf(".");
    QString jsonFile = mFileName.left(lastPoint);
    jsonFile.append(".json");
    
    //Json::get().getJsonData( jsonFile, &violation );
    Json::get().loadJsonData(Json::get().getJsonObject(jsonFile), &violation );
    
    CCoordTransforms Transforms;
    config_type RadarConfig;
    
    memset((void *)&RadarConfig, 0, sizeof(config_type));
    
    Utils::get().getRadarConfig(&RadarConfig);
    
    RadarConfig.radar_data_is_roadway = false;
    RadarConfig.port = serial;
    RadarConfig.num_to_show = 4;

    // Init the coordinate transformations
    Transforms.InitCoordTransforms(violation.Xs,
				   violation.Zs,
				   violation.Zt,
				   RadarConfig.FOVh * PI/180.0f,      // Convert to radians
				   RadarConfig.FOVv * PI/180.0f);     // Convert to radians
    
    // Relative values cacuated here
    float theta_hs_rel = violation.theta_hs - violation.theta_hs_ref;
    if(theta_hs_rel > PI) theta_hs_rel = PI - theta_hs_rel;
    else if(theta_hs_rel < -PI) theta_hs_rel = PI + theta_hs_rel;
    
    float theta_rs_rel = violation.theta_rs - violation.theta_rs_ref;
    if(theta_rs_rel > PI) theta_rs_rel = PI - theta_rs_rel;
    else if(theta_rs_rel < -PI) theta_rs_rel = PI + theta_rs_rel;
    
    float theta_vs_rel = violation.theta_vs - violation.theta_vs_ref;
    if(theta_vs_rel > PI) theta_vs_rel = PI - theta_vs_rel;
    else if(theta_vs_rel < -PI) theta_vs_rel = PI + theta_vs_rel;
    
    
    Transforms.UpdateSensor(theta_vs_rel, theta_rs_rel, theta_hs_rel);
    
    int targetNum;
    coord_struct Radar_Coords;
    coord_struct Video_Coords[MAX_TARGETS];
    coord_struct Roadway_Coords;
    
    QPainter im( &image );
    
    for( targetNum=0; targetNum < violation.Targets.numTargets; targetNum++ ) {
      RadarTargetResponse_t *target = &violation.Targets.RadarTargets[targetNum];
      
      memset( &Radar_Coords, 0, sizeof(coord_struct));
      memset( &Video_Coords, 0, sizeof(coord_struct));
      memset( &Roadway_Coords, 0, sizeof(coord_struct));
      
      Radar_Coords.type = radar;
      Radar_Coords.X = target->xCoord;
      Radar_Coords.Y = target->yCoord;
      // Since the 3D radar does not measure target height (Z axis), fake it
      //    Radar_Coords.Z = target->zCoord;
      //    Radar_Coords.Z = mpRadarData->Data.RadarOrientation.targetHeight - mpRadarData->Data.RadarOrientation.radarHeight;  // Usually negative
      Radar_Coords.Z = 1.0f - 3.0f;  // Usually negative
      
      Radar_Coords.R = sqrtf(target->xCoord * target->xCoord +
			     target->yCoord * target->yCoord +
			     target->zCoord * target->zCoord);
      
      Radar_Coords.Vx = target->xVelocity;
      Radar_Coords.Vy = target->yVelocity;
      Radar_Coords.Vz = target->zVelocity;
      
      Radar_Coords.V = sqrtf(target->xVelocity * target->xVelocity +
			     target->yVelocity * target->yVelocity +
			     target->zVelocity * target->zVelocity);
      
      Radar_Coords.Theta_Vy = 0.0f;
      Radar_Coords.Theta_Vz = 0.0f;
      Radar_Coords.Ix = 0.0f;
      Radar_Coords.Iz = 0.0f;
      
      Roadway_Coords.type = roadway;
      
      Transforms.Transform(&Radar_Coords, &Roadway_Coords);
      //    printf("Target %d at radar X = %f, Y = %f, Z = %f\n", targetNum, Radar_Coords.X, Radar_Coords.Y, Radar_Coords.Z);
      //    printf("  maps to roadway X = %f, Y = %f, Z = %f\n", Roadway_Coords.X, Roadway_Coords.Y, Roadway_Coords.Z);
      
      Video_Coords[targetNum].type = video;
      Transforms.Transform(&Radar_Coords, &Video_Coords[targetNum]);
      
      float x = ( 0.5 * Video_Coords[targetNum].X + 0.5) * 4000.0;
      float y = (-0.5 * Video_Coords[targetNum].Z + 0.5) * 3000.0;
      
      printf("Target %d speed = %6.2f, distance = %6.2f X = %6.2f, Z = %6.2f XX %6.2f YY %6.2f\n",
	     targetNum, Radar_Coords.V, Radar_Coords.R, Video_Coords[targetNum].X, Video_Coords[targetNum].Z,
	     x,
	     y );
      buff[0] = 0;
      sprintf(buff, "Target %d speed = %6.2f, distance = %6.2f", target->targetId, Radar_Coords.V, Radar_Coords.R);
      doPrint( &im,
	       100,
	       int( INDENT ),
	       int( YMAX - (LINEHEIGTH * (4-targetNum)) ),
	       buff);
      
      //    printf("Target %d maps to video X = %f, Z = %f x %6.2f z %6.2f\n", targetNum, Video_Coords[targetNum].X, Video_Coords[targetNum].Z,
      //	   (0.5 *Video_Coords[targetNum].X + 0.5) * 4000, (-0.5 * Video_Coords[targetNum].Z + 0.5) * 3000 );
      
      buff[0] = 0;
      sprintf(buff, "%d", target->targetId);
      
      doPrint( &im,
	       100,
	       int( x ),
	       int( y ),
	       buff);
    }
    
    // resize the pic
    QPixmap resizedImage = image.scaled( DISPLAYSCREENSIZE_X, DISPLAYSCREENSIZE_Y, Qt::KeepAspectRatio );
    
    //  DEBUG() << "elapsed time " << sysTimer.elapsed() << "ui " << ui;
    ui->picture->setPixmap( resizedImage);
    
  }else{//playback AVI   
#ifdef IS_TI_ARM
    int value = (int)mFileName.toLatin1().data();
    int retv = Utils::get().sendCmdToCamera(CMD_PLAYBACK, value);
    if(retv)
      DEBUG() << "Error: Start Play, ret " << retv;
#else
    // Need way to play video on Tampa VN
#endif
    // else start timer to read metadata file and draw boxes
  }
}

void playBack::on_pb_exit_clicked()
{
  QList<QString> ext_list;
  ext_list<<"jpg";
  QFileInfo fi(mFileName);
  QString ext = fi.suffix();

  if (ext_list.contains(ext)){
  }else{
    // Stop
#ifdef IS_TI_ARM
    Utils& u = Utils::get();
    int retv = u.sendCmdToCamera(CMD_STOPPLAY, NULL);
    if(retv)
      DEBUG() << "Error: Stop Play, ret " << retv;
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
#endif
  }
  close();
  return;
}

void playBack::on_pb_restart_clicked()
{
#ifdef IS_TI_ARM
   Utils& u = Utils::get();
   u.sendCmdToCamera(CMD_STOPPLAY, NULL);
   u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
   ui->pb_pause->setText("PAUSE");
   u.sendCmdToCamera(CMD_PLAYBACK, (int)mFileName.toLatin1().data());
#endif
}

void playBack::on_pb_pause_clicked()
{
#ifdef IS_TI_ARM
   Utils& u = Utils::get();
   if (ui->pb_pause->text() == "PAUSE")
   {  // Pause
      u.sendCmdToCamera(CMD_PAUSEPLAY, NULL);
      ui->pb_pause->setText("RESUME");
   }
   else
   {  // Resume
      u.sendCmdToCamera(CMD_RESUMEPLAY, NULL);
      ui->pb_pause->setText("PAUSE");
   }
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
#endif
}

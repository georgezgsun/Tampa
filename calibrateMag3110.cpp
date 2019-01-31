#include "calibrateMag3110.h"
#include "ui_calibrateMag3110.h"
#include "state.h"
#include "debug.h"
#include "utils.h"
#include "Tilt_Buff.h"
#include "Mag_Buff.h"
#include "TiltSensor/TiltSensor.h"

TiltSensor *pSensor;
QTimer *m_timer;
struct Tilt_Buff *tilt = NULL;
struct Mag_Buff *mag = NULL;
float maxAccX = 0;
float maxAccY = 0;
float maxAccZ = 0;

float minAccX = 0;
float minAccY = 0;
float minAccZ = 0;

float maxMagX = 0;
float maxMagY = 0;
float maxMagZ = 0;

float minMagX = 0;
float minMagY = 0;
float minMagZ = 0;

QString qs1;

calibrateMag3110::calibrateMag3110(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::calibrateMag3110)
{
    ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();
    this->buildHashTables();

    state& v = state::get();
    v.setState(STATE_CALIBRATEMAG3110);
    //    m_listIndex = m_prevListIndex = 0;
    //    m_command = m_cmdList.at(m_listIndex);
    Utils& u = Utils::get();
    tilt = (Tilt_Buff *)u.TILTBuf();
    DEBUG() << "Tilt " << tilt;

    mag = (Mag_Buff *)u.MAGBuf();
    DEBUG() << "Mag " << mag;
   
}

calibrateMag3110::~calibrateMag3110()
{
    delete ui;
}

void calibrateMag3110::initLists()
{
  //    m_list << ui->pb_start
  //		   << ui->pb_stop;

  //    m_cmdList << CMD_FAC_START
  //			  << CMD_FAC_STOP;
	
  //    this->connectWidgetSigs();
  connect( ui->pb_start, SIGNAL(clicked()), this, SLOT(start()));
  connect( ui->pb_stop, SIGNAL(clicked()), this, SLOT(stop()));
}


void calibrateMag3110::setInittoggleValues()
{
//    m_refClkIdx = 0;
}

void calibrateMag3110::buildHashTables()
{
//    m_refClkList << "OFF" << "OUTPUT";

//    m_hashValueList[CMD_FAC_REF_CLK] = &m_refClkList;
//    m_hashValueIndex[CMD_FAC_REF_CLK] = &m_refClkIdx;
}

void calibrateMag3110::start()
{
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(calTimerHit()));
  m_timer->start( 250 );

  // Get Tilt Sensor interface
  //  pSensor = new TiltSensor;
  //  pSensor->init();

  maxAccX = tilt->raw_X_Axis;
  minAccX = tilt->raw_X_Axis;
  
  maxAccY = tilt->raw_Y_Axis;
  minAccY = tilt->raw_Y_Axis;
  
  maxAccZ = tilt->raw_Z_Axis;
  minAccZ = tilt->raw_Z_Axis; 

  maxMagX = mag->raw_X_Axis;
  minMagX = mag->raw_X_Axis;
  
  maxMagY = mag->raw_Y_Axis;
  minMagY = mag->raw_Y_Axis;
  
  maxMagZ = mag->raw_Z_Axis;
  minMagZ = mag->raw_Z_Axis; 
  DEBUG() << "MaxAccX " << maxAccX << "MaxAccY " << maxAccY  << "MaxAccZ " << maxAccZ;
  DEBUG() << "MinAccX " << minAccX << "MinAccY " << minAccY  << "MinAccZ " << minAccZ;
  DEBUG() << "MaxMagX " << maxMagX << "MaxMagY " << maxMagY  << "MaxMagZ " << maxMagZ;
  DEBUG() << "MinMagX " << minMagX << "MinMagY " << minMagY  << "MinMagZ " << minMagZ;

  qs1 = QString::number( maxMagX);
  ui->m_MXMax->setText(qs1);
  
  qs1 = QString::number( minMagX);
  ui->m_MXMin->setText(qs1);

  qs1 = QString::number( maxMagY);
  ui->m_MYMax->setText(qs1);
  qs1 = QString::number( minMagY);
  ui->m_MYMin->setText(qs1);

  qs1 = QString::number( maxMagZ);
  ui->m_MZMax->setText(qs1);
  qs1 = QString::number( minMagZ);
  ui->m_MZMin->setText(qs1);


  qs1 = QString::number( maxAccX);
  ui->m_AXMax->setText(qs1);
  
  qs1 = QString::number( minAccX);
  ui->m_AXMin->setText(qs1);

  qs1 = QString::number( maxAccY);
  ui->m_AYMax->setText(qs1);
  qs1 = QString::number( minAccY);
  ui->m_AYMin->setText(qs1);

  qs1 = QString::number( maxAccZ);
  ui->m_AZMax->setText(qs1);
  qs1 = QString::number( minAccZ);
  ui->m_AZMin->setText(qs1);

  
}

void calibrateMag3110::stop()
{
  m_timer->stop();

#ifdef JUNK
  DEBUG() << "MaxAccX " << maxAccX << "MaxAccY " << maxAccY  << "MaxAccZ " << maxAccZ;
  DEBUG() << "MinAccX " << minAccX << "MinAccY " << minAccY  << "MinAccZ " << minAccZ;
  DEBUG() << "MaxMagX " << maxMagX << "MaxMagY " << maxMagY  << "MaxMagZ " << maxMagZ;
  DEBUG() << "MinMagX " << minMagX << "MinMagY " << minMagY  << "MinMagZ " << minMagZ;
#endif

  // put the data in the db
  Utils & u = Utils::get();
  Sensor & sensorData = u.getSensorFromDB();

  //  i2cDev = sensorData.i2cDevice;
  sensorData.magXmax = maxMagX;
  sensorData.magXmin = minMagX;
  sensorData.magYmax = maxMagY;
  sensorData.magYmin = minMagY;
  sensorData.magZmax = maxMagZ;
  sensorData.magZmin = minMagZ;

  sensorData.accXmax = maxAccX;
  sensorData.accXmin = minAccX;
  sensorData.accYmax = maxAccY;
  sensorData.accYmin = minAccY;
  sensorData.accZmax = maxAccZ;
  sensorData.accZmin = minAccZ;

  u.setSensorInDB( sensorData );
  
  // TODO put the data in stalker.conf
  QProcess process;
  process.start("/bin/mv /usr/local/stalker/stalker.conf /usr/local/stalker/stalker.conf.bak");
  process.waitForFinished(1000);
 
  // Create a new file     
  QFile file("/usr/local/stalker/stalker.conf");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  
#ifdef JUNK
  // take sensorData and write a new /usr/local/stalker/stalker.conf
  DEBUG() << MAG_XMAX << " " << QString::number( sensorData.magXmax );
  DEBUG() << MAG_XMIN << " " << QString::number( sensorData.magXmin );
  DEBUG() << MAG_YMAX << " " << QString::number( sensorData.magYmax );
  DEBUG() << MAG_YMIN << " " << QString::number( sensorData.magYmin );
  DEBUG() << MAG_ZMAX << " " << QString::number( sensorData.magZmax );
  DEBUG() << MAG_ZMIN << " " << QString::number( sensorData.magZmin);

  //    MAG_THETA_X
  //    MAG_THETA_Y
  //    MAG_THETA_Z
  //    ACC_THETA_X
  //    ACC_THETA_Y
  //    ACC_THETA_Z

  DEBUG() << ACC_XMAX << " " << QString::number( sensorData.accXmax );
  DEBUG() << ACC_XMIN << " " << QString::number( sensorData.accXmin );
  DEBUG() << ACC_YMAX << " " << QString::number( sensorData.accYmax );
  DEBUG() << ACC_YMIN << " " << QString::number( sensorData.accYmin );
  DEBUG() << ACC_ZMAX << " " << QString::number( sensorData.accZmax );
  DEBUG() << ACC_ZMIN << " " << QString::number( sensorData.accZmin);
#endif
  
  // take sensorData and write a new /usr/local/stalker/stalker.conf
  out << MAG_XMAX << " " << QString::number( sensorData.magXmax ) << endl;
  out << MAG_XMIN << " " << QString::number( sensorData.magXmin ) << endl;
  out << MAG_YMAX << " " << QString::number( sensorData.magYmax ) << endl;
  out << MAG_YMIN << " " << QString::number( sensorData.magYmin ) << endl;
  out << MAG_ZMAX << " " << QString::number( sensorData.magZmax ) << endl;
  out << MAG_ZMIN << " " << QString::number( sensorData.magZmin) << endl;

  out << ACC_XMAX << " " << QString::number( sensorData.accXmax ) << endl;
  out << ACC_XMIN << " " << QString::number( sensorData.accXmin ) << endl;
  out << ACC_YMAX << " " << QString::number( sensorData.accYmax ) << endl;
  out << ACC_YMIN << " " << QString::number( sensorData.accYmin ) << endl;
  out << ACC_ZMAX << " " << QString::number( sensorData.accZmax ) << endl;
  out << ACC_ZMIN << " " << QString::number( sensorData.accZmin) << endl;


  SysConfig & cfg = u.getConfiguration();

  //  DEBUG() << SERIALNUMBER << " " << cfg.serialNumber;

  out << SERIALNUMBER << " " << cfg.serialNumber << endl;
    
  // optional, as QFile destructor will already do it:
  file.close(); 
}

void calibrateMag3110::calTimerHit()
{
#ifdef JUNK
  DEBUG() << "X " << tilt->raw_X_Axis << " Y " << tilt->raw_Y_Axis << " Z " << tilt->raw_Z_Axis;
  DEBUG() << "X " << mag->raw_X_Axis << " Y " << mag->raw_Y_Axis << " Z " << mag->raw_Z_Axis;
  DEBUG() << "MaxAccX " << maxAccX << "MaxAccY " << maxAccY  << "MaxAccZ " << maxAccZ;
  DEBUG() << "MinAccX " << minAccX << "MinAccY " << minAccY  << "MinAccZ " << minAccZ;
#endif
  
  if( tilt->raw_X_Axis > maxAccX ) {
    maxAccX = tilt->raw_X_Axis;
    DEBUG() << "MaxAccX " << maxAccX;
    qs1 = QString::number( maxAccX);
    ui->m_AXMax->setText(qs1);
  }
  if(tilt->raw_X_Axis < minAccX ) {
    minAccX = tilt->raw_X_Axis;
    DEBUG() << "MinAccX " << minAccX;
    qs1 = QString::number( minAccX);
    ui->m_AXMin->setText(qs1);
  }

  if( tilt->raw_Y_Axis > maxAccY) {
    maxAccY = tilt->raw_Y_Axis;
    DEBUG() << "MaxAccY " << maxAccY;
    qs1 = QString::number( maxAccY);
    ui->m_AYMax->setText(qs1);
  }
  if( tilt->raw_Y_Axis < minAccY ) {
    minAccY = tilt->raw_Y_Axis;
    DEBUG() << "MinAccY " << minAccY;
    qs1 = QString::number( minAccY);
    ui->m_AYMin->setText(qs1);
  }

  if( tilt->raw_Z_Axis > maxAccZ) {
    maxAccZ = tilt->raw_Z_Axis;
    DEBUG() << "MaxAccZ " << maxAccZ;
    qs1 = QString::number( maxAccZ );
    ui->m_AZMax->setText(qs1);
  }
  if( tilt->raw_Z_Axis < minAccZ){
    minAccZ = tilt->raw_Z_Axis;
    DEBUG() << "MinAccZ " << minAccZ;
    qs1 = QString::number( minAccZ );
    ui->m_AZMin->setText(qs1);
  }


  if( mag->raw_X_Axis > maxMagX ) {
    maxMagX = mag->raw_X_Axis;
    DEBUG() << "MaxMagX " << maxMagX;
    qs1 = QString::number( maxMagX);
    ui->m_MXMax->setText(qs1);
  }
  if(mag->raw_X_Axis < minMagX ) {
    minMagX = mag->raw_X_Axis;
    DEBUG() << "MinMagX " << minMagX;
    qs1 = QString::number( minMagX);
    ui->m_MXMin->setText(qs1);
  }

  if( mag->raw_Y_Axis > maxMagY) {
    maxMagY = mag->raw_Y_Axis;
    DEBUG() << "MaxMagY " << maxMagY;
    qs1 = QString::number( maxMagY);
    ui->m_MYMax->setText(qs1);
  }
  if( mag->raw_Y_Axis < minMagY ) {
    minMagY = mag->raw_Y_Axis;
    DEBUG() << "MinMagY " << minMagY;
    qs1 = QString::number( minMagY);
    ui->m_MYMin->setText(qs1);
  }

  if( mag->raw_Z_Axis > maxMagZ) {
    maxMagZ = mag->raw_Z_Axis;
    DEBUG() << "MaxMagZ " << maxMagZ;
    qs1 = QString::number( maxMagZ);
    ui->m_MZMax->setText(qs1);
  }
  if( mag->raw_Z_Axis < minMagZ){
    minMagZ = mag->raw_Z_Axis;
    DEBUG() << "MinMagZ " << minMagZ;
    qs1 = QString::number( minMagZ);
    ui->m_MZMin->setText(qs1);
  }
}


#include "loc_setup.h"
#include "ui_loc_setup.h"
#include "utils.h"
#include <QDebug>
#include "state.h"
#include "utils.h"
#include "debug.h"
#include "Lidar.h"
#include "db_types.h"

locSetup::locSetup(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::locSetup)
{
    ui->setupUi(this);

#ifdef LIDARCAM
    // Steven Cao, 11/29/2017
    Utils& u = Utils::get();
    struct Lidar_Buff *ptr = u.lidarDataBuf();
    LIDAR *pLidar = &(ptr->lidarStruct);
    int units = pLidar->DISPLAY_UNITS;
#else
    Utils& u = Utils::get();
    SysConfig mConf = u.getConfiguration();
    int units = mConf.units;
#endif
    if (units  == 0)
        {
            ui->lb_captSpd_2->setText("MPH");
            ui->lb_spdLmt_2->setText("MPH");
            ui->lb_capDist_2->setText("Feet");
        }
    else if ( units == 1)
    {
        ui->lb_captSpd_2->setText("km/h");
        ui->lb_spdLmt_2->setText("km/h");
        ui->lb_capDist_2->setText("Meters");
    }
    else if (units  == 2)
    {
        ui->lb_captSpd_2->setText("KNOTS");
        ui->lb_spdLmt_2->setText("KNOTS");
        ui->lb_capDist_2->setText("Feet");
    }

    this->initLists();
    this->buildHashTables();
    this->setInittoggleValues();
    //connect(ui->pb_save, SIGNAL(clicked()), this, SLOT(saveSettings()));
    QDate d = QDate::currentDate();
    QTime t = QTime::currentTime();
    this->setDateTime(d, t);

    m_listIndex =
    m_prevListIndex = 1;
    m_command = m_cmdList.at(m_listIndex);
	state& v = state::get();
    v.setState(STATE_LOC_SETUP_MENU);

	locSetupInitTimer();
	
	doGPS();
}

locSetup::~locSetup()
{
    struct Location l;
    Utils& u = Utils::get();
    int retv;

    l.description = ui->le_location->text();
    l.speedLimit = ui->le_spdLmt->text();
    l.captureSpeed = ui->le_captSpd->text();
    l.captureDistance = ui->le_captDist->text();
    l.roadCondition = ui->pb_roadCondition->text();
    l.numberOfLanes = ui->pb_lanes->text().toInt();
    if (l.description.isNull() || l.description.isEmpty())
        l.description = " ";
    if (l.speedLimit.isNull() || l.speedLimit.isEmpty())
        l.speedLimit = "40";
    if (l.captureSpeed.isNull() || l.captureSpeed.isEmpty())
        l.captureSpeed = "45";
    if (l.captureDistance.isNull() || l.captureDistance.isEmpty())
        l.captureDistance = "50";
    if (l.roadCondition.isNull() || l.roadCondition.isEmpty())
        l.roadCondition = "NORMAL";
    if (!l.numberOfLanes)
        l.numberOfLanes = 2;

    //update default location entry
    l.index = CAMS_DEFAULT_INDEX;
    retv = u.db()->updateEntry(TBL_LOCATION, (DBStruct *)&l);

    delete ui;
}

void locSetup::initLists()
{
    m_list << ui->pb_save
           << ui->pb_load
           << ui->pb_camera
           << ui->le_location
           << ui->le_spdLmt
           << ui->le_captSpd
           << ui->le_captDist
           << ui->pb_roadCondition
           << ui->pb_lanes;

    m_cmdList << CMD_LOC_SAVE
           << CMD_LOC_LOAD
           << CMD_LOC_CAMERA
           << CMD_LOC_DESC
           << CMD_SPD_LMT
           << CMD_CAPT_SPD
           << CMD_CAPT_DIST
           << CMD_LOC_ENV
           << CMD_NUM_LANES;

    this->connectWidgetSigs();

#ifdef HH1
   ui->pb_camera->setVisible(false);
#endif
}

void locSetup::buildHashTables()
{
    m_envList << "NORMAL" << "RAIN" << "SLEET" << "SNOW" << "FOG" << "ICE" ;
    m_envIndex = 0;

    m_laneList << "1" << "2" << "3"<< "4";
    m_laneIndex = 0;
    m_hashValueList[CMD_LOC_ENV] = &m_envList;
    m_hashValueIndex[CMD_LOC_ENV] = &m_envIndex;

    m_hashValueList[CMD_NUM_LANES] = &m_laneList;
    m_hashValueIndex[CMD_NUM_LANES] = &m_laneIndex;
}

void locSetup::setInittoggleValues()
{
    struct Location l;
    int retv, flag = 0;
    Utils& u = Utils::get();

    l.index = CAMS_DEFAULT_INDEX;

    retv = u.db()->queryEntry(TBL_LOCATION, (DBStruct *)&l, QRY_BY_KEY);

    if (retv == 1)
    {
        retv = u.db()->getNextEntry(TBL_LOCATION, (DBStruct *)&l);

        if (retv)
        {
            DEBUG() << "Get default location setting failed";
            flag = 1;
        }
        else
        {
            if (!l.description.isNull() && !l.description.isEmpty())
                ui->le_location->setText(l.description);
            ui->le_spdLmt->setText(l.speedLimit);
            retv = m_envList.indexOf(l.roadCondition);
            m_envIndex = (retv >= 0) ? retv : 0;
            ui->pb_roadCondition->setText(m_envList.at(m_envIndex));
            ui->pb_lanes->setText(QString::number(l.numberOfLanes)) ;
            ui->le_captSpd->setText(l.captureSpeed);
            ui->le_captDist->setText(l.captureDistance);
        }
    }
    else if (!retv)
    {
        //add default location setting entry with the default settings
        ui->le_spdLmt->setText("65");
        ui->pb_roadCondition->setText(m_envList.at(m_envIndex));
        ui->pb_lanes->setText("2");
        ui->le_captSpd->setText("70");
        ui->le_captDist->setText("50");
        l.description = " ";
        l.speedLimit = ui->le_spdLmt->text();
        l.captureSpeed = ui->le_captSpd->text();
        l.captureDistance = ui->le_captDist->text();
        l.roadCondition = ui->pb_roadCondition->text();
        l.numberOfLanes = ui->pb_lanes->text().toInt();
        retv = u.db()->addEntry(TBL_LOCATION, (DBStruct *)&l);
        if (retv)
        {
            DEBUG() << "Add default location setting failed";
        }
    }
    else
        flag = 1;

    if (flag)
    {   // Use default
        ui->le_spdLmt->setText("65");
        ui->pb_roadCondition->setText(m_envList.at(m_envIndex));
        ui->pb_lanes->setText("1");
        ui->le_captSpd->setText("70");
        ui->le_captDist->setText("50");
    }

    // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
   ui->pb_camera->setEnabled(false);

}

void locSetup::toggleValue(int cmd, int idx, int /*f*/)
{
    switch (cmd) {
    case CMD_SPD_LMT:
    case CMD_CAPT_SPD:
    case CMD_CAPT_DIST:

       m_vkb->setNumKeyboard();
       break;
    case CMD_LOC_DESC:
    case CMD_LOC_ENV:
        break;
    default:
        baseMenu::toggleValue(cmd, idx);
    }
}

/*
void locSetup::saveSettings()
{
    qDebug() << "saveSettings";

}
*/

void locSetup::setCmd()
{
    QObject *o = QObject::sender();

    //qDebug() << "location setup setCmd called";

    if (o->objectName() == ui->pb_save->objectName())
    {
        /*
         * 19983
        if (ui->le_location->text() == m_currLoc.description
                && ui->le_spdLmt->text() == m_currLoc.speedLimit
                && ui->le_captSpd->text() == m_currLoc.captureSpeed
                && ui->le_roadCondition->text() == m_currLoc.roadCondition
                && ui->le_lanes->text().toInt() == m_currLoc.numberLanes)
            return;
        */

        // Update 'm_currLoc' for 'Save' button
        struct Location &l = m_currLoc;
        Utils& u = Utils::get();

        l.index = 0;
        l.description = ui->le_location->text();
        l.speedLimit = ui->le_spdLmt->text();
        l.captureSpeed = ui->le_captSpd->text();
        l.captureDistance = ui->le_captDist->text();
        l.roadCondition = ui->pb_roadCondition->text();
        l.numberOfLanes = ui->pb_lanes->text().toInt();

        //set transit value
        u.setTransitData(CMD_LOC_SAVE, (DBStruct *)&l);
    }

    baseMenu::setCmd();
}

void locSetup::refreshData()
{
    struct Location& l = m_currLoc;
    Utils& u = Utils::get();
    // Update 'm_currLoc' here. Steven Cao, 11/28/2017
    int retv = u.getTransitData(CMD_LOC_LOAD, (DBStruct *)&l);

    if (retv > 0)   //no update is needed
        return;

    if (retv < 0) {
        qWarning() << "get transit data failed";
        return;
    }

    ui->le_location->setText(m_currLoc.description);
    ui->le_spdLmt->setText(m_currLoc.speedLimit);
    ui->le_captSpd->setText(m_currLoc.captureSpeed);
    ui->le_captDist->setText(m_currLoc.captureDistance);
    ui->pb_roadCondition->setText(m_currLoc.roadCondition);
    ui->pb_lanes->setText(QString("%1").arg(m_currLoc.numberOfLanes));
    u.setCurrentLoc(m_currLoc); // All can see this

    m_envIndex = m_envList.indexOf(m_currLoc.roadCondition);
}


void locSetup::setDateTime(QDate &d, QTime &t)
{
    if (m_date == d && m_time == t)
        return;

    m_date = d;
    m_time = t;
/*
    QString display = QString("%1  %2")
            .arg(m_date.toString("MMM dd, yyyy"))
            .arg(m_time.toString());
    //qDebug() << display;
    ui->pb_dateTime->setText(display);
*/
}


void locSetup::on_pb_roadCondition_clicked()
{
    if (m_envIndex < (m_envList.size() - 1))
        m_envIndex++;
    else
        m_envIndex = 0;
    ui->pb_roadCondition->setText(m_envList.at(m_envIndex));

}

void locSetup::on_pb_lanes_clicked()
{
    if (m_laneIndex < (m_laneList.size() - 1))
        m_laneIndex++;
    else
        m_laneIndex = 0;
    ui->pb_lanes->setText(m_laneList.at(m_laneIndex));
}

void locSetup:: doGPS()
{
#if defined(LIDARCAM) || defined(HH1)
  Utils& u = Utils::get();
  
  if( u.GPSBuf()->GPS_Fixed == true ) {
    // Get the float of lat/long
    double lat = atof( (const char*)u.GPSBuf()->Latitude );
    double lon = atof( (const char*)u.GPSBuf()->Longitude );
    
    // get the integer part of the lat/long
    int llat = (int) lat;
    int llon = (int) lon;
    // get the degree part of the lat/long 
    int llatH = llat / 100;  
    int llonH = llon / 100;  
    // Get the minutes part of the lat/long
    int llatMin = llat - (llatH * 100);
    int llonMin = llon - (llonH * 100);
    
    // get the fractional part of the lat/long
    double latf = lat - llat;
    double lonf = lon - llon;
    
    // caculate the second part of the lat/log
    double lats = 60.0 * latf;
    double lons = 60.0 * lonf;
    
    char latstring[50];
    memset(latstring, 0, 50);
    
    // create the lat string.
    if( u.GPSBuf()->Latitude_Direction == 'N' ) {
      sprintf(latstring, "%02d\u00B0%02d\'%06.3f\"N", llatH, llatMin, lats);
    }else{
      sprintf(latstring, "%02d\u00B0%02d\'%06.3f\"S", llatH, llatMin, lats);
    }
    
    char longstring[50];
    memset(longstring, 0, 50);
    
    // create the long string
    if( u.GPSBuf()->Longitude_Direction == 'W' ) {
      sprintf(longstring, "%03d\u00B0%02d\'%06.3f\"W", llonH, llonMin, lons);
    }else{
      sprintf(longstring, "%03d\u00B0%02d\'%06.3f\"E", llonH, llonMin, lons);
    }
    // string to QString conversion
    QString gpsInfo;
    gpsInfo.append( latstring );
    gpsInfo.append( "   " );
    gpsInfo.append( longstring );
    
    //	  DEBUG() << gpsInfo;
    
    ui->lb_gpsInfo->setText( gpsInfo );
    
  }else{
    // No Gps data to display on the screen
    ui->lb_gpsInfo->setText( "No GPS Data" );
  }
#endif
}

void locSetup::locSetupInitTimer( void )
{
  m_timer = new QTimer(this);
  m_timer->setInterval(1000);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(locSetupTimerHit()));
  m_timer->start();
}

void locSetup::locSetupTimerHit( void )
{
  //  DEBUG();
  doGPS();
}


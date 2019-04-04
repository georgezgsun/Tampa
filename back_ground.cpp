#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>
#include "global.h"
#include "debug.h"
#include "utils.h"
#include "Lidar_Buff.h"
#include "hh1MetaData.h"
//#include "RadarInterface/CoordTransforms.h"
#include "back_ground.h"


void backGround::run()
{
   connect(this, SIGNAL(jsonFile(QString *)), this, SLOT(startTimer(QString*)));
   DEBUG() << "backGround running" << thread()->currentThreadId();
   exec();
   DEBUG() << "backGround exit";
}

void backGround::startTimer(QString *pFile)
{
   QTimer::singleShot(1500, this, SLOT(timerHit()));
   mFileName = *pFile;
}

// Video file should be ready now
void backGround::timerHit()
{
//   DEBUG() << "timerHit";
   Create_Evidence_JSON(&mFileName);
}

QString backGround::dNumber( QString s, float f)
{
  QString tmp = QString("    \"stalker-");
  tmp.append(s);
  tmp.append("\": \"");
  tmp.append( QString::number(f));
  tmp.append("\",\n");;
  return tmp;
}

void backGround::Create_Evidence_JSON(QString *pFile)
{
   Utils& u = Utils::get();
   // Get the current time
   QDateTime dateTime = QDateTime::currentDateTime();

   // Get the base file name
   QStringList path_list = pFile->split("/");
   QString base_file_name = path_list.value(path_list.size() - 1);

   // Convert the evidence ID to an int
/*
   QStringList fileNameParameters = pFile->split("_");
   QString evidenceID = fileNameParameters.value(3);
   int id = evidenceID.toInt(NULL,10);
*/
   int id = u.getEvidenceId();

   // Get the avi file hash
   QString avi_hash = "";
   QString avi_file_path = *pFile + ".avi";
   QFile avi_file(avi_file_path);
   if(avi_file.open(QFile::ReadOnly) != false)
   {
      QCryptographicHash hash(QCryptographicHash::Sha256);
      hash.addData(&avi_file);
      avi_hash = hash.result().toHex();
//      DEBUG() << "AVI HASH: " << avi_hash;
      avi_file.close();
   }
   // Create the JSON file
   QString json_path = *pFile + ".json";
   qDebug() << "Creating file:" << json_path;
   QFile json_file(json_path);
   json_file.open(QIODevice::WriteOnly | QIODevice::Text);

   // Create the JSON writer
   QTextStream json_writer(&json_file);
   
   json_writer << "{\n";
#ifdef LIDARCAM
   json_writer << "  \"type\": \"LidarCamEvidence.Manifest\",\n";
#else
   json_writer << "  \"type\": \"RadarCamEvidence.Manifest\",\n";
#endif
   json_writer << "  \"version\": \"1.0\",\n";
   json_writer << "  \"signature\": \"SHA256withRSA\",\n";

   // Write out the evidence report
   json_writer << "  \"report\": {\n";
   json_writer << "    \"id\": " << id <<",\n";
   json_writer << "    \"stalker-id\": \"" << id <<"\",\n";
   json_writer << "    \"manufacturer\": \"Stalker Radar\",\n";
#ifdef LIDARCAM
   json_writer << "    \"model\": \"LidarCam II\",\n";
#else
   json_writer << "    \"model\": \"RadarCam II\",\n";
#endif
#ifdef LIDARCAM
   char SN_str[10];
   memcpy(SN_str, u.lidarDataBuf()->lidarStruct.SERIAL_NUMBER, 8);
   SN_str[8] = '\0';
   QString qs1 = (QString)SN_str;
#else
   SysConfig & cfg = u.getConfiguration();
   QString qs1 = cfg.serialNumber;
#endif
   json_writer << "    \"serial-number\": " << "\"" << qs1 << "\"" <<",\n";
   if (u.GPSBuf()->GPS_Fixed == true)
      json_writer << "    \"gps-status\": true,\n";
   else
      json_writer << "    \"gps-status\": false,\n";
   json_writer << "    \"time-local\": \"" << dateTime.toString( "yyyy-MM-dd hh:mm:ss" ) << "\",\n";
   dateTime = QDateTime::currentDateTimeUtc();
   json_writer << "    \"time-utc\": \"" << dateTime.toString( "yyyy-MM-dd hh:mm:ss" ) << "\",\n";
   Location loc1 =u.getCurrentLoc();
   json_writer << "    \"speed-limit\": " << loc1.speedLimit << ",\n";
   json_writer << "    \"speed\": " << u.getTopSpeed() << ",\n";
   json_writer << "    \"stalker-speed-limit\": \"" << loc1.speedLimit << "\",\n";
   json_writer << "    \"stalker-speed-actual\": \"" << u.getTopSpeed() << "\",\n";

   double lat = atof( (const char*)u.GPSBuf()->Latitude );
   double lon = atof( (const char*)u.GPSBuf()->Longitude );
   json_writer << "    \"latitude\": " << lat << ",\n";
   json_writer << "    \"longitude\": " << lon << ",\n";
   json_writer << "    \"stalker-latitude\": \"" << lat << "\",\n";
   json_writer << "    \"stalker-longitude\": \"" << lon << "\",\n";
   json_writer << "    \"stalker-range\": \"" << u.getDistance() << "\",\n";

   Session *currentSession = u.session();
   if (currentSession)
   {
      json_writer << "    \"officer\": \"" << currentSession->user()->loginName << "\",\n";
   }
   json_writer << "    \"location\": \"" << loc1.description << "\",\n";

   json_writer << "    \"timestamp-local\": 0,\n";
   json_writer << "    \"timestamp-utc\": 0,\n";

   json_writer << dNumber( "timeMillieSecs", mViolation.timeMillieSecs);

   // radar config information
   json_writer << dNumber( "Xs", mViolation.Xs);
   json_writer << dNumber( "Zs", mViolation.Zs);
   json_writer << dNumber( "Zt", mViolation.Zt);
   json_writer << dNumber( "FocalLength", mViolation.FocalLength);

   // processed accelerometer and magneticometer
   json_writer << dNumber( "theta_ref_vs", mViolation.theta_vs_ref);
   json_writer << dNumber( "theta_ref_rs", mViolation.theta_rs_ref);
   json_writer << dNumber( "theta_ref_hs", mViolation.theta_hs_ref);
   json_writer << dNumber( "theta_vs", mViolation.theta_vs);
   json_writer << dNumber( "theta_rs", mViolation.theta_rs);
   json_writer << dNumber( "theta_hs", mViolation.theta_hs);

   // Raw radar data
   json_writer << "\n" << dNumber( "numTargets", mViolation.Targets.numTargets);

   int i;
   DEBUG() << MAX_TARGETS << " "  << mViolation.Targets.maxTargets << " " << mViolation.Targets.numTargets;

   // Open MD file to find the fastest speed for all targets
   QString mdPath = *pFile + ".md";
   QFile mdFile(mdPath);
   bool mdStatus = true;
   float topSpeed[NUM_TARGETS_SHOWN];
   RadarTargetResponse_t topSpeedPara[NUM_TARGETS_SHOWN];

   if (!mdFile.open(QIODevice::ReadOnly | QFile::Truncate))
   {
      DEBUG() << "Failed to open metat data file " << mdPath;
      mdStatus = false;
   }
   else
   {
      char data1[sizeof(struct metaDataGet)];
      UINT32	j, tId;
      coord_struct Radar_Coords;
      coord_struct Roadway_Coords;
      for (i = 0; i < mViolation.Targets.numTargets; i++)
         topSpeed[i] = 0;

      CCoordTransforms Transforms;
      config_type RadarConfig;

      memset((void *)&RadarConfig, 0, sizeof(config_type));

      RadarConfig.radar_data_is_roadway = false;
      RadarConfig.Xs = -2.0f;
      RadarConfig.Zs = 3.0f;
      RadarConfig.FocalLength = 35.0f;
      RadarConfig.SensorWidth = 6.2f;   // IMX 172, pixel size = 1.55 u x 4000 pixels
      RadarConfig.SensorHeight = 4.65f; // IMX 172, pixel size = 1.55 u x 3000 pixels
      RadarConfig.FOVh = RadarConfig.SensorWidth/RadarConfig.FocalLength * 180.0/PI;  //(10.15 degrees)
      RadarConfig.FOVv = RadarConfig.SensorHeight/RadarConfig.FocalLength * 180.0/PI; //(7.61 degrees)
      RadarConfig.port = serial;
      RadarConfig.num_to_show = NUM_TARGETS_SHOWN;

      // Init the coordinate transformations
      Transforms.InitCoordTransforms(RadarConfig.Xs,
                 RadarConfig.Zs,
                 RadarConfig.Zt,
                 RadarConfig.FOVh * PI/180.0f,      // Convert to radians
                 RadarConfig.FOVv * PI/180.0f);     // Convert to radians

      for (i = 0; i < (FRAMESPERSECOND * MAX_RECORDING_SECS); i++)
      {
         mdFile.read((char *)data1, sizeof(struct metaDataGet)); // Read one record of 1012 bytes

         struct metaDataGet *ptr = (struct metaDataGet *)data1;
         for(j = 0; j < mViolation.Targets.numTargets; j++)
         {
            tId = mViolation.Targets.RadarTargets[j].targetId;
            RadarTargetResponse_t *pTarget = &(ptr->target.RadarTargets[j]);
            if (tId != pTarget->targetId)
               continue;   // not this target

            // Here we found the correct target
            memset( &Radar_Coords, 0, sizeof(coord_struct));
//            memset( &Video_Coords, 0, sizeof(coord_struct));
            memset( &Roadway_Coords, 0, sizeof(coord_struct));

            Radar_Coords.type = radar;
            Radar_Coords.X = pTarget->xCoord;
            Radar_Coords.Y = pTarget->yCoord;
            // Since the 3D radar does not measure target height (Z axis), fake it
            Radar_Coords.Z = 1.0f - 3.0f;  // Usually negative

            Radar_Coords.R =  sqrtf(pTarget->xCoord * pTarget->xCoord +
                              pTarget->yCoord * pTarget->yCoord +
                              pTarget->zCoord * pTarget->zCoord);

            Radar_Coords.Vx = pTarget->xVelocity;
            Radar_Coords.Vy = pTarget->yVelocity;
            Radar_Coords.Vz = pTarget->zVelocity;

            Radar_Coords.V =  sqrtf(pTarget->xVelocity * pTarget->xVelocity +
                              pTarget->yVelocity * pTarget->yVelocity +
                              pTarget->zVelocity * pTarget->zVelocity);

            Radar_Coords.Theta_Vy = 0.0f;
            Radar_Coords.Theta_Vz = 0.0f;
            Radar_Coords.Ix = 0.0f;
            Radar_Coords.Iz = 0.0f;

            Roadway_Coords.type = roadway;

            Transforms.Transform(&Radar_Coords, &Roadway_Coords);
            if (Roadway_Coords.V > topSpeed[j])
            {
               topSpeed[j] = Roadway_Coords.V;
               memcpy((void *)&(topSpeedPara[j]), (const void *)pTarget, sizeof(RadarTargetResponse_t));
            }
         }
       }
      mdFile.close();
   }

#ifdef LIDARCAM
   struct Lidar_Buff *ptr = u.lidarDataBuf();
   LIDAR *pLidar = &(ptr->lidarStruct);
   int units = pLidar->DISPLAY_UNITS;
#else
   SysConfig mConf = u.getConfiguration();
   int units = mConf.units;
#endif

   for( i=0; i<mViolation.Targets.numTargets; i++ )
   {
     QString num("_");
     num.append(QString::number(i));
     json_writer << dNumber( QString("targetId").append(num), mViolation.Targets.RadarTargets[i].targetId);
     json_writer << dNumber( QString("xCoord").append(num), mViolation.Targets.RadarTargets[i].xCoord);
     json_writer << dNumber( QString("yCoord").append(num), mViolation.Targets.RadarTargets[i].yCoord);
     json_writer << dNumber( QString("zCoord").append(num), mViolation.Targets.RadarTargets[i].zCoord);
     json_writer << dNumber( QString("xVelocity").append(num), mViolation.Targets.RadarTargets[i].xVelocity);
     json_writer << dNumber( QString("yVelocity").append(num), mViolation.Targets.RadarTargets[i].yVelocity);
     json_writer << dNumber( QString("zVelocity").append(num), mViolation.Targets.RadarTargets[i].zVelocity);
     json_writer << dNumber( QString("tLane").append(num), mViolation.Targets.RadarTargets[i].tLane);
     json_writer << dNumber( QString("tClass").append(num), mViolation.Targets.RadarTargets[i].tClass);

     // Process fastest speed for this target
      if (mdStatus)
      {
         float speed1 = topSpeed[i];

         if (!units)
            speed1 /= 1.60934;   // Convert to Mile
         json_writer << dNumber( QString("topSpeed").append(num), speed1);
         json_writer << dNumber( QString("topXcoord").append(num), topSpeedPara[i].xCoord);
         json_writer << dNumber( QString("topYcoord").append(num), topSpeedPara[i].yCoord);
         json_writer << dNumber( QString("topZcoord").append(num), topSpeedPara[i].zCoord);
         json_writer << dNumber( QString("topXvelocity").append(num), topSpeedPara[i].xVelocity);
         json_writer << dNumber( QString("topYvelocity").append(num), topSpeedPara[i].yVelocity);
         json_writer << dNumber( QString("topZvelocity").append(num), topSpeedPara[i].zVelocity);
      }
   }

   json_writer << "    \"units\": {\n";
   if (units == 1)
   {
      json_writer << "      \"speed\": \"km/h\",\n";
      json_writer << "      \"stalker-speed-units\": \"KM/h\",\n";
      json_writer << "      \"distance\": \"meter\",\n";
      json_writer << "      \"stalker-range-units\": \"M\"\n";
   }
   else if (units == 2)
   {
      json_writer << "      \"speed\": \"knots\",\n";
      json_writer << "      \"stalker-speed-units\": \"KNOTS\",\n";
      json_writer << "      \"distance\": \"feet\",\n";
      json_writer << "      \"stalker-range-units\": \"FT\"\n";
   }
   else
   {
      json_writer << "      \"speed\": \"mph\",\n";
      json_writer << "      \"stalker-speed-units\": \"MPH\",\n";
      json_writer << "      \"distance\": \"feet\",\n";
      json_writer << "      \"stalker-range-units\": \"FT\"\n";
   }
   json_writer << "    },\n";

   json_writer << "    \"approaching\": true,\n";
   json_writer << "    \"violations\": {\n";
   float speedL = loc1.speedLimit.toFloat();
   if (u.getTopSpeed() > speedL)
      json_writer << "      \"speeding\": true\n";
   else
      json_writer << "      \"speeding\": false\n";
   json_writer << "    }\n";
   json_writer << "},\n";

   // Write out the files
   json_writer << "  \"files\": [\n";
   // Write out the video file
   json_writer << "    {\n";
   json_writer << "      \"filename\": \"" << base_file_name << ".avi\",\n";
   json_writer << "      \"file-type\": \"video\",\n";
   json_writer << "      \"name\": \"lidarcam video camera\",\n";
   json_writer << "      \"checksum\": \{\n";
   json_writer << "        \"algorithm\": \"SHA256\",\n";
   json_writer << "        \"value\": \"" << avi_hash << "\"\n";
   json_writer << "      }\n";

   QString image_file_path = *pFile + "_1.jpg";
   if(QFileInfo::exists(image_file_path) == true)
   {
      json_writer << "    },\n";
      // Get the JPG file hash
      QString jpeg_hash = "";
      QString jpeg_file_path = *pFile + "_1.jpg";
      QFile jpeg_file(jpeg_file_path);
      if(jpeg_file.open(QFile::ReadOnly))
      {
         QCryptographicHash hash(QCryptographicHash::Sha256);
         hash.addData(&jpeg_file);
         jpeg_hash = hash.result().toHex();
         jpeg_file.close();
      }

      // Write out the image file
      json_writer << "    {\n";
      json_writer << "      \"filename\": \"" << base_file_name << "_1.jpg\",\n";
      json_writer << "      \"file-type\": \"image jpeg\",\n";
      json_writer << "      \"name\": \"trigger photo\",\n";
      json_writer << "      \"checksum\": \{\n";
      json_writer << "        \"algorithm\": \"SHA256\",\n";
      json_writer << "        \"value\": \"" << jpeg_hash << "\"\n";
      json_writer << "      }\n";
      json_writer << "    }\n";
   }
   else
      json_writer << "    }\n";

   // Close the files section
   json_writer << "  ]\n";
   json_writer << "}\n";

   // Close the file
   json_file.close();
}

#ifdef HH1
void* RunRadarData(void* radarDataArgs);

int backGround::initRadarComm()
{
    UINT response;

    // get configuration data
    Utils& u = Utils::get();
    if(! u.getRadarConfig(& mConfig))
    {
      printf("Error getting configuration settings...Aborting\n");
      return 0;
    }

    mRadarDataArgs.RadarData = &mRadarData;
    mRadarDataArgs.pConfig = &mConfig;
    //    mRadarDataArgs.Radar = &Radar;

    // Init the Radar Orientation
    RadarMemory *radarData = (RadarMemory *)u.RADARBuf();
    radarData->Data.RadarOrientation.azimuthAngle = 0.0f;
    radarData->Data.RadarOrientation.elevationAngle = 0.0f;
    radarData->Data.RadarOrientation.rollAngle = 0.0f;
    radarData->Data.RadarOrientation.radarHeight = mConfig.Zs;
    radarData->Data.RadarOrientation.targetHeight = mConfig.Zt;

    int err = pthread_create(&(radarThreadId), NULL, &RunRadarData, (void *)&mRadarDataArgs);
    DEBUG() << "Created radar communication thread return " << err;
    if (err != 0)
    {
      DEBUG() << "Can't create RunRadarData thread " << strerror(err);
      return 0;
    }

    sleep(1);
    while((!mRadarData.portIsOpen) || (mRadarData.messageState != idle));
//    printf("Radar Data state machine is idle in initRadarComm()\n");
    mRadarData.Message1(&radarData->Data.CalibrationParameters); // Get calibration parameters
    while((response = mRadarData.CheckResponse()) == 0);

    mRadarData.Message3(&radarData->Data.RadarParameters);  // Get radar parameters
    while((response = mRadarData.CheckResponse()) == 0);

    mRadarData.Message5(&radarData->Data.RadarMode);  // Get the Radar Mode
    while((response = mRadarData.CheckResponse()) == 0);

    mRadarData.Message9(&radarData->Data.RadarOrientation);    // Init the Radar Orientation
    while((response = mRadarData.CheckResponse()) == 0);

    return 0;
}

//
// Radar relarted C functions
//
void* RunRadarData(void* radarDataArgs)
{
    UINT32 ret;

    radarDataArgs_t * pRadarDataArgs = (radarDataArgs_t * )radarDataArgs;
    config_type *pConfig = pRadarDataArgs->pConfig;
    // Here is where the work is done.  RunRadarDataStateMachine should never exit.
    //    printf("RunRadarData thread started - initializing serial port.\n");
    ret = pRadarDataArgs->RadarData->RunRadarDataStateMachine(pConfig);
    pthread_exit(&ret);
    return NULL;
}
#endif

void backGround::saveViolation(  int timeMillieSecs,
				 float theta_vs_ref, float theta_rs_ref, float theta_hs_ref, // base from the start
				 float theta_vs,     float theta_rs,     float theta_hs,     // current values
				 float Xs, float Zs, float Zt, float FocalLength,
				 Targets_t *Targets )             // Radar data

 {
   mViolation.timeMillieSecs = timeMillieSecs;
   mViolation.theta_vs_ref = theta_vs_ref;
   mViolation.theta_rs_ref = theta_rs_ref;
   mViolation.theta_hs_ref = theta_hs_ref;
   mViolation.theta_vs = theta_vs;
   mViolation.theta_rs = theta_rs;
   mViolation.theta_hs = theta_hs;
   mViolation.Xs = Xs;
   mViolation.Zs = Zs;
   mViolation.Zt = Zt;
   mViolation.FocalLength = FocalLength;
   memcpy( (void *)&mViolation.Targets, (void *)Targets, sizeof(Targets_t) );

   return;
 }

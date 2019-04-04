#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include "smbus.h"
#include <pthread.h>
#include "TiltSensor.h"
#include <math.h>
#include "RadarTypes.h"
#include <QDebug>
#include <QString>
#include "utils.h"
#include "debug.h"
#include "Tilt_Buff.h"
#include "Mag_Buff.h"

// make smaller will cause more lag, larger follows more realtime
#define TIMECONST 0.2f

static char i2cDev[64] = "/dev/i2c-2";
  
static void *ReadThread(void *);
int mReadThreadFlag;
Tilt_Buff *tiltData;
Mag_Buff *magData;
int accFd;
int magFd;

float mag_x_avg;
float mag_y_avg;
float mag_z_avg;

float mag_x_scale;
float mag_y_scale;
float mag_z_scale;

float acc_x_avg;
float acc_y_avg;
float acc_z_avg;

float acc_x_scale;
float acc_y_scale;
float acc_z_scale;

TiltSensor::TiltSensor()
{
    accFd = -1;
    magFd = -1;
}

TiltSensor::~TiltSensor()
{
    mReadThreadFlag = 0;
    pthread_join(mReadThreadId, NULL);

//    printf("accFd: %d\n", accFd);
    if (accFd != -1)
        close(accFd);

//    printf("magFd: %d\n", magFd);
    if (magFd != -1)
        close(magFd);
}

int TiltSensor::init()
{
    int ret;

   if (GetSensorConfig() == -1)
        return -1;

    /* Debug */
   //    DEBUG() << i2cDev;

#ifdef IS_TI_ARM
    accFd = open(i2cDev, O_RDWR);
    if (accFd == -1)
    {
       fprintf(stderr, "Open ACC I2C device failed. Errno: %s\n", strerror(errno));
       return -1;
    }

    // Init the ACC sensor
    // old acc device #define ACC_SLAVE_ADDR 0x1c
#define ACC_SLAVE_ADDR 0x19
    if (ioctl(accFd, I2C_SLAVE, ACC_SLAVE_ADDR) < 0) {
      // Never fails, only error is when you write to it
      printf("Failed to acquire bus access and/or talk to slave.\n");
      /* ERROR HANDLING; you can check errno to see what went wrong */
      DEBUG() << "Failed to acquire bus access and/or talk to slave.";
      exit(1);
    }

    ret = i2c_smbus_read_byte_data(accFd, 0x0f);
    //    QString valueInHex= QString("%1").arg(ret , 0, 16);
    //    DEBUG() << "WhoamI " << valueInHex;
 
    if( ret != 0x33 ) {
      /* ERROR HANDLING; you can check errno to see what went wrong */
      DEBUG() << "Wrong whoami found";
      exit(1);
    }

    // OLD #define ACC_CTRL_REG1 0x2a
#define ACC_CTRL_REG1 0x20
    // set to zero, put in standby mode
    ret = i2c_smbus_write_byte_data(accFd, ACC_CTRL_REG1, 0);
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }
    usleep( 250 );

    // put in active mode, DDR is period 20ms, 50 times per second
    //old     ret = i2c_smbus_write_byte_data(accFd, ACC_CTRL_REG1, 0x21 );
    //    ret = i2c_smbus_write_byte_data(accFd, ACC_CTRL_REG1, 0x9f );
    ret = i2c_smbus_write_byte_data(accFd, ACC_CTRL_REG1, 0x97 );
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

#define ACC_CTRL_REG2 0x21
    // Hight resolution mode
    ret = i2c_smbus_write_byte_data(accFd, ACC_CTRL_REG2, 0x20 );
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

#define ACC_CTRL_REG4 0x23
    // Hight resolution mode
    ret = i2c_smbus_write_byte_data(accFd, ACC_CTRL_REG4, 0x88 );
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

    // Init the MAG sensor
    magFd = open(i2cDev, O_RDWR);
    if (magFd == -1)
    {
       fprintf(stderr, "Open MAG I2C device failed. Errno: %s\n", strerror(errno));
       return -1;
    }

    // OLD #define MAG_SLAVE_ADDR 0x0e
#define MAG_SLAVE_ADDR 0x1e
    if (ioctl(magFd, I2C_SLAVE, MAG_SLAVE_ADDR) < 0) {
      printf("Failed to acquire bus access and/or talk to slave.\n");
      /* ERROR HANDLING; you can check errno to see what went wrong */
      DEBUG() << "Failed to acquire bus access and/or talk to slave.";
      exit(1);
    }

    // OLD #define MAG_CTRL_REG1 0x10
    // OLD #define MAG_CTRL_REG2 0x11
    // OLD #define WHO_AM_I 0x07

#define MAG_CTRL_REGA 0x60
#define MAG_CTRL_REGA_VALUE 0x04
#define MAG_CTRL_REGB 0x61
#define MAG_CTRL_REGB_VALUE 0x13
#define MAG_CTRL_REGC 0x62
#define MAG_CTRL_REGC_VALUE 0x10
#define INT_CTRL_REG 0x63 
#define INT_CTRL_REG_VALUE 0x00
#define WHO_AM_I 0x4F
    
    ret = i2c_smbus_write_byte_data(magFd, MAG_CTRL_REGA, MAG_CTRL_REGA_VALUE);
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

    usleep( 250 );

    ret = i2c_smbus_write_byte_data(magFd, MAG_CTRL_REGB, MAG_CTRL_REGB_VALUE );
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

    usleep( 250 );

    ret = i2c_smbus_write_byte_data(magFd, MAG_CTRL_REGC, MAG_CTRL_REGC_VALUE );
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

    usleep( 250 );

    ret = i2c_smbus_write_byte_data(magFd, INT_CTRL_REG, INT_CTRL_REG_VALUE );
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

    usleep( 250 );

    ret = i2c_smbus_read_byte_data(magFd, WHO_AM_I);
    //    printf("%s(%d): Device ID: 0x%02x\n",__FUNCTION__, __LINE__, ret );

    mReadThreadFlag = 1;
    ret = 0;
    ret = pthread_create(&mReadThreadId, NULL, &ReadThread, NULL);
    //    DEBUG() << "ReadThread ret = " << ret;
    if (ret)
    {
        mReadThreadFlag = 0;
        fprintf(stderr, "Error - pthread_create() failure. Code: %d\n", ret);
        close(accFd);
        close(magFd);
        return 1;
    }
#endif
    return 0;
}

int TiltSensor::GetSensorConfig(void)
{
  Utils & u = Utils::get();
  Sensor & sensorData = u.getSensorFromDB();

  //  i2cDev = sensorData.i2cDevice;
  Mag_Xmax = sensorData.magXmax;
  Mag_Xmin = sensorData.magXmin;
  Mag_Ymax = sensorData.magYmax;
  Mag_Ymin = sensorData.magYmin;
  Mag_Zmax = sensorData.magZmax;
  Mag_Zmin = sensorData.magZmin;
  Mag_Theta_X = sensorData.magThetaX;
  Mag_Theta_Y = sensorData.magThetaY;
  Mag_Theta_Z = sensorData.magThetaZ;
  Acc_Xmax = sensorData.accXmax;
  Acc_Xmin = sensorData.accXmin;
  Acc_Ymax = sensorData.accYmax;
  Acc_Ymin = sensorData.accYmin;
  Acc_Zmax = sensorData.accZmax;
  Acc_Zmin = sensorData.accZmin;
  Acc_Theta_X = sensorData.accThetaX;
  Acc_Theta_Y = sensorData.accThetaY;
  Acc_Theta_Z = sensorData.accThetaZ;

  mag_x_avg = ( Mag_Xmax + Mag_Xmin ) / 2;
  mag_y_avg = ( Mag_Ymax + Mag_Ymin ) / 2;
  mag_z_avg = ( Mag_Zmax + Mag_Zmin ) / 2;
  
  mag_x_scale = 1.0 / (float)(Mag_Xmax - Mag_Xmin);
  mag_y_scale = 1.0 / (float)(Mag_Ymax - Mag_Ymin);
  mag_z_scale = 1.0 / (float)(Mag_Zmax - Mag_Zmin);
  
  acc_x_avg = ( Acc_Xmax + Acc_Xmin ) / 2;
  acc_y_avg = ( Acc_Ymax + Acc_Ymin ) / 2;
  acc_z_avg = ( Acc_Zmax + Acc_Zmin ) / 2;

  acc_x_scale = 1.0 / (float)(Acc_Xmax - Acc_Xmin);
  acc_y_scale = 1.0 / (float)(Acc_Ymax - Acc_Ymin);
  acc_z_scale = 1.0 / (float)(Acc_Zmax - Acc_Zmin);
  
  return 0;
}

static float SCI_s14frac_Out ( Tilt data)
{
  short result;

  //  printf("%s(%d): data.tilt_s 0x%04x\r\n", __FUNCTION__, __LINE__, data.tilt_s );

  /*
  ** ignore possible garabage
  */
  data.tilt_s &= 0xFFF0;

  //  printf("%s(%d): data.tilt_s 0x%04x\r\n", __FUNCTION__, __LINE__, data.tilt_s );

  /*
  ** Determine mantissa value
  */
  result = 0;
  //  printf("%s(%d): data.tilt_s 0x%04x\r\n", __FUNCTION__, __LINE__, data.tilt_s );

  float x = (float)(data.tilt_s / 32768.0f);
  //  printf("%s(%d): neg %d result %04f\n", __FUNCTION__, __LINE__, neg, x);

  return x;
}

static void *ReadThread(void *)
{
    float X_Axis, Y_Axis, Z_Axis;
    static int ret = 0;
    unsigned char packetData[20];

//    printf("ReadThread Start\n");

    Utils& u = Utils::get();
    tiltData = (Tilt_Buff *)u.TILTBuf();
    //    DEBUG() << "tiltData " << tiltData;
    magData = (Mag_Buff *)u.MAGBuf();
    //    DEBUG() << "magData " << magData;

    Mag tmpMag;

    while (mReadThreadFlag)
    {
        for (int i = 1; i<7; i++) {
	  // OLD           ret = i2c_smbus_read_byte_data(accFd, i);
          ret = i2c_smbus_read_byte_data(accFd, i+0x27);
          packetData[i] = ret;
	  //	  printf("%s(%d): 0x%02x\n",__FUNCTION__, __LINE__, ret );
        }
        Tilt tmpTilt;

        tmpTilt.tilt_c[0] = packetData[1];
        tmpTilt.tilt_c[1] = packetData[2];
        Z_Axis = -SCI_s14frac_Out(tmpTilt);
        tmpTilt.tilt_c[0] = packetData[3];
        tmpTilt.tilt_c[1] = packetData[4];
        X_Axis = SCI_s14frac_Out(tmpTilt);
        tmpTilt.tilt_c[0] = packetData[5];
        tmpTilt.tilt_c[1] = packetData[6];
        Y_Axis = SCI_s14frac_Out(tmpTilt);

        tiltData->raw_X_Axis = X_Axis;
        tiltData->raw_Y_Axis = Y_Axis;
        tiltData->raw_Z_Axis = Z_Axis;

	// Rework raw data based on calibration
	float newX = (X_Axis - acc_x_avg)*acc_x_scale;
	float newY = (Y_Axis - acc_y_avg)*acc_y_scale;
        float newZ = (Z_Axis - acc_z_avg)*acc_z_scale;

#if 0
    // difference between current reworked data and previous X, Y, and Z
	float tiltXdiff = newX - tiltData->X_Axis;
	float tiltYdiff = newY - tiltData->Y_Axis;
        float tiltZdiff = newZ - tiltData->Z_Axis;
	// previous tilt data
        float tiltMag = sqrtf( tiltData->X_Axis * tiltData->X_Axis + tiltData->Y_Axis * tiltData->Y_Axis + tiltData->Z_Axis * tiltData->Z_Axis);
        float tiltDiff = sqrtf( tiltXdiff * tiltXdiff + tiltYdiff * tiltYdiff + tiltZdiff * tiltZdiff);
//        printf("tiltMag = %f tiltDiff = %f\n", tiltMag, tiltDiff);
        float tiltTimeConst;
        if(tiltMag == 0) tiltTimeConst = 1.0f;
        else tiltTimeConst = 3.0f * tiltDiff/tiltMag;
        if(tiltTimeConst > 1.0f)tiltTimeConst = 1.0f;
        
	tiltData->X_Axis = (1-tiltTimeConst)*tiltData->X_Axis + tiltTimeConst * newX;
    tiltData->Y_Axis = (1-tiltTimeConst)*tiltData->Y_Axis + tiltTimeConst * newY;
    tiltData->Z_Axis = (1-tiltTimeConst)*tiltData->Z_Axis + tiltTimeConst * newZ;
#endif
    tiltData->X_Axis = (1-TIMECONST)*tiltData->X_Axis + TIMECONST * newX;
    tiltData->Y_Axis = (1-TIMECONST)*tiltData->Y_Axis + TIMECONST * newY;
    tiltData->Z_Axis = (1-TIMECONST)*tiltData->Z_Axis + TIMECONST * newZ;

	//#define TILT_DEBUG
#ifdef TILT_DEBUG
        printf("Tilt: ");
        printf("X: %06.4f ",X_Axis);
        printf("Y: %06.4f ",Y_Axis);
        printf("Z: %06.4f ",Z_Axis);
        printf("sum: %06.4f\n", sqrtf( (X_Axis * X_Axis) + (Y_Axis * Y_Axis) + (Z_Axis * Z_Axis)));
        printf("Tilt2: ");
        printf("X: %06.4f ",tiltData->X_Axis);
        printf("Y: %06.4f ",tiltData->Y_Axis);
        printf("Z: %06.4f ",tiltData->Z_Axis);
        printf("sum: %06.4f\n", sqrtf( (tiltData->X_Axis * tiltData->X_Axis) + (tiltData->Y_Axis * tiltData->Y_Axis) + (tiltData->Z_Axis * tiltData->Z_Axis)));
#endif


	for (int i = 0; i<6; i++) {
	  ret = i2c_smbus_read_byte_data(magFd, i+0x68);
	  packetData[i] = ret;
	  //      printf("0x%02x ", ret );
	}
	//    printf("\r\n");
	
	
	tmpMag.mag_c[0] = packetData[0];
	tmpMag.mag_c[1] = packetData[1];
	float X_Axis = -tmpMag.mag_s;
	tmpMag.mag_c[0] = packetData[2];
	tmpMag.mag_c[1] = packetData[3];
	float Z_Axis = tmpMag.mag_s;
	tmpMag.mag_c[0] = packetData[4];
	tmpMag.mag_c[1] = packetData[5];
	float Y_Axis = -tmpMag.mag_s;
	
	magData->raw_X_Axis = X_Axis;
	magData->raw_Y_Axis = Y_Axis;
	magData->raw_Z_Axis = Z_Axis;

	// rework raw data based calibration data
        newX = (X_Axis - mag_x_avg) * mag_x_scale;
	newY = (Y_Axis - mag_y_avg) * mag_y_scale;
	newZ = (Z_Axis - mag_z_avg) * mag_z_scale;
	
        float magXdiff = newX - magData->X_Axis;
        float magYdiff = newY - magData->Y_Axis;
        float magZdiff = newZ - magData->Z_Axis;

	// magMag based on previous data
        float magMag = sqrtf(magData->X_Axis * magData->X_Axis + magData->Y_Axis * magData->Y_Axis + magData->Z_Axis * magData->Z_Axis);
	float magDiff = sqrtf(magXdiff * magXdiff + magYdiff * magYdiff + magZdiff * magZdiff);
//        printf("magMag = %f magDiff = %f\n", magMag, magDiff);
        // change pow( magDiff/magMag, exponent) control ratio between bounce and real movement
	// constant to control bounce
#define  TIMECONSTSCALE 3.0f
        float magTimeConst;
        if(magMag == 0) magTimeConst = 1.0f;
        else magTimeConst = TIMECONSTSCALE * magDiff/magMag;
        if(magTimeConst > 1.0f) magTimeConst = 1.0f;
        magData->X_Axis = (1-magTimeConst)*magData->X_Axis + magTimeConst * newX;
        magData->Y_Axis = (1-magTimeConst)*magData->Y_Axis + magTimeConst * newY;
        magData->Z_Axis = (1-magTimeConst)*magData->Z_Axis + magTimeConst * newZ;
	
	//#define MAG_DEBUG
#ifdef MAG_DEBUG
        // Output
	//        float sq_term_y = sqrtf((float)(magData->X_Axis*magData->X_Axis + magData->Y_Axis*magData->Y_Axis));
	//        float angle1 = (float)180 * atan2f((float)magData->Z_Axis, sq_term_y) / M_PI;
	//        sq_term_y = sqrtf((float)(magData->Y_Axis*magData->Y_Axis + magData->Z_Axis*magData->Z_Axis));
	//        float angle2 = (float)180 * atan2f((float)magData->X_Axis, sq_term_y) / M_PI;
	//        sq_term_y = sqrtf((float)(magData->Z_Axis*magData->Z_Axis + magData->X_Axis*magData->X_Axis));
	//        float angle3 = (float)180 * atan2f((float)magData->Y_Axis, sq_term_y) / M_PI;
        printf("Mag: ");
        printf("X: %6.3f ", X_Axis);
        printf("Y: %6.3f ", Y_Axis);
        printf("Z: %6.3f\n", Z_Axis);

        printf("Mag2: ");
        printf("X: %6.3f ", magData->X_Axis);
        printf("Y: %6.3f ", magData->Y_Axis);
        printf("Z: %6.3f\n", magData->Z_Axis);
	//        printf("Xavg: %f ",magData->X_Axis);
	//        printf("Yavg: %f ",magData->Y_Axis);
	//        printf("Zavg: %f\n",magData->Z_Axis);
	//        printf(" angle1 = %f ", angle1);
	//        printf(" angle2 = %f ", angle2);
	//        printf(" angle3 = %f\r\n", angle3);
#endif


        // 10 times a second update the averages
        usleep( 100000 );

    }
    printf("Read Thread Exit\n");
    return &ret;
}

int TiltSensor::ReadSensor(float *theta_vs, float *theta_rs, float *theta_hs)
{
#ifdef IS_TI_ARM
    struct Tilt_Buff save1Tilt;

    // snap shot of the current tilt data
    save1Tilt.X_Axis = tiltData->X_Axis;
    save1Tilt.Y_Axis = tiltData->Y_Axis;
    save1Tilt.Z_Axis = tiltData->Z_Axis;

    float sum = sqrtf( (save1Tilt.X_Axis * save1Tilt.X_Axis) + (save1Tilt.Y_Axis * save1Tilt.Y_Axis) + (save1Tilt.Z_Axis * save1Tilt.Z_Axis));
    float x = (float)(save1Tilt.X_Axis)/sum;
    float y = (float)(save1Tilt.Y_Axis)/sum;
    float z = (float)(save1Tilt.Z_Axis)/sum;
    //    float rollAngle = asinf(x);
    //    if(z > 0 && rollAngle > 0) rollAngle = PI - rollAngle;
    //    else if(z > 0 && rollAngle < 0) rollAngle = -PI + rollAngle;
    //    float pitchAngle = asinf(-y);
    //    if(z > 0 && pitchAngle > 0) pitchAngle = PI - pitchAngle;
    //    else if(z > 0 && pitchAngle < 0) pitchAngle = -PI + pitchAngle;

    // unit at rest level, both rollAnge and picthAngle should be close to zero
    float rollAngle = atan2f(x, -z);

    //    float newX_Axis = x * cosf(-rollAngle) - z * sinf(-rollAngle);  // Rotate around Y axis
    float newY_Axis = y;
    float newZ_Axis = x * sinf(rollAngle) - z * cosf(rollAngle);
    float pitchAngle = atan2f( -newY_Axis, newZ_Axis );

    //#define TILT_DEBUG
#ifdef TILT_DEBUG
    printf("Tilt: ");
#ifdef JUNK
    printf("X: %06.4f ",save1Tilt.X_Axis);
    printf("Y: %06.4f ",save1Tilt.Y_Axis);
    printf("Z: %06.4f ",save1Tilt.Z_Axis);
#endif
    printf("sum: %06.4f x %f y %f z %f ", sum, x, y, z);
    printf("roll %f pitch %f\r", rollAngle * DEG_PER_RAD, pitchAngle * DEG_PER_RAD);
    printf("\r\n");
#endif

    struct Mag_Buff save1Mag;

    // snap shot of the current mag data
    save1Mag.X_Axis = magData->X_Axis;
    save1Mag.Y_Axis = magData->Y_Axis;
    save1Mag.Z_Axis = magData->Z_Axis;
    
    float Xm = magData->X_Axis;
    float Ym = magData->Y_Axis;
    float Zm = magData->Z_Axis;

    float Xp = Xm;  // Rotate around X axis
    float Yp = Ym * cosf(pitchAngle) - Zm * sinf(pitchAngle);
    float Zp = Ym * sinf(pitchAngle) + Zm * cosf(pitchAngle);

    save1Mag.X_Axis = Xp * cosf(-rollAngle) - Zp * sinf(-rollAngle);  // Rotate around Y axis
    save1Mag.Y_Axis = Yp;
    save1Mag.Z_Axis = Xp * sinf(-rollAngle) + Zp * cosf(-rollAngle);
    
    float heading  = atan2f(-save1Mag.X_Axis, save1Mag.Y_Axis);
    
    //#define MAG_DEBUG
#ifdef MAG_DEBUG
    printf("Mag:\n");
    printf("Raw X: %06.4f ", Xm);
    printf("X: %06.4f ", save1Mag.X_Axis);
    printf("Raw Y: %06.4f ", Ym);
    printf("Y: %06.4f ", save1Mag.Y_Axis);
    printf("Raw Z: %06.4f ", Zm);
    printf("Z: %06.4f ", save1Mag.Z_Axis);
    printf("sum: %06.4f ", sqrtf( (save1Mag.X_Axis * save1Mag.X_Axis) + (save1Mag.Y_Axis * save1Mag.Y_Axis) + (save1Mag.Z_Axis * save1Mag.Z_Axis)));
    printf(" heading = H %f ", heading);
    printf("\r\n");
#endif

    *theta_hs = heading;
    *theta_rs = rollAngle;
    *theta_vs = pitchAngle;

    //#define SENSOR_DEBUG
#ifdef SENSOR_DEBUG
    printf("Tilt: ");
    printf("sum: %06.4f x %f y %f z %f\n", sum, x, y, z);
    printf("Mag: ");
    printf("sum: %06.4f ", sqrtf( (save1Mag.X_Axis * save1Mag.X_Axis) + (save1Mag.Y_Axis * save1Mag.Y_Axis) + (save1Mag.Z_Axis * save1Mag.Z_Axis)));
    printf("X: %06.4f ", save1Mag.X_Axis);
    printf("Y: %06.4f ", save1Mag.Y_Axis);
    printf("Z: %06.4f\n",save1Mag.Z_Axis);
    printf("theta_hs %8.4f theta_rs %8.4f theta_vs %8.4f\n", *theta_hs, *theta_rs, *theta_vs);
#endif
#endif
    return 1;
  }

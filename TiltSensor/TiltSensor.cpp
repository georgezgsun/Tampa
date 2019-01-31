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
    DEBUG() << i2cDev;

#ifdef IS_TI_ARM
    accFd = open(i2cDev, O_RDWR);
    if (accFd == -1)
    {
       fprintf(stderr, "Open ACC I2C device failed. Errno: %s\n", strerror(errno));
       return -1;
    }

    // Init the ACC sensor
#define ACC_SLAVE_ADDR 0x1c
    if (ioctl(accFd, I2C_SLAVE, ACC_SLAVE_ADDR) < 0) {
      printf("Failed to acquire bus access and/or talk to slave.\n");
      /* ERROR HANDLING; you can check errno to see what went wrong */
      DEBUG() << "Failed to acquire bus access and/or talk to slave.";
      exit(1);
    }
#define ACC_CTRL_REG1 0x2a
    // set to zero, put in standby mode
    ret = i2c_smbus_write_byte_data(accFd, ACC_CTRL_REG1, 0);
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }
    usleep( 250 );

    // put in active mode, DDR is period 20ms, 50 times per second
    ret = i2c_smbus_write_byte_data(accFd, ACC_CTRL_REG1, 0x21 );
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

#define MAG_SLAVE_ADDR 0x0e
    if (ioctl(magFd, I2C_SLAVE, MAG_SLAVE_ADDR) < 0) {
      printf("Failed to acquire bus access and/or talk to slave.\n");
      /* ERROR HANDLING; you can check errno to see what went wrong */
      DEBUG() << "Failed to acquire bus access and/or talk to slave.";
      exit(1);
    }

#define MAG_CTRL_REG1 0x10
#define MAG_CTRL_REG2 0x11
#define WHO_AM_I 0x07

    ret = i2c_smbus_write_byte_data(magFd, MAG_CTRL_REG2, 0x80);
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

    usleep( 250 );

    // activate the part
    // ret = i2c_smbus_write_byte_data(magFd, MAG_CTRL_REG1, 0x11 );
    ret = i2c_smbus_write_byte_data(magFd, MAG_CTRL_REG1, 0x19 );
    if ( ret != 0 ) {
      printf("%s:%s(%d): write returned %d %s\n", __FILE__, __FUNCTION__, __LINE__, ret, strerror( errno) );
    }

    usleep( 250 );

    ret = i2c_smbus_read_byte_data(magFd, WHO_AM_I);
    // printf("%s(%d): Device ID: 0x%02x\n",__FUNCTION__, __LINE__, ret );

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
  int neg = 0;

  //  printf("%s(%d): data.tilt_s 0x%04x\r\n", __FUNCTION__, __LINE__, data.tilt_s );

  /*
  ** Determine sign and output
  */
  if (data.tilt_c[1] > 0x7F) {
    data.tilt_s &= 0xFFFC;
    data.tilt_s = ~data.tilt_s + 1;
    neg = 1;
  }

  //  printf("%s(%d): data.tilt_s 0x%04x\r\n", __FUNCTION__, __LINE__, data.tilt_s );

  /*
  ** Determine integer value and output
  */
  data.tilt_s = data.tilt_s <<2;

  /*
  ** Determine mantissa value
  */
  result = 0;
  //  printf("%s(%d): data.tilt_s 0x%04x\r\n", __FUNCTION__, __LINE__, data.tilt_s );

  float x = (float)(data.tilt_s / 65536.0f);
  //  printf("%s(%d): neg %d result %04f\n", __FUNCTION__, __LINE__, neg, x);

  if ( neg == 1 ) {
    return (-x);
  }else{
    return x;
  }
}

static void *ReadThread(void *)
{
    float X_Axis, Y_Axis, Z_Axis;
    static int ret = 0;
    unsigned char packetData[20];

//    printf("ReadThread Start\n");

    Utils& u = Utils::get();
    tiltData = (Tilt_Buff *)u.TILTBuf();
    DEBUG() << "tiltData " << tiltData;
    magData = (Mag_Buff *)u.MAGBuf();
    DEBUG() << "magData " << magData;

    while (mReadThreadFlag)
    {
        for (int i = 1; i<7; i++) {
          ret = i2c_smbus_read_byte_data(accFd, i);
          packetData[i] = ret;
//  	  printf("%s(%d): 0x%02x\n",__FUNCTION__, __LINE__, ret );
        }
        Tilt tmpTilt;

        tmpTilt.tilt_c[0] = packetData[2];
        tmpTilt.tilt_c[1] = packetData[1];
        X_Axis = SCI_s14frac_Out(tmpTilt);
        tmpTilt.tilt_c[0] = packetData[4];
        tmpTilt.tilt_c[1] = packetData[3];
        Y_Axis = SCI_s14frac_Out(tmpTilt);
        tmpTilt.tilt_c[0] = packetData[6];
        tmpTilt.tilt_c[1] = packetData[5];
        Z_Axis = SCI_s14frac_Out(tmpTilt);

//#define TILT_DEBUG
#ifdef TILT_DEBUG
        printf("Tilt: \n");
        printf("X: %06.4f ",X_Axis);
        printf("Y: %06.4f ",Y_Axis);
        printf("Z: %06.4f ",Z_Axis);
        printf("sum: %06.4f\n", sqrtf( (X_Axis * X_Axis) + (Y_Axis * Y_Axis) + (Z_Axis * Z_Axis)));
#endif

        tiltData->raw_X_Axis = X_Axis;
        tiltData->raw_Y_Axis = Y_Axis;
        tiltData->raw_Z_Axis = Z_Axis;

        tiltData->X_Axis = (1-TIMECONST)*tiltData->X_Axis + TIMECONST*X_Axis;
        tiltData->Y_Axis = (1-TIMECONST)*tiltData->Y_Axis + TIMECONST*Y_Axis;
        tiltData->Z_Axis = (1-TIMECONST)*tiltData->Z_Axis + TIMECONST*Z_Axis;

        for (int i = 0; i<18; i++) {
          ret = i2c_smbus_read_byte_data(magFd, i);
          packetData[i] = ret;
          //	  printf("0x%02x ", ret );
        }
        //	printf("\r\n");

        Mag tmpMag;

        tmpMag.mag_c[1] = packetData[1];
        tmpMag.mag_c[0] = packetData[2];
        float X_Axis = tmpMag.mag_s;
        tmpMag.mag_c[1] = packetData[3];
        tmpMag.mag_c[0] = packetData[4];
        float Y_Axis = tmpMag.mag_s;
        tmpMag.mag_c[1] = packetData[5];
        tmpMag.mag_c[0] = packetData[6];
        float Z_Axis = tmpMag.mag_s;

        magData->raw_X_Axis = X_Axis;
        magData->raw_Y_Axis = Y_Axis;
        magData->raw_Z_Axis = Z_Axis;

	magData->X_Axis = (1-TIMECONST)*magData->X_Axis + TIMECONST*(X_Axis - mag_x_avg)*mag_x_scale;
        magData->Y_Axis = (1-TIMECONST)*magData->Y_Axis + TIMECONST*(Y_Axis - mag_y_avg)*mag_y_scale;
        magData->Z_Axis = (1-TIMECONST)*magData->Z_Axis + TIMECONST*(Z_Axis - mag_z_avg)*mag_z_scale;

//#define MAG_DEBUG
#ifdef MAG_DEBUG
        // Output
        float sq_term_y = sqrtf((float)(magData->X_Axis*magData->X_Axis + magData->Y_Axis*magData->Y_Axis));
        float angle1 = (float)180 * atan2f((float)magData->Z_Axis, sq_term_y) / M_PI;
        sq_term_y = sqrtf((float)(magData->Y_Axis*magData->Y_Axis + magData->Z_Axis*magData->Z_Axis));
        float angle2 = (float)180 * atan2f((float)magData->X_Axis, sq_term_y) / M_PI;
        sq_term_y = sqrtf((float)(magData->Z_Axis*magData->Z_Axis + magData->X_Axis*magData->X_Axis));
        float angle3 = (float)180 * atan2f((float)magData->Y_Axis, sq_term_y) / M_PI;
        printf("Mag: ");
        printf("X: %f ",X_Axis);
        printf("Y: %f ",Y_Axis);
        printf("Z: %f\n",Z_Axis);

        printf("Xavg: %f ",magData->X_Axis);
        printf("Yavg: %f ",magData->Y_Axis);
        printf("Zavg: %f\n",magData->Z_Axis);
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

    save1Tilt.X_Axis = tiltData->X_Axis;
    save1Tilt.Y_Axis = tiltData->Z_Axis;
    save1Tilt.Z_Axis = tiltData->Y_Axis;

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
    float rollAngle = atan2f(x, -z);
    float pitchAngle = atan2f(-y * cosf(rollAngle), fabsf(z));

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
  //  printf("\r\n");
#endif

    struct Mag_Buff save1Mag;
    float Xm = magData->X_Axis;
    float Ym = magData->Y_Axis;
    float Zm = magData->Z_Axis;
    float Xp = Xm * cosf(pitchAngle) - Zm * sinf(pitchAngle);  // Rotate around Y axis
    float Yp = Ym;
    float Zp = Xm * sinf(pitchAngle) + Zm * cosf(pitchAngle);

    save1Mag.X_Axis = Xp * cosf(-rollAngle) - Yp * sinf(-rollAngle);  // Rotate around Z axis
    save1Mag.Y_Axis = Xp * sinf(-rollAngle) + Yp * cosf(-rollAngle);
    save1Mag.Z_Axis = Zp;

    float heading  = atan2f(-save1Mag.Y_Axis, save1Mag.Z_Axis);


//#define MAG_DEBUG
#ifdef MAG_DEBUG
    printf("Mag:\n");
    printf("Raw X: %06.4f ", Xm);
    printf("X: %06.4f\n", save1Mag.X_Axis);
    printf("Raw Y: %06.4f ", Ym);
    printf("Y: %06.4f\n", save1Mag.Y_Axis);
    printf("Raw Z: %06.4f ", Zm);
    printf("Z: %06.4f\n", save1Mag.Z_Axis);
    printf("sum: %06.4f\n", sqrtf( (save1Mag.X_Axis * save1Mag.X_Axis) + (save1Mag.Y_Axis * save1Mag.Y_Axis) + (save1Mag.Z_Axis * save1Mag.Z_Axis)));
    printf(" heading = %f", heading * DEG_PER_RAD);
    printf("\r\n");
#endif
    *theta_hs = -heading;
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
    printf("theta_hs %8.4f theta_rs %8.4f theta_vs %8.4f\n", *theta_hs * DEG_PER_RAD, *theta_rs * DEG_PER_RAD, *theta_vs * DEG_PER_RAD);
#endif
#endif
    return 1;
}

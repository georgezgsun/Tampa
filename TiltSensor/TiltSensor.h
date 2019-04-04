#ifndef TILTSENSOR_H
#define TILTSENSOR_H

#define SCALE_ACC
#define SCALE_MAG

typedef struct
{
   int xAxis;
   int yAxis;
   int zAxis;
}AccMagData;

typedef union
{
  signed short tilt_s;
  unsigned char tilt_c[2];
} Tilt;


typedef union
{
  short mag_s;
  unsigned char mag_c[2];
} Mag;

class TiltSensor
{
public:
    TiltSensor();
    ~TiltSensor();

    int GetSensorConfig(void);
//    void ReadThread(void);
    pthread_t mReadThreadId;

    int init(void);
    int ReadSensor(float *theta_vs, float *theta_rs, float *theta_hs);

private:
    float Mag_Xmax;
    float Mag_Xmin;
    float Mag_Ymax;
    float Mag_Ymin;
    float Mag_Zmax;
    float Mag_Zmin;
    float Mag_Theta_X;
    float Mag_Theta_Y;
    float Mag_Theta_Z;
    float Acc_Xmax;
    float Acc_Xmin;
    float Acc_Ymax;
    float Acc_Ymin;
    float Acc_Zmax;
    float Acc_Zmin;
    float Acc_Theta_X;
    float Acc_Theta_Y;
    float Acc_Theta_Z;
};

#endif // TILTSENSOR_H

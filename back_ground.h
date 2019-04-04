#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QString>
#include <QThread>
#include <QTimer>

#ifdef HH1
#include "RadarTypes.h"
#include "RadarInterface/Serial.h"
#include "RadarInterface/RadarData.h"
#include "RadarInterface/CoordTransforms.h"

typedef struct
{
    config_type * pConfig;
    CRadarData * RadarData;
    RadarData_t * Radar;
    void * menuClass;
} radarDataArgs_t;
#endif

typedef struct saveData {
  int timeMillieSecs;
  float theta_vs_ref;
  float theta_rs_ref;
  float theta_hs_ref;
  float theta_vs;
  float theta_rs;
  float theta_hs;
  float Xs;
  float Zs;
  float Zt;
  float FocalLength;
  Targets_t Targets;
} saveData;

enum {NUM_TARGETS_SHOWN = 4};

class backGround : public QThread
{
   Q_OBJECT

   void run();

public:
//   explicit backGround(QObject *parent = 0);
   static backGround& get()
   {
      static backGround instance;
      return instance;
   }

   // Emit the signal
   void createJson(QString *pFile) {emit jsonFile(pFile);}
#ifdef HH1
   // add interface to save HH1 params
   void saveViolation( int timeMillieSecs,
		       float theta_vs_ref, float theta_rs_ref, float theta_hs_ref, // base from the start
		       float theta_vs,     float theta_rs,     float theta_hs,     // current values
		       float Xs, float Zs, float Zt, float FocalLength,
		       Targets_t *Targets );             // Radar data

#endif
   

#ifdef HH1
   // Radar related stuffs
   int initRadarComm(void);
   radarDataArgs_t *getRadarDataArgs(void) {return &mRadarDataArgs;}
   CRadarData &getRadarData(void) {return mRadarData;}
#endif

private slots:
   // Create JSON info for back office
   void startTimer(QString *);
   void timerHit();
   void Create_Evidence_JSON(QString *pFile);

signals:
   void jsonFile(QString *);

public slots:

private:
   QString dNumber( QString s, float f);
   QString mFileName;


   saveData mViolation;
   
#ifdef HH1
   pthread_t radarThreadId;
   config_type mConfig;
   radarDataArgs_t mRadarDataArgs;
   CRadarData mRadarData;	// This object provides the Oculii radar interface
#endif
};

#endif // BACKGROUND_H

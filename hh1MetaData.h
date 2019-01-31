#ifndef HH1METADATA_H
#define HH1METADATA_H

#include "RadarTypes.h"


// 15 frames per second,
// MAX_RECORDING_SECS seconds of data
// 2 double for the circular buffer
#define DATAMAX (2 * FRAMESPERSECOND * MAX_RECORDING_SECS )

struct metaDataHdr {
  int timeMilliSecs;  // timestamp
  // Sensor data
  // theta_ref_vs rx and hs is in the json file
  float theta_vs;
  float theta_rs;
  float theta_hs;
};

struct metaDataGet {
  struct metaDataHdr hdr;
  Targets_t target;  // Raw radar data kind of large
};

struct metaDataSet {
  struct metaDataHdr hdr;
  Targets_t *target;  // Raw radar data kind of large
};

class HH1MetaData
{
public:
  static HH1MetaData & get() {
    static HH1MetaData instance;
    return instance;
  }

  HH1MetaData();
  ~HH1MetaData();
  int init();
  int reset();
  struct metaDataGet *getMD( int offset, int *nextOffset );
  int setMD( struct metaDataSet *data);
  int findMDOffset( int timeMS );
};
#endif

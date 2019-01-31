#include <QDebug>
#include <cstring>
#include <QTextStream>
#include <QElapsedTimer>
#include "hh1MetaData.h"
#include "debug.h"
#include "global.h"

extern QElapsedTimer sysTimer;

int next = 0;

struct metaDataGet aa[DATAMAX];

struct metaDataGet dataArea;

#ifdef JUNK
static void hexDump(char *desc, void *addr, int len)
{
  int i;
  unsigned char buff[17];
  unsigned char *pc = (unsigned char*)addr;
  
  // Output description if given.
  if (desc != NULL)
    printf ("%s:  len 0x%x\n", desc, len);
  
  // Process every byte in the data.
  int cnt = 0;
  for (i = 0; i < len; i++) {
    // Multiple of 16 means new line (with line offset).
    
    if( cnt > 0 ) {
      cnt--;
      continue;
    }
    if ((i % 16) == 0) {
      // Just don't print ASCII for the zeroth line.
      if (i != 0) {
	if( buff[0] != 0 ) {
	  printf("  %s\n", buff);
	  memset(buff,0,17);
	}
      }
      
      // Check to see if line of zero's
      int k;
      cnt = 0;
      for(k=0;k<16;k++){
	if(pc[i+k] == 0 ) {
	  cnt++;
	}
      }
      if( cnt == 16 ) {
	cnt--;
	continue;
      }else{
	cnt = 0;
      }

      // Output the offset.
      printf("  %04x ", i);
    }
    
    // Now the hex code for the specific character.
    printf(" %02x", pc[i]);
    
    // And store a printable ASCII character for later.
    if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
      buff[i % 16] = '.';
    } else {
      buff[i % 16] = pc[i];
    }
    
    buff[(i % 16) + 1] = '\0';
  }
  
  // Pad out last line if not exactly 16 characters.
  while ((i % 16) != 0) {
    printf("   ");
    i++;
  }
  
  // And print the final ASCII bit.
  printf("  %s\n", buff);
}
#endif

HH1MetaData::HH1MetaData()
{
}

HH1MetaData::~HH1MetaData()
{
}

int HH1MetaData::init()
{
  next = 0;
  return 0;
}

int HH1MetaData::reset()
{
  next = 0;
  return 0;
}
 
struct metaDataGet *HH1MetaData::getMD( int offset, int *nextOffset )
{
  //  DEBUG() << "offset " << offset;
  if( offset < 0 ) {
    return (struct metaDataGet *)-1;
  }
  if( offset >= DATAMAX ) {
    return (struct metaDataGet *)-1;
  }
  memcpy( &dataArea, &aa[offset], sizeof(struct metaDataGet));
#ifdef JUNK
  if( offset == 0 ){
    hexDump((char *)"Get aa ", &aa[offset], sizeof(struct metaDataGet));
  }
#endif

  *nextOffset = offset + 1;
  if( *nextOffset >= DATAMAX ){
    *nextOffset = 0;
  }

  return &dataArea;
}

int HH1MetaData::setMD( struct metaDataSet *data )
{
#ifdef JUNK
  if( next == 0 ){
    hexDump((char *)"Data ", data, sizeof(struct metaDataSet));
    hexDump((char *)"Target  ", data->target, sizeof(Targets_t));
  }
  
  if( ( next % (3 * 15) ) == 0 ) {
    DEBUG() << "Elapse Time" << sysTimer.elapsed() << " next " << next;
  }
#endif

  memcpy( &aa[next].hdr, &data->hdr, sizeof(struct metaDataHdr));
  memcpy( &aa[next].target, data->target, sizeof(Targets_t));

#ifdef JUNK
  if( next == 0 ){
    hexDump((char *)"Set aa ", &aa[next], sizeof(struct metaDataGet));
  }
#endif
  next = next + 1;
  if( next >= DATAMAX ) {
    //    DEBUG() << "DataPtr " << data << " next " << next;
    next = 0;
  }
  return 0;
}

int HH1MetaData::findMDOffset( int timeMS )
{
  //  DEBUG() << "timeMS " << timeMS;
  int i;
  for(i=0;i< DATAMAX;i++ ) {
    if( aa[i].hdr.timeMilliSecs == timeMS ) {
      //      DEBUG() << "found i " << i;
      return i;
    }
  }
  return -1;
}

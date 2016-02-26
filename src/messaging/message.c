#include "message.h"

#define addByte(data, i) ((char *)data)[i] << (3 - i) * 0x8
#define charsToInt(data) addByte(data, 3) | addByte(data, 2) | addByte(data, 1) | addByte(data, 0)


int message_cksum(char * data, int len) {
  int res = 0;
  int buf = 0;
  int garbage_size = (4 - (len % 4)) * 0x8;

  for(int i=0; i < len; i+=4) {
    buf = charsToInt(data + i);
    /* if we should remove garbage on this step */
    /* int garbage_act = (i + 4) / len ; */
    if(i + 4 >= len) {
      buf = buf >> garbage_size << garbage_size;
    }
    res ^= buf;
  }
  
  return res;
}


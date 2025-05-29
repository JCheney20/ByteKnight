#include "debug.h"
#include "defs.h"
#include "stdio.h"
#include <stdlib.h>

#ifdef WIN32
#include "windows.h"
#else
#include "sys/time.h"
#endif /* ifdef WIN32*/

int GetTimeMS(){
  #ifdef WIN32
  return GetTickCount();
#else 
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
  #endif 
}

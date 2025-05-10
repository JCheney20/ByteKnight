#include "stdio.h"
#define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed ",#n); \
printf("on %s ",__DATE__); \
printf("@ %s ",__TIME__); \
printf("in File %s ",__FILE__); \
printf("on Line %d\n",__LINE__); \
exit(1);}
#endif // !DEBUG


#include "stdio.h"
#define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif // !DEBUG


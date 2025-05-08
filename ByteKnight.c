#include "defs.h"
#include "stdio.h"

int main(){
  int i = 0;
  AllInit();


  for (i=0;i<BRD_SQ_NUM; ++i) {
    if(i%10 == 0) printf("\n");
    printf("%5d", SQ120toSQ64[i]);
  }

  printf("\n");
  printf("\n");
  for (i=0; i<64; ++i) {
    if(i%8 == 0) printf("\n");
    printf("%5d", SQ64toSQ120[i]);
  }
  


  return 0;
}

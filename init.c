#include "defs.h"

int SQ120toSQ64[BRD_SQ_NUM];
int SQ64toSQ120[64];

void InitSQ120to64(){

  int i = 0;
  int file = FILE_A;
  int rank = RANK_1;
  int sq = A1;
  int sq64 = 0;

  for(i=0; i<BRD_SQ_NUM;++i){
    SQ120toSQ64[i] = 65;
  }

  for(i=0; i<64;++i){
    SQ64toSQ120[i] = 120;
  }

  for (rank = RANK_1; rank <= RANK_8; ++rank) {
    for (file = FILE_A; file <= FILE_H; ++file) {
      sq = FR2SQ(file, rank);
      SQ64toSQ120[sq64] = sq;
      SQ120toSQ64[sq] = sq64;
      sq64++;
    }
  }
}

void AllInit(){
  InitSQ120to64();

}

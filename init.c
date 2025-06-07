#include "defs.h"
#include "stdlib.h"
#include "stdio.h"

int SQ120toSQ64[BRD_SQ_NUM];
int SQ64toSQ120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys [13][120];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

U64 FileBBMask[8];
U64 RanksBBMask[8];

U64 BlackPassedMask[64];
U64 WhitePassedMask[64];
U64 IsolatedMask[64];
S_OPTIONS EngineOpt[1];

void InitEvalMasks(){
  int sq, t_sq, r, f;

  for (sq = 0; sq < 8; ++sq) {
    FileBBMask[sq] = 0ULL;
    RanksBBMask[sq] = 0ULL;
  }

  for (r = RANK_8; r >= RANK_1; r--) {
    for (f = FILE_A; f<= FILE_H; f++) {
      sq = r * 8 + f;
      FileBBMask[f] |= (1ULL << sq);
      RanksBBMask[r] |= (1ULL << sq);
    }
  }

  for (sq = 0; sq < 64; ++sq) {
    IsolatedMask[sq] = 0ULL;
    WhitePassedMask[sq] = 0ULL;
    BlackPassedMask[sq] = 0ULL;
  }

  for (sq = 0; sq < 64; ++sq) {
    t_sq = sq + 8;

    while (t_sq < 64) {
      WhitePassedMask[sq] |= (1ULL << t_sq);
      t_sq += 8;
    }

    t_sq = sq-8;
    while (t_sq >= 0) {
      BlackPassedMask[sq] |= (1ULL << t_sq);
      t_sq -= 8;
    }

    if (FilesBrd[SQ120(sq)] > FILE_A) {
      IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] -1];

      t_sq = sq + 7;
      while (t_sq < 64) {
        WhitePassedMask[sq] |= (1ULL << t_sq);
        t_sq += 8;
      }
      
      t_sq = sq - 9;
      while (t_sq >= 0) {
        BlackPassedMask[sq] |= (1ULL << t_sq);
        t_sq -= 8;
      }
    }
    
    if (FilesBrd[SQ120(sq)] < FILE_H) {
      IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] +1];

      t_sq = sq + 9;
      while (t_sq < 64) {
        WhitePassedMask[sq] |= (1ULL << t_sq);
        t_sq += 8;
      }
      
      t_sq = sq - 7;
      while (t_sq >= 0) {
        BlackPassedMask[sq] |= (1ULL << t_sq);
        t_sq -= 8;
      }
    }
  }
}

void InitFilesRanksBrd(){
  int i = 0;
  int file = FILE_A;
  int rank = RANK_1;
  int sq = A1;

  for (i=0; i<BRD_SQ_NUM;++i) {
    FilesBrd[i] = NO_SQ;
    RanksBrd[i] = NO_SQ;
  }

  for (rank = RANK_1; rank <=RANK_8; ++rank) {
    for (file = FILE_A; file <= FILE_H; ++file) {
      sq = FR2SQ(file, rank);
      FilesBrd[sq] = file;
      RanksBrd[sq] = rank;
    }
  }
}

void InitHashKeys(){
  int i, j;
  for (i=0; i<13; ++i) {
    for (j=0;j<120;++j) {
      PieceKeys[i][j] = RAND_64;
    }
  }
  SideKey = RAND_64;
  for (i=0;i<16;++i) {
    CastleKeys[i] = RAND_64;
  }

}

void InitBitMasks(){
  int i = 0;
  for (i=0; i<64; i++) {
    SetMask[i] = 0ULL;
    ClearMask[i] = 0ULL;
  
  }
  
  for (i=0; i<64; i++) {
    SetMask[i] |= (1ULL << i);
    ClearMask[i] = ~SetMask[i];
  }
}

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
  InitBitMasks();
  InitHashKeys();
  InitFilesRanksBrd();
  InitEvalMasks();
  InitMvvLva();
  InitPolyBook();

}

#include "debug.h"
#include "defs.h"

#ifdef WIN32
#include "windows.h"
#else
#include "sys/time.h"
#include "unistd.h"
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


// int get_pst_value(int piece, int square, int is_endgame) {
//     /*
//      * Get piece square table value
//      * piece: 0=pawn, 1=knight, 2=bishop, 3=rook, 4=queen, 5=king
//      * square: 0-63 (a1=0, b1=1, ..., h8=63)
//      * is_endgame: 0=middlegame, 1=endgame
//      */
//     if (is_endgame) {
//         return eg_table[piece][MIRROR64(square)];
//     } else {
//         return mg_table[piece][MIRROR64(square)];
//     }
// }

// void TestPST(){
//   int sq, file, rank, piece;
//
// printf("MIDDLE GAME BOARDS");CR;
//     for (int pce = PAWN; pce <=KING ; ++pce) {
//       printf("PST of: %d\n", pce);
//       for (sq = 0; sq < 64; sq++) {
//         if (sq % 8 == 0) CR;
//         int val = get_pst_value(pce+1, sq, 0);
//         printf("%d\t",val);
//       }
//   CR;
//     }
//
//
//   printf("ENDGAME BOARDS");CR;
//     for (int pce = PAWN; pce <=KING ; ++pce) {
//       printf("PST of: %d\n", pce);
//       for (sq = 0; sq < 64; sq++) {
//         if (sq % 8 == 0) CR;
//         int val = get_pst_value(pce+1, sq, 1);
//         printf("%d\t",val);
//       }
//   CR;
//     }
//
// }



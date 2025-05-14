#include "defs.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"

#define PAWNMV_FEN_W "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define PAWNMV_FEN_B "rnbqkbnr/p1p1p3/3p3p/1p1p4/2P1Pp2/8/PP1P1PpP/RNBQKB1R b KQkq e3 0 1"
#define KN_KINGS "5k2/1n6/4n3/6N1/8/3N4/8/5K2 b - - 0 1"
#define POS "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"

#define ROOKS "6k1/8/5r2/8/1nR5/5N2/8/6K1 w - - 0 1"
#define QUEENS_FEN "6k1/8/4nq2/8/1nQ5/5N2/1N6/6K1 w - - 0 1"
#define BISHOPS_FEN "6k1/1b6/4n3/8/1n4B1/1B3N2/1N6/2b3K1 b - - 0 1"
#define PLAY "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1"

int main(){
  AllInit();
  S_BOARD board[1];
  S_MOVELIST list[1];

  // ParseFen(KN_KINGS, board);
  // ParseFen(KN_KINGS, board);
  ParseFen(PLAY, board);
  // ParseFen(PAWNMV_FEN_B, board);
  PrintBoard(board);

  GenerateAllMvs(board, list);

  PrintMvList(list);

  return 0;
}

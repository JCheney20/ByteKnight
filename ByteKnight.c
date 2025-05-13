#include "defs.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"

#define FEN1 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" 
#define FEN2 "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2"
#define FEN3 "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
#define FEN4 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define FEN5 "8/3q1p2/8/5P2/4Q3/8/8/8 w KQkq - 0 1"
#define PAWNMV_FEN_W "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define PAWNMV_FEN_B "rnbqkbnr/p1p1p3/3p3p/1p1p4/2P1Pp2/8/PP1P1PpP/RNBQKB1R b KQkq e3 0 1"
#define KN_KINGS "5k2/1n6/4n3/6N1/8/3N4/8/5K2 b - - 0 1"
#define POS "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"

int main(){
  AllInit();
  S_BOARD board[1];
  S_MOVELIST list[1];

  // ParseFen(KN_KINGS, board);
  ParseFen(POS, board);
  PrintBoard(board);


  GenerateAllMvs(board, list);

  PrintMvList(list);

  return 0;
}

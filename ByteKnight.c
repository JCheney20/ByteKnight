#include "defs.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"


#define PERFT_FEN "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define INTERACT_FEN "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1"

int main(){
  AllInit();
  S_BOARD board[1];
  S_MOVELIST list[1];
  ParseFen(INTERACT_FEN, board);

  // PerftTest(3, board);
  //
  char input[6];
  int move = NOMOVE;

  while (TRUE) {
    PrintBoard(board);
    printf("Please enter a move :>> ");
    fgets(input, 6, stdin);

    if (input[0] == 'q') {
      break;
    } else if (input[0] == 't') {
      takeMv(board);
    } else {
      move = ParseMv(input, board);
      if (move != NOMOVE) {
        makeMv(board, move);
      }
    }

    fflush(stdin);
  }




  return 0;
}

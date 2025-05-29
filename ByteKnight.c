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

  ParseFen(START_FEN, board);

  int PvNum, Max;
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
    } else if (input[0] == 'p'){
      // PerftTest(4, board);
      Max = GetPvLine(4, board);
      printf("PvLine of %d moves: ", Max);
      for (PvNum = 0; PvNum<Max; ++PvNum) {
        move = board->PvArr[PvNum];
        printf(" %s", PrMv(move));
      }
      CR;
    } else {
      move = ParseMv(input, board);
      if (move != NOMOVE) {
        StorePvMove(board, move);
        makeMv(board, move);
      } else printf("Move Not Parsed:%s\n",input);
    }
    fflush(stdin);
  }




  return 0;
}

#include "defs.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"


#define PERFT_FEN "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define INTERACT_FEN "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1"
#define WAC1 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"


int main(){
  AllInit();


  S_BOARD board[1];
  S_MOVELIST list[1];
  S_SEARCHINFO info[1];

  initPvTable(board->PvTable);
  ParseFen(WAC1, board);

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
    } else if (input[0] == 's'){
      info->depth = 6;
      SearchPosition(board, info);
    } else {
      move = ParseMv(input, board);
      if (move != NOMOVE) {
        StorePvMove(board, move);
        makeMv(board, move);
      } else printf("Move Not Parsed:%s\n",input);
    }
    fflush(stdin);
  }

  free(board->PvTable->pTable);
  return 0;
}

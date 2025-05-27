#include "debug.h"
#include "defs.h"
#include "stdio.h"

long leafNodes;

void Perft(int depth, S_BOARD *pos){

  ASSERT(CheckBrd(pos));

  if (depth == 0) {
    leafNodes++;
    return;
  }

  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);

  int MvNum =0;
  for (MvNum = 0; MvNum<list->count; ++MvNum) {
    if (!makeMv(pos, list->moves[MvNum].mv)) {
      continue;
    }
    Perft(depth-1, pos);
    takeMv(pos);
  }
  return;
}

void PerftTest(int depth, S_BOARD *pos){

  ASSERT(CheckBrd(pos));

  PrintBoard(pos);
  printf("\nStarting Test to Depth:%d\n", depth);

  leafNodes=0;

  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);

  int MvNum =0;
  int move;
  for (MvNum = 0; MvNum<list->count; ++MvNum) {
    move = list->moves[MvNum].mv;
    if (!makeMv(pos, move)) {
      continue;
    }
    long CumNodes = leafNodes;
    Perft(depth-1, pos);
    takeMv(pos);
    long OldNodes = leafNodes - CumNodes;
    printf("move %d : %s : %ld nodes visited\n", MvNum+1,PrMv(move), OldNodes);
  }
  printf("\nTest Complete: %ld nodes visited\n", leafNodes);
  return;
}



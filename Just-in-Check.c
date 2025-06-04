#include "defs.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#define PERFT_FEN "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define INTERACT_FEN "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1"
#define WAC1 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"


int main(){
  AllInit();

  S_BOARD pos[1];
  S_SEARCHINFO info[1];
  initPvTable(pos->PvTable);


  printf("Welcome to %s! Type 'console' for console mode...\n", NAME);


  char ln[256];
  while (TRUE) {
    memset(&ln[0], 0, sizeof(ln));
    fflush(stdout);
    if (!fgets(ln, 256, stdin)) continue; 
    if (ln[0] == '\n') continue; 
    if (!strncmp(ln, "uci", 3)) {
      UCI_Loop(pos, info);
      if (info->quit == TRUE) break; 
      continue;
    } else if (!strncmp(ln, "xboard",6)) {
      xBoard_Loop(pos, info);
      if (info->quit == TRUE) break; 
      continue;
    } else if (!strncmp(ln, "console", 4)) {
      Console_Loop(pos, info);
      if (info->quit == TRUE) break; 
      continue;
    } else if (!strncmp(ln, "quit", 4)) break;
  }

  free(pos->PvTable->pTable);

  return 0;
}

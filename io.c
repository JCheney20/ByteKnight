#include "stdio.h"
#include "defs.h"
#include "debug.h"

char *PrSq(const int sq){
  static char SqStr[3];

  int file = FilesBrd[sq];
  int rank = RanksBrd[sq];

  sprintf(SqStr, "%c%c", ('a'+file), ('1'+rank));

  return SqStr;
}

char *PrMv(const int mv){
  static char MvStr[6];
  int ff = FilesBrd[FROMSQ(mv)];
  int rf = RanksBrd[FROMSQ(mv)];
  int ft = FilesBrd[TOSQ(mv)];
  int rt = RanksBrd[TOSQ(mv)];

  int promoted = PROMOTED(mv);

  if (promoted) {
    char pchar = 'q';
    if (isKn(promoted)) {
      pchar = 'n';
    } else if (isRQ(promoted) && !isBQ(promoted)) {
      pchar = 'r';
    } else if (!isRQ(promoted) && isBQ(promoted)) {
      pchar = 'b';
    }
    sprintf(MvStr, "%c%c%c%c%c", ('a'+ff),('1'+rf), ('a'+ft), ('1'+rt), pchar);
  } else {
    sprintf(MvStr, "%c%c%c%c", ('a'+ff),('1'+rf), ('a'+ft), ('1'+rt));
  }

  return MvStr;
}

int ParseMv(char *ptrChar, S_BOARD *pos){
  if(ptrChar[1]> '8' || ptrChar[1] <'1') return NOMOVE;
  if(ptrChar[3]> '8' || ptrChar[3] <'1') return NOMOVE;
  if(ptrChar[0]> 'h' || ptrChar[0] <'a') return NOMOVE;
  if(ptrChar[2]> 'h' || ptrChar[2] <'a') return NOMOVE;

  int from = FR2SQ(ptrChar[0] - 'a', ptrChar[1] - '1');
  int to = FR2SQ(ptrChar[2] - 'a', ptrChar[3] - '1');

  printf("ptrChar: %s from: %d to:%d\n",ptrChar, from, to);

  ASSERT(SqOnBoard(from) && SqOnBoard(to));

  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);
  int MvNum, move;
  int PromPce = EMPTY;

  for (MvNum = 0; MvNum < list->count; ++MvNum) {
    move = list->moves[MvNum].mv;
    if (FROMSQ(move)==from && TOSQ(move)==to) {
      PromPce = PROMOTED(move);
      if (PromPce != EMPTY) {
        if (isRQ(PromPce) && !isBQ(PromPce) && ptrChar[4]=='r') {
          return move;
        } else if (!isRQ(PromPce) && isBQ(PromPce) && ptrChar[4]=='b') {
          return move;
        } else if (isRQ(PromPce) && isBQ(PromPce) && ptrChar[4]=='q') {
          return move;
        } else if (isKn(PromPce) && ptrChar[4]=='n') {
          return move;
        }
        continue;
      }
      return move;
    }
  }
  return NOMOVE;
}

void PrintMvList(const S_MOVELIST *list){
  int i,score,mv;
  printf("MoveList:\n");

  for (i=0;i<list->count;++i) {
    mv = list->moves[i].mv;
    score = list->moves[i].score;

    printf("Move: %d > %s (score:%d)\n",i+1,PrMv(mv), score);
  }
  printf("MoveList Total %d Moves:\n\n",list->count);
}


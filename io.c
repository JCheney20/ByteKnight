#include "stdio.h"
#include "defs.h"

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

void PrintMvList(const S_MOVELIST *list){
  int i,score,mv;
  printf("MoveList:\n");

  for (i=0;i<list->count;++i) {
    mv = list->mv[i].mv;
    score = list->mv[i].score;

    printf("Move: %d > %s (score:%d)\n",i+1,PrMv(mv), score);
  }
  printf("MoveList Total %d Moves:\n\n",list->count);
}

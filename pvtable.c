#include "debug.h"
#include "defs.h"
#include "stdio.h"
#include <stdlib.h>


const int PvSize = 0x100000 * 2;

int GetPvLine(const int depth, S_BOARD *pos){
  ASSERT(depth < MAXDEPTH);

  int mv = ProbePvTable(pos);
  int count = 0;

  while (mv != NOMOVE && count < depth) {
    ASSERT(count < MAXDEPTH);

    if (MoveExists(pos, mv)) {
      makeMv(pos, mv);
      pos->PvArr[count++] = mv;
    } else break;
    mv = ProbePvTable(pos);
  }

  while (pos->ply > 0) {
    takeMv(pos);
  }

  return count;
}

void ClearPvTable(S_PVTABLE *table){
  S_PVENTRY *pvEntry;
  for (pvEntry = table->pTable; pvEntry < table->pTable + table->numEntries; pvEntry++) {
    pvEntry->posKey = 0ULL;
    pvEntry->mv = NOMOVE;
  
  }
}


void initPvTable(S_PVTABLE *table){
  table->numEntries = PvSize / sizeof(S_PVENTRY);
  table->numEntries -= 2;
  free(table->pTable);
  table->pTable = (S_PVENTRY *) malloc(table->numEntries * sizeof(S_PVENTRY));
  ClearPvTable(table);
  printf("PVTable init complete with %d entries\n", table->numEntries);
}

int ProbePvTable(const S_BOARD *pos){
  int i = pos->posKey % pos->PvTable->numEntries -1;

  ASSERT(i>=0 && i<=pos->PvTable->numEntries-1);

  if (pos->PvTable->pTable[i].posKey == pos->posKey) {
    return pos->PvTable->pTable[i].mv;
  }

  return NOMOVE;
}

void StorePvMove(const S_BOARD *pos, const int move){
  int i = pos->posKey % pos->PvTable->numEntries -1;

  ASSERT(i>=0 && i<=pos->PvTable->numEntries-1);

  pos->PvTable->pTable[i].mv = move;
  pos->PvTable->pTable[i].posKey = pos->posKey;

}


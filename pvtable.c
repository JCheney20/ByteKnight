#include "debug.h"
#include "defs.h"
#include "stdio.h"
#include <stdlib.h>

S_HASHTABLE HashTable[1];


void ClearHashTable(S_HASHTABLE *table){
  S_HASHENTRY *tableEntry;
  for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
    tableEntry->posKey = 0ULL;
    tableEntry->mv = NOMOVE;
    tableEntry->depth = 0;
    tableEntry->score = 0;
    tableEntry->flags = 0;
    tableEntry->age = 0;
  }
  table->newWrite = 0;
  table->curAge = 0;
}


void InitHashTable(S_HASHTABLE *table, int MB){
  table->numEntries = (0x100000 * MB) / sizeof(S_HASHENTRY);
  table->numEntries -= 2;

  if (table->pTable != NULL) free(table->pTable);
  table->pTable = (S_HASHENTRY *) malloc(table->numEntries * sizeof(S_HASHENTRY));

  if(table->pTable == NULL) {
    printf("Hash Allocation Failed, trying %dMB...\n",MB/2);
    InitHashTable(table,MB/2);
  } else {
    ClearHashTable(table);
    printf("HashTable init complete with %d entries\n",table->numEntries);
  } 
}

int ProbeHashEntry(S_BOARD *pos, S_HASHTABLE *table, int *move, int *score, int alpha, int beta, int depth) {

  int i = pos->posKey % table->numEntries ;
  
    ASSERT(i >= 0 && i <= table->numEntries - 1);
    ASSERT(depth>=1&&depth<MAXDEPTH);
    ASSERT(alpha<beta);
    ASSERT(alpha>=-INF&&alpha<=INF);
    ASSERT(beta>=-INF&&beta<=INF);
    ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	

  if (table->pTable[i].posKey == pos->posKey) {
    *move = table->pTable[i].mv;
    if (table->pTable[i].depth>=depth) {
      table->hit++;

      ASSERT(table->pTable[i].depth>=1 && table->pTable[i].depth<MAXDEPTH);
      ASSERT(table->pTable[i].flags>=HFALPHA&&table->pTable[i].flags<=HFEXACT);

      *score = table->pTable[i].score;
      if (*score > MATE) *score -= pos->ply; 
      else if (*score < -MATE) *score += pos->ply;

      switch (table->pTable[i].flags) {

        case HFALPHA:
          if (*score<=alpha) {*score=alpha; return TRUE;}
          break;
        case HFBETA:
          if (*score>=beta) {*score=beta; return TRUE;}
          break;
        case HFEXACT:
          return TRUE;
          break;
        default: ASSERT(FALSE); break;
      }
    }
  }

  return FALSE;
}

void StoreHashEntry(S_BOARD *pos, S_HASHTABLE *table, const int move, int score, const int flags, const int depth){
  int i = pos->posKey % table->numEntries ;

  ASSERT(i>=0 && i<=table->numEntries-1);
  ASSERT(depth>=1&&depth<MAXDEPTH);
  ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
  ASSERT(score>=-INF&&score<=INF);
  ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);

  int replace = FALSE;

  if (table->pTable[i].posKey == 0) {
    table->newWrite++;
    replace = TRUE;
  } else if (table->pTable[i].age < table->curAge || table->pTable[i].depth <= depth) {
    replace = TRUE;
    table->overWrite++;
  } 

  if (replace == FALSE) return;
  

  if (score>MATE) score += pos->ply;
  else if (score < -MATE) score -= pos->ply;
  

  table->pTable[i].mv = move;
  table->pTable[i].posKey = pos->posKey;
  table->pTable[i].flags = flags;
  table->pTable[i].score = score;
  table->pTable[i].depth = depth;
  table->pTable[i].age = table->curAge;
}

int ProbePvMove(const S_BOARD *pos, const S_HASHTABLE *table) {

  int i = pos->posKey % table->numEntries ;
  
  ASSERT(i >= 0 && i <= table->numEntries - 1);
  ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);


  if (table->pTable[i].posKey == pos->posKey) {
    return table->pTable[i].mv;
  }

  return NOMOVE;
}

int GetPvLine(const int depth, S_BOARD *pos, const S_HASHTABLE *table){
  ASSERT(depth < MAXDEPTH && depth >= 1);

  int mv = ProbePvMove(pos, table);
  int count = 0;

  while (mv != NOMOVE && count < depth) {
    ASSERT(count < MAXDEPTH);

    if (MoveExists(pos, mv)) {
      makeMv(pos, mv);
      pos->PvArr[count++] = mv;
    } else break;
    mv = ProbePvMove(pos, table);
  }

  while (pos->ply > 0) takeMv(pos);

  return count;
}


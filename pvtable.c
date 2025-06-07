#include "debug.h"
#include "defs.h"
#include "stdio.h"
#include <stdlib.h>

#define EXTRACT_SCORE(x) ((x & 0xFFFF) - INF)
#define EXTRACT_DEPTH(x) ((x >> 16) & 0x3F)
#define EXTRACT_FLAGS(x) ((x >> 23) & 0x3)
#define EXTRACT_MOVE(x) ((int)(x >> 25))

#define FOLD_DATA(sc, de, fl, mv)((sc + INF) | (de << 16) | (fl << 23) | ((U64)mv << 25))

S_HASHTABLE HashTable[1];

/*void VerifyEntrySMP(S_HASHENTRY *entry){
  U64 data = FOLD_DATA(entry->score, entry->depth, entry->flags, entry->mv);
  U64 key = entry->posKey ^ data;

  if (data != entry->smp_data) { printf("data err:"); exit(1); }
  if (key != entry->smp_key) { printf("key err:"); exit(1); }

  int move = EXTRACT_MOVE(data);
  int flag = EXTRACT_FLAGS(data);
  int score = EXTRACT_SCORE(data);
  int depth = EXTRACT_DEPTH(data);

  if (move != entry->mv) { printf("move err:"); exit(1); }
  if (flag != entry->flags) { printf("flags err:"); exit(1); }
  if (score != entry->score) { printf("score err:"); exit(1); }
  if (depth != entry->depth) { printf("depth err:"); exit(1); }

}*/

void ClearHashTable(S_HASHTABLE *table){
  S_HASHENTRY *tableEntry;
  for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
  /*  tableEntry->posKey = 0ULL;
    tableEntry->mv = NOMOVE;
    tableEntry->depth = 0;
    tableEntry->score = 0;
    tableEntry->flags = 0;*/
    tableEntry->age = 0;
    tableEntry->smp_data = 0ULL;
    tableEntry->smp_key = 0ULL;
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
    ASSERT(alpha>=-AB_BOUND&&alpha<=AB_BOUND);
    ASSERT(beta>=-AB_BOUND&&beta<=AB_BOUND);
    ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);
	
    U64 t_key = pos->posKey ^ table->pTable[i].smp_data;

  if (table->pTable[i].smp_key == t_key) {

    int smp_depth = EXTRACT_DEPTH(table->pTable[i].smp_data);
    int smp_move = EXTRACT_MOVE(table->pTable[i].smp_data);
    int smp_flags = EXTRACT_FLAGS(table->pTable[i].smp_data);
    int smp_score = EXTRACT_SCORE(table->pTable[i].smp_data);
    
    *move = smp_move;
    if (smp_depth>=depth) {
      table->hit++;

      ASSERT(smp_depth>=1 && smp_depth<MAXDEPTH);
      ASSERT(smp_flags>=HFALPHA&&smp_flags<=HFEXACT);

      *score = smp_score;
      if (*score > MATE) *score -= pos->ply; 
      else if (*score < -MATE) *score += pos->ply;

      switch (smp_flags) {

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
  ASSERT(score>=-AB_BOUND&&score<=AB_BOUND);
  ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);

  int replace = FALSE;

  if (table->pTable[i].smp_key == 0) {
    table->newWrite++;
    replace = TRUE;
  } else if (table->pTable[i].age < table->curAge || EXTRACT_DEPTH(table->pTable[i].smp_data) <= depth) {
    replace = TRUE;
    table->overWrite++;
  } 

  if (replace == FALSE) return;
  

  if (score>MATE) score += pos->ply;
  else if (score < -MATE) score -= pos->ply;

  U64 smp_data = FOLD_DATA(score, depth, flags, move);
  U64 smp_key = pos->posKey ^ smp_data;
  
  // table->pTable[i].mv = move;
  // table->pTable[i].posKey = pos->posKey;
  // table->pTable[i].flags = flags;
  // table->pTable[i].score = score;
  // table->pTable[i].depth = depth;
  table->pTable[i].age = table->curAge;
  table->pTable[i].smp_key = smp_key;
  table->pTable[i].smp_data = smp_data;

  // VerifyEntrySMP(&table->pTable[i]);


}

int ProbePvMove(const S_BOARD *pos, const S_HASHTABLE *table) {

  int i = pos->posKey % table->numEntries ;
  U64 t_key = pos->posKey ^ table->pTable[i].smp_data;

  
  ASSERT(i >= 0 && i <= table->numEntries - 1);
  ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH);


  if (table->pTable[i].smp_key == t_key) {
    return EXTRACT_MOVE(table->pTable[i].smp_data);
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


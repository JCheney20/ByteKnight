#include "pthread.h"
#include "stdio.h"
#include "string.h"
#include "defs.h"
#include "debug.h"

pthread_t tid[MAXTHREADS];

pthread_t LaunchSearch_t(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table){
  S_SEARCH_THREAD_DATA *pSearchData = malloc(sizeof(S_SEARCH_THREAD_DATA));

  pSearchData->pos_t = pos;
  pSearchData->info_t = info;
  pSearchData->table_t = table;

  pthread_t t;
  pthread_create(&t, NULL,  SearchPos_t, (void *)pSearchData);

  return t;
}

void JoinSearch_t(pthread_t tid,S_SEARCHINFO *info){
  info->stopped = TRUE;
  pthread_join(tid, NULL);
}

void* SearchPos_t(void *data){
  S_SEARCH_THREAD_DATA *searchData = (S_SEARCH_THREAD_DATA *)data;
  S_BOARD *pos = malloc(sizeof(S_BOARD));
  memcpy(pos, searchData->pos_t, sizeof(S_BOARD));

  SearchPosition(pos, searchData->table_t, searchData->info_t);
  free(pos);
  // printf("Freed\n");
  return NULL;
}

void* start_t(void *data){
  S_SEARCH_WORKER_DATA *t_Data = (S_SEARCH_WORKER_DATA *)data;

    IterativeDeepen(t_Data);

  if (t_Data->t_Num == 0) {
    if (t_Data->info->GAME_MODE == UCIMODE) {
      printf("bestmove %s\n", PrMv(t_Data->bestMv));
    } else if (t_Data->info->GAME_MODE == XBOARDMODE) {
      printf("move %s\n", PrMv(t_Data->bestMv));
      makeMv(t_Data->pos, t_Data->bestMv);
    } else {
      CR;CR;
      printf("===** %s makes move %s **===", NAME, PrMv(t_Data->bestMv));
      CR;CR;
      makeMv(t_Data->pos, t_Data->bestMv);
      PrintBoard(t_Data->pos);
    }
  }

  free(t_Data);
  return NULL;
}

void setUp_t(int t_Num, pthread_t *tid, S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table){
  S_SEARCH_WORKER_DATA *pt_Data = malloc(sizeof(S_SEARCH_WORKER_DATA));
  pt_Data->pos = malloc(sizeof(S_BOARD));
  memcpy(pt_Data->pos, pos, sizeof(S_BOARD));
  pt_Data->info = info;
  pt_Data->table_t = table;
  pt_Data->t_Num = t_Num;
  pthread_create(tid, NULL, start_t, (void *)pt_Data);
}

void creatSearch_t(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info){
  printf("Creating Search Workers: %d\n", info->NumThreads);
  for (int i = 0; i < info->NumThreads; i++) {
    setUp_t(i, &tid[i], pos, info, table);
  }
}



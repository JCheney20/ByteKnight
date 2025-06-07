#include "pthread.h"
#include "stdio.h"
#include "string.h"
#include "defs.h"
#include "debug.h"

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
  printf("Freed\n");
  return NULL;
}



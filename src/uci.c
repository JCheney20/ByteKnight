#include "defs.h"
#include "debug.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <time.h>

#define INPUTBUFFER 400 * 6
pthread_t main_t;

void ParseGo(char* ln, S_SEARCHINFO *info, S_BOARD *pos, S_HASHTABLE *table){


  int depth = -1, movestogo = 30, movetime = -1;
  int time = -1, inc  = 0;
  char *ptr = NULL;
  info->t_set = FALSE;

  if ((ptr = strstr(ln, "infinite"))) {
    ;
  }

  if ((ptr = strstr(ln, "binc")) && pos->side == BLACK) {
    inc = atoi(ptr + 5);
  }

  if ((ptr = strstr(ln, "winc")) && pos->side == WHITE) {
    inc = atoi(ptr + 5);
  }
  
  if ((ptr = strstr(ln, "btime")) && pos->side == BLACK) {
    time = atoi(ptr + 6);
  }

  if ((ptr = strstr(ln, "wtime")) && pos->side == WHITE) {
    time = atoi(ptr + 6);
  }

  if ((ptr = strstr(ln, "movestogo"))) {
    movestogo = atoi(ptr + 10);
  }
  
  if ((ptr = strstr(ln, "movetime"))) {
    movetime = atoi(ptr + 9);
  }

  if ((ptr = strstr(ln, "depth"))) {
    depth = atoi(ptr + 6);
  }

  if (movetime != -1) {
    time = movetime;
    movestogo = 1;
  }
  
  info->start_time = GetTimeMS();
  info->depth = depth;

  if (time != -1) {
    info->t_set = TRUE;
    time /= movestogo;
    time -= 50;
    info->stop_time = info->start_time + time + inc;
  }

  if (depth == -1) {
    info->depth = MAXDEPTH;
  }

  printf("time: %d start: %d stop: %d depth: %d timeset: %d\n", time, info->start_time,info->stop_time, info->depth,info->t_set);

  main_t = LaunchSearch_t(pos, info, table);
}

void ParsePos(char* lnIn, S_BOARD *pos){

  lnIn += 9;
  char *ptrChar = lnIn;

  if (strncmp(lnIn, "startpos", 8) == 0) {
    ParseFen(START_FEN, pos);
  } else {
    ptrChar = strstr(lnIn, "fen");
    if (ptrChar == NULL) {
      ParseFen(START_FEN, pos);
    } else {
      ptrChar +=4;
      ParseFen(ptrChar, pos);
    }
  }

  ptrChar = strstr(lnIn, "moves");
  int mv;

  if (ptrChar != NULL) {
    ptrChar += 6;
    while (*ptrChar) {
      mv = ParseMv(ptrChar, pos);
      if (mv == NOMOVE) break;
      makeMv(pos, mv);
      pos->ply=0;
      while (*ptrChar && *ptrChar != ' ') ptrChar++;
      ptrChar++;
    }
  }
  PrintBoard(pos);
}

void UCI_Loop(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info){
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  info->GAME_MODE = UCIMODE;

  char ln[INPUTBUFFER];
  printf("id name %s \n", NAME);
  printf("id author %s\n",AUTHOR);
  printf("uciok\n");

  int MB = 64;
  int threads = MAXTHREADS;


  while (TRUE) {
    memset(&ln[0], 0, sizeof(ln));
    fflush(stdout);
    
    if (!fgets(ln, INPUTBUFFER, stdin)) continue;
    if (ln[0] == '\n') continue;

    if (!strncmp(ln, "isready", 7)) {
      printf("readyok\n");
      continue;
    } else if (!strncmp(ln, "position", 8)) {
      ParsePos(ln, pos);
    } else if (!strncmp(ln, "ucinewgame", 10)) {
      ClearHashTable(HashTable);
      ParsePos("position startpos\n", pos);
    } else if (!strncmp(ln, "go", 2)) {
      ParseGo(ln, info, pos, table);
    } else if (!strncmp(ln, "run", 3)) {
      ParseFen(START_FEN, pos);
      ParseGo("go infinite", info, pos, table);
    } else if (!strncmp(ln, "quit", 4)) {
      info->quit = TRUE;
      JoinSearch_t(main_t, info);
      break;
    } else if (!strncmp(ln, "stop", 4)) {
      JoinSearch_t(main_t, info);
    } else if (!strncmp(ln, "uci", 3)) {
      printf("id name %s\n", NAME);
      printf("id author Justin Cheney\n");
      printf("uciok\n");
    } else if (!strncmp(ln, "setoption name Hash value ", 26)) {			
      sscanf(ln,"%*s %*s %*s %*s %d",&MB);
      if(MB < 4) MB = 4;
      if(MB > MAXHASH) MB = MAXHASH;
      printf("Set Hash to %d MB\n",MB);
      InitHashTable(table, MB);
    } else if (!strncmp(ln, "setoption name Threads value ", 29)) {			
      sscanf(ln,"%*s %*s %*s %*s %d",&threads);
      if(threads > MAXTHREADS) threads = MAXTHREADS;
      printf("Set Num Threads to %d\n",threads);
      info->NumThreads = threads;
    } else if (!strncmp(ln, "setoption name Book value ", 26)) {			
      char *ptrTrue = NULL;
      ptrTrue = strstr(ln, "true");
      if(ptrTrue != NULL) {
        EngineOpt->USE_BOOK = TRUE;
        printf("Book Use: ON\n");
      } else {
        EngineOpt->USE_BOOK = FALSE;
        printf("Book Use: OFF\n");
      }
    }
    if (info->quit) break;
  }
}


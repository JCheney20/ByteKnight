#include "stdio.h"
#include "defs.h"
#include "string.h"

int threeFoldRep(const S_BOARD *pos){
  int i = 0, r = 0;
  for (i = 0; i < pos->histPly; ++i) {
    if (pos->history[i].posKey == pos->posKey) r++;
  }
  return r;
}

int DrawMaterial(const S_BOARD *pos){
  if (pos->pceNum[wP] || pos->pceNum[bP]) return FALSE;
  if (pos->pceNum[wQ] || pos->pceNum[bQ] || pos->pceNum[wR] || pos->pceNum[bR]) return FALSE;
  if (pos->pceNum[wB] > 1 || pos->pceNum[bB] > 1) return FALSE;
  if (pos->pceNum[wN] > 1 || pos->pceNum[bN] > 1) return FALSE;
  if (pos->pceNum[wN] && pos->pceNum[wB] ) return FALSE;
  if (pos->pceNum[bN] && pos->pceNum[bB] ) return FALSE;

  return TRUE;
}

int checkresult(S_BOARD *pos){
  if (pos->fiftyMv > 100) {
    printf(" 1/2 - 1/2 {fifty move rule [claimed by %s]}\n", NAME); return TRUE;
  }
  
  if (threeFoldRep(pos)>= 2) {
    printf(" 1/2 - 1/2 {3-fold repetition [claimed by %s]}\n",NAME); return TRUE;
  }
  
  if (DrawMaterial(pos) == TRUE) {
    printf(" 1/2 - 1/2 {Insufficient mating material [claimed by %s]}\n", NAME); return TRUE;
  }

  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);
  
  int MvNum = 0, found = 0;

  for (MvNum = 0; MvNum < list->count; ++MvNum) {
    if (!makeMv(pos, list->moves[MvNum].mv)) continue; 
    found++;
    takeMv(pos);
    break;
  }

  if (found != 0 ) return FALSE;

  if (InCheck(pos)) {
    if (pos->side == WHITE) {
      printf(" 0 - 1 {black mates [claimed by %s]}\n",NAME);return TRUE;
    } else {
      printf(" 0 - 1 {white mates [claimed by %s]}\n",NAME);return TRUE;
    }
  } else {
    printf("\n 1/2 - 1/2 {stalemate [claimed by %s]}\n", NAME); return TRUE;
  }

  return FALSE;
}

void PrintOpt(){
      printf("feature ping=1 setboard=1 colors=0 usermove=1\n");
      printf("feature done=1\n");
}

void xBoard_Loop(S_BOARD *pos, S_SEARCHINFO *info){
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  info->GAME_MODE = XBOARDMODE;
  info->POST_THINKING = TRUE;
  PrintOpt();

  int depth = -1, movestogo[2] = {30, 30}, movetime = -1;
  int time = -1, inc = 0, engineSide = BLACK, mv = NOMOVE;
  int timeLeft, mps, i, score, sec;
  char inBuf[80], cmd[80];
  ParseFen(START_FEN, pos);

  while (TRUE) {
    fflush(stdout);

    if (pos->side == engineSide && checkresult(pos) == FALSE) {
      info->start_time = GetTimeMS();
      info->depth = depth;

      if (time != -1) {
        info->t_set = TRUE;
        time /= movestogo[pos->side];
        time -= 50;
        info->stop_time = info->start_time + time + inc;
      }

      if (depth == -1 || depth > MAXDEPTH) info->depth = MAXDEPTH;

      printf("time: %d start: %d stop: %d depth: %d timeset: %d movestogo: %d mps: %d\n",
             time, info->start_time, info->stop_time, info->depth, info->t_set, movestogo[pos->side], mps);
      SearchPosition(pos, info);

      if (mps != 0) {
        movestogo[pos->side^1]--;
        if (movestogo[pos->side^1] < 1) movestogo[pos->side^1] = mps;
      }
    }

    fflush(stdout);

    memset(&inBuf[0], 0, sizeof(inBuf));
    fflush(stdout);
    if (!fgets(inBuf, 80, stdin)) continue;

    sscanf(inBuf, "%s", cmd);
    printf("command seen:%s\n",inBuf);

    if (!strcmp(cmd, "quit")) break;
    if (!strcmp(cmd, "force")) engineSide = BOTH; continue;
    if (!strcmp(cmd, "protover")) PrintOpt(); continue;

    if (!strcmp(cmd, "sd")) sscanf(inBuf, "sd %d", &depth); continue;
    if (!strcmp(cmd, "st")) sscanf(inBuf, "st %d", &movetime); continue;
    
    if (!strcmp(cmd, "level")) {
      sec = 0;
      movetime = -1;
      if (sscanf(inBuf, "level %d %d %d", &mps, &timeLeft, &inc) != 3) {
        sscanf(inBuf, "level %d %d %d %d", &mps, &timeLeft, &sec, &inc) ;
        printf("DEBUG level with :\n");
      } else {
        printf("DEBUG level without :\n");
      }
      timeLeft *= 60000;
      timeLeft += sec * 1000;
      movestogo[0] = movestogo[1] = 30;
      if (mps != 0) movestogo[0] = movestogo[1] = mps;
      time = -1;
      printf("DEBUG level timeLeft: %d movesToGo: %d inc: %d mps: %d\n", timeLeft, movestogo[0], inc, mps);
      continue;
    }

    if (!strcmp(cmd, "ping")) sscanf(inBuf, "pong %s\n", inBuf+4); continue;

    if (!strcmp(cmd, "new")) {
      engineSide = BLACK;
      ParseFen(START_FEN, pos);
      depth = -1;
      continue;
    }
    
    if (!strcmp(cmd, "setboard")) {
      engineSide = BOTH;
      ParseFen(inBuf+9, pos);
      continue;
    }

    if (!strcmp(cmd, "go")) engineSide = pos->side; continue;

    if (!strcmp(cmd, "usermove")) {
      movestogo[pos->side]--;
      mv = ParseMv(inBuf+9, pos);
      if (mv == NOMOVE) continue;
      makeMv(pos, mv);
      pos->ply=0;
    }

  }
}

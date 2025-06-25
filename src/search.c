#include "debug.h"
#include "stdio.h"
#include "defs.h"
#include "stdlib.h"
#include "string.h"
#include"pthread.h"
#include <time.h>

int rootDepth;

static void CheckUp(S_SEARCHINFO *info){
  if (info->t_set == TRUE && GetTimeMS()>info->stop_time) info->stopped = TRUE;
}

static void PickNextMv(int mvNum, S_MOVELIST *list){
  S_MOVE temp;
  int i = 0, bestScore = 0, bestNum = mvNum;

  for (i = mvNum; i<list->count; ++i) {
    if (list->moves[i].score > bestScore) {
      bestScore = list->moves[i].score;
      bestNum = i;
    }
  }
  
  ASSERT(mvNum>=0 && mvNum<list->count);
	ASSERT(bestNum>=0 && bestNum<list->count);
	ASSERT(bestNum>=mvNum);

  temp = list->moves[mvNum];
  list->moves[mvNum] = list->moves[bestNum];
  list->moves[bestNum] = temp;
}

int isRepetition(const S_BOARD *pos){
  int i;

  for (i=pos->histPly - pos->fiftyMv; i < pos->histPly-1; ++i) {
    ASSERT(i>= 0 && i<= MAXGAMEMOVES);
    if (pos->posKey == pos->history[i].posKey) return TRUE;
  }
  return FALSE;
}


static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table){
  int i,j;

  for (i=0; i<13; ++i) {
    for (j=0; j<BRD_SQ_NUM; ++j) {
      pos->searchHist[i][j] = 0;
    
    }
  }

  for (i=0; i<2; ++i) {
    for (j=0; j<MAXDEPTH; ++j) {
      pos->searchKillers[i][j] = 0;
    
    }
  }

  table->overWrite=0;
  table->hit=0;
  table->cut=0;
  table->curAge++;
  pos->ply = 0;

  info->stopped = 0;
  info->nodes = 0;
  info->fhf = 0;
  info->fh = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info){
  ASSERT(CheckBrd(pos));
  ASSERT(beta>alpha);

  if ((info->nodes & 2047) == 0 ) CheckUp(info);

  info->nodes++;

  if (isRepetition(pos) || pos->fiftyMv >= 100)  return 0;
  if (pos->ply > MAXDEPTH - 1) return evalPos(pos);

  int Score = evalPos(pos);

  ASSERT(Score > -AB_BOUND && Score<AB_BOUND);

  if (Score >= beta) return beta; 
  if (Score > alpha) alpha = Score; 
  
  S_MOVELIST list[1];
  GenerateAllCaps(pos, list);
  
  int MvNum = 0;
  int Legal = 0;
  Score = -AB_BOUND;
  
  for (MvNum = 0; MvNum<list->count; ++MvNum) {
    PickNextMv(MvNum, list);
    if (!makeMv(pos, list->moves[MvNum].mv)) continue;

    Legal++;
    Score = -Quiescence(-beta, -alpha, pos, info);
    takeMv(pos);
    
    if(info->stopped == TRUE) return 0;

    if (Score>alpha) {
      if (Score>=beta) {
        if (Legal == 1) info->fhf++;
        info->fh++;
        return beta;
      }

      alpha = Score;
    }
  }
  
  return alpha;
}


static int AlphaBeta(int alpha, int beta, int depth, S_SEARCHINFO *info, S_BOARD *pos, int DoNULL, S_HASHTABLE *table){

  ASSERT(CheckBrd(pos));

  if (depth <= 0) return Quiescence(alpha, beta, pos, info);
  if ((info->nodes & 2047) == 0 ) CheckUp(info);

  info->nodes++;

  if ((isRepetition(pos) || pos->fiftyMv >= 100) && pos->ply) return 0;
  if (pos->ply > MAXDEPTH -1) return evalPos(pos);

  int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side^1, pos);

  if (InCheck == TRUE) depth++; 

  int Score = -AB_BOUND;
  int PvMv = NOMOVE;

  if (ProbeHashEntry(pos, table, &PvMv, &Score, alpha, beta, depth) == TRUE) {
    table->cut++;
    return Score;
  
  }

  if (DoNULL && !InCheck && pos->ply && (pos->bigPce[pos->side] > 1) && depth >= 4) {
    makeNullMv(pos);
    Score = -AlphaBeta(-beta, -beta + 1, depth-4, info, pos, FALSE, table);
    takeNullMv(pos);
    if (info->stopped == TRUE) return 0;
    if (Score>= beta && abs(Score) < MATE) {info->nullCut++; return beta;}
  
  }
  
  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);

  int Legal = 0;
  int OldAlpha = alpha;
  int BestMv = NOMOVE;
  Score = -AB_BOUND;
  int MvNum = 0;
  int BestScore = -AB_BOUND;
  int PVMvFound = FALSE;

  if (PvMv != NOMOVE) {
  for (MvNum = 0; MvNum<list->count; ++MvNum) {
      if (list->moves[MvNum].mv == PvMv) {
        list->moves[MvNum].score = 2000000;
        PVMvFound = TRUE;
        break;
      }
    }
    if (!PVMvFound) PvMv = NOMOVE; 
  }

  for (MvNum = 0; MvNum<list->count; ++MvNum) {
    PickNextMv(MvNum, list);
    if (!makeMv(pos, list->moves[MvNum].mv)) continue;

    Legal++;
    Score = -AlphaBeta(-beta, -alpha, depth-1, info, pos, TRUE, table);
    takeMv(pos);

    if(info->stopped == TRUE) return 0;

   if (Score>BestScore) {
      BestScore = Score;
      BestMv = list->moves[MvNum].mv;
      if (Score>alpha) {
        if (Score>=beta) {
          if (Legal == 1) info->fhf++;
          info->fh++;

          if (!(list->moves[MvNum].mv & CAP_FLAG)) {
            pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
            pos->searchKillers[0][pos->ply] = list->moves[MvNum].mv;
          }

          StoreHashEntry(pos, table, BestMv, beta, HFBETA, depth);

          return beta;
        }

        alpha = Score;

        if (!(list->moves[MvNum].mv & CAP_FLAG)) {
          pos->searchHist[pos->pieces[FROMSQ(BestMv)]][TOSQ(BestMv)] += depth;
        }
      }
    }
  }

  if (Legal == 0) {
    if (InCheck == TRUE) {
      return -AB_BOUND + pos->ply;
    } else return 0;
  }

  ASSERT(alpha>=OldAlpha);

  if (alpha != OldAlpha) {
    StoreHashEntry(pos, table, BestMv, BestScore, HFEXACT, depth);
  } else StoreHashEntry(pos, table, BestMv, alpha, HFALPHA, depth);

  return alpha;
}

void IterativeDeepen(S_SEARCH_WORKER_DATA *t_Data){
  t_Data->bestMv = NOMOVE;
  int lastbestMv = NOMOVE;
  int bestScore = -AB_BOUND;
  int curDepth = 0;
  int pvMoves = 0;
  int pvNum = 0;

  bestScore = AlphaBeta(-AB_BOUND, AB_BOUND, 1, t_Data->info, t_Data->pos, TRUE, t_Data->table_t);
  if (t_Data->info->stopped != TRUE) {
    pvMoves = GetPvLine(1, t_Data->pos, t_Data->table_t);
      if (pvMoves > 0) {
        lastbestMv = t_Data->pos->PvArr[0];
        t_Data->bestMv = lastbestMv;
      }
  }
    
  for (curDepth = 2; curDepth <=t_Data->info->depth; ++curDepth) {
    rootDepth = curDepth;
    bestScore = AlphaBeta(-AB_BOUND, AB_BOUND, curDepth, t_Data->info, t_Data->pos, TRUE, t_Data->table_t);
    if(t_Data->info->stopped == TRUE) {t_Data->bestMv = lastbestMv; break;}
    if (t_Data->t_Num == 0) {
      pvMoves = GetPvLine(curDepth, t_Data->pos, t_Data->table_t);
      if (pvMoves > 0) {
        lastbestMv = t_Data->pos->PvArr[0];
        t_Data->bestMv = lastbestMv;
      }
      if (t_Data->info->GAME_MODE == UCIMODE) {
        printf("info score cp %d depth %d nodes %ld time %d ",bestScore, curDepth, t_Data->info->nodes, (GetTimeMS()-t_Data->info->start_time));
      } else if (t_Data->info->GAME_MODE == XBOARDMODE && t_Data->info->POST_THINKING == TRUE) {
        printf("%d %d %d %ld ", curDepth, bestScore, (GetTimeMS() - t_Data->info->start_time)/10,t_Data->info->nodes);
      } else if (t_Data->info->POST_THINKING == TRUE) {
        printf("score:%d depth: %d nodes: %ld time: %d (ms) ",bestScore, curDepth, t_Data->info->nodes, (GetTimeMS()-t_Data->info->start_time));
      }
      if (t_Data->info->GAME_MODE == UCIMODE || t_Data->info->POST_THINKING == TRUE) {
        pvMoves = GetPvLine(curDepth, t_Data->pos, t_Data->table_t);
        if (!(t_Data->info->GAME_MODE == XBOARDMODE)) printf("pv");
        for (pvNum = 0; pvNum<pvMoves; ++pvNum) {
          printf(" %s", PrMv(t_Data->pos->PvArr[pvNum]));
        }
        CR;
      }

    }

  }

  if (t_Data->bestMv == NOMOVE || !MoveExists(t_Data->pos, t_Data->bestMv)) {
    t_Data->bestMv = ProbePvMove(t_Data->pos, t_Data->table_t);
    if (t_Data->bestMv == NOMOVE || !MoveExists(t_Data->pos, t_Data->bestMv)) {
      S_MOVELIST list[1];
      GenerateAllMvs(t_Data->pos, list);

      int bestMvScore = -INF;
      for (int i = 0; i<list->count; i++) {
        if (makeMv(t_Data->pos, list->moves[i].mv)) {
          takeMv(t_Data->pos);
          if (list->moves[i].score > bestMvScore) {
            bestMvScore = list->moves[i].score;
            t_Data->bestMv = list->moves[i].mv;
          }
        }
      }
    }
  }
}
  
  

void SearchPosition(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info){
  ClearForSearch(pos, info, table);

  int bestMv = NOMOVE;

  creatSearch_t(pos, table, info);

  for (int i =0; i < info->NumThreads; i++) {
    pthread_join(tid[i], NULL);
  }
}

#include "debug.h"
#include "stdio.h"
#include "defs.h"

static void CheckUp(S_SEARCHINFO *info){
  if (info->t_set == TRUE && GetTimeMS()>info->stop_time) info->stopped = TRUE;
  ReadInput(info);
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


static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info){
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

  ClearPvTable(pos->PvTable);
  pos->ply = 0;

  info->stopped = 0;
  info->nodes = 0;
  info->fhf = 0;
  info->fh = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info){
  ASSERT(CheckBrd(pos));

  if ((info->nodes & 2047) == 0 ) {
    CheckUp(info);
  }

  info->nodes++;

  if ((isRepetition(pos) || pos->fiftyMv >= 100) && pos->ply) return 0;
  if (pos->ply > MAXDEPTH - 1) return evalPos(pos);

  int Score = evalPos(pos);

  if (Score >= beta) return beta; 
  if (Score > alpha) alpha = Score; 
  
  S_MOVELIST list[1];
  GenerateAllCaps(pos, list);
  
  int MvNum = 0;
  int Legal = 0;
  Score = -INF;
  
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


static int AlphaBeta(int alpha, int beta, int depth, S_SEARCHINFO *info, S_BOARD *pos, int DoNULL){

  ASSERT(CheckBrd(pos));

  if (depth == 0) return Quiescence(alpha, beta, pos, info);
  if ((info->nodes & 2047) == 0 ) CheckUp(info);

  info->nodes++;

  if ((isRepetition(pos) || pos->fiftyMv >= 100) && pos->ply) return 0;
  if (pos->ply > MAXDEPTH -1) return evalPos(pos);

  if (InCheck(pos) == TRUE) depth++; 

  int Score = -INF;

  if (DoNULL && !InCheck(pos) && pos->ply && (pos->bigPce[pos->side] > 0) && depth >= 4) {
    makeNullMv(pos);
    Score = -AlphaBeta(-beta, -beta + 1, depth-4, info, pos, FALSE);
    takeNullMv(pos);
    if (info->stopped == TRUE) return 0;
    if (Score>= beta) return beta;
  
  }
  

  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);

  int Legal = 0;
  int OldAlpha = alpha;
  int BestMv = NOMOVE;
  Score = -INF;
  int MvNum = 0;
  int pvMove = ProbePvTable(pos);

  if (pvMove != NOMOVE) {
  for (MvNum = 0; MvNum<list->count; ++MvNum) {
      if (list->moves[MvNum].mv == pvMove) {
        list->moves[MvNum].score = 2000000;
        break;
      }
    }
  
  }

  for (MvNum = 0; MvNum<list->count; ++MvNum) {
    PickNextMv(MvNum, list);
    if (!makeMv(pos, list->moves[MvNum].mv)) continue;

    Legal++;
    Score = -AlphaBeta(-beta, -alpha, depth-1, info, pos, TRUE);
    takeMv(pos);

    if(info->stopped == TRUE) return 0;

    if (Score>alpha) {
      if (Score>=beta) {
        if (Legal == 1) info->fhf++;
        info->fh++;

        if (!(list->moves[MvNum].mv & CAP_FLAG)) {
          pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
          pos->searchKillers[0][pos->ply] = list->moves[MvNum].mv;
        }
        return beta;
      }

      alpha = Score;
      BestMv = list->moves[MvNum].mv;
      if (!(list->moves[MvNum].mv & CAP_FLAG)) {
        pos->searchHist[pos->pieces[FROMSQ(BestMv)]][TOSQ(BestMv)] += depth;
      }
    }
  }

  if (Legal == 0) {
    if (InCheck(pos) == TRUE) {
      return -INF + pos->ply;
    } else return 0;
  }

  if (alpha != OldAlpha) StorePvMove(pos, BestMv);

  return alpha;
}


void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info){

  int bestMv = NOMOVE;
  int bestScore = -INF;
  int curDepth = 0;
  int pvMoves = 0;
  int pvNum = 0;
  ClearForSearch(pos, info);

  if (bestMv == NOMOVE) {
    for (curDepth = 1; curDepth <=info->depth; ++curDepth) {
      bestScore = AlphaBeta(-INF, INF, curDepth, info, pos, TRUE);

      if (info->stopped == TRUE) break;

      
      pvMoves = GetPvLine(curDepth, pos);
      bestMv = pos->PvArr[0];
      if (info->GAME_MODE == UCIMODE) {
        printf("info score cp %d depth %d nodes %ld time %d ",bestScore, curDepth, info->nodes, (GetTimeMS()-info->start_time));
      } else if (info->GAME_MODE == XBOARDMODE && info->POST_THINKING == TRUE) {
        printf("%d %d %d %ld ", curDepth, bestScore, (GetTimeMS() - info->start_time)/10,info->nodes);
      } else if (info->POST_THINKING == TRUE) {
        printf("score:%d depth: %d nodes: %ld time: %d (ms) ",bestScore, curDepth, info->nodes, (GetTimeMS()-info->start_time));
      }
      if (info->GAME_MODE == UCIMODE || info->POST_THINKING == TRUE) {
        pvMoves = GetPvLine(curDepth, pos);
        if (!(info->GAME_MODE == XBOARDMODE)) printf("pv");
        for (pvNum = 0; pvNum<pvMoves; ++pvNum) {
          printf(" %s", PrMv(pos->PvArr[pvNum]));
        }
        CR;
      }
    }
  
  }


  if (info->GAME_MODE == UCIMODE) {
    printf("bestmove %s\n", PrMv(bestMv));
  } else if (info->GAME_MODE == XBOARDMODE) {
    printf("move %s\n", PrMv(bestMv));
    makeMv(pos, bestMv);
  } else {
    CR;CR;
    printf("===** %s makes move %s **===", NAME, PrMv(bestMv));
    CR;CR;
    makeMv(pos, bestMv);
    PrintBoard(pos);
  }
}

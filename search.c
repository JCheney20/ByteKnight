#include "debug.h"
#include "stdio.h"
#include "defs.h"

static void CheckUp(){
  // TODO: Check if time up, or interupt from GUI

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

  info->start_time = GetTimeMS();
  info->stop_time = 0;
  info->nodes = 0;
}

static int AlphaBeta(int alpha, int beta, int depth, S_SEARCHINFO *info, S_BOARD *pos, int DoNULL){

  ASSERT(CheckBrd(pos));

  if (depth == 0) {
    info->nodes++;
    return evalPos(pos);
  }

  info->nodes++;

  if (isRepetition(pos) || pos->fiftyMv>=100)  return 0;
  if (pos->ply > MAXDEPTH -1) return evalPos(pos);

  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);

  int Legal = 0;
  int OldAlpha = alpha;
  int BestMv = NOMOVE;
  int Score = -INF;
  int MvNum = 0;

  for (MvNum = 0; MvNum<list->count; ++MvNum) {
    if (!makeMv(pos, list->moves[MvNum].mv)) continue;

    Legal++;
    Score = -AlphaBeta(-beta, -alpha, depth-1, info, pos, TRUE);
    takeMv(pos);

    if (Score>alpha) {
      if (Score>=beta) return beta; 
      alpha = Score;
      BestMv = list->moves[MvNum].mv;
    }
  }

  if (Legal == 0) {
    if (SqAttacked(pos->KingSq[pos->side], pos->side^1, pos)) {
      return -MATE + pos->ply;
    } else return 0;
  }

  if (alpha != OldAlpha) StorePvMove(pos, BestMv);

  return alpha;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info){
  
  return 0;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info){

  int bestMv = NOMOVE;
  int bestScore = -INF;
  int curDepth = 0;
  int pvMoves = 0;
  int pvNum = 0;
  ClearForSearch(pos, info);

  for (curDepth = 1; curDepth <=info->depth; ++curDepth) {
    bestScore = AlphaBeta(-INF, INF, curDepth, info, pos, TRUE);

    //TODO: Check if out of time
    
    pvMoves = GetPvLine(curDepth, pos);
    bestMv = pos->PvArr[0];

    printf("Depth:%d score:%d move: %s nodes:%ld",curDepth, bestScore, PrMv(bestMv), info->nodes );

    pvMoves = GetPvLine(curDepth, pos);
    printf("pv");
    
    for (pvNum = 0; pvNum<pvMoves; ++pvNum) {
      printf(" %s", PrMv(pos->PvArr[pvNum]));
    }
    CR;
  }
  
}

int isRepetition(const S_BOARD *pos){
  int i;

  for (i=pos->histPly - pos->fiftyMv; i < pos->histPly-1; ++i) {
    ASSERT(i>= 0 && i<= MAXGAMEMOVES);
    if (pos->posKey == pos->history[i].posKey) {
      return TRUE;
    }
  }
  return FALSE;
}


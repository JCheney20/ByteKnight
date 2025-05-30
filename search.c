#include "debug.h"
#include "stdio.h"
#include "defs.h"

static void CheckUp(){
  // TODO: Check if time up, or interupt from GUI

}

static void PickNextMv(int mvNum, S_MOVELIST *list){
  S_MOVE temp;
  int i = 0;
  int bestScore = 0;
  int bestNum = mvNum;

  for (i = mvNum; i<list->count; ++i) {
    if (list->moves[i].score > bestScore) {
      bestScore = list->moves[i].score;
      bestNum = i;
    }
  }
  temp = list->moves[mvNum];
  list->moves[mvNum] = list->moves[bestNum];
  list->moves[bestNum] = temp;
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
  info->fhf = 0;
  info->fh = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info){
  ASSERT(CheckBrd(pos));

  info->nodes++;

  if (isRepetition(pos) || pos->fiftyMv >= 100) return 0;
  if (pos->ply > MAXDEPTH - 1) return evalPos(pos);

  int Score = evalPos(pos);

  if (Score >= beta) return beta; 
  if (Score > alpha) alpha = Score; 
  
  S_MOVELIST list[1];
  GenerateAllCaps(pos, list);
  
  int Legal = 0;
  int OldAlpha = alpha;
  int BestMv = NOMOVE;
  Score = -INF;
  int MvNum = 0;
  
  for (MvNum = 0; MvNum<list->count; ++MvNum) {
    PickNextMv(MvNum, list);
    if (!makeMv(pos, list->moves[MvNum].mv)) continue;

    Legal++;
    Score = -Quiescence(-beta, -alpha, pos, info);
    takeMv(pos);

    if (Score>alpha) {
      if (Score>=beta) {
        if (Legal == 1) info->fhf++;
        info->fh++;
        return beta;
      }

      alpha = Score;
      BestMv = list->moves[MvNum].mv;
    }
  }

  if (alpha != OldAlpha) StorePvMove(pos, BestMv);
  
  return alpha;
}


static int AlphaBeta(int alpha, int beta, int depth, S_SEARCHINFO *info, S_BOARD *pos, int DoNULL){

  ASSERT(CheckBrd(pos));

  if (depth == 0) {
    info->nodes++;
    // return evalPos(pos);
    return Quiescence(alpha, beta, pos, info);
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
    if (SqAttacked(pos->KingSq[pos->side], pos->side^1, pos)) {
      return -MATE + pos->ply;
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
    printf("Ordering: %.2f\n",(info->fhf/info->fh));
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


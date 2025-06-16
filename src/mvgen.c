#include "stdio.h"
#include "defs.h"
#include "debug.h"

#define MOVE(f, t, ca, pro, fl) ( (f) | ((t) << 7) | ((ca) << 14) | ((pro) << 20) | (fl))
#define SQOFFBOARD(sq) (FilesBrd[sq] == NO_SQ)


//Big Piece Arrays
int LoopSlidePce[8] = {
  wB, wR, wQ, 0, bB, bR, bQ, 0 
};

int LoopNonSlidePce[6] = {
  wN, wK, 0, bN, bK, 0
};

int PceDir[13][8] = {
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { -8, -19, -21, -12, 8, 19, 21, 12},
  { -9, -11, 11, 9, 0, 0, 0, 0},
  { -1, -10, 1, 10, 0, 0, 0, 0},
  { -1, -10, 1, 10, -9, -11, 11, 9},
  { -1, -10, 1, 10, -9, -11, 11, 9},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { -8, -19, -21, -12, 8, 19, 21, 12},
  { -9, -11, 11, 9, 0, 0, 0, 0},
  { -1, -10, 1, 10, 0, 0, 0, 0},
  { -1, -10, 1, 10, -9, -11, 11, 9},
  { -1, -10, 1, 10, -9, -11, 11, 9},
};

int NumDir[13] = {0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8};
int LoopSlideIndex[2] = {0, 4};
int LoopNonSlideIndex[2] = {0, 3};

const int VicScore[13] =  { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int MvvLvaScores[13][13];

void InitMvvLva(){
  int Attacker;
  int Vic;

  for (Attacker = wP; Attacker <= bK; ++Attacker) {
    for (Vic = wP; Vic<=bK; ++Vic) {
      MvvLvaScores[Vic][Attacker] = VicScore[Vic] + 6 - (VicScore[Attacker] / 100);
    }
  }
}

int MoveExists(S_BOARD *pos, const int move){
  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);

  int i;
  for (i = 0 ; i < list->count; ++i) {
    if (!makeMv(pos, list->moves[i].mv)) continue;
    takeMv(pos);
    if (list->moves[i].mv == move) return TRUE;
  }
  return FALSE;
}

void AddQuietMv( const S_BOARD *pos, int move, S_MOVELIST *list){

  ASSERT(SqOnBoard(FROMSQ(move)));
  ASSERT(SqOnBoard(TOSQ(move)));

  list->moves[list->count].mv = move;

  if (pos->searchKillers[0][pos->ply] == move ) {
    list->moves[list->count].score = 90000;
  } else if (pos->searchKillers[1][pos->ply] == move) {
    list->moves[list->count].score = 80000;
  } else {
    list->moves[list->count].score = pos->searchHist[pos->pieces[FROMSQ(move)]][TOSQ(move)];
  }
  list->count++;
}

void AddCaptureMv( const S_BOARD *pos, int mv, S_MOVELIST *list){
  
  ASSERT(SqOnBoard(FROMSQ(mv)));
  ASSERT(SqOnBoard(TOSQ(mv)));
  ASSERT(PieceValid(CAPTURED(mv)));

  list->moves[list->count].mv = mv;
  list->moves[list->count].score = MvvLvaScores[CAPTURED(mv)][pos->pieces[FROMSQ(mv)]] + 10000;
  list->count++;
}

void AddEpMv( const S_BOARD *pos, int mv, S_MOVELIST *list){

  ASSERT(SqOnBoard(FROMSQ(mv)));
  ASSERT(SqOnBoard(TOSQ(mv)));
  ASSERT(CheckBrd(pos));
  ASSERT((RanksBrd[TOSQ(mv)]==RANK_6 && pos->side == WHITE) || (RanksBrd[TOSQ(mv)]==RANK_3 && pos->side == BLACK));

  list->moves[list->count].mv = mv;
  list->moves[list->count].score = 105 + 1000000;
  list->count++;
}

void AddPawnMv( const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list, const int side){
  ASSERT(SqOnBoard(from));
  ASSERT(SqOnBoard(to));
  ASSERT(CheckBrd(pos));

  if (FilesBrd[from] == FilesBrd[to]) {
    if (side == WHITE && RanksBrd[from] == RANK_7 ) {
        AddQuietMv(pos, MOVE(from, to, EMPTY, wQ, 0), list);
        AddQuietMv(pos, MOVE(from, to, EMPTY, wR, 0), list);
        AddQuietMv(pos, MOVE(from, to, EMPTY, wB, 0), list);
        AddQuietMv(pos, MOVE(from, to, EMPTY, wN, 0), list);
    } else if (side == BLACK && RanksBrd[from] == RANK_2 ) {
        AddQuietMv(pos, MOVE(from, to, EMPTY, bQ, 0), list);
        AddQuietMv(pos, MOVE(from, to, EMPTY, bR, 0), list);
        AddQuietMv(pos, MOVE(from, to, EMPTY, bB, 0), list);
        AddQuietMv(pos, MOVE(from, to, EMPTY, bN, 0), list);
    } else {
      AddQuietMv(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
    }
  } else {
      ASSERT(PieceValidEmpty(cap));
    if (side == WHITE && RanksBrd[from] == RANK_7 ) {
          AddCaptureMv(pos, MOVE(from, to, cap, wQ, 0), list);
          AddCaptureMv(pos, MOVE(from, to, cap, wR, 0), list);
          AddCaptureMv(pos, MOVE(from, to, cap, wB, 0), list);
          AddCaptureMv(pos, MOVE(from, to, cap, wN, 0), list);
    } else if (side == BLACK && RanksBrd[from] == RANK_2 ) {
          AddCaptureMv(pos, MOVE(from, to, cap, bQ, 0), list);
          AddCaptureMv(pos, MOVE(from, to, cap, bR, 0), list);
          AddCaptureMv(pos, MOVE(from, to, cap, bB, 0), list);
          AddCaptureMv(pos, MOVE(from, to, cap, bN, 0), list);
      } else {
        AddCaptureMv(pos, MOVE(from, to, cap, EMPTY, 0), list);
      }
  }
}

void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list){
    ASSERT(CheckBrd(pos));

    list->count = 0;

    int pce = EMPTY;
    int side = pos->side;
    int sq = 0; int t_sq = 0;
    int pceNum = 0;
    int dir = 0;
    int index = 0;
    int pceIndex = 0;

    if(side == WHITE) {

      for(pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
        sq = pos->pList[wP][pceNum];
        ASSERT(SqOnBoard(sq));

        if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
          AddPawnMv(pos, sq, sq+9, pos->pieces[sq + 9], list, side);
        }

        if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
          AddPawnMv(pos, sq, sq+11, pos->pieces[sq + 11], list, side);
        }

        if(pos->enPas != NO_SQ) {
          if(sq + 9 == pos->enPas) {
            AddEpMv(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,EP_FLAG), list);
          }
          if(sq + 11 == pos->enPas) {
            AddEpMv(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,EP_FLAG), list);
          }
        }
      }
      /* Loop for slide pieces */
      pceIndex = LoopSlideIndex[side];
      pce = LoopSlidePce[pceIndex++];
      while( pce != 0) {
        ASSERT(PieceValid(pce));

        for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
          sq = pos->pList[pce][pceNum];
          ASSERT(SqOnBoard(sq));

          for(index = 0; index < NumDir[pce]; ++index) {
            dir = PceDir[pce][index];
            t_sq = sq + dir;

            while(!SQOFFBOARD(t_sq)) {
              if(pos->pieces[t_sq] != EMPTY) {
                if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                  AddCaptureMv(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                }
                break;
              }
              t_sq += dir;
            }
          }
        }
        pce = LoopSlidePce[pceIndex++];
      }

    /* Loop for non slide */
      pceIndex = LoopNonSlideIndex[side];
      pce = LoopNonSlidePce[pceIndex++];

      while( pce != 0) {
        ASSERT(PieceValid(pce));

        for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
          sq = pos->pList[pce][pceNum];
          ASSERT(SqOnBoard(sq));

          for(index = 0; index < NumDir[pce]; ++index) {
            dir = PceDir[pce][index];
            t_sq = sq + dir;

            if(SQOFFBOARD(t_sq)) {
              continue;
            }

            if(pos->pieces[t_sq] != EMPTY) {
              if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                AddCaptureMv(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
              }
              continue;
            }
          }
        }

        pce = LoopNonSlidePce[pceIndex++];
      }
    } else {

      for(pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
        sq = pos->pList[bP][pceNum];
        ASSERT(SqOnBoard(sq));

        if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
          AddPawnMv(pos, sq, sq-9, pos->pieces[sq - 9], list, side);
        }

        if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
          AddPawnMv(pos, sq, sq-11, pos->pieces[sq - 11], list, side);
        }
        if(pos->enPas != NO_SQ) {
          if(sq - 9 == pos->enPas) {
            AddEpMv(pos, MOVE(sq,sq - 9,EMPTY,EMPTY, EP_FLAG), list);
          }
          if(sq - 11 == pos->enPas) {
            AddEpMv(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,EP_FLAG), list);
          }
        }
      }


    /* Loop for slide pieces */
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++];
    while( pce != 0) {
      ASSERT(PieceValid(pce));

      for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        ASSERT(SqOnBoard(sq));

        for(index = 0; index < NumDir[pce]; ++index) {
          dir = PceDir[pce][index];
          t_sq = sq + dir;

          while(!SQOFFBOARD(t_sq)) {
            if(pos->pieces[t_sq] != EMPTY) {
              if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                AddCaptureMv(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
              }
              break;
            }
            t_sq += dir;
          }
        }
      }
      pce = LoopSlidePce[pceIndex++];
    }

  /* Loop for non slide */
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];

    while( pce != 0) {
      ASSERT(PieceValid(pce));

      for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        ASSERT(SqOnBoard(sq));

        for(index = 0; index < NumDir[pce]; ++index) {
          dir = PceDir[pce][index];
          t_sq = sq + dir;

          if(SQOFFBOARD(t_sq)) {
            continue;
          }

          if(pos->pieces[t_sq] != EMPTY) {
            if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
              AddCaptureMv(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
            }
            continue;
          }
        }
      }

      pce = LoopNonSlidePce[pceIndex++];
    }
  }
}

void GenerateAllMvs(const S_BOARD *pos, S_MOVELIST *list){
  ASSERT(CheckBrd(pos));

	list->count = 0;

	int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pceNum = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == WHITE) {

		for(pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
			sq = pos->pList[wP][pceNum];
			ASSERT(SqOnBoard(sq));

			if(pos->pieces[sq + 10] == EMPTY) {
				AddPawnMv(pos, sq, sq+10, EMPTY, list, side);
				if(RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
					AddQuietMv(pos, MOVE(sq,(sq+20),EMPTY,EMPTY,PAWN_START_FLAG),list);
				}
			}

			if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
				AddPawnMv(pos, sq, sq+9, pos->pieces[sq + 9], list, side);
			}

			if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
				AddPawnMv(pos, sq, sq+11, pos->pieces[sq + 11], list, side);
			}

			if(pos->enPas != NO_SQ) {
				if(sq + 9 == pos->enPas) {
					AddEpMv(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,EP_FLAG), list);
				}
				if(sq + 11 == pos->enPas) {
					AddEpMv(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,EP_FLAG), list);
				}
			}
		}

		if(pos->castlePerm & WKCA) {
			if(pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(F1,BLACK,pos) ) {
					AddQuietMv(pos, MOVE(E1, G1, EMPTY, EMPTY, CASTLE_FLAG), list);
				}
			}
		}

		if(pos->castlePerm & WQCA) {
			if(pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(D1,BLACK,pos) ) {
					AddQuietMv(pos, MOVE(E1, C1, EMPTY, EMPTY, CASTLE_FLAG), list);
				}
			}
		}
  
      /* Loop for slide pieces */
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++];
    while( pce != 0) {
      ASSERT(PieceValid(pce));

      for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        ASSERT(SqOnBoard(sq));

        for(index = 0; index < NumDir[pce]; ++index) {
          dir = PceDir[pce][index];
          t_sq = sq + dir;

          while(!SQOFFBOARD(t_sq)) {
            if(pos->pieces[t_sq] != EMPTY) {
              if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                AddCaptureMv(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
              }
              break;
            }
            AddQuietMv(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
            t_sq += dir;
          }
        }
      }
      pce = LoopSlidePce[pceIndex++];
    }

  /* Loop for non slide */
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];

    while( pce != 0) {
      ASSERT(PieceValid(pce));

      for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        ASSERT(SqOnBoard(sq));

        for(index = 0; index < NumDir[pce]; ++index) {
          dir = PceDir[pce][index];
          t_sq = sq + dir;

          if(SQOFFBOARD(t_sq)) {
            continue;
          }

          if(pos->pieces[t_sq] != EMPTY) {
            if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
              AddCaptureMv(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
            }
            continue;
          }
          AddQuietMv(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
        }
      }

      pce = LoopNonSlidePce[pceIndex++];
    }

	} else {

		for(pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
			sq = pos->pList[bP][pceNum];
			ASSERT(SqOnBoard(sq));

			if(pos->pieces[sq - 10] == EMPTY) {
				AddPawnMv(pos, sq, sq-10, EMPTY, list, side);
				if(RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
					AddQuietMv(pos, MOVE(sq,(sq-20),EMPTY,EMPTY,PAWN_START_FLAG),list);
				}
			}

			if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
				AddPawnMv(pos, sq, sq-9, pos->pieces[sq - 9], list, side);
			}

			if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
				AddPawnMv(pos, sq, sq-11, pos->pieces[sq - 11], list, side);
			}
			if(pos->enPas != NO_SQ) {
				if(sq - 9 == pos->enPas) {
					AddEpMv(pos, MOVE(sq,sq - 9,EMPTY,EMPTY, EP_FLAG), list);
				}
				if(sq - 11 == pos->enPas) {
					AddEpMv(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,EP_FLAG), list);
				}
			}
		}

		// castling
		if(pos->castlePerm &  BKCA) {
			if(pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(F8,WHITE,pos) ) {
					AddQuietMv(pos, MOVE(E8, G8, EMPTY, EMPTY, CASTLE_FLAG), list);
				}
			}
		}

		if(pos->castlePerm &  BQCA) {
			if(pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(D8,WHITE,pos) ) {
					AddQuietMv(pos, MOVE(E8, C8, EMPTY, EMPTY, CASTLE_FLAG), list);
				}
			}
		}
  }

  /* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++];
	while( pce != 0) {
		ASSERT(PieceValid(pce));

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];
			ASSERT(SqOnBoard(sq));

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while(!SQOFFBOARD(t_sq)) {
					if(pos->pieces[t_sq] != EMPTY) {
						if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMv(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					AddQuietMv(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
					t_sq += dir;
				}
			}
		}
		pce = LoopSlidePce[pceIndex++];
	}

/* Loop for non slide */
	pceIndex = LoopNonSlideIndex[side];
	pce = LoopNonSlidePce[pceIndex++];

	while( pce != 0) {
		ASSERT(PieceValid(pce));

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];
			ASSERT(SqOnBoard(sq));

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				if(pos->pieces[t_sq] != EMPTY) {
					if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMv(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
				AddQuietMv(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}
}


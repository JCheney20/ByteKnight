#include "defs.h"
#include "debug.h"
#include <stdio.h>

#define HASH_PCE(pce, sq) (pos->posKey ^= (PieceKeys[pce][sq]))
#define HASH_CA (pos->posKey ^= (CastleKeys[pos->castlePerm]))
#define HASH_SIDE (pos->posKey ^= (SideKey))
#define HASH_EP (pos->posKey ^= (PieceKeys[EMPTY][pos->enPas]))

const int CastlePerm[120] = {
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15 
};

static void ClearPiece(const int sq, S_BOARD *pos){
  ASSERT(SqOnBoard(sq));

  int pce = pos->pieces[sq];

  ASSERT(PieceValid(pce));

  int col = PieceCol[pce];
  int i=0;
  int t_pceNum = -1;

  HASH_PCE(pce, sq);

  pos->pieces[sq] = EMPTY;
  pos->material[col] -= PieceVal[pce];

  if(PieceBig[pce]){
    pos->bigPce[col]--;
    if(PieceMaj[pce]){
      pos->majPce[col]--;
    } else {
      pos->minPce[col]--;
    }
  } else {
    CLRBIT(pos->pawns[col], SQ64(sq));
    CLRBIT(pos->pawns[BOTH], SQ64(sq));
  }

  for (i=0; i<pos->pceNum[pce]; ++i) {
    if (pos->pList[pce][i] == sq) {
      t_pceNum = i;
      break;
    }
  }

  ASSERT(t_pceNum != -1);

  pos->pceNum[pce]--;
  pos->pList[pce][t_pceNum] = pos->pList[pce][pos->pceNum[pce]];
};

static void AddPiece(const int sq, S_BOARD *pos, const int pce){
  ASSERT(PieceValid(pce));
  ASSERT(SqOnBoard(sq));

  int col = PieceCol[pce];
  HASH_PCE(pce, sq);

  pos->pieces[sq] = pce;

  if(PieceBig[pce]){
    pos->bigPce[col]++;
    if(PieceMaj[pce]){
      pos->majPce[col]++;
    } else {
      pos->minPce[col]++;
    }
  } else {
    SETBIT(pos->pawns[col], SQ64(sq));
    SETBIT(pos->pawns[BOTH], SQ64(sq));
  }

  pos->material[col] += PieceVal[pce];
  pos->pList[pce][pos->pceNum[pce]++] = sq;

}

static void MovePiece(const int from, const int to, S_BOARD *pos){
  ASSERT(SqOnBoard(from));
  ASSERT(SqOnBoard(to));

  int i = 0;
  int pce = pos->pieces[from];
  int col = PieceCol[pce];

  ASSERT(SideValid(col));
  ASSERT(PieceValid(pce));

#ifdef DEBUG
  int t_PieceNum = FALSE;
#endif 

  HASH_PCE(pce, from);
  pos->pieces[from] = EMPTY;

  HASH_PCE(pce, to);
  pos->pieces[to] = pce;

  if (!PieceBig[pce]) {
    CLRBIT(pos->pawns[col], SQ64(from));
    CLRBIT(pos->pawns[BOTH], SQ64(from));
    SETBIT(pos->pawns[col], SQ64(to));
    SETBIT(pos->pawns[BOTH], SQ64(to));
  }

  for (i = 0; i < pos->pceNum[pce]; ++i) {
    if(pos->pList[pce][i] == from){
      pos->pList[pce][i] = to;
#ifdef DEBUG
      t_PieceNum = TRUE;
#endif 
      break;
    }
  }
  ASSERT(t_PieceNum);
}

void takeMv(S_BOARD *pos){

  ASSERT(CheckBrd(pos))

  pos->histPly--;
  pos->ply--;

  int mv = pos->history[pos->histPly].move;
  int from = FROMSQ(mv);
  int to = TOSQ(mv);

  ASSERT(SqOnBoard(from));
  ASSERT(SqOnBoard(to));

  if(pos->enPas != NO_SQ) HASH_EP;
  HASH_CA;

  pos->castlePerm = pos->history[pos->histPly].castlePerm;
  pos->fiftyMv = pos->history[pos->histPly].fiftyMv;
  pos->enPas = pos->history[pos->histPly].enPas;

  if (pos->enPas != NO_SQ) HASH_EP; 
  HASH_CA;

  pos->side ^= 1;
  HASH_SIDE;

  if (EP_FLAG & mv) {
    if (pos->side == WHITE) {
      AddPiece(to-10, pos, bP);
    } else {
      AddPiece(to+10, pos, wP);
    }
  } else if (CASTLE_FLAG & mv) {
    switch (to) {
      case C1: MovePiece(D1, A1, pos); break;
      case C8: MovePiece(D8, A8, pos); break;
      case G1: MovePiece(F1, H1, pos); break;
      case G8: MovePiece(F8, H8, pos); break;
      default: ASSERT(FALSE);
    }
  }

  MovePiece(to, from, pos);

  if (PieceKing[pos->pieces[from]]) {
    pos->KingSq[pos->side] = from;
  }

  int cap = CAPTURED(mv);
  if (cap != EMPTY) {
    ASSERT(PieceValid(cap));
    AddPiece(to, pos, cap);
  }

  if (PROMOTED(mv) != EMPTY) {
    ASSERT(PieceValid(PROMOTED(mv)) && !PiecePawn[PROMOTED(mv)]);
    ClearPiece(from, pos);
    AddPiece(from, pos, (PieceCol[PROMOTED(mv)]) == WHITE ? wP: bP);
  }

  ASSERT(CheckBrd(pos));

}

int makeMv(S_BOARD *pos, int mv){

  ASSERT(CheckBrd(pos));

  int from = FROMSQ(mv);
  int to = TOSQ(mv);
  int side = pos->side;

  ASSERT(SqOnBoard(from));
  ASSERT(SqOnBoard(to));
  ASSERT(SideValid(side));
  ASSERT(PieceValid(pos->pieces[from]));

  pos->history[pos->histPly].posKey = pos->posKey;

  if (mv & EP_FLAG) {
    if(side == WHITE) {
            ClearPiece(to-10,pos);
        } else {
            ClearPiece(to+10,pos);
        }
  } else if (mv & CASTLE_FLAG) {
    switch (to) {
      case C1:
        MovePiece(A1, D1, pos);
      break;
      case C8:
        MovePiece(A8, D8, pos);
      break;
      case G1:
        MovePiece(H1, F1, pos);
      break;
      case G8:
        MovePiece(H8, F8, pos);
      break;
      default: ASSERT(FALSE); break;
    }

  }

  if (pos->enPas != NO_SQ) HASH_EP;
  HASH_CA;

  pos->history[pos->histPly].move = mv;
  pos->history[pos->histPly].fiftyMv = pos->fiftyMv;
  pos->history[pos->histPly].enPas = pos->enPas;
  pos->history[pos->histPly].castlePerm = pos->castlePerm;

  pos->castlePerm &= CastlePerm[from];
  pos->castlePerm &= CastlePerm[to];
  pos->enPas = NO_SQ;

  HASH_CA;

  int cap = CAPTURED(mv);
  pos->fiftyMv++;

  if (cap != EMPTY) {
    ASSERT(PieceValid(cap))
      ClearPiece(to, pos);
    pos->fiftyMv = 0;
  }

  pos->histPly++;
  pos->ply++;

if(PiecePawn[pos->pieces[from]]) {
        pos->fiftyMv = 0;
        if(mv & PAWN_START_FLAG) {
            if(side==WHITE) {
                pos->enPas=from+10;
                ASSERT(RanksBrd[pos->enPas]==RANK_3);
            } else {
                pos->enPas=from-10;
                ASSERT(RanksBrd[pos->enPas]==RANK_6);
            }
            HASH_EP;
        }
  }


  MovePiece(from, to, pos);

  int prPce = PROMOTED(mv);
    if (prPce != EMPTY) {
    ASSERT(PieceValidEmpty(prPce) && !PiecePawn[prPce]);
    ClearPiece(to, pos);
    AddPiece(to, pos, prPce);
    }

  if (PieceKing[pos->pieces[to]]) {
    pos->KingSq[pos->side] = to;
  }

  pos->side ^= 1;
  HASH_SIDE;

  ASSERT(CheckBrd(pos));

  if (SqAttacked(pos->KingSq[side], pos->side, pos)) {
    takeMv(pos);
    return (FALSE);
  }

  return TRUE;
}


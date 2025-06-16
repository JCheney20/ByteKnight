#include "stdio.h"
#include "defs.h"
#include "debug.h"

//TODO: Research into PST -> ML (Automated Eval)
/*
 */

const int PawnIsolated = -10;
const int PawnPassed[8] = {0, 5, 10, 20, 35, 60, 100, 200};
const int RookOpenFile = 10;
const int RookSemiOpenFile = 5;
const int QueenOpenFile = 5;
const int QueenSemiOpenFile = 3;
const int BishopPair = 30;

int mg_table[13][64];
int eg_table[13][64];

/* piece/sq tables */
/* values from Rofchade: http://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311&start=19 */
// MIDDLEGAME PIECE SQUARE TABLES

// Pawn PST - Middlegame
const int pawn_mg[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10,-20,-20, 10, 10,  5,
      5, -5,-10,  0,  0,-10, -5,  5,
      0,  0,  0, 20, 20,  0,  0,  0,
      5,  5, 10, 25, 25, 10,  5,  5,
     10, 10, 20, 30, 30, 20, 10, 10,
     50, 50, 50, 50, 50, 50, 50, 50,
      0,  0,  0,  0,  0,  0,  0,  0
};

// Knight PST - Middlegame
const int knight_mg[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

// Bishop PST - Middlegame
const int bishop_mg[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

// Rook PST - Middlegame
const int rook_mg[64] = {
      0,  0,  0,  5,  5,  0,  0,  0,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      5, 10, 10, 10, 10, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
};

// Queen PST - Middlegame
const int queen_mg[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -10,  5,  5,  5,  5,  5,  0,-10,
      0,  0,  5,  5,  5,  5,  0, -5,
     -5,  0,  5,  5,  5,  5,  0, -5,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

// King PST - Middlegame
const int king_mg[64] = {
     20, 30, 10,  0,  0, 10, 30, 20,
     20, 20,  0,  0,  0,  0, 20, 20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30
};

// ENDGAME PIECE SQUARE TABLES

// Pawn PST - Endgame
const int pawn_eg[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      5,  5,  5,  5,  5,  5,  5,  5,
     10, 10, 10, 10, 10, 10, 10, 10,
     20, 20, 20, 20, 20, 20, 20, 20,
     30, 30, 30, 30, 30, 30, 30, 30,
     50, 50, 50, 50, 50, 50, 50, 50,
      0,  0,  0,  0,  0,  0,  0,  0
};

// Knight PST - Endgame
const int knight_eg[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

// Bishop PST - Endgame
const int bishop_eg[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

// Rook PST - Endgame
const int rook_eg[64] = {
      0,  0,  0,  5,  5,  0,  0,  0,
     -5,  0,  0,  5,  5,  0,  0, -5,
     -5,  0,  0,  5,  5,  0,  0, -5,
     -5,  0,  0,  5,  5,  0,  0, -5,
     -5,  0,  0,  5,  5,  0,  0, -5,
     -5,  0,  0,  5,  5,  0,  0, -5,
      5, 10, 10, 10, 10, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
};

// Queen PST - Endgame
const int queen_eg[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -10,  5,  5,  5,  5,  5,  0,-10,
      0,  0,  5,  5,  5,  5,  0, -5,
     -5,  0,  5,  5,  5,  5,  0, -5,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

// King PST - Endgame (King becomes more active)
const int king_eg[64] = {
    -50,-30,-30,-30,-30,-30,-30,-50,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -50,-40,-30,-20,-20,-30,-40,-50
};

// Array of pointers for easy access
const int* mg_pst[6] = {
    pawn_mg, knight_mg, bishop_mg, rook_mg, queen_mg, king_mg
};

const int* eg_pst[6] = {
    pawn_eg, knight_eg, bishop_eg, rook_eg, queen_eg, king_eg
};

int MaterialDraw(const S_BOARD *pos){
if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
	  if (!pos->pceNum[bB] && !pos->pceNum[wB]) {
	      if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3) {return TRUE;} 
	  } else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {
	     if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2) { return TRUE; }
	  } else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {
	    if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN]))  { return TRUE; }
	  }
	} else if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {
        if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {
            if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2)	{ return TRUE; }
        } else if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {
            if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && (((pos->pceNum[bN] + pos->pceNum[bB]) == 1) || ((pos->pceNum[bN] + pos->pceNum[bB]) == 2))) { return TRUE; }
        } else if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {
            if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && (((pos->pceNum[wN] + pos->pceNum[wB]) == 1) || ((pos->pceNum[wN] + pos->pceNum[wB]) == 2))) { return TRUE; }
        }
    }
  return FALSE;
}

/* https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function8*/

void InitPSTtable(){
    int pc, p, sq;
    for (p = PAWN, pc = wP; p <= KING; pc ++, p++) {
        for (sq = 0; sq < 64; sq++) {
            mg_table[pc]  [sq] = mPieceVal[pc] + mg_pst[p][sq];
            eg_table[pc]  [sq] = ePieceVal[pc] + eg_pst[p][sq];
            mg_table[pc+6][sq] = mPieceVal[pc+6] + mg_pst[p][MIRROR64(sq)];
            eg_table[pc+6][sq] = ePieceVal[pc+6] + eg_pst[p][MIRROR64(sq)];
        }
    }
}


int get_pst_value(int piece, int square, int is_endgame) {
    /*
     * Get piece square table value
     * piece: 0=pawn, 1=knight, 2=bishop, 3=rook, 4=queen, 5=king
     * square: 0-63 (a1=0, b1=1, ..., h8=63)
     * is_endgame: 0=middlegame, 1=endgame
     */
    if (is_endgame) {
        return eg_table[piece][MIRROR64(square)];
    } else {
        return mg_table[piece][MIRROR64(square)];
    }
}

void TestPST(){
  int sq, file, rank, piece;

printf("MIDDLE GAME BOARDS");CR;
    for (int pce = PAWN; pce <=KING ; ++pce) {
      printf("PST of: %d\n", pce);
      for (sq = 0; sq < 64; sq++) {
        if (sq % 8 == 0) CR;
        int val = get_pst_value(pce+1, sq, 0);
        printf("%d\t",val);
      }
  CR;
    }


  printf("ENDGAME BOARDS");CR;
    for (int pce = PAWN; pce <=KING ; ++pce) {
      printf("PST of: %d\n", pce);
      for (sq = 0; sq < 64; sq++) {
        if (sq % 8 == 0) CR;
        int val = get_pst_value(pce+1, sq, 1);
        printf("%d\t",val);
      }
  CR;
    }

}


int pceAdjustments(int pce, int sq, const S_BOARD *pos){
  int score;
  switch (pce) {
    case wP:
      if ((IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
        score += PawnIsolated;
      }

      if ((WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
        score += PawnPassed[RanksBrd[sq]];
      }
    break;
    case bP:
      if ((IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
        score += PawnIsolated;
      }

      if ((BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
        score += PawnPassed[RanksBrd[sq]];
      }

    break;
    case wR:
      if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
        score += RookOpenFile;
      } else if (!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
        score += RookSemiOpenFile;
      }

    break;
    case bR:
      if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
        score += RookOpenFile;
      } else if (!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
        score += RookSemiOpenFile;
      }
    break;
    case wQ:
      if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
        score += QueenOpenFile;
      } else if (!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
        score += QueenSemiOpenFile;
      }

    break;
    case bQ:
      if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
        score += QueenOpenFile;
      } else if (!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
        score += QueenSemiOpenFile;
      }
    break;
  }
  return score;
}

int evalPos(const S_BOARD *pos){
  int mg[2] = {0, 0};
  int eg[2] = {0, 0};

  int pce, pceNum, sq, rank, file;

  if (!pos->pceNum[wP] && !pos->pceNum[bP] && MaterialDraw(pos) == TRUE) {
    return 0;
  }
  
  for (rank = RANK_8;rank>=RANK_1; rank--) {
    for(file = FILE_A; file <= FILE_H; file++){
      sq = FR2SQ(file, rank);
      pce = pos->pieces[sq];
      if (pce != EMPTY) {
        int col = pce<7? WHITE:BLACK;
        mg[col] += mg_table[pce][SQ64(sq)];
        eg[col] += eg_table[pce][SQ64(sq)];
        mg[col] += pceAdjustments(pce, sq, pos);
        eg[col] += pceAdjustments(pce, sq, pos);
      }
    }
  }

  if (pos->pceNum[wB] >= 2) mg[WHITE] += BishopPair;
  if (pos->pceNum[wB] >= 2) eg[WHITE] += BishopPair;
  if (pos->pceNum[bB] >= 2) mg[BLACK] += BishopPair;
  if (pos->pceNum[bB] >= 2) eg[BLACK] += BishopPair;

  int mgScore = mg[pos->side] - mg[(pos->side)^1];
  int egScore = eg[pos->side] - eg[(pos->side)^1];
  int mgPhase = (pos->gamePhase > 24)? 24:pos->gamePhase;
  int egPhase = 24 - mgPhase;
  return ((mgScore * mgPhase) + (egScore * egPhase))/24;

}

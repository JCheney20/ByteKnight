#ifndef DEFS_H //INCLUDE GUARD
#define DEFS_H

#include "stdlib.h"

typedef unsigned long long U64;

#define  NAME "ByteKnight 1.0"
#define BRD_SQ_NUM 120

#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256
#define MAXDEPTH 64

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define INF 30000
#define MATE (INF - MAXDEPTH)

enum PIECES {EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK};
enum FILES {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE};
enum RANKS {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE};

enum COLOURS { WHITE, BLACK, BOTH};

enum SQRS {
  A1 = 21, B1, C1, D1, E1, F1, G1, H1,
  A2 = 31, B2, C2, D2, E2, F2, G2, H2,
  A3 = 41, B3, C3, D3, E3, F3, G3, H3,
  A4 = 51, B4, C4, D4, E4, F4, G4, H4,
  A5 = 61, B5, C5, D5, E5, F5, G5, H5,
  A6 = 71, B6, C6, D6, E6, F6, G6, H6,
  A7 = 81, B7, C7, D7, E7, F7, G7, H7,
  A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ
};

enum TF {FALSE, TRUE};

enum CASTLING {WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8};

typedef struct{
  int mv;
  int score;
} S_MOVE ;

typedef struct {
  S_MOVE moves[MAXPOSITIONMOVES];
  int count;
} S_MOVELIST ;

typedef struct {
  U64 posKey;
  int mv;
} S_PVENTRY;

typedef struct {
  S_PVENTRY *pTable;
  int numEntries;
} S_PVTABLE;

typedef struct{
  
  int move;
  int castlePerm;
  int enPas;
  int fiftyMv;
  U64 posKey;

} S_UNDO ;

typedef struct {

  int pieces[BRD_SQ_NUM];
  U64 pawns[3];

  int KingSq[2];

  int side;
  int enPas;
  int fiftyMv;

  int ply;
  int histPly;

  int castlePerm;

  U64 posKey; //Unique pos key for identification

  int pceNum[13];
  int bigPce[2];
  int majPce[2];
  int minPce[2];
  int material[2];

  S_UNDO history[MAXGAMEMOVES];

  // piece lst
  int pList[13][10];

  S_PVTABLE PvTable[1];
  int PvArr[MAXDEPTH];

  int searchHist[13][BRD_SQ_NUM];
  int searchKillers[2][MAXDEPTH];

} S_BOARD; 

typedef struct {

  int start_time;
  int stop_time;
  int t_set;
  int t_lim;
  int depth;
  int d_set;
  int movestogo;

  long nodes;

  int quit;
  int stopped;

} S_SEARCHINFO;


// MACROS
#define FR2SQ(f,r) ( (21 + (f)) + ((r) * 10))
#define SQ64(sq120) SQ120toSQ64[sq120]
#define SQ120(sq64) SQ64toSQ120[sq64]
#define CR printf("\n")
#define POP(b) PopBit(b)
#define CNT(b) CountBits(b)
#define CLRBIT(bb, sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb, sq) ((bb) |= SetMask[(sq)])
#define RAND_64 ((U64) rand() | \
                 (U64) rand() << 15 | \
                 (U64) rand() << 30 | \
                 (U64) rand() << 45 | \
                 ((U64) rand()  & 0xf) << 60)
#define isBQ(p) (PieceBishopQueen[p])
#define isRQ(p) (PieceRookQueen[p])
#define isKn(p) (PieceKnight[p])
#define isKi(p) (PieceKing[p])

/* GAME MOVES
* 0000 0000 0000 0000 0000 0111 1111 -> From {0x7F}
* 0000 0000 0000 0011 1111 1000 0000 -> To { >> 7 0x7F}
* 0000 0000 0011 1100 0000 0000 0000 -> Capture {>> 14 0xF}
* 0000 0000 0100 0000 0000 0000 0000 -> EP {0x40000}
* 0000 0000 1000 0000 0000 0000 0000 -> Pawn Start {0x80000}
* 0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece {>> 20 0xF}
* 0001 0000 0000 0000 0000 0000 0000 -> Castle {0x1000000}
*/

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

#define EP_FLAG 0x40000
#define PAWN_START_FLAG 0x80000
#define CASTLE_FLAG 0x1000000
#define CAP_FLAG 0x7C000
#define PROM_FLAG 0xF00000

#define NOMOVE 0


// GLOBALS
extern int SQ120toSQ64[BRD_SQ_NUM];
extern int SQ64toSQ120[64];
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKeys [13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];

extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

extern int PieceBig[13];
extern int PieceMaj[13];
extern int PieceMin[13];
extern int PieceVal[13];
extern int PieceCol[13];


extern int PiecePawn[13];
extern int PieceKnight[13];
extern int PieceKing[13];
extern int PieceBishopQueen[13];
extern int PieceRookQueen[13];
extern int PieceSlides[13];

extern int FilesBrd[BRD_SQ_NUM];
extern int RanksBrd[BRD_SQ_NUM];

// FUNCTIONS
// init.c
extern void AllInit();

//bitboards.c
extern void PrintBitBoard(U64 bb);
extern int CountBits(U64 b); 
extern int PopBit(U64 *bb);

//hashkeys.c
extern U64 GeneratePosKey(const S_BOARD *pos);

//board.c
extern void ResetBoard(S_BOARD *pos);
extern int ParseFen(char *fen, S_BOARD *pos);
extern void PrintBoard(const S_BOARD *pos);
extern void UpdateListMaterial(S_BOARD *pos);
extern int CheckBrd(const S_BOARD *pos);

//attacks.c
extern int SqAttacked(const int sq, const int side, const S_BOARD *pos);

//io.c
extern char *PrSq(const int sq);
extern char *PrMv(const int mv);
extern void PrintMvList(const S_MOVELIST *list);
extern int ParseMv(char *ptrChar, S_BOARD *pos);

//validate.c
extern int SqOnBoard(const int sq);
extern int SideValid(const int side);
extern int FileRankValid(const int fr);
extern int PieceValidEmpty(const int pce);
extern int PieceValid(const int pce);

//mvgen.c
extern void GenerateAllMvs(const S_BOARD *pos, S_MOVELIST *list);
extern int MoveExists(S_BOARD *pos, const int move);

//makemv.c
extern void takeMv(S_BOARD *pos);
extern int makeMv(S_BOARD *pos, int mv);

// perft.c
extern void PerftTest(int depth, S_BOARD *pos);

//search.c
extern void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info);
extern int isRepetition(const S_BOARD *pos);

//misc.c
extern int GetTimeMS();

//pvtable.c
extern void initPvTable(S_PVTABLE *table);
extern void StorePvMove(const S_BOARD *pos, const int move);
extern int ProbePvTable(const S_BOARD *pos);
extern int GetPvLine(const int depth, S_BOARD *pos);
extern void ClearPvTable(S_PVTABLE *table);

//eval.c
extern int evalPos(const S_BOARD *pos);

#endif // !DEFS_H

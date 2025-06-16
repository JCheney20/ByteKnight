#include "stdio.h"
#include "defs.h"
#include "debug.h"

int const phaseInc[13] = {0, 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};


void ResetBoard(S_BOARD *pos){
  int i;

  for (i = 0; i<BRD_SQ_NUM; ++i) {
    pos->pieces[i] = NO_SQ;
  }

  for (i = 0; i<64; ++i) {
    pos->pieces[SQ120(i)] = EMPTY;
  }

  for (i=0;i<2;++i) {
    pos->bigPce[i] = 0;
    pos->majPce[i] = 0;
    pos->minPce[i] = 0;
    pos->pawns[i] = 0ULL;
    pos->EGmaterial[i] = 0;
    pos->MGmaterial[i] = 0;
  }

  for (i = 0; i<13; ++i) {
    pos->pceNum[i] = 0;
  }

  pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;

  pos->side = BOTH;
  pos->enPas = NO_SQ;
  pos->fiftyMv = 0;

  pos->ply = 0;
  pos->histPly = 0;

  pos->castlePerm = 0;

  pos->posKey = 0ULL;
  pos->gamePhase = 0;


}

int ParseFen(char* fen, S_BOARD *pos){

  ASSERT(fen!=NULL);
  ASSERT(pos!=NULL);

  int rank = RANK_8;
  int file = FILE_A;
  int piece, count, i, sq64, sq120;

  ResetBoard(pos);

  while((rank >=RANK_1) && *fen){
    count = 1;
    switch (*fen) {
      case 'p': piece = bP; break; 
      case 'r': piece = bR; break;
      case 'n': piece = bN; break;
      case 'b': piece = bB; break;
      case 'k': piece = bK; break;
      case 'q': piece = bQ; break;
      case 'P': piece = wP; break;
      case 'R': piece = wR; break;
      case 'N': piece = wN; break;
      case 'B': piece = wB; break;
      case 'K': piece = wK; break;
      case 'Q': piece = wQ; break;

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
          piece = EMPTY;
          count = *fen - '0';
          break;

      case '/':
      case ' ':
          rank--;
          file = FILE_A;
          fen++;
          continue;

      default:
          printf("FEN error \n");
          return -1;
    }

    for (i=0;i<count;i++) {
      sq64 = rank*8+file;
      sq120 = SQ120(sq64);
      if(piece != EMPTY){
        pos->pieces[sq120] = piece;
      }
      file++;
    }
    fen++;
  }

  ASSERT(*fen == 'w' || *fen == 'b');
  pos->side = (*fen == 'w')? WHITE : BLACK;
  fen += 2;

  for (i=0;i<4;i++) {
    if(*fen == ' '){
      break;
    }
    switch (*fen) {
      case 'K': pos->castlePerm |= WKCA; break;
      case 'Q': pos->castlePerm |= WQCA; break;
      case 'k': pos->castlePerm |= BKCA; break;
      case 'q': pos->castlePerm |= BQCA; break;
      default: break;
    }
    fen++;
  }
  fen++;
  
  ASSERT(pos->castlePerm>=0 && pos->castlePerm<=15);

  if(*fen != '-'){
    file = fen[0] - 'a';
    rank = fen[1] - '1';

    ASSERT(file>=FILE_A && file <= FILE_H );
    ASSERT(rank>=RANK_1 && rank <=RANK_8);

    pos->enPas = FR2SQ(file, rank);
  }
  pos->posKey = GeneratePosKey(pos);
  UpdateListMaterial(pos);

  return 0;
}

void PrintBoard(const S_BOARD *pos){
  int sq, file, rank, piece;

  printf("\nGame Board:\n\n");

  for (rank = RANK_8;rank>=RANK_1; rank--) {
    printf("%d\t",rank+1);
    for(file = FILE_A; file <= FILE_H; file++){
      sq = FR2SQ(file, rank);
      piece = pos->pieces[sq];
      printf("%3c", PceChar[piece]);
    }
    CR;
  }
  CR; printf("\t");

  for(file = FILE_A; file <= FILE_H; file++){
    printf("%3c",'a'+file);
  }
  CR;CR;

  printf("side: %c\n", SideChar[pos->side]);
  printf("enPas: %s\n", (pos->enPas != NO_SQ)? PrSq(pos-> enPas): "-");
  printf("castle: %c%c%c%c\n",
         pos->castlePerm & WKCA ? 'K' : '-',
         pos->castlePerm & WQCA ? 'Q' : '-',
         pos->castlePerm & BKCA ? 'k' : '-',
         pos->castlePerm & BQCA ? 'q' : '-'
         );
  printf("PosKey:%llX\n",pos->posKey);
}

void UpdateListMaterial(S_BOARD *pos){
  int piece, sq, i, colour;

  for (i=0; i<BRD_SQ_NUM; ++i) {
    sq=i;
    piece = pos ->pieces[i];
    if(piece!=NO_SQ && piece!=EMPTY){
      colour = PieceCol[piece];
      if (PieceBig[piece] == TRUE) pos->bigPce[colour]++;
      if (PieceMaj[piece] == TRUE) pos->majPce[colour]++;
      if (PieceMin[piece] == TRUE) pos->minPce[colour]++;


      pos->MGmaterial[colour] += mPieceVal[piece];
      pos->EGmaterial[colour] += ePieceVal[piece];
      pos->gamePhase += phaseInc[piece];

      //Piece List
      pos->pList[piece][pos->pceNum[piece]] = sq;
      pos->pceNum[piece]++;

      if(piece==wK) pos->KingSq[WHITE] = sq;
      if(piece==bK) pos->KingSq[BLACK] = sq;


      if (piece==wP) {
        SETBIT(pos->pawns[WHITE], SQ64(sq));
        SETBIT(pos->pawns[BOTH], SQ64(sq));
      } else if (piece==bP) {
        SETBIT(pos->pawns[BLACK], SQ64(sq));
        SETBIT(pos->pawns[BOTH], SQ64(sq));
      }
      
      
    }
  }
}

int CheckBrd(const S_BOARD *pos){
  int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int t_bigPce[2] = { 0, 0};
  int t_majPce[2] = { 0, 0};
  int t_minPce[2] = { 0, 0};
  int t_MGmaterial[2] = { 0, 0};
  int t_EGmaterial[2] = { 0, 0};

  int sq64, t_piece, t_pce_num, sq120, colour, pcount;
  U64 t_pawns[3] = {0ULL, 0ULL, 0ULL};

  t_pawns[WHITE] = pos->pawns[WHITE];
  t_pawns[BLACK] = pos->pawns[BLACK];
  t_pawns[BOTH] = pos->pawns[BOTH];

  //check piece lists
  for (t_piece = wP; t_piece <=bK; ++t_piece) {
    for (t_pce_num = 0; t_pce_num<pos->pceNum[t_piece]; ++t_pce_num) {
      sq120 = pos->pList[t_piece][t_pce_num];
      ASSERT(pos->pieces[sq120] == t_piece);
    }
  }

  //check piece count and other counters
  for (sq64 = 0; sq64<64; ++sq64) {
    sq120 = SQ120(sq64);
    t_piece = pos->pieces[sq120];
    t_pceNum[t_piece]++;
    colour=PieceCol[t_piece];

    if (PieceBig[t_piece] == TRUE) t_bigPce[colour]++;
    if (PieceMin[t_piece] == TRUE) t_minPce[colour]++;
    if (PieceMaj[t_piece] == TRUE) t_majPce[colour]++;

    t_MGmaterial[colour] += mPieceVal[t_piece];
    t_EGmaterial[colour] += ePieceVal[t_piece];
  }

  for (t_piece = wP; t_piece <= bK; ++t_piece) {
    ASSERT(t_pceNum[t_piece] == pos->pceNum[t_piece]);
  }

  // check bitboards count
  pcount = CNT(t_pawns[WHITE]);
  ASSERT(pcount == pos->pceNum[wP]);
  pcount = CNT(t_pawns[BLACK]);
  ASSERT(pcount == pos->pceNum[bP]);
  pcount = CNT(t_pawns[BOTH]);
  ASSERT(pcount == (pos->pceNum[wP] + pos->pceNum[bP]));

  //check bitboards squares
  while (t_pawns[WHITE]) {
    sq64 = POP(&t_pawns[WHITE]);
    ASSERT(pos->pieces[SQ120(sq64)] == wP);
  }
  
  while (t_pawns[BLACK]) {
    sq64 = POP(&t_pawns[BLACK]);
    ASSERT(pos->pieces[SQ120(sq64)] == bP);
  }

  while (t_pawns[BOTH]) {
    sq64 = POP(&t_pawns[BOTH]);
    ASSERT(pos->pieces[SQ120(sq64)] == wP || pos->pieces[SQ120(sq64)] == bP );
  }

  ASSERT(t_MGmaterial[WHITE] == pos->MGmaterial[WHITE] && t_MGmaterial[BLACK] == pos->MGmaterial[BLACK]);
  ASSERT(t_EGmaterial[WHITE] == pos->EGmaterial[WHITE] && t_EGmaterial[BLACK] == pos->EGmaterial[BLACK]);
  ASSERT(t_minPce[WHITE] == pos->minPce[WHITE] && t_minPce[BLACK] == pos->minPce[BLACK]);
  ASSERT(t_majPce[WHITE] == pos->majPce[WHITE] && t_majPce[BLACK] == pos->majPce[BLACK]);
  ASSERT(t_bigPce[WHITE] == pos->bigPce[WHITE] && t_bigPce[BLACK] == pos->bigPce[BLACK]);

  ASSERT(pos->side == WHITE || pos->side==BLACK); 
  ASSERT(GeneratePosKey(pos)==pos->posKey);

  ASSERT(pos->enPas==NO_SQ || (RanksBrd[pos->enPas]==RANK_6 && pos->side == WHITE) || (RanksBrd[pos->enPas]==RANK_3 && pos->side == BLACK));

  ASSERT(pos->pieces[pos->KingSq[WHITE]] == wK);
  ASSERT(pos->pieces[pos->KingSq[BLACK]] == bK);

  return TRUE;
}

void MirrorBrd(S_BOARD *pos){
  int tempPieceArr[64];
  int tempSide = pos->side^1;
  int SwapPiece[13] = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK};
  int tempCastlePerm = 0;
  int tempEP = NO_SQ;
  int sq, tp;

  if (pos->castlePerm & WKCA) tempCastlePerm |= BKCA;
  if (pos->castlePerm & WQCA) tempCastlePerm |= BQCA;
  
  if (pos->castlePerm & BKCA) tempCastlePerm |= WKCA;
  if (pos->castlePerm & BQCA) tempCastlePerm |= WQCA;

  if (pos->enPas != NO_SQ) tempEP = SQ120(MIRROR64(SQ64(pos->enPas)));

  for (sq = 0; sq < 64; sq++) tempPieceArr[sq] = pos->pieces[SQ120(MIRROR64(sq))];

  ResetBoard(pos);

  for (sq = 0; sq < 64; sq++) {
    tp = SwapPiece[tempPieceArr[sq]];
    pos->pieces[SQ120(sq)] = tp;
  }

  pos->side = tempSide;
  pos->castlePerm = tempCastlePerm;
  pos->enPas = tempEP;

  pos->posKey = GeneratePosKey(pos);

  UpdateListMaterial(pos);

  ASSERT(CheckBrd(pos));
  
}

